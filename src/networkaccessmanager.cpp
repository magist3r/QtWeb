/*
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
 * Copyright (C) 2007-2008 Benjamin C. Meyer <ben@meyerhome.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "networkaccessmanager.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "webview.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"
#include "autocomplete.h"
#include "cookiejar.h"
#include <QtCore/QSettings>

#include <QtGui/QDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStyle>
#include <QtGui/QTextDocument>

#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslSocket>
#include <QBuffer>
#include <QStringList>
#include <QRegExp>

#include <qnetworkdiskcache.h>

#ifdef Q_WS_WIN
    #include <windows.h>
#endif

QString GetDiskCacheLocation()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    QString location = settings.value(QLatin1String("DiskCacheLocation"), "").toString();

    if (location.isEmpty())
        location = BrowserApplication::dataLocation() + QLatin1String("/cache");

    return location;
}

NetworkAccessManager::NetworkAccessManager(QObject *parent, bool is_proxi)
    : QNetworkAccessManager(parent)
    , m_proxyManager(0)
    , m_isProxy(is_proxi)
    , m_useProxy(false)
    , m_proxyExceptions(0)
    , m_adBlockEx(0)
    , m_adBlock(0)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif

    loadSettings();

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    if (settings.value(QLatin1String("enableDiskCache"), false).toBool())
    {
        QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
        if (diskCache)
        {
            
            QString location = GetDiskCacheLocation();

            diskCache->setCacheDirectory(location);
            setCache(diskCache);
            if(m_proxyManager)
                m_proxyManager->setCache(diskCache);
        }
    }
}
NetworkAccessManager::~NetworkAccessManager()
{
    if (m_proxyManager)
        delete m_proxyManager;

    if (m_proxyExceptions)
        delete m_proxyExceptions;

    if (m_adBlock)
        delete m_adBlock;

    if (m_adBlockEx)
        delete m_adBlockEx;
}

void NetworkAccessManager::loadSettings()
{
    if (isProxy())
        return;

    if (m_proxyManager)
    {
        delete m_proxyManager;
        m_proxyManager = 0;
    }
    if (m_proxyExceptions)
    {
        delete m_proxyExceptions;
        m_proxyExceptions = 0;
    }

    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    QNetworkProxy pxy;
    setProxy(pxy);
    if (settings.value(QLatin1String("enabled"), false).toBool()) 
    {
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
            pxy = QNetworkProxy::Socks5Proxy;
        else
            pxy = QNetworkProxy::HttpProxy;
        pxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        pxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        pxy.setUser(settings.value(QLatin1String("userName")).toString());
        pxy.setPassword(settings.value(QLatin1String("password")).toString());

        if (settings.value(QLatin1String("useExceptions"), false).toBool())
        {
            m_proxyExceptions = new QStringList;
            QString exc = settings.value(QLatin1String("Exceptions")).toString();
            QStringList list = exc.split(QChar(';'));
            foreach(QString s, list)
                *m_proxyExceptions << s.trimmed();
        }
        m_useProxy = true;
        m_proxyManager =  new NetworkAccessManager(0, true);
        m_proxyManager->setCookieJar(new CookieJar);
        m_proxyManager->setProxy(pxy);
    }
    else
    if (settings.value(QLatin1String("autoProxy"), false).toBool()) 
    {
        m_useProxy = false;

#ifdef Q_WS_WIN
        HKEY hKey;
        wchar_t key[256];
        wcscpy(key, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\");
        if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey))
        {
            DWORD dwEnabled = FALSE; DWORD dwBufSize = sizeof(dwEnabled);
            if (RegQueryValueExW(hKey, L"ProxyEnable", NULL, NULL, (LPBYTE)&dwEnabled, &dwBufSize ) == ERROR_SUCCESS
                && dwEnabled)
            {
                memset(key,0,sizeof(key)); dwBufSize = sizeof(key) / sizeof(TCHAR);
                if (RegQueryValueExW(hKey, L"ProxyServer", NULL, NULL, (LPBYTE)&key, &dwBufSize ) == ERROR_SUCCESS)
                {
                    pxy = QNetworkProxy::HttpProxy;
                    QStringList lst = QString::fromWCharArray(key).split(':');
                    if (lst.size() > 0)
                    {
                        pxy.setHostName(lst.at(0));
                        pxy.setPort( lst.size() > 1 ? lst.at(1).toInt() : 1080 );

                        m_useProxy = true;
                        m_proxyManager =  new NetworkAccessManager(0, true);
                        m_proxyManager->setCookieJar(new CookieJar);
                        m_proxyManager->setProxy(pxy);
                    }
                }
                memset(key,0,sizeof(key));
                dwBufSize = sizeof(key) / sizeof(TCHAR);
                if (RegQueryValueExW(hKey, L"ProxyOverride", NULL, NULL, (LPBYTE)&key, &dwBufSize ) == ERROR_SUCCESS)
                {
                    m_proxyExceptions = new QStringList;
                    QStringList list = QString::fromWCharArray(key).split(';');
                    foreach(QString s, list)
                        *m_proxyExceptions << s.trimmed();
                }
            }
            RegCloseKey(hKey);
        }
#endif
    }
    else
    {
        m_useProxy = false;
    }
    settings.endGroup();
    
    if (m_adBlock)
    {
        delete m_adBlock;
        m_adBlock = 0;
    }

    if (m_adBlockEx)
    {
        delete m_adBlockEx;
        m_adBlockEx = 0;
    }

    settings.beginGroup(QLatin1String("AdBlock"));
    if (settings.value(QLatin1String("useAdBlock"), false).toBool())
    {
        m_adBlock = new QStringList(settings.allKeys());
    }
    settings.endGroup();

    settings.beginGroup(QLatin1String("AdBlockEx"));
    if (settings.value(QLatin1String("useAdBlockEx"), false).toBool())
    {
        m_adBlockEx = new QStringList(settings.allKeys());
    }
    settings.endGroup();

}

void NetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(Qt::escape(reply->url().toString())).arg(Qt::escape(reply->url().toString()));
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
}

void NetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(Qt::escape(proxy.hostName()));
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
}

#ifndef QT_NO_OPENSSL
void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    static QString ignore_host;

    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
    if (mainWindow && mainWindow->currentTab())
        mainWindow->currentTab()->setSslErrors();

    QSettings settings;
    settings.beginGroup(QLatin1String("ApprovedSSL"));

    if (reply->url().host() == ignore_host || 
        settings.value(reply->url().host(), false) == true ||
        settings.value(QLatin1String("ApproveAll"), false) == true )
    {
        reply->ignoreSslErrors();
        return;
    }

    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));
    if (errors.toLower().indexOf("no error")!= -1)
    {
        reply->ignoreSslErrors();
        return;
    }
    int ret = QMessageBox::warning(mainWindow, QCoreApplication::applicationName(),
        tr("Secure connection errors (SSL):\n\nProblem host:  %1\n\n%2\n\n"
                            "Do you want to approve this SSL certificate and load the web page?\n"
                            "(approving ignores these errors for this particular site in a future)").arg(reply->url().host()).arg(errors),
                           QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,
                           QMessageBox::No);
    if (ret == QMessageBox::Yes || QMessageBox::YesToAll)
    {
        reply->ignoreSslErrors();
        ignore_host = reply->url().host();
        settings.setValue(ignore_host, true);
        if (ret == QMessageBox::YesToAll)
        {
            settings.setValue(QLatin1String("ApproveAll"), true);
        }
    }
}
#endif

QNetworkReply * NetworkAccessManager::createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData )
{
    if (m_useProxy && m_proxyManager && !isUrlProxyException(req.url()))
        return m_proxyManager->createRequest(op, req, outgoingData);

    if ( m_adBlock)
    {
        bool isInWhiteList = false;
        QString url = req.url().toString().replace("http://", "");
        QRegExp rx;
        rx.setPatternSyntax(QRegExp::Wildcard);

        if (m_adBlockEx)
        {
            foreach(QString ad, *m_adBlockEx)
            {   
                rx.setPattern(ad); 
                if (rx.exactMatch(url))
                {
                    isInWhiteList = true;
                    break;
                }
            }
        }
        if (!isInWhiteList )
        {
            foreach(QString ad, *m_adBlock)
            {
                rx.setPattern(ad); 
                if (rx.exactMatch(url))
                {   
                    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, req, outgoingData);
                    reply->abort();
                    return reply;
                }
            }
        }
    }

    if (outgoingData && (op == PostOperation))
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        if ( settings.value(QLatin1String("savePasswords"), false).toBool())
        {
            m_data = outgoingData->readAll();
            QUrl u = req.url();
            if (!req.rawHeader("Referer").isEmpty())
                u = QUrl(req.rawHeader("Referer"));
            BrowserApplication::autoCompleter()->setFormData( u, m_data );
            BrowserApplication::autoCompleter()->evaluate( u );
            QBuffer* buf = new QBuffer(&m_data, this);
            buf->open(QIODevice::ReadOnly);
            return QNetworkAccessManager::createRequest(op, req, buf);
        }
    }
    
    return QNetworkAccessManager::createRequest(op, req, outgoingData);
}

void NetworkAccessManager::blockAd(QString ad)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("AdBlock"));
    if (!settings.value(QLatin1String("useAdBlock"), false).toBool())
        settings.setValue(QLatin1String("useAdBlock"), true);
    settings.setValue(ad, "");

    if (!m_adBlock)
        m_adBlock = new QStringList(settings.allKeys());

    settings.endGroup();

}

bool NetworkAccessManager::isUrlProxyException(const QUrl& url)
{
    if (!m_proxyExceptions)
        return false;
    
    QString host = url.host();
    QRegExp rx;
    rx.setCaseSensitivity ( Qt::CaseInsensitive );
    rx.setPatternSyntax(QRegExp::Wildcard);

    foreach(QString ex, *m_proxyExceptions)
    {
        if (ex.length() > 0)
        {
            rx.setPattern(ex); 
            if (host.startsWith(ex, Qt::CaseInsensitive) || rx.exactMatch(host) )
                return true;
        }
    }
    return false;
}

//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\