/*
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
 * Copyright (C) 2008 Benjamin C. Meyer <ben@meyerhome.net>
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
 * Copyright (C) 2008-2008 Trolltech ASA. All rights reserved.
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

#ifndef BROWSERAPPLICATION_H
#define BROWSERAPPLICATION_H

#include <QtGui/QApplication>

#include <QtCore/QUrl>
#include <QtCore/QPointer>

#include <QtGui/QIcon>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class BookmarksManager;
class BrowserMainWindow;
class CookieJar;
class DownloadManager;
class HistoryManager;
class NetworkAccessManager;
class QNetworkAccessManager;
class QNetworkReply;
class AutoComplete;
class TorrentWindow;

class BrowserApplication : public QApplication
{
    Q_OBJECT

public:
    BrowserApplication(int &argc, char **argv);
    ~BrowserApplication();
    static BrowserApplication *instance();
    static int getApplicationBuild();
	void definePortableRunMode();
    void loadSettings();
    void closeMainWindows();
    void closeTabs();

    bool isTheOnlyBrowser() const;
    BrowserMainWindow *mainWindow();
    QList<BrowserMainWindow*> mainWindows();
    QIcon icon(const QUrl &url) const;

	void CheckIcon(const QUrl &url);
	void CheckSetTranslator();

    void saveSession();
    bool canRestoreSession() const;

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static DownloadManager *downloadManager();
	static TorrentWindow *torrents();
    static NetworkAccessManager *networkAccessManager();
    static BookmarksManager *bookmarksManager();
	static bool	resetOnQuit() { return s_resetOnQuit; }
	static void	setResetOnQuit( bool reset) {s_resetOnQuit = reset; }
    static AutoComplete *autoCompleter();
	static bool startResizeOnMouseweelClick() { return s_startResizeOnMouseweelClick; }

	static bool handleMIME(QString content, const QUrl& url);

public:
	static void historyClear();
	static void emptyCaches();
	static void clearDownloads();
	static void clearCookies();
	static void clearIcons();
	static void clearPasswords();
	static void clearSearches();
	static void clearSSL();
	static void closeExtraWindows();
	static void resetSettings( bool reload);
	static QString dataLocation();
	static QString downloadsLocation(bool create_dir);
	static bool existDownloadManager() {return s_downloadManager != NULL;}
	static QString exeLocation() {return s_exeLocation;}
public slots:
    BrowserMainWindow *newMainWindow();
    void restoreLastSession();

private slots:
    void postLaunch();
    void openUrl(const QUrl &url);
    void newLocalSocketConnection();
	void iconDownloadFinished(QNetworkReply*);

private:
    void clean();
    void installTranslator(const QString &name);
	static QIcon getHostIcon(const QString &host);
	static void setHostIcon(const QString &host, const QIcon& icon);

	static QMap<QString, QIcon> s_hostIcons;
    static HistoryManager *s_historyManager;
    static DownloadManager *s_downloadManager;
    static TorrentWindow *s_torrents;
    static NetworkAccessManager *s_networkAccessManager;
    static BookmarksManager *s_bookmarksManager;
	static bool s_resetOnQuit;
	static bool s_startResizeOnMouseweelClick;
	static AutoComplete *s_autoCompleter;
	static bool s_portableRunMode;
	static QString s_exeLocation;

    QList<QPointer<BrowserMainWindow> > m_mainWindows;
    QLocalServer *m_localServer;
    QByteArray m_lastSession;
    mutable QIcon m_defaultIcon;
    mutable QIcon m_defaultSecureIcon;
    QNetworkAccessManager m_iconManager;
    bool quiting;
};

#endif // BROWSERAPPLICATION_H

