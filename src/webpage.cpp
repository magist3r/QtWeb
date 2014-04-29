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

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webpage.h"
#include "webview.h"
#include "autocomplete.h"

#include <QtGui/QClipboard>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QProgressDialog>
#include <QFileDialog>
#include <QtNetwork>
#include <QtWebKitWidgets/QWebHitTestResult>
#include <qdesktopservices.h>
#include <QtUiTools/QUiLoader>
#include <QRegExp>

#include <QtCore/QDebug>
#include <QtCore/QBuffer>

WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
    , m_openAction(OpenDefault)
{
    setNetworkAccessManager(BrowserApplication::networkAccessManager());
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)),
            this, SLOT(handleUnsupportedContent(QNetworkReply *)));
}

BrowserMainWindow *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) {
        if (BrowserMainWindow *mw = qobject_cast<BrowserMainWindow*>(w))
            return mw;
        w = w->parent();
    }
    return BrowserApplication::instance()->mainWindow();
}

QString WebPage::m_userAgent;
QString WebPage::m_defaultAgent;
bool WebPage::m_useCustomAgent;

void WebPage::setUserAgent(QString agent) 
{
    if (agent == "default") {
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        bool bUseCustomAgent = settings.value(QLatin1String("customUserAgent"), false).toBool();
        if (bUseCustomAgent) {
            m_useCustomAgent = true;

            m_userAgent = settings.value(QLatin1String("UserAgent"), "").toString();
            if (m_userAgent == "Internet Explorer")
                m_userAgent = "Mozilla/4.0 (compatible; MSIE 8.0; %W;  Trident/4.0)";
            else if (m_userAgent == "Firefox")
                m_userAgent = "Mozilla/5.0 (Windows; U; %W; %L; rv:1.9.0.5) Gecko/2008120121 Firefox/3.0.5";
            else if (m_userAgent == "Opera")
                m_userAgent = "Opera/9.63 (%W; U; en) Presto/2.1.1";
            else if (m_userAgent == "Safari")
                m_userAgent = "Mozilla/5.0 (Windows; U; %W; %L) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Safari/528.17";
            else if (m_userAgent == "Chrome")
                m_userAgent = "Mozilla/5.0 (Windows; U; %W; %L) AppleWebKit/525.13 (KHTML, like Gecko) Chrome/0.A.B.C Safari/525.13";

            m_userAgent = m_userAgent.replace("%L", QLocale::system().name());
            QRegExp rx("([^;]*)\\)");
            if (rx.indexIn(m_defaultAgent) >= 0 && m_userAgent.contains("%W"))
                m_userAgent = m_userAgent.replace("%W", rx.cap(1));

        } else {
            m_userAgent = m_defaultAgent;
            m_useCustomAgent = false;
        }
    } else {
        m_useCustomAgent = true;
        m_userAgent = agent;
    }
}

const QString& WebPage::getUserAgent()
{
    return m_userAgent;
}

QString WebPage::userAgentForUrl(const QUrl& url) const
{
    QString userAgent = QWebPage::userAgentForUrl(url);//getUserAgent();
    if (m_defaultAgent != userAgent)
        m_defaultAgent = userAgent;

    if (m_userAgent.isNull())
        m_userAgent = userAgent;

    if (m_useCustomAgent) {
        QRegExp rx("([^;]*)\\)");
        if (rx.indexIn(m_defaultAgent) >= 0 && m_userAgent.contains("%W"))
            m_userAgent = m_userAgent.replace("%W", rx.cap(1));
    }

    return m_userAgent;
}


extern bool ShellOpenApp(QString app, QString cmd);

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if (!request.url().isEmpty())
    {
        QString scheme = request.url().scheme();
        QString url = request.url().toString().toLower();
        if ( scheme == "mms" || scheme == QLatin1String("mailto") ||
                url.indexOf(".wmv") == url.length() - 4 || url.indexOf(".wma") == url.length() - 4)
        {
            if (QDesktopServices::openUrl(request.url()))
                return false;
        }
        
        /* AC: v .3.3.45 - PDF PlugIn is handled properly now in Qt 4.6.1++
        if( url.indexOf(".pdf") == url.length() - 4 )
        {
            if (BrowserApplication::downloadManager())
                BrowserApplication::downloadManager()->download(request.url(), false);
            return false;
        }*/

    }

    if (frame && (type == NavigationTypeFormSubmitted || type == NavigationTypeFormResubmitted))
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        if ( settings.value(QLatin1String("savePasswords"), false).toBool())
        {
            QUrl u = request.url();
            if (!request.rawHeader("Referer").isEmpty())
                u = QUrl(request.rawHeader("Referer"));

            BrowserApplication::autoCompleter()->setFormHtml( u, frame->toHtml() );
        }
    }

    // ctrl open in new tab
    // ctrl-shift open in new tab and select
    // ctrl-alt open in new window
    if (type == QWebPage::NavigationTypeLinkClicked && (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)) 
    {
            QSettings settings;
            settings.beginGroup(QLatin1String("general"));
            int openLinksIn = settings.value(QLatin1String("openLinksIn"), 0).toInt();

            bool newWindow = (m_keyboardModifiers & Qt::AltModifier);
            WebView* webView;
            if (newWindow || (openLinksIn == 1)) 
            {
                BrowserApplication::instance()->newMainWindow();
                BrowserMainWindow *newMainWindow = BrowserApplication::instance()->mainWindow();
                webView = newMainWindow->currentTab();
                newMainWindow->raise();
                newMainWindow->activateWindow();
                webView->setFocus();
            } 
            else 
            {
                bool selectNewTab = (m_keyboardModifiers & Qt::ShiftModifier);
                webView = mainWindow()->tabWidget()->newTab(selectNewTab);
            }
            webView->load(request);
            m_keyboardModifiers = Qt::NoModifier;
            m_pressedButtons = Qt::NoButton;
            return false;
    }

    if (frame == NULL && type == QWebPage::NavigationTypeLinkClicked) // Check for open links in tabs
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("general"));
        int openLinksIn = settings.value(QLatin1String("openLinksIn"), 0).toInt();
        if (openLinksIn == 0 && !(m_keyboardModifiers & Qt::AltModifier))
        {
            WebView* webView = mainWindow()->tabWidget()->newTab(true);
            webView->load(request);
            return false;
        }
    }

    if (frame == mainFrame()) 
    {
        if ( !request.url().isEmpty() )
        {
            if (request.url().scheme() != "ftp") 
            {
                m_loadingUrl = request.url();
                //emit loadingUrl(m_loadingUrl); // ??? to avoid unnecessary LineURL change
                return true;
            }
            else
            {
                m_loadingUrl = request.url();
                WebView* view = (WebView* )parent();
                if (!view->isLoading())
                {   
                    view->loadUrl( request.url() );
                    return false;
                }
                return true;
            }
        }
    }

    return QWebPage::acceptNavigationRequest(frame, request, type);
}

bool WebPage::extension(QWebPage::Extension extension, const QWebPage::ExtensionOption *option, QWebPage::ExtensionReturn *output)
{
    if (extension == QWebPage::ErrorPageExtension) {
        const QWebPage::ErrorPageExtensionOption* info = static_cast<const QWebPage::ErrorPageExtensionOption*>(option);
        QWebPage::ErrorPageExtensionReturn* errorPage = static_cast<QWebPage::ErrorPageExtensionReturn*>(output);

        QFile file(QLatin1String(":/notfound.html"));
        file.open(QIODevice::ReadOnly);
        QString title = tr("Loading error: %1").arg(info->url.toString());
        QString html = QString(QLatin1String(file.readAll()))
                            .arg(title)
                            .arg(info->errorString)
                            .arg(info->url.toString());

        QBuffer imageBuffer;
        imageBuffer.open(QBuffer::ReadWrite);
        QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
        QPixmap pixmap = icon.pixmap(QSize(32,32));
        if (pixmap.save(&imageBuffer, "PNG")) {
            html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                         QString(QLatin1String(imageBuffer.buffer().toBase64())));
        }
        file.close();
        errorPage->content = html.toUtf8();
        errorLoadingUrl();
        return true;
    }
    return QWebPage::extension(extension, option, output);
}

bool WebPage::supportsExtension(QWebPage::Extension extension) const
{
    if (extension == QWebPage::ErrorPageExtension)
        return true;

    return QWebPage::supportsExtension(extension);
}

QWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
    Q_UNUSED(type);

    if (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton)
        m_openAction = WebPage::OpenNewTab;

    if (m_openAction == WebPage::OpenNewTab )
    {
        m_openAction = WebPage::OpenDefault;
        return mainWindow()->tabWidget()->newTab()->page();
    }
    BrowserApplication::instance()->newMainWindow();
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
    return mainWindow->currentTab()->page();
}

#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)

void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) 
    {
        QVariant content = reply->header(QNetworkRequest::ContentTypeHeader);
        QString c = content.toString();
        
        if ( content.isValid() && !BrowserApplication::handleMIME( c, reply->url() ) )
        {
            BrowserApplication::downloadManager()->handleUnsupportedContent(reply);

        }
        return;
    }
}

