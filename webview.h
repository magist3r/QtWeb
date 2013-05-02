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

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QHash>
class QFile;
class QFtp;
class QProgressDialog;
class QUrlInfo;

#include <QDateTime>
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebHitTestResult>

class WebPage;

class WebView : public QWebView {
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);
    WebPage *webPage() const { return m_page; }

    void loadUrl(const QUrl &url, const QString &title = QString());
    void loadFtpUrl(const QUrl &url);
    QUrl url() const;

	void slotInspectElement();

    QString lastStatusBarText() const;
    inline int progress() const { return m_progress; }
	bool sslErrors() { return m_ssl_errors_detected; }
	void setSslErrors() { m_ssl_errors_detected = true; }
	bool isLoading() {return m_is_loading; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);
	void applyEncoding();

private slots:
    void loadStartedCustom();
    void setProgress(int progress);
    void loadFinishedCustom(bool ok);
    void setStatusBarText(const QString &string);
    void downloadRequested(const QNetworkRequest &request);
    void openLinkInNewTab();
    void openLinkInNewWin();
    void openImageInNewTab();
    void openImageInNewWin();
	void copyMailtoAddress();
    void clickedUrl(const QUrl &url);
    void adBlock();

private:
    QString m_statusBarText;
    QUrl	m_initialUrl;
    int		m_progress;
	bool	m_is_loading;
	QWebHitTestResult m_hitResult;
	QPoint  m_gestureStartPos;
	bool	m_gestureStarted;
	QUrl	m_gestureUrl;
	QDateTime m_gestureTime;

    WebPage *m_page;
	bool	m_font_resizing;
	bool	m_encoding_in_progress;
	QString m_current_encoding;
	QUrl	m_current_encoding_url;
	bool	m_ssl_errors_detected;

//////////////////////////////////////////////////// AC: FTP implementation
	QFtp*  m_ftp;
    QFile* m_ftpFile;
	QString m_ftpHtml;

	QProgressDialog			*m_ftpProgressDialog;
    QHash<QString, bool>	m_ftpIsDirectory;
    QString					m_ftpCurrentPath;

private:
	void ftpCheckDisconnect();
	void ftpDownloadFile(const QUrl &url, QString filename );

private slots:
    void ftpCancelDownload();
    void ftpCommandFinished(int commandId, bool error);
    void ftpAddToList(const QUrlInfo &urlInfo);
    void ftpUpdateDataTransferProgress(qint64 readBytes, qint64 totalBytes);
};

#endif
