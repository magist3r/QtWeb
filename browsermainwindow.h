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

#ifndef BROWSERMAINWINDOW_H
#define BROWSERMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QIcon>
#include <QtCore/QUrl>

class AutoSaver;
class BookmarksToolBar;
class ChaseWidget;
class QWebFrame;
class QWebPage;
class QSplitter;
class TabWidget;
class ToolbarSearch;
class WebView;
class FindWidget;

/*!
    The MainWindow of the Browser Application.

    Handles the tab widget and all the actions
 */
class BrowserMainWindow : public QMainWindow {
    Q_OBJECT

public:
    BrowserMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BrowserMainWindow();
    QSize sizeHint() const;

public:
    static QUrl guessUrlFromString(const QString &url);
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    QByteArray saveState(bool withTabs = true) const;
    bool restoreState(const QByteArray &state);
	void emptyCache();
	void setLoadIcon();
    void checkToolBarButtons();

public slots:
    void loadPage(const QString &url);
    void slotHome();
	void setDumpFile(const QString &, bool quit);
	void checkDumpAction(QWebPage *);
	void checkQuitAction();
    void find(const QString &text, bool forward);

protected:
    void closeEvent(QCloseEvent *event);
	void setLastSizing(int value);

private slots:
    void save();

    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateWindowTitle(const QString &title = QString());

    void loadUrl(const QUrl &url);
    void slotPreferences();

    void slotFileNew();
    void slotFileOpen();
    void slotFilePrint();
    void slotFilePrintPreview();
    void slotPrivateBrowsing();
    void slotFileSaveAs();
    void slotFileSavePdf();
    void slotEditFind();
    void slotEditFindNext();
    void slotEditFindPrevious();
	void slotEmptyCache();
    void slotShowBookmarksDialog();
    void slotAddBookmark();
    void slotViewTextBigger();
    void slotViewTextNormal();
    void slotViewTextSmaller();
	void slotZoomTextOnly(bool);
    void slotViewToolbar();
    void slotViewMenubar();
    void slotViewBookmarksBar();
    void slotViewStatusbar();
    void slotViewPageSource();
    void slotViewFullScreen(bool enable);

	void slotStyleChange();
	void slotNextStyle();
	void slotResetQtWeb();
	void slotFullCleanUpOnQuit();
	void slotToggleProxy();
	void slotToggleImages();

	void slotDisableJavaScript();
	void slotDisableImages();
	void slotDisableCookies();
	void slotDisablePlugIns();
	void slotDisableUserAgent();
	void slotEnableProxy();
	void slotDisablePopUps();

	void slotWebSearch();
    void slotToggleInspector(bool enable);
    void slotAboutApplication();
	void slotCheckUpdates();
	void slotHelp();
	void slotHelpOnline();

    void slotDownloadManager();
    void slotTorrents();
    void slotSelectLineEdit();

	void slotAboutToShowStyles();
	void slotAboutToShowBackMenu();
    void slotAboutToShowForwardMenu();
    void slotAboutToShowWindowMenu();
	void slotAboutToShowEncodingMenu();
	void slotOpenActionUrl(QAction *action);
    void slotShowWindow();
    void slotSwapFocus();
	void slotAboutToShowPrivacyMenu();
	void slotAboutToShowToolsMenu();
	void slotAboutToShowCompatibility();
	void slotEncodingChange();
	void slotVirtualKeyboard();
	void slotChangeTextSize();
	void slotInspectElement();
	void slotCompatChange();
	void slotNextUserAgent();
	
	void slotLoadPage();

    void printRequested(QWebFrame *frame);
    void geometryChangeRequested(const QRect &geometry);
    void updateToolbarActionText(bool visible);
    void updateMenubarActionText(bool visible);
    void updateBookmarksToolbarActionText(bool visible);

private:
    void loadDefaultState();
    void setupMenu();
    void setupToolBar();
    void updateStatusbarActionText(bool visible);
	QAction* menuAddEncoding(QString name, QString encoding);

	QStringList compatibleUserAgents();
	void setCompatibilityAgent(QString agent);
	QString getAgentIcon(QString agent );
	QString currentListedAgent();

public:
	bool showMenuIcons() {return m_showMenuIcons;}
	void loadSettings();
	void setCurrentAgentIcon();

    FindWidget *findWidget;
	ToolbarSearch *m_toolbarSearch;
	QString m_currentEncoding;
	bool	m_showMenuIcons;

private:
    QToolBar *m_navigationBar;
	QToolBar *m_buttonsBar;
    BookmarksToolBar *m_bookmarksToolbar;
    ChaseWidget *m_chaseWidget;
    TabWidget *m_tabWidget;
    AutoSaver *m_autoSaver;
	bool	m_positionRestored;
	QString  m_dumpFile;
	bool	 m_dumpActionQuit;
	QSplitter *m_navSplit;

    QAction *m_historyBack;
    QMenu	*m_historyBackMenu;
    QAction *m_historyForward;
    QMenu	*m_historyForwardMenu;
    QMenu	*m_windowMenu;
    QAction *m_styles;
	QMenu	*m_stylesMenu;
	QMenu	*m_encodingMenu;
	QMenu	*m_sizesMenu;
	QMenu	*m_compMenu;
	//QMenu	*m_compatMenu;

    QAction *m_stop;
    QAction *m_reload;
    QAction *m_load;
    QAction *m_stopReload;
    QAction *m_viewMenuBar;
    QAction *m_viewToolBar;
    QAction *m_viewBookmarkBar;
    QAction *m_viewStatusBar;
	QAction *m_viewZoomTextOnly;
    QAction *m_restoreLastSession;
    QAction *m_restoreLastTab;
    QAction *m_addBookmark;
    QAction *m_privateBrowsing;
    QAction *m_privateBrowsingMenu;
    QAction *m_fullCleanUpOnQuitMenu;
    QAction *m_emptyCache;
	QAction *m_disableJavaScript;
	QAction *m_disableImages;
	QAction *m_disableCookies;
	QAction *m_disablePlugIns;
	QAction *m_disableUserAgent;
	QAction *m_enableProxy;
	QAction *m_disablePopUps;


    QAction *m_goBackAction;
    QAction *m_goForwardAction;
    QAction *m_addBookmarkAction;
	QAction *m_homeAction;
	QAction *m_prefsAction;
	QAction *m_imagesAction;
	QAction *m_proxyAction;
	QAction *m_restoreTabAction;
	QAction *m_resetAction;
	QAction *m_enableInspector; 
	QAction *m_inspectElement; 

	QAction *m_inspectAction; 
	QAction *m_keyboardAction; 
	QAction *m_textSizeAction; 
	QAction *m_bookmarksAction; 

	QAction	*m_compatAction;
	QAction *m_compIE; 
	QAction *m_compMozilla; 
	QAction *m_compOpera; 
	QAction *m_compSafari; 
	QAction *m_compQtWeb; 
	QAction *m_compChrome; 
	QAction *m_compCustom;


	QAction *m_textSizeLarger; 
	QAction *m_textSizeNormal; 
	QAction *m_textSizeSmaller; 

    QIcon m_reloadIcon;
    QIcon m_stopIcon;
    QIcon m_loadIcon;

    QIcon m_privacyIcon;
    QIcon m_privacyIconOn;

	QString	m_title;
};

#endif // BROWSERMAINWINDOW_H

