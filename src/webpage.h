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

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QHash>
class QFile;
class QFtp;
class QProgressDialog;
class QUrlInfo;

#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QMouseEvent;
class QNetworkProxy;
class QNetworkReply;
class QSslError;
QT_END_NAMESPACE

class BrowserMainWindow;
class WebPage : public QWebPage {
    Q_OBJECT

signals:
    void loadingUrl(const QUrl &url);

public:
    WebPage(QObject *parent = 0);
    BrowserMainWindow *mainWindow();

    static void setUserAgent(QString agent);
	static void setDefaultAgent( );
	static const QString& getUserAgent();

	enum OpenAction {
		OpenDefault,
		OpenNewTab,
		OpenNewWin
	};
protected:
    virtual QString userAgentForUrl(const QUrl& url) const;
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
    QWebPage *createWindow(QWebPage::WebWindowType type);
	void DefineHostIcon(const QUrl& host);

#if !defined(QT_NO_UITOOLS)
    QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
#endif

private slots:
    void handleUnsupportedContent(QNetworkReply *reply);

private:
    friend class WebView;

    // set the webview mousepressedevent
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;
 
	OpenAction m_openAction;
    QUrl m_loadingUrl;
	static QString m_userAgent;
};

#endif // WEBPAGE_H
