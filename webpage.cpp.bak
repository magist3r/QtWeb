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
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QProgressDialog>
#include <QFileDialog>
#include <QtNetwork>
#include <QtWebKit/QWebHitTestResult>
#include <qdesktopservices.h>
#include <QtUiTools/QUiLoader>

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

void WebPage::setUserAgent(QString agent) 
{
	m_userAgent = agent;
}

void WebPage::setDefaultAgent(  )
{
    QString ver;
#ifdef Q_WS_WIN
    switch(QSysInfo::WindowsVersion) 
	{
        case QSysInfo::WV_32s:
            ver = "Windows 3.1";
            break;
        case QSysInfo::WV_95:
            ver = "Windows 95";
            break;
        case QSysInfo::WV_98:
            ver = "Windows 98";
            break;
        case QSysInfo::WV_Me:
            ver = "Windows 98; Win 9x 4.90";
            break;
        case QSysInfo::WV_NT:
            ver = "WinNT4.0";
            break;
        case QSysInfo::WV_2000:
            ver = "Windows NT 5.0";
            break;
        case QSysInfo::WV_XP:
            ver = "Windows NT 5.1";
            break;
        case QSysInfo::WV_2003:
            ver = "Windows NT 5.2";
            break;
        case QSysInfo::WV_VISTA:
            ver = "Windows NT 6.0";
            break;
        case QSysInfo::WV_CE:
            ver = "Windows CE";
            break;
        case QSysInfo::WV_CENET:
            ver = "Windows CE .NET";
            break;
        case QSysInfo::WV_CE_5:
            ver = "Windows CE 5.x";
            break;
        case QSysInfo::WV_CE_6:
            ver = "Windows CE 6.x";
            break;
		default:
			ver = "Windows NT based";
    }
#else

	#ifdef Q_WS_MAC
		switch(QSysInfo::MacintoshVersion) 
		{
			case QSysInfo::MV_10_3:
				ver = "Intel MacOS X 10.3";
				break;
			case QSysInfo::MV_10_4:
				ver = "Intel MacOS X 10.4";
				break;
			case QSysInfo::MV_10_5:
				ver = "Intel MacOS X 10.5";
				break;
			case QSysInfo::MV_10_6:
				ver = "Intel MacOS X 10.6";
				break;
			default:
				ver = "MacOS X";
		}
	#else
		ver = "Linux/Unix";
	#endif
#endif
	// language
    QString name = QLocale::system().name();
    name[2] = QLatin1Char('-');

	QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bUseCustomAgent = settings.value(QLatin1String("customUserAgent"), false).toBool();
	if (bUseCustomAgent)
	{
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

		m_userAgent = m_userAgent.replace("%W", ver);
		m_userAgent = m_userAgent.replace("%L", name);
		return;
	}

#ifdef Q_WS_MAC
    QString ua = QLatin1String("Mozilla/5.0 (Macintosh; %1; %2; ");
#else
    QString ua = QLatin1String("Mozilla/5.0 (Windows; %1; %2; ");
#endif

    QChar securityStrength(QLatin1Char('N'));
    if (QSslSocket::supportsSsl())
        securityStrength = QLatin1Char('U');
    ua = ua.arg(securityStrength);

    ua = QString(ua).arg(ver);
	
	// Language
    ua.append(name);
    ua.append(QLatin1String(") "));

    // webkit/qt version
    ua.append(QLatin1String("AppleWebKit/533.3 (KHTML, like Gecko) "));

    // Application name/version
    QString appName = QCoreApplication::applicationName();
    if (!appName.isEmpty()) {
        ua.append(QLatin1Char(' ') + appName);
        QString appVer = QCoreApplication::applicationVersion();
        if (!appVer.isEmpty())
            ua.append(QLatin1Char('/') + appVer);
		ua.append(QLatin1String(" http://www.QtWeb.net"));
    } 

    m_userAgent = ua;
}

const QString& WebPage::getUserAgent()
{
	return m_userAgent;
}

QString WebPage::userAgentForUrl(const QUrl& url) const
{
	return getUserAgent();
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
		if(	url.indexOf(".pdf") == url.length() - 4 )
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
			DefineHostIcon(request.url());
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
			//DefineHostIcon(request.url());
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
				DefineHostIcon(request.url());
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

void WebPage::DefineHostIcon(const QUrl& url)
{
	BrowserApplication::instance()->CheckIcon(url);
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

    QFile file(QLatin1String(":/notfound.html"));
    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);
    QString title = tr("Loading error: %1").arg(reply->url().toString());
    QString html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg(reply->errorString())
                        .arg(reply->url().toString());

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
    QPixmap pixmap = icon.pixmap(QSize(32,32));
    if (pixmap.save(&imageBuffer, "PNG")) {
        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                     QString(QLatin1String(imageBuffer.buffer().toBase64())));
    }

    QList<QWebFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url()) {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebFrame *> children = frame->childFrames();
        foreach(QWebFrame *frame, children)
            frames.append(frame);
    }
    if (m_loadingUrl == reply->url()) {
        mainFrame()->setHtml(html, reply->url());
    }
}

