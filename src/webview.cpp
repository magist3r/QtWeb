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
#include "webview.h"
#include "webpage.h"
#include "autocomplete.h"
#include "commands.h"

#include <QtGui/QClipboard>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QProgressDialog>
#include <QFileDialog>
#include <QtNetwork>
#include <QtUiTools/QUiLoader>

#include <QtCore/QDebug>
#include <QtCore/QBuffer>


WebView::WebView(QWidget* parent)   
    : QWebView(parent)
    , m_progress(0)
    , m_page(new WebPage(this))
    , m_font_resizing(false)
    , m_is_loading(false)
    , m_gestureStarted(false)
    , m_encoding_in_progress(false)
    , m_ssl_errors_detected(false)
{
    setPage(m_page);
    connect(page(), SIGNAL(statusBarMessage(const QString&)),
            SLOT(setStatusBarText(const QString&)));

    connect(this, SIGNAL(loadStarted()),
            this, SLOT(loadStartedCustom()));

    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinishedCustom(bool)));
    
    connect(page(), SIGNAL(loadingUrl(const QUrl&)),
            this, SIGNAL(urlChanged(const QUrl &)));

    connect(this, SIGNAL(linkClicked(const QUrl&)),
            this, SLOT(clickedUrl(const QUrl &)));

    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)),
            this, SLOT(downloadRequested(const QNetworkRequest &)));
    page()->setForwardUnsupportedContent(true);

    ////////////////////////////////////////////////////////
    // AC: FTP impl
    m_ftp = NULL;
    m_ftpFile = NULL;
    m_ftpProgressDialog = new QProgressDialog(this);
    connect(m_ftpProgressDialog, SIGNAL(canceled()), this, SLOT(ftpCancelDownload()));

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
    qreal ratio = (qreal)settings.value(QLatin1String("ZoomRatio"), 1.0).toDouble();
    bool zoom_text_only = settings.value(QLatin1String("zoom_text_only"), false).toBool();
    if (ratio != 1.0)
    {
        setTextSizeMultiplier(ratio);
        QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, zoom_text_only);
    }
}

void WebView::slotInspectElement()
{
    if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
        page()->triggerAction(QWebPage::InspectElement, false);
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    // do not display the context menu if gesture was previously invoked
    if (m_gestureTime.isValid() && !m_gestureTime.isNull())
    { 
        int secs = m_gestureTime.secsTo(QDateTime::currentDateTime());
        if (secs >= 0 && secs <=1)
            return;
    }

    QMenu menu(this);

    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    m_hitResult = r;
    if (!r.linkUrl().isEmpty()) 
    {
        MenuCommands cmds;

        QAction* newwin = new QAction(pageAction(QWebPage::OpenLinkInNewWindow)->text(), this);
        connect(newwin, SIGNAL(triggered()), this, SLOT(openLinkInNewWin()));
        menu.addAction(newwin);

        QAction* newtab = new QAction(cmds.OpenNewTabTitle(), this);
        newtab->setShortcuts(cmds.OpenNewTabShortcuts());
        connect(newtab, SIGNAL(triggered()), this, SLOT(openLinkInNewTab()));
        menu.addAction(newtab);
    
        menu.addSeparator();

        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
        
        menu.addSeparator();
        
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));

        {
            QAction* copyadr = new QAction(cmds.CopyAddrTitle(), this); 
            connect(copyadr, SIGNAL(triggered()), this, SLOT(copyMailtoAddress()));
            menu.addAction( copyadr );
        }


        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
        {
            QAction* inspect = new QAction(cmds.InspectTitle(), this);
            //inspect->setShortcuts(cmds.InspectShortcuts());
            connect(inspect, SIGNAL(triggered()), this, SLOT(slotInspectElement()));
            menu.addAction(inspect);
        }
    }

    if (!r.imageUrl().isEmpty()) 
    {
        if (!menu.isEmpty())
            menu.addSeparator();

        menu.addAction( pageAction(QWebPage::DownloadImageToDisk));
        menu.addAction( pageAction(QWebPage::CopyImageToClipboard));

        QAction* newwin = new QAction(pageAction(QWebPage::OpenImageInNewWindow)->text(), this);
        connect(newwin, SIGNAL(triggered()), this, SLOT(openImageInNewWin()));
        menu.addAction(newwin);

        MenuCommands cmds;
        QAction* newtab = new QAction(cmds.OpenNewTabTitle(), this);
        newtab->setShortcuts(cmds.OpenNewTabShortcuts());
        connect(newtab, SIGNAL(triggered()), this, SLOT(openImageInNewTab()));
        menu.addAction( newtab );

        menu.addSeparator();
        QAction* adblock = new QAction(cmds.OpenAdBlockTitle(), this);
        adblock->setShortcuts(cmds.OpenAdBlockShortcuts());
        connect(adblock, SIGNAL(triggered()), this, SLOT(adBlock()));
        adblock->setToolTip(r.imageUrl().toString());
        menu.addAction( adblock );
        menu.addSeparator();
    }

    if (! (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled)))
    {
        MenuCommands cmds;

        menu.addAction(pageAction(QWebPage::Copy));
        menu.addAction(pageAction(QWebPage::Paste));
        menu.addAction(pageAction(QWebPage::Cut));
        menu.addSeparator();

        menu.addAction(pageAction(QWebPage::Back));
        menu.addAction(pageAction(QWebPage::Forward));
        menu.addAction(pageAction(QWebPage::Reload));
        menu.addSeparator();

        QAction* viewsource = new QAction(cmds.SourceTitle(), this);
        connect(viewsource, SIGNAL(triggered()), BrowserApplication::instance()->mainWindow(), SLOT(slotViewPageSource()));
        menu.addAction(viewsource);

        
    }

    if (!menu.isEmpty()) 
    {
        menu.exec(mapToGlobal(event->pos()));
        return;
    }

    QWebView::contextMenuEvent(event);
}

#include <QInputDialog>

void WebView::adBlock()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString url = action->toolTip().replace("http://", "");
    bool ok;
    QString text = QInputDialog::getText(this, tr("Ad Block"),
        tr("Check URL or pattern to block:"), QLineEdit::Normal, url, &ok);
    if (ok && !text.isEmpty())
    {
        NetworkAccessManager* n = BrowserApplication::networkAccessManager();
        n->blockAd( text );
    }
}


void WebView::copyMailtoAddress()
{
    if (!m_hitResult.isNull() && !m_hitResult.linkUrl().isEmpty())
    {
        if (m_hitResult.linkUrl().scheme() == "mailto")
            QApplication::clipboard()->setText( m_hitResult.linkUrl().encodedPath() );
        else
        {
            if (!m_hitResult.linkUrl().scheme().isEmpty())
            {
                QString s = m_hitResult.linkUrl().toString(QUrl::RemoveScheme | QUrl::StripTrailingSlash | 
                    QUrl::RemoveUserInfo | QUrl::RemoveQuery);
                if (s.indexOf("//") == 0)
                    s = s.remove(0,2);
                QApplication::clipboard()->setText( s );
            }
            else
                QApplication::clipboard()->setText( m_hitResult.linkUrl().toString() );
        }
    }
}

void WebView::openImageInNewWin()
{
        m_page->m_openAction = WebPage::OpenNewWin;
    if (!m_hitResult.isNull() && !m_hitResult.imageUrl().isEmpty())
    {
        loadUrl(m_hitResult.imageUrl());
    }
}

void WebView::openImageInNewTab()
{
        m_page->m_openAction = WebPage::OpenNewTab;
    if (!m_hitResult.isNull() && !m_hitResult.imageUrl().isEmpty())
    {
        m_page->mainWindow()->tabWidget()->newTab( true )->loadUrl(m_hitResult.imageUrl());
    }
}

void WebView::openLinkInNewTab()
{
        m_page->m_openAction = WebPage::OpenNewTab;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
}

void WebView::openLinkInNewWin()
{
    m_page->m_openAction = WebPage::OpenNewWin;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
}

void WebView::loadStartedCustom()
{
    m_is_loading = true;
}

void WebView::setProgress(int progress)
{
    m_progress = progress;
    QString status = tr("Waiting for ") + url().host() + QString("... (%1%)").arg(progress);
    emit statusBarMessage ( status )  ;
}

void WebView::applyEncoding()
{
    if (m_encoding_in_progress)
        return;

    if (webPage() && webPage()->mainWindow())
    {
        QString enc = webPage()->mainWindow()->m_currentEncoding;
        if (enc.isEmpty())
            return;

        if (enc == m_current_encoding && m_current_encoding_url == url() )
            return;

        QWebPage *page = webPage();
        if (!page)
            return;

        QWebFrame *mainframe = page->mainFrame();
        if (!mainframe)
            return;

        QString html = mainframe->toHtml();

        QTextCodec *codec = QTextCodec::codecForName( enc.toAscii() );
        if (!codec)
            return;

        QTextDecoder *decoder = codec->makeDecoder();
        if (!decoder)
            return;

        m_encoding_in_progress = true;
        m_current_encoding = enc;
        m_current_encoding_url = url();
        QString output = decoder->toUnicode(html.toAscii());
        mainframe->setHtml(output, mainframe->url());

        QList<QWebFrame *> children = mainframe->childFrames();
        foreach(QWebFrame *frame, children)
        {
            html = frame->toHtml();
            output = decoder->toUnicode(html.toAscii());
            frame->setHtml(output, frame->url());
        }
        m_encoding_in_progress = false;
    }
}


void WebView::loadFinishedCustom(bool ok)
{
    if (100 != m_progress) 
        qWarning() << "Recieved finished signal while progress is still:" << progress() << "Url:" << url();
    
    if (ok && m_progress >= 100)
    {
        setStatusBarText(tr("Done"));

        if (page() && page()->currentFrame())
        {
            BrowserApplication::autoCompleter()->complete( page()->currentFrame());
            applyEncoding();

    
            BrowserApplication::instance()->mainWindow()->checkDumpAction(page());

        }
    }
    else
    {
        setStatusBarText(tr("Cancelled"));
        BrowserApplication::instance()->mainWindow()->checkQuitAction();
    }

    m_progress = 0;
    m_is_loading = false;
}



void WebView::loadUrl(const QUrl &url, const QString &title )
{
    m_current_encoding = "";
    m_current_encoding_url = "";
    m_ssl_errors_detected = false;

    // loading is already in progress
    if (m_is_loading && url == m_initialUrl)
        return;

/*  QString script = "function FindProxyForURL(url, host){ if (isPlainHostName(host)) return \"DIRECT\"; else return \"PROXY proxy:80\"; } FindProxyForURL('q','w');";
    QVariant res = page()->mainFrame()->evaluateJavaScript(script);
    if (res.canConvert(QVariant::String)) 
    {
        script = res.toString();
    }
*/

    if (url.scheme() == QLatin1String("javascript")) 
    {
        QString scriptSource = url.toString().mid(11);
        QVariant result = page()->mainFrame()->evaluateJavaScript(scriptSource);
        if (result.canConvert(QVariant::String)) {
            QString newHtml = result.toString();
            setHtml(newHtml);
        }
        return;
    }

    m_initialUrl = url;

    if (!title.isEmpty())
        emit titleChanged(tr("Loading..."));
    else
        emit titleChanged(title);

    if (url.toString().toLower().indexOf("ftp") == 0)
    {
        webPage()->mainWindow()->tabWidget()->setTabText(webPage()->mainWindow()->tabWidget()->currentIndex(), url.toString() );
        loadFtpUrl(url);
        return;
    }
    else
    {
        ftpCheckDisconnect();
    }

    load(url);
}
void WebView::clickedUrl(const QUrl &url)
{
    loadUrl(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

QUrl WebView::url() const
{
    QUrl url = QWebView::url();
    if (!url.isEmpty())
        return url; 

    return m_initialUrl;
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    if (m_font_resizing && (event->buttons() & Qt::MidButton) )
    {
        m_page->m_pressedButtons = (Qt::MouseButtons)((int)m_page->m_pressedButtons - (int)Qt::MidButton); 
    }
    m_page->m_keyboardModifiers = event->modifiers();
    m_font_resizing = false;
    
    // Start mouse gestures processing
    if (event->buttons() & Qt::RightButton)
    {
        m_gestureStartPos = event->pos();
        m_gestureStarted = true;
        m_gestureUrl = QUrl();
    }

    QWebView::mousePressEvent(event);
}

void WebView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_gestureStarted)
    {
        QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
        m_hitResult = r;
        if (!r.linkUrl().isEmpty()) 
        {
            m_gestureUrl = r.linkUrl();
        }
    }

    QWebView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    // Complete mouse gestures processing
    if ( m_gestureStarted )
    {
        m_gestureStarted = false;
        QPoint p = event->pos();

        int difX = p.x() - m_gestureStartPos.x();
        int difY = p.y() - m_gestureStartPos.y();
        
        if (abs(difX) >= 5 || abs(difY) >= 5 )
        {
            bool left(false), right(false), upper_left(false), upper_right(false), 
                 down_left(false), down_right(false), down(false), up(false);
            
            if (difX < 0 && (difY == 0 || abs(difX / difY) >= 2))
                left = true;
            else
            if (difX > 0 && (difY == 0 || abs(difX / difY) >= 2))
                right = true;
            else
            if (difY < 0 && (difX == 0 || abs(difY / difX) >= 2))
                up = true;
            else
            if (difY > 0 && (difX == 0 || abs(difY / difX) >= 2))
                down = true;
            else
            if (difX < 0 && difY < 0 && abs(difX / difY) < 2 && abs((float)difX / (float)difY) > 0.5)
                upper_left = true; 
            else
            if (difX > 0 && difY < 0 && abs(difX / difY) < 2 && abs((float)difX / (float)difY) > 0.5)
                upper_right = true;
            else
            if (difX > 0 && difY > 0 && abs(difX / difY) < 2 && abs((float)difX / (float)difY) > 0.5)
                down_right = true;
            else
            if (difX < 0 && difY > 0 && abs(difX / difY) < 2 && abs((float)difX / (float)difY) > 0.5)
                down_left = true;
        
            if (left)
                back();
            
            if (right)
                forward();

            if (BrowserApplication::instance() && BrowserApplication::instance()->mainWindow() && BrowserApplication::instance()->mainWindow()->tabWidget())
            {
                BrowserMainWindow *mw = BrowserApplication::instance()->mainWindow();
                    
                if (up) 
                {
                    if (!m_gestureUrl.isEmpty())
                    {
                        mw->tabWidget()->newTab( true )->loadUrl(m_gestureUrl);
                    }
                    else
                        if (difX > 0)
                            upper_right = true;
                        else
                            upper_left = true;
                }

                if (down) 
                {
                    if (!m_gestureUrl.isEmpty())
                    {
                        mw->tabWidget()->newTab( false )->loadUrl(m_gestureUrl);
                    }
                    else
                        if (difX > 0)
                            down_right = true;
                        else
                            down_left = true;
                }

                if (upper_right)
                {
                    mw->tabWidget()->nextTab(); // Next tab
                }

                if (upper_left)
                {
                    mw->tabWidget()->previousTab(); // Prev tab
                }
                    
                if (down_right)
                {
                    mw->tabWidget()->closeTab(); // Close Tab
                }

                if (down_left)
                {
                    mw->tabWidget()->prevSelectedTab(); // Previously selected tab
                }

            }

            m_gestureTime = QDateTime::currentDateTime();
            return;
        }
    }

    if (/*!event->isAccepted() &&*/ (m_page->m_pressedButtons & Qt::MidButton)) 
    {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) 
        {
            loadUrl(url);
            event->accept();
            return;
        }
    }

    if (/*!event->isAccepted() &&*/ (m_page->m_pressedButtons & Qt::XButton1))
    {
        back();
        event->accept();
        return;
    }

    if (/*!event->isAccepted() &&*/ (m_page->m_pressedButtons & Qt::XButton2))
    {
        forward();
        event->accept();
        return;
    }


    QWebView::mouseReleaseEvent(event);
}

void WebView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) 
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        bool zoom_text_only = settings.value(QLatin1String("zoom_text_only"), false).toBool();
        QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, zoom_text_only);
        event->accept();
        return;
    }

    if (m_page->m_pressedButtons & Qt::MidButton && BrowserApplication::startResizeOnMouseweelClick())
    {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        QSettings settings;
        settings.beginGroup(QLatin1String("websettings"));
        bool zoom_text_only = settings.value(QLatin1String("zoom_text_only"), false).toBool();
        QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, zoom_text_only);
        m_font_resizing = true;
        event->accept();
        return;
    }


    QWebView::wheelEvent(event);
}

void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
    emit statusBarMessage ( string )  ;
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    BrowserApplication::downloadManager()->download(request);
}

///////////////////////////////////////////////////////////////
// AC: FTP impl

void WebView::ftpCancelDownload()
{
    if (m_ftp)
        m_ftp->abort();
}


void WebView::ftpCheckDisconnect()
{
    if (m_ftp)
    {
        m_ftp->abort();
        m_ftp->deleteLater();
        m_ftp = NULL;
        setCursor(Qt::ArrowCursor);
    }
}

void WebView::loadFtpUrl(const QUrl &url)
{
    m_is_loading = true;

    m_ftpHtml.clear();
    setCursor(Qt::WaitCursor);

    m_ftp = new QFtp(this);
    connect(m_ftp, SIGNAL(commandFinished(int, bool)),
            this, SLOT(ftpCommandFinished(int, bool)));
    connect(m_ftp, SIGNAL(listInfo(const QUrlInfo &)),
            this, SLOT(ftpAddToList(const QUrlInfo &)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
            this, SLOT(ftpUpdateDataTransferProgress(qint64, qint64)));

    m_ftpHtml.clear();
    m_ftpCurrentPath.clear();
    m_ftpIsDirectory.clear();

    if (!url.isValid() || url.scheme().toLower() != QLatin1String("ftp")) {
        m_ftp->connectToHost(url.toString(), 21);
        m_ftp->login();
    } else {
        m_ftp->connectToHost(url.host(), url.port(21));

        if (!url.userName().isEmpty())
            m_ftp->login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
        else
            m_ftp->login();
        if (!url.path().isEmpty())
        {
            if (!url.hasQuery())
            {
                m_ftp->cd(url.path());
            }
            else
            {
                // Downloading file
                QString f =  url.path();
                int ind = f.lastIndexOf('/');
                QString dir = "";
                if (ind != -1 )
                {
                    dir = f.left(ind + 1);
                }

                m_ftp->cd(dir);

                setStatusBarText(tr("Downloading file %1...").arg(url.toString()));
                return;
            }
        }
    }

    setStatusBarText(tr("Connecting to FTP server %1...").arg(url.toString()));
}

void WebView::ftpDownloadFile(const QUrl &url, QString fileName )
{
    if (!m_ftp)
        return;

    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("There already exists a file called %1 in "
                                    "the current directory.")
                                 .arg(fileName));
        return;
    }

    QString downloadDirectory = dirDownloads(true);
    if (!downloadDirectory.isEmpty() && downloadDirectory.right(1) != QDir::separator())
        downloadDirectory += QDir::separator();
    
    QString fn = QFileDialog::getSaveFileName(this, tr("Save File"), downloadDirectory + fileName);
    if (fn.isEmpty()) 
    {
        QMessageBox::information(this, tr("FTP"), tr("Download canceled."));
        return;
    }

   
    m_ftpFile = new QFile(fn);
    if (!m_ftpFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fn).arg(m_ftpFile->errorString()));
        delete m_ftpFile;
        return;
    }

    m_ftp->get(fileName, m_ftpFile);

    m_ftpProgressDialog->setLabelText(QString("&nbsp;<br>" + tr("Downloading <b>%1</b>")).arg(fn));
    m_ftpProgressDialog->setWindowTitle(tr("FTP Download"));
    m_ftpProgressDialog->setModal(true);
}


void WebView::ftpCommandFinished(int res, bool error)
{
    setCursor(Qt::ArrowCursor);

    if (!m_ftp)
        return;

    if (m_ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server at %1. Please check that the host name is correct.")
                                     .arg(m_initialUrl.toString()));
            ftpCheckDisconnect();
            return;
        }

        setStatusBarText(tr("Logged onto %1.").arg(m_initialUrl.host()));
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Login)
    {
        setStatusBarText(tr("Listing %1...").arg(m_initialUrl.toString()));
        m_ftpHtml.clear();
        QString u = m_initialUrl.toString();
        int ind = u.lastIndexOf('/');
        if (ind != -1 && ind > 7 && ind < u.length() -1 )
        {
            u = u.left(ind);
        }
        else
            u = "";

        if ( !m_initialUrl.hasQuery() )
        {
            //<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">
            m_ftpHtml = "<html><body><PRE><H2>Contents of " + m_initialUrl.toString() + "</H2>" + 
                (u.length() > 0 ? "<p> Go to <a href=\"" + u + "\">parent directory</a>" : QString(""))+
                "<p><table cellpadding=3 cellspacing=3><tr><td></td><td><b>Name</b></td><td width=70 align=right><b>Size</b></td><td width=110 align=center><b>Date/Time</b></td></tr><tr><td>&nbsp;</td><td></td></tr>";
            setCursor(Qt::WaitCursor);
            m_ftp->list();
        }
        else
        {
            QString f =  m_initialUrl.toString().left(m_initialUrl.toString().length() - 4);
            int ind = f.lastIndexOf('/');
            if (ind != -1 )
            {
                f = f.right(f.length() - ind - 1);
            }
            ftpDownloadFile(m_initialUrl, f );
        }
    }

    if (m_ftp->currentCommand() == QFtp::Get) 
    {
        if (error) 
        {
            setStatusBarText(tr("Canceled download of %1").arg(m_ftpFile->fileName()));
            m_ftpFile->close();
            m_ftpFile->remove();
        } 
        else 
        {
            DownloadManager* dm = BrowserApplication::downloadManager();
            setStatusBarText(tr("Successfully downloaded file %1").arg(m_ftpFile->fileName()));
            m_ftpFile->close();

            dm->addItem(m_initialUrl, m_ftpFile->fileName(), true );
        }
        delete m_ftpFile;
        m_ftpProgressDialog->hide();
    } 
    else 
    if (m_ftp->currentCommand() == QFtp::List) 
    {
        setStatusBarText(tr("Listed %1").arg(m_initialUrl.toString()));
        if (m_ftpIsDirectory.isEmpty()) 
        {
            m_ftpHtml += "<tr><td>Directory is empty</td></tr>";
        }
        m_ftpHtml += "</table></PRE></body></html>";
        setHtml(m_ftpHtml, m_initialUrl);
        //page()->mainFrame()->setHtml(m_ftpHtml, m_initialUrl);
        //QString html = page()->mainFrame()->toHtml () ;
        //page()->setViewportSize(size()); 
        m_is_loading = false;

    }

    urlChanged(m_initialUrl);
}


void WebView::ftpAddToList(const QUrlInfo &urlInfo)
{
    m_ftpHtml += "<tr><td>" + (urlInfo.isDir() ? QString("DIR") : QString("")) +  "</td>"; 
    m_ftpHtml += "<td>" + (urlInfo.isDir() ? QString("<b>") : QString("")) + 
        (urlInfo.isDir() ? QString("<a href=\"" + m_initialUrl.toString() + (m_initialUrl.toString().right(1) == "/" ? QString(""): QString("/"))
        + urlInfo.name() + "\">") : QString("")) + 
        urlInfo.name() + (urlInfo.isDir() ? QString("</a>") : QString("")) + "</td>";
    m_ftpHtml += "<td align=right>" + (urlInfo.isDir() ? "" : QString::number(urlInfo.size())) + "</td>";
    m_ftpHtml += "<td align=center>" + urlInfo.lastModified().toString("MMM dd yyyy") + "</td>";
    m_ftpHtml += "<td>" + 
        (!urlInfo.isDir() ? QString("<a href=\"" + m_initialUrl.toString() + (m_initialUrl.toString().right(1) == "/" ? QString(""): QString("/"))
        + urlInfo.name() + "?get\">") : QString("")) + 
        (!urlInfo.isDir() ? QString("GET FILE</a>") : QString("")) + "</td></tr>";

    m_ftpIsDirectory[urlInfo.name()] = urlInfo.isDir();

}

void WebView::ftpUpdateDataTransferProgress(qint64 readBytes,
                                           qint64 totalBytes)
{
    if (m_ftpProgressDialog)
    {
        m_ftpProgressDialog->setMaximum(totalBytes);
        m_ftpProgressDialog->setValue(readBytes);
    }
}
