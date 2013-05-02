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
 * MERCHANTABILITY or FITNESS FO BrowserMainWindow *mw = BrowserApplication::instance()->mainWindow()R A PARTICULAR PURPOSE.  See the
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

#include "browsermainwindow.h"
#include "aboutdialog.h"
#include "autosaver.h"
#include "bookmarks.h"
#include "browserapplication.h"
#include "chasewidget.h"
#include "downloadmanager.h"
#include "history.h"
#include "settings.h"
#include "tabwidget.h"
#include "toolbarsearch.h"
#include "ui_passworddialog.h"
#include "webview.h"
#include "webpage.h"
#include "resetsettings.h"
#include "commands.h"
#include "cookiejar.h"
#include "viewsource.h"
#include "findwidget.h"

#include <QtCore/QSettings>
#include <QtCore/QMetaEnum>
#include <QTextCodec>
#include <QtGui/QDesktopWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QInputDialog>
#include <QThread>
#include <QFile>
#include <QStyleFactory>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebHistory>
#include <tabwidget.h>
#include <QtCore/QDebug>
#include <networkaccessmanager.h>
#include <qsplitter.h>
#include "savepdf.h"
#include "torrent/torrentwindow.h"

extern bool ShellExec(QString path);

BrowserMainWindow::BrowserMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_tabWidget(new TabWidget(this))
    , m_autoSaver(new AutoSaver(this))
    , findWidget(0)
    , m_navSplit(0)
	, m_buttonsBar(0)
    , m_historyBack(0)
    , m_historyForward(0)
    , m_stop(0)
    , m_reload(0)
	, m_load(0)
	, m_stylesMenu(0)
	, m_encodingMenu(0)
	, m_styles(0)
	, m_emptyCache(0)
	, m_viewZoomTextOnly(0)
	, m_positionRestored(0)
    , m_goBackAction(0)
    , m_goForwardAction(0)
    , m_addBookmarkAction(0)
	, m_homeAction(0)
	, m_prefsAction(0)
	, m_imagesAction(0)
	, m_proxyAction(0)
	, m_restoreTabAction(0)
	, m_resetAction(0)
	, m_enableInspector(0)
	, m_inspectElement(0)
	, m_dumpActionQuit(false)
	, m_showMenuIcons(false)
	, m_inspectAction(0) 
	, m_keyboardAction(0)
	, m_textSizeAction(0)
	, m_bookmarksAction(0)
	, m_sizesMenu(0)
	, m_textSizeLarger(0)
	, m_textSizeNormal(0) 
	, m_textSizeSmaller(0)
	, m_compMenu(0)
//	, m_compatMenu(0)
	, m_compatAction(0)
	, m_compIE(0)
	, m_compMozilla(0)
	, m_compQtWeb(0)
	, m_compOpera(0) 
	, m_compSafari(0)
	, m_compChrome(0)
	, m_compCustom(0)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    statusBar()->setSizeGripEnabled(true);
    setupMenu();
    setupToolBar();

    QWidget *centralWidget = new QWidget(this);
    BookmarksModel *boomarksModel = BrowserApplication::bookmarksManager()->bookmarksModel();
    m_bookmarksToolbar = new BookmarksToolBar(boomarksModel, this);
    connect(m_bookmarksToolbar, SIGNAL(openUrl(const QUrl&, TabWidget::Tab, const QString&)),
            m_tabWidget, SLOT(loadUrl(const QUrl&, TabWidget::Tab, const QString&)));

    connect(m_bookmarksToolbar->toggleViewAction(), SIGNAL(toggled(bool)),
            this, SLOT(updateBookmarksToolbarActionText(bool)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

	addToolBarBreak();
    addToolBar(Qt::RightToolBarArea, m_bookmarksToolbar);

	m_bookmarksToolbar->setVisible(true);
    updateBookmarksToolbarActionText(false);
	

    layout->addWidget(m_tabWidget);
    centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

    connect(m_tabWidget, SIGNAL(loadPage(const QString &)),
        this, SLOT(loadPage(const QString &)));
    connect(m_tabWidget, SIGNAL(setCurrentTitle(const QString &)),
        this, SLOT(slotUpdateWindowTitle(const QString &)));
    connect(m_tabWidget, SIGNAL(showStatusBarMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(m_tabWidget, SIGNAL(linkHovered(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(m_tabWidget, SIGNAL(loadProgress(int)),
            this, SLOT(slotLoadProgress(int)));
    connect(m_tabWidget, SIGNAL(tabsChanged()),
            m_autoSaver, SLOT(changeOccurred()));
    connect(m_tabWidget, SIGNAL(geometryChangeRequested(const QRect &)),
            this, SLOT(geometryChangeRequested(const QRect &)));
    connect(m_tabWidget, SIGNAL(printRequested(QWebFrame *)),
            this, SLOT(printRequested(QWebFrame *)));
    connect(m_tabWidget, SIGNAL(menuBarVisibilityChangeRequested(bool)),
            menuBar(), SLOT(setVisible(bool)));
    connect(m_tabWidget, SIGNAL(statusBarVisibilityChangeRequested(bool)),
            statusBar(), SLOT(setVisible(bool)));
    connect(m_tabWidget, SIGNAL(toolBarVisibilityChangeRequested(bool)),
            m_navigationBar, SLOT(setVisible(bool)));
    connect(m_tabWidget, SIGNAL(toolBarVisibilityChangeRequested(bool)),
            m_bookmarksToolbar, SLOT(setVisible(bool)));

	connect(m_tabWidget, SIGNAL(lastTabClosed()),
            this, SLOT(close()));

    slotUpdateWindowTitle();
    loadDefaultState();
    m_tabWidget->newTab();

    int size = m_tabWidget->lineEditStack()->sizeHint().height();
    m_navigationBar->setIconSize(QSize(size, size));
	m_buttonsBar->setIconSize(QSize(size, size));

   	WebPage::setDefaultAgent( );

    findWidget = new FindWidget(this);
    layout->addWidget(findWidget);
    findWidget->hide();

    connect(findWidget, SIGNAL(findNext()), this, SLOT(slotEditFindNext()));
    connect(findWidget, SIGNAL(findPrevious()), this, SLOT(slotEditFindPrevious()));
    connect(findWidget, SIGNAL(find(QString, bool)), this,
        SLOT(find(QString, bool)));
    connect(findWidget, SIGNAL(escapePressed()), this, SLOT(slotShowWindow()));
}

BrowserMainWindow::~BrowserMainWindow()
{
    m_autoSaver->changeOccurred();
    m_autoSaver->saveIfNeccessary();
}

void BrowserMainWindow::loadDefaultState()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QByteArray data = settings.value(QLatin1String("defaultState")).toByteArray();
    restoreState(data);
    settings.endGroup();

#ifdef WINPE
	QWebSettings::setMaximumPagesInCache ( 5 );
	QWebSettings::setObjectCacheCapacities ( 1024*1024, 3*1024*1024, 4*1024*1024 );
#endif

}

QSize BrowserMainWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * 0.9;
    return size;
}

void BrowserMainWindow::save()
{
    BrowserApplication::instance()->saveSession();

    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QByteArray data = saveState(false);
    settings.setValue(QLatin1String("defaultState"), data);
    settings.endGroup();

}

static const qint32 BrowserMainWindowMagic = 0xFEEFFEEF;

QByteArray BrowserMainWindow::saveState(bool withTabs) const
{
    int version = 6;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(BrowserMainWindowMagic);
    stream << qint32(version);

	if (withTabs)
        stream << tabWidget()->saveState();
    else
        stream << QByteArray();

    stream << m_tabWidget->tabBar()->showTabBarWhenOneTab();
    stream << !statusBar()->isHidden();

	QByteArray geo = QMainWindow::saveGeometry();
	stream << geo;

	QByteArray state = QMainWindow::saveState(version);
	stream << state;

	stream << m_navigationBar->saveGeometry();
	stream << m_bookmarksToolbar->saveGeometry();
    stream << m_navSplit->saveState();

    return data;
}

bool BrowserMainWindow::restoreState(const QByteArray &state)
{
    int version = 6;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker, v;
    stream >> marker;
    stream >> v;
    if (marker != BrowserMainWindowMagic || v != version)
        return false;

    QByteArray tabState;
    QByteArray mainwindowState;
    QByteArray mainwindowGeo;
    QByteArray navGeo;
    QByteArray bookGeo;
    bool showStatusbar;
    bool showTabBarWhenOneTab;
    QByteArray splitterState1;//, splitterState2;
	bool bMenu = true;

    stream >> tabState;

    stream >> showTabBarWhenOneTab;
    stream >> showStatusbar;
	stream >> mainwindowGeo;
	stream >> mainwindowState;
	stream >> navGeo;
	stream >> bookGeo;
    stream >> splitterState1;

    if (!tabState.isEmpty() && !tabWidget()->restoreState(tabState))
        return false;

    m_tabWidget->tabBar()->setShowTabBarWhenOneTab(showTabBarWhenOneTab);


	if (!m_positionRestored)
	{
		m_positionRestored = true;
		QMainWindow::restoreGeometry(mainwindowGeo);
	}
	QMainWindow::restoreState(mainwindowState, version);

    statusBar()->setVisible(showStatusbar);
    updateStatusbarActionText(showStatusbar);

    m_navSplit->restoreState(splitterState1);

    return true;
}

extern QString DefaultAppStyle();

void BrowserMainWindow::loadSettings()
{
	QSettings settings;

	settings.beginGroup(QLatin1String("general"));
	bool bMenu = settings.value(QLatin1String("ShowMenu"), true).toBool();
	menuBar()->setVisible(bMenu);
	m_showMenuIcons = !settings.value(QLatin1String("hideMenuIcons"), false).toBool();

	bool bEnableInspector = settings.value(QLatin1String("EnableWebInspector"), false).toBool();
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, bEnableInspector);

	settings.endGroup();
}

void BrowserMainWindow::setupMenu()
{
	loadSettings();

	MenuCommands cmds;

	QAction* swap_focus = new QAction(cmds.SwapFocusTitle(), this);
	swap_focus->setShortcuts(cmds.SwapFocusShortcuts());
	connect(swap_focus, SIGNAL(triggered()), this, SLOT(slotSwapFocus()));
	this->addAction( swap_focus);

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(cmds.FileTitle());

	// &New Window
	QAction* new_win = new QAction(cmds.NewWinTitle(), this);
	new_win->setShortcuts(cmds.NewWinShortcuts());
	connect(new_win, SIGNAL(triggered()), this, SLOT(slotFileNew()));
    fileMenu->addAction( new_win);
	this->addAction( new_win);

	// New Tab
    fileMenu->addAction(m_tabWidget->newTabAction());
	this->addAction( m_tabWidget->newTabAction() );

	// &Open File
	QAction* open_file = new QAction(cmds.OpenFileTitle(), this);
	open_file->setShortcuts(cmds.OpenFileShortcuts());
	connect(open_file, SIGNAL(triggered()), this, SLOT(slotFileOpen()));
    fileMenu->addAction( open_file  );
	this->addAction( open_file);

	// &Open Location
	QAction* open_loc = new QAction(cmds.OpenLocTitle(), this);
	open_loc->setShortcuts(cmds.OpenLocShortcuts());
	connect(open_loc, SIGNAL(triggered()), this, SLOT(slotSelectLineEdit()));
    fileMenu->addAction( open_loc  );
	this->addAction( open_loc);


    fileMenu->addSeparator();

	// Close Tab
    fileMenu->addAction( m_tabWidget->closeTabAction() );
	this->addAction( m_tabWidget->closeTabAction() );

    fileMenu->addSeparator();

	// &Save As...
	QAction* save_as = new QAction(cmds.SaveAsTitle(), this);
	if (showMenuIcons()) 
		save_as->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	save_as->setShortcuts(cmds.SaveAsShortcuts());
	connect(save_as, SIGNAL(triggered()), this, SLOT(slotFileSaveAs()));
    fileMenu->addAction( save_as  );
	this->addAction( save_as);

	// &Save As PDF...
	QAction* save_pdf = new QAction(cmds.SavePdfTitle(), this);
	save_pdf->setShortcuts(cmds.SavePdfShortcuts());
	connect(save_pdf, SIGNAL(triggered()), this, SLOT(slotFileSavePdf()));
    fileMenu->addAction( save_pdf  );
	this->addAction( save_pdf );

	fileMenu->addSeparator();

	// Import Bookmarks
    BookmarksManager *bookmarksManager = BrowserApplication::bookmarksManager();
    QMenu *importMenu = fileMenu->addMenu(  cmds.ImportTitle() );

#ifdef Q_WS_WIN
	// Import from Internet Explorer
	QAction* import_ie = new QAction(cmds.ImportIETitle(), this);
	import_ie->setShortcuts(cmds.ImportIEShortcuts());
	connect(import_ie, SIGNAL(triggered()), bookmarksManager, SLOT(importFromIE()));
    importMenu->addAction( import_ie  );
	this->addAction( import_ie );

	// Import from Mozilla FireFox
	QAction* import_ff = new QAction(cmds.ImportFFTitle(), this);
	import_ff->setShortcuts(cmds.ImportFFShortcuts());
	connect(import_ff, SIGNAL(triggered()), bookmarksManager, SLOT(importFromMozilla()));
    importMenu->addAction( import_ff  );
	this->addAction( import_ff );
#endif

	// Import from HTML 
	QAction* import_html = new QAction(cmds.ImportHtmlTitle(), this);
	import_html->setShortcuts(cmds.ImportHtmlShortcuts());
	connect(import_html, SIGNAL(triggered()), bookmarksManager, SLOT(importFromHTML()));
    importMenu->addAction( import_html  );
	this->addAction( import_html );

	// Import from XML 
	QAction* import_xml = new QAction(cmds.ImportXmlTitle(), this);
	import_xml->setShortcuts(cmds.ImportXmlShortcuts());
	connect(import_xml, SIGNAL(triggered()), bookmarksManager, SLOT(importBookmarks()));
    importMenu->addAction( import_xml );
	this->addAction( import_xml );

	// Export Bookmarks
	QAction* export_ = new QAction(cmds.ExportTitle(), this);
	export_->setShortcuts(cmds.ExportShortcuts());
	connect(export_, SIGNAL(triggered()), bookmarksManager, SLOT(exportBookmarks()));
    fileMenu->addAction( export_ );
	this->addAction( export_ );

    fileMenu->addSeparator();

	// Print
	QAction* print = new QAction(cmds.PrintTitle(), this);
	if (showMenuIcons()) 
		print->setIcon(QIcon(QLatin1String(":printer3.png")));
	print->setShortcuts(cmds.PrintShortcuts());
	connect(print, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
    fileMenu->addAction( print );
	this->addAction( print );

	// Print Preview
	QAction* preview = new QAction(cmds.PreviewTitle(), this);
	preview->setShortcuts(cmds.PreviewShortcuts());
	connect(preview, SIGNAL(triggered()), this, SLOT(slotFilePrintPreview()));
    fileMenu->addAction( preview );
	this->addAction( preview );

	fileMenu->addSeparator();

	// Quit
	QAction* quit = new QAction(cmds.QuitTitle(), this);
	if (showMenuIcons()) 
		quit->setIcon( style()->standardIcon(QStyle::SP_DialogCloseButton) );
	quit->setShortcuts(cmds.QuitShortcuts());
	connect(quit, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction( quit );
	this->addAction( quit );

    // Edit Menu
    QMenu *editMenu = menuBar()->addMenu(cmds.EditTitle());

	// Undo
	QAction* m_undo = new QAction(cmds.UndoTitle(), this);
	m_undo->setShortcuts(cmds.UndoShortcuts());
	editMenu->addAction( m_undo );
	this->addAction( m_undo );
  	m_tabWidget->addWebAction(m_undo, QWebPage::Undo);

	// Redo
	QAction *m_redo = new QAction(cmds.RedoTitle(), this);
	m_redo->setShortcuts(cmds.RedoShortcuts());
	editMenu->addAction( m_redo );
	this->addAction( m_redo );
  	m_tabWidget->addWebAction(m_redo, QWebPage::Redo);

    editMenu->addSeparator();

	// Cut
	QAction *m_cut = new QAction(cmds.CutTitle(), this);
	m_cut->setShortcuts(cmds.CutShortcuts());
	editMenu->addAction( m_cut );
	this->addAction( m_cut );
    m_tabWidget->addWebAction(m_cut, QWebPage::Cut);
   
	// Copy
	QAction *m_copy = new QAction(cmds.CopyTitle(), this);
	m_copy->setShortcuts(cmds.CopyShortcuts());
	editMenu->addAction( m_copy );
	this->addAction( m_copy );
    m_tabWidget->addWebAction(m_copy, QWebPage::Copy);

	// Paste
	QAction *m_paste = new QAction(cmds.PasteTitle(), this);
	m_paste->setShortcuts(cmds.PasteShortcuts());
	editMenu->addAction( m_paste );
	this->addAction( m_paste );
    m_tabWidget->addWebAction(m_paste, QWebPage::Paste);
    
	editMenu->addSeparator();

	// Find
	QAction* m_find = new QAction(cmds.FindTitle(), this);
	if (showMenuIcons()) 
		m_find->setIcon( QIcon(QLatin1String(":document_view.png")) );
	m_find->setShortcuts(cmds.FindShortcuts());
	connect(m_find, SIGNAL(triggered()), this, SLOT(slotEditFind()));
    editMenu->addAction( m_find );
	this->addAction( m_find );

	// Find Next
	QAction* m_findNext = new QAction(cmds.NextTitle(), this);
	m_findNext->setShortcuts(cmds.NextShortcuts());
	connect(m_findNext, SIGNAL(triggered()), this, SLOT(slotEditFindNext()));
    editMenu->addAction( m_findNext );
	this->addAction( m_findNext );

	// Find Previous
	QAction* m_findPrevious = new QAction(cmds.PrevTitle(), this);
	m_findPrevious->setShortcuts(cmds.PrevShortcuts());
	connect(m_findPrevious, SIGNAL(triggered()), this, SLOT(slotEditFindPrevious()));
    editMenu->addAction( m_findPrevious );
	this->addAction( m_findPrevious );

    editMenu->addSeparator();

	// Preferences
	QAction* prefs = new QAction(cmds.PrefsTitle(), this);
	if (showMenuIcons()) 
		prefs->setIcon( QIcon(QLatin1String(":settings.png")) );
	prefs->setShortcuts(cmds.PrefsShortcuts());
	connect(prefs, SIGNAL(triggered()), this, SLOT(slotPreferences()));
    editMenu->addAction( prefs );
	this->addAction( prefs );

    // View Menu
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    connect(viewMenu , SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowStyles()));

	// Application Styles
    m_stylesMenu = viewMenu->addMenu(cmds.AppStylesTitle());
	if (showMenuIcons()) 
		m_stylesMenu->setIcon( QIcon(QLatin1String(":app_style.png")) );
	
	viewMenu->addSeparator();

	// Show/Hide Bookmarks Bar
	m_viewBookmarkBar = new QAction(this);
    updateBookmarksToolbarActionText(true);
    m_viewBookmarkBar->setShortcuts(cmds.BooksShortcuts());
    connect(m_viewBookmarkBar, SIGNAL(triggered()), this, SLOT(slotViewBookmarksBar()));
    viewMenu->addAction(m_viewBookmarkBar);
    this->addAction(m_viewBookmarkBar);

	// Show/Hide Menu Bar
	m_viewMenuBar = new QAction(this);
    updateMenubarActionText(true);
    m_viewMenuBar->setShortcuts(cmds.MenuShortcuts());
    connect(m_viewMenuBar, SIGNAL(triggered()), this, SLOT(slotViewMenubar()));
    viewMenu->addAction(m_viewMenuBar);
    this->addAction(m_viewMenuBar);

	// Show/Hide Tool Bar
    m_viewToolBar = new QAction(this);
    updateToolbarActionText(true);
    m_viewToolBar->setShortcuts(cmds.NavShortcuts());
    connect(m_viewToolBar, SIGNAL(triggered()), this, SLOT(slotViewToolbar()));
    viewMenu->addAction(m_viewToolBar);
    this->addAction(m_viewToolBar);

	// Show/Hide Tab Bar
    QAction *viewTabBarAction = m_tabWidget->tabBar()->viewTabBarAction();
    viewTabBarAction ->setShortcuts(cmds.TabShortcuts());
	viewMenu->addAction(viewTabBarAction);
	this->addAction(viewTabBarAction);
    connect(viewTabBarAction, SIGNAL(changed()),
            m_autoSaver, SLOT(changeOccurred()));

	// Show/Hide Stastus Bar
    m_viewStatusBar = new QAction(this);
    updateStatusbarActionText(false);
	statusBar()->hide();
    m_viewStatusBar->setShortcuts(cmds.StatusShortcuts());
    connect(m_viewStatusBar, SIGNAL(triggered()), this, SLOT(slotViewStatusbar()));
    viewMenu->addAction(m_viewStatusBar);
    this->addAction(m_viewStatusBar);

    viewMenu->addSeparator();

	// Reload
    m_reload = new QAction(cmds.ReloadTitle(), this);
	if (showMenuIcons()) 
	{
		m_reload->setIcon(QIcon(QLatin1String(":nav_refresh_green.png")));
	    m_reload->setIconVisibleInMenu(true);
	}
    m_reload->setShortcuts(cmds.ReloadShortcuts());
	viewMenu->addAction( m_reload );
	this->addAction( m_reload );
    m_tabWidget->addWebAction(m_reload, QWebPage::Reload);

	// Stop
    m_stop = new QAction(cmds.StopTitle(), this);
	m_stop->setShortcuts(cmds.StopShortcuts());
	viewMenu->addAction( m_stop );
	this->addAction( m_stop );
    m_tabWidget->addWebAction(m_stop, QWebPage::Stop);

	// Make Text Larger
	QAction* larger = new QAction(cmds.LargerTitle(), this);
	if (showMenuIcons()) 
		larger->setIcon(QIcon(QLatin1String(":zoom-in-24.png")));
    larger->setShortcuts(cmds.LargerShortcuts());
    connect(larger, SIGNAL(triggered()), this, SLOT(slotViewTextBigger()));
	viewMenu->addAction( larger );
	this->addAction( larger );

	// Make Text Normal
	QAction* normal = new QAction(cmds.NormalTitle(), this);
    normal->setShortcuts(cmds.NormalShortcuts());
    connect(normal, SIGNAL(triggered()), this, SLOT(slotViewTextNormal()));
	viewMenu->addAction( normal );
	this->addAction( normal );

	// Make Text Smaller
	QAction* smaller = new QAction(cmds.SmallerTitle(), this);
	if (showMenuIcons()) 
		smaller->setIcon(QIcon(QLatin1String(":zoom-out-24.png")));
    smaller->setShortcuts(cmds.SmallerShortcuts());
    connect(smaller, SIGNAL(triggered()), this, SLOT(slotViewTextSmaller()));
	viewMenu->addAction( smaller );
	this->addAction( smaller );

	m_viewZoomTextOnly = new QAction(cmds.TextOnlyTitle(), this);
    m_viewZoomTextOnly->setShortcuts(cmds.TextOnlyShortcuts());
	m_viewZoomTextOnly->setCheckable(true);
    connect(m_viewZoomTextOnly, SIGNAL(toggled(bool)), this, SLOT(slotZoomTextOnly(bool)));

	viewMenu->addAction( m_viewZoomTextOnly );
	this->addAction( m_viewZoomTextOnly );

	viewMenu->addSeparator();
	
	// Text Encoding
	m_encodingMenu = viewMenu->addMenu( cmds.EncodeTitle() );
	connect(m_encodingMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowEncodingMenu()));

    viewMenu->addSeparator();

	// Page Source
	QAction* source = new QAction(cmds.SourceTitle(), this);
    source->setShortcuts(cmds.SourceShortcuts());
    connect(source, SIGNAL(triggered()), this, SLOT(slotViewPageSource()));
	viewMenu->addAction( source );
	this->addAction( source );

	// Full Screen
	QAction* full = new QAction(cmds.FullTitle(), this);
	if (showMenuIcons()) 
		full->setIcon(QIcon(QLatin1String(":fullscreen.png")));
    full->setShortcuts(cmds.FullShortcuts());
    connect(full, SIGNAL(triggered(bool)), this, SLOT(slotViewFullScreen( bool )));
    full->setCheckable(true);
	viewMenu->addAction( full );
	this->addAction( full );

    // History
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, SIGNAL(openUrl(const QUrl&)),	m_tabWidget, SLOT(loadUrlInCurrentTab(const QUrl&)));
    connect(historyMenu, SIGNAL(hovered(const QString&)), this, SLOT(slotUpdateStatusbar(const QString&)));
    historyMenu->setTitle( cmds.HistoryTitle());
    menuBar()->addMenu(historyMenu);
    QList<QAction*> historyActions;

	// Go Back
    m_historyBack = new QAction( cmds.BackTitle(), this);
    m_tabWidget->addWebAction(m_historyBack, QWebPage::Back);
    m_historyBack->setShortcuts(cmds.BackShortcuts());
	if (showMenuIcons()) 
	{
		m_historyBack->setIconVisibleInMenu(true);
		m_historyBack->setIcon(QIcon(QLatin1String(":arrow_left_green.png"))); 
	}
    m_historyBackMenu = new QMenu(this);
    m_historyBack->setMenu(m_historyBackMenu);
    connect(m_historyBackMenu, SIGNAL(aboutToShow()),this, SLOT(slotAboutToShowBackMenu()));
    connect(m_historyBackMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotOpenActionUrl(QAction *)));
	this->addAction( m_historyBack );

	// Go Forward
    m_historyForward = new QAction(cmds.ForwTitle(), this);
    m_tabWidget->addWebAction(m_historyForward, QWebPage::Forward);
    m_historyForward->setShortcuts(cmds.ForwShortcuts());
	if (showMenuIcons()) 
	{
		m_historyForward->setIconVisibleInMenu(true);
		m_historyForward->setIcon(QIcon(QLatin1String(":arrow_right_green.png")));
	}
    m_historyForwardMenu = new QMenu(this);
    m_historyForward->setMenu(m_historyForwardMenu);
    connect(m_historyForwardMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowForwardMenu()));
    connect(m_historyForwardMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotOpenActionUrl(QAction *)));
	this->addAction( m_historyForward );

	// Go Home
	QAction *m_historyHome = new QAction(cmds.HomeTitle(), this);
	if (showMenuIcons()) 
		m_historyHome->setIcon(QIcon(QLatin1String(":house.png")));
    connect(m_historyHome, SIGNAL(triggered()), this, SLOT(slotHome()));
    m_historyHome->setShortcuts(cmds.HomeShortcuts());
	this->addAction( m_historyHome );

	// Restore Last Closed Tab
	m_restoreLastTab = new QAction(cmds.LastTabTitle(), this);
    connect(m_restoreLastTab, SIGNAL(triggered()), m_tabWidget, SLOT(openLastTab()));
    m_restoreLastTab->setShortcuts(cmds.LastTabShortcuts());
	this->addAction( m_restoreLastTab );

	// Restore Last Session
    m_restoreLastSession = new QAction(cmds.SessionTitle(), this);
    connect(m_restoreLastSession, SIGNAL(triggered()), BrowserApplication::instance(), SLOT(restoreLastSession()));
    m_restoreLastSession->setEnabled(BrowserApplication::instance()->canRestoreSession());
    m_restoreLastSession->setShortcuts(cmds.SessionShortcuts());
	this->addAction( m_restoreLastSession );

    historyActions.append(m_historyBack);
    historyActions.append(m_historyForward);
    historyActions.append(m_historyHome);
    historyActions.append(m_tabWidget->recentlyClosedTabsAction());
    historyActions.append(m_restoreLastTab);
    historyActions.append(m_restoreLastSession);
    historyMenu->setInitialActions(historyActions);

    // Bookmarks
    BookmarksMenu *bookmarksMenu = new BookmarksMenu(this);
    connect(bookmarksMenu, SIGNAL(openUrl(const QUrl&)),
            m_tabWidget, SLOT(loadUrlInCurrentTab(const QUrl&)));
    connect(bookmarksMenu, SIGNAL(hovered(const QString&)),
            this, SLOT(slotUpdateStatusbar(const QString&)));
    bookmarksMenu->setTitle(cmds.BookmarksTitle());
    menuBar()->addMenu(bookmarksMenu);

    QList<QAction*> bookmarksActions;

	// Show All Bookmarks
    QAction *showAllBookmarksAction = new QAction(cmds.AllBooksTitle(), this);
    connect(showAllBookmarksAction, SIGNAL(triggered()), this, SLOT(slotShowBookmarksDialog()));
	if (showMenuIcons()) 
		showAllBookmarksAction->setIcon( QIcon(QLatin1String(":bookmarks.png")) );
	showAllBookmarksAction->setShortcuts( cmds.AllBooksShortcuts() );
	this->addAction( showAllBookmarksAction );

	// Add Bookmark
    m_addBookmark = new QAction(cmds.AddBookTitle(), this);
	if (showMenuIcons()) 
		m_addBookmark->setIcon(	QIcon(QLatin1String(":add2.png")) );
    connect(m_addBookmark, SIGNAL(triggered()), this, SLOT(slotAddBookmark()));
    m_addBookmark->setShortcuts(cmds.AddBookShortcuts());
	this->addAction(m_addBookmark);

    bookmarksActions.append(showAllBookmarksAction);
    bookmarksActions.append(m_addBookmark);
    bookmarksMenu->setInitialActions(bookmarksActions);

    // Privacy Menu
	QMenu *privacyMenu = menuBar()->addMenu(cmds.PrivacyTitle());
    connect(privacyMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowPrivacyMenu()));

	// Private Browsing ...
    m_privateBrowsingMenu = new QAction( cmds.PrivateTitle(), this);
	if (showMenuIcons())
		m_privateBrowsingMenu->setIcon(QIcon(QLatin1String(":spy.png")));
	m_privateBrowsingMenu->setShortcuts(cmds.PrivateShortcuts());
    m_privateBrowsingMenu->setCheckable(true);
    connect(m_privateBrowsingMenu, SIGNAL(triggered()), this, SLOT(slotPrivateBrowsing()));
	privacyMenu->addAction( m_privateBrowsingMenu );
	this->addAction( m_privateBrowsingMenu );
	QSettings local_settings;
	local_settings.beginGroup(QLatin1String("privacy"));
	m_privateBrowsingMenu->setChecked(local_settings.value(QLatin1String("private_browsing"), false).toBool());

	privacyMenu->addSeparator();

	// Disable JavaScript
    m_disableJavaScript = new QAction( cmds.JavaScriptTitle(), this);
	m_disableJavaScript->setShortcuts(cmds.JavaScriptShortcuts());
    connect(m_disableJavaScript, SIGNAL(triggered()), this, SLOT(slotDisableJavaScript()));
	privacyMenu->addAction(m_disableJavaScript);
	this->addAction(m_disableJavaScript);
	m_disableJavaScript->setCheckable(true);

	// Disable Images
    m_disableImages = new QAction( cmds.ImagesTitle(), this);
	m_disableImages->setShortcuts(cmds.ImagesShortcuts());
    connect(m_disableImages, SIGNAL(triggered()), this, SLOT(slotDisableImages()));
	privacyMenu->addAction(m_disableImages);
	this->addAction(m_disableImages);
	m_disableImages->setCheckable(true);

	// Disable Cookies
    m_disableCookies = new QAction( cmds.CookiesTitle(), this);
	m_disableCookies->setShortcuts(cmds.CookiesShortcuts());
    connect(m_disableCookies, SIGNAL(triggered()), this, SLOT(slotDisableCookies()));
	privacyMenu->addAction(m_disableCookies);
	this->addAction(m_disableCookies);
	m_disableCookies->setCheckable(true);

	// Disable Plug-Ins
    m_disablePlugIns = new QAction( cmds.PlugInsTitle(), this);
	m_disablePlugIns->setShortcuts(cmds.PlugInsShortcuts());
    connect(m_disablePlugIns, SIGNAL(triggered()), this, SLOT(slotDisablePlugIns()));
	privacyMenu->addAction(m_disablePlugIns);
	this->addAction(m_disablePlugIns);
	m_disablePlugIns->setCheckable(true);

	// Disable UserAgent
    m_disableUserAgent = new QAction( cmds.AgentTitle(), this);
	m_disableUserAgent->setShortcuts(cmds.AgentShortcuts());
    connect(m_disableUserAgent, SIGNAL(triggered()), this, SLOT(slotDisableUserAgent()));
	privacyMenu->addAction(m_disableUserAgent);
	this->addAction(m_disableUserAgent);
	m_disableUserAgent->setCheckable(true);

	// Disable PopUps
    m_disablePopUps = new QAction( cmds.PopUpsTitle(), this);
	m_disablePopUps->setShortcuts(cmds.PopUpsShortcuts());
    connect(m_disablePopUps, SIGNAL(triggered()), this, SLOT(slotDisablePopUps()));
	privacyMenu->addAction(m_disablePopUps);
	this->addAction(m_disablePopUps);
	m_disablePopUps->setCheckable(true);

	// Enable Proxy
    m_enableProxy = new QAction( cmds.ProxyTitle(), this);
	m_enableProxy->setShortcuts(cmds.ProxyShortcuts());
    connect(m_enableProxy, SIGNAL(triggered()), this, SLOT(slotEnableProxy()));
	privacyMenu->addAction(m_enableProxy);
	this->addAction(m_enableProxy);
	m_enableProxy->setCheckable(true);


	privacyMenu->addSeparator();

	// Empty Cache ...
    m_emptyCache = new QAction( cmds.EmptyTitle(), this);
	m_emptyCache->setShortcuts(cmds.EmptyShortcuts());
    connect(m_emptyCache, SIGNAL(triggered()), this, SLOT(slotEmptyCache()));
	privacyMenu->addAction(m_emptyCache);
	this->addAction(m_emptyCache);
	m_emptyCache->setEnabled( BrowserApplication::networkAccessManager() && BrowserApplication::networkAccessManager()->cache() );

	// Reset QtWeb...
	QAction* reset = new QAction(cmds.ResetTitle(), this);
	reset->setShortcuts(cmds.ResetShortcuts());
	if (showMenuIcons())
		reset->setIcon(QIcon(QLatin1String(":delete.png")));
    connect(reset, SIGNAL(triggered()), this, SLOT(slotResetQtWeb()));
	privacyMenu->addAction(reset);
	this->addAction(reset);

	// Full Reset on Quit
	m_fullCleanUpOnQuitMenu = new QAction(cmds.FullResetTitle(), this);
	m_fullCleanUpOnQuitMenu->setShortcuts(cmds.FullResetShortcuts());
	if (showMenuIcons())
		m_fullCleanUpOnQuitMenu->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(m_fullCleanUpOnQuitMenu, SIGNAL(triggered()), this, SLOT(slotFullCleanUpOnQuit()));
	privacyMenu->addAction(m_fullCleanUpOnQuitMenu);
	this->addAction(m_fullCleanUpOnQuitMenu);
    m_fullCleanUpOnQuitMenu->setCheckable(true);

    // Tools Menu
	QMenu *toolsMenu = menuBar()->addMenu(cmds.ToolsTitle());
    connect(toolsMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowToolsMenu()));

	m_compMenu = toolsMenu->addMenu(cmds.CompatTitle());
    connect(m_compMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowCompatibility()));

	toolsMenu->addSeparator();

	// Web Search
	QAction* search = new QAction(cmds.SearchTitle(), this);
	search->setShortcuts(cmds.SearchShortcuts());
	if (showMenuIcons())
		search->setIcon(QIcon(QLatin1String(":view.png")));
    connect(search, SIGNAL(triggered()), this, SLOT(slotWebSearch()));
	toolsMenu->addAction(search);
	this->addAction(search);

#ifdef Q_WS_WIN
	// Enable Virtual Keyboard
	QAction* keyboard = new QAction(cmds.KeyboardTitle(), this);
	if (showMenuIcons())
		keyboard->setIcon(QIcon(QLatin1String(":keyboard.png")));
	keyboard->setShortcuts(cmds.KeyboardShortcuts());
    connect(keyboard, SIGNAL(triggered()), this, SLOT(slotVirtualKeyboard()));
	toolsMenu->addAction(keyboard);
	this->addAction(keyboard);
#endif

	// Enable Web Inspector
	m_enableInspector = new QAction(cmds.InspectorTitle(), this);
	m_enableInspector->setShortcuts(cmds.InspectorShortcuts());
    connect(m_enableInspector, SIGNAL(triggered(bool)), this, SLOT(slotToggleInspector(bool)));
    m_enableInspector->setCheckable(true);
	toolsMenu->addAction(m_enableInspector);
	this->addAction(m_enableInspector);

	// Inspect
	m_inspectElement = new QAction(cmds.InspectTitle(), this);
	if (showMenuIcons())
		m_inspectElement->setIcon(QIcon(QLatin1String(":inspect.png")));
	m_inspectElement->setShortcuts(cmds.InspectShortcuts());
    connect(m_inspectElement, SIGNAL(triggered()), this, SLOT(slotInspectElement()));
	toolsMenu->addAction(m_inspectElement);
	this->addAction(m_inspectElement);

    toolsMenu->addSeparator();

	// Options
	QAction* opts = new QAction(cmds.OptionsTitle(), this);
	opts->setShortcuts(cmds.OptionsShortcuts());
	connect(opts, SIGNAL(triggered()), this, SLOT(slotPreferences()));
    toolsMenu->addAction( opts );
	this->addAction( opts );

    // Window Menu
    m_windowMenu = menuBar()->addMenu(cmds.WindowTitle());
    connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShowWindowMenu()));
    slotAboutToShowWindowMenu();

	// Help Menu
    QMenu *helpMenu = menuBar()->addMenu(cmds.HelpTitle());

#ifdef Q_WS_WIN
	// Help F1
	QAction* help = new QAction(cmds.HelpTitle(), this);
	if (showMenuIcons())
	    help->setIcon(QIcon(QLatin1String(":help.png")));
	help->setShortcuts(cmds.HelpShortcuts());
    connect(help, SIGNAL(triggered()), this, SLOT(slotHelp()));
	helpMenu->addAction(help);
	this->addAction(help);
#endif

	// Online Help
	QAction* online = new QAction(cmds.OnlineTitle(), this);
	online->setShortcuts(cmds.OnlineShortcuts());
    connect(online, SIGNAL(triggered()), this, SLOT(slotHelpOnline()));
	helpMenu->addAction(online);
	this->addAction(online);

	// Check for Updates
	QAction* updates = new QAction(cmds.UpdatesTitle(), this);
	updates->setShortcuts(cmds.UpdatesShortcuts());
    connect(updates, SIGNAL(triggered()), this, SLOT(slotCheckUpdates()));
	helpMenu->addAction(updates);
	this->addAction(updates);

	// About QtWeb
	QAction* about = new QAction(cmds.AboutTitle(), this);
	if (showMenuIcons())
	    about->setIcon(QIcon(QLatin1String(":QtWeb.png")));
	about->setShortcuts(cmds.AboutShortcuts());
    connect(about, SIGNAL(triggered()), this, SLOT(slotAboutApplication()));
	helpMenu->addAction(about);
	this->addAction(about);
}

void BrowserMainWindow::slotVirtualKeyboard()
{
	ShellExec(QLatin1String("osk.exe"));
}

void BrowserMainWindow::slotResetQtWeb()
{
	ResetSettings *dialog = new ResetSettings(m_toolbarSearch, this);
	dialog->exec();
}

void BrowserMainWindow::slotStyleChange()
{
	QAction *action = qobject_cast<QAction *>(sender());
	QApplication::setStyle(QStyleFactory::create(action->text()));

	QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	settings.setValue(QLatin1String("style"), action->text());
}

QAction* BrowserMainWindow::menuAddEncoding(QString name, QString encoding)
{
	QAction* a = m_encodingMenu->addAction(name + " (" + encoding + ")", this, SLOT(slotEncodingChange( )));
	a->setCheckable(true);
	a->setStatusTip( encoding );
	if (encoding == m_currentEncoding)
		a->setChecked(true);

	return a;
}

void BrowserMainWindow::slotCheckUpdates()
{
    if (currentTab())
	{
		currentTab()->loadUrl(QUrl(QLatin1String("http://www.qtweb.net/update.html")));
	}

    QMessageBox::information(this, QString("QtWeb"),
                             tr("Current QtWeb version is <b>%1</b>  (build %2)<br><br>Please check the web site whether a new version is available.")
                             .arg(QCoreApplication::applicationVersion()).arg(BrowserApplication::getApplicationBuild(),3,10,QChar('0'))
							 );
}

void BrowserMainWindow::slotAboutToShowEncodingMenu()
{
	if (!m_encodingMenu)
		return;

	m_encodingMenu->clear();

	QActionGroup* alignmentGroup = new QActionGroup(this);
	alignmentGroup->setExclusive(true);

	QAction* a = m_encodingMenu->addAction("Auto Detect", this, SLOT(slotEncodingChange( )));
	alignmentGroup->addAction(a);
	a->setCheckable(true);
	if (m_currentEncoding == "")
		a->setChecked(true);

	alignmentGroup->addAction(menuAddEncoding("Unicode",	"UTF-8"));
	alignmentGroup->addAction(menuAddEncoding("Western",	"ISO-8859-1"));
	alignmentGroup->addAction(menuAddEncoding("Western",	"windows-1252"));
	alignmentGroup->addAction(menuAddEncoding("Cyrillic",	"windows-1251"));
	alignmentGroup->addAction(menuAddEncoding("Cyrillic",	"KOI8-R"));
	m_encodingMenu->addSeparator();

	alignmentGroup->addAction(menuAddEncoding("Unicode",	"UTF-16LE"));
	m_encodingMenu->addSeparator();

	alignmentGroup->addAction(menuAddEncoding("Western",	"ISO-8859-15"));
	alignmentGroup->addAction(menuAddEncoding("Western",	"macintosh"));
	alignmentGroup->addAction(menuAddEncoding("Central European", "ISO-8859-2"));
	alignmentGroup->addAction(menuAddEncoding("Central European", "windows-1250"));
	m_encodingMenu->addSeparator();

	// chinese
	alignmentGroup->addAction(menuAddEncoding("Chinese Simplified",		"GBK"));
	alignmentGroup->addAction(menuAddEncoding("Chinese Simplified",		"GB2312"));
	alignmentGroup->addAction(menuAddEncoding("Chinese Traditional",	"Big5"));
	// korean
	alignmentGroup->addAction(menuAddEncoding("Korean",	"EUC-KR"));
	// japanese
	alignmentGroup->addAction(menuAddEncoding("Japanese",	"ISO-2022-JP"));
	alignmentGroup->addAction(menuAddEncoding("Japanese",	"EUC-JP"));
	m_encodingMenu->addSeparator();

	alignmentGroup->addAction(menuAddEncoding("Cyrillic",	"ISO-8859-5"));
	alignmentGroup->addAction(menuAddEncoding("Cyrillic",	"KOI8-U"));
	m_encodingMenu->addSeparator();

	alignmentGroup->addAction(menuAddEncoding("Greek",		"ISO-8859-7"));
	alignmentGroup->addAction(menuAddEncoding("Greek",		"windows-1253"));
	alignmentGroup->addAction(menuAddEncoding("Turkish",	"ISO-8859-9"));
	alignmentGroup->addAction(menuAddEncoding("Turkish",	"windows-1254"));
	alignmentGroup->addAction(menuAddEncoding("Arabic",		"ISO-8859-6"));
	alignmentGroup->addAction(menuAddEncoding("Arabic",		"windows-1256"));
	alignmentGroup->addAction(menuAddEncoding("Hebrew",		"ISO-8859-8"));
	alignmentGroup->addAction(menuAddEncoding("Hebrew",		"windows-1255"));
	alignmentGroup->addAction(menuAddEncoding("Vietnamese",	"windows-1258"));
	alignmentGroup->addAction(menuAddEncoding("Baltic",		"ISO-8859-4"));
	alignmentGroup->addAction(menuAddEncoding("Baltic",		"ISO-8859-13"));
	alignmentGroup->addAction(menuAddEncoding("Baltic",		"windows-1257"));
	alignmentGroup->addAction(menuAddEncoding("South European", "ISO-8859-3"));
	alignmentGroup->addAction(menuAddEncoding("Nordic",		"ISO-8859-10"));
	alignmentGroup->addAction(menuAddEncoding("Celtic",		"ISO-8859-14"));
	alignmentGroup->addAction(menuAddEncoding("Romanian",	"ISO-8859-16"));
	//alignmentGroup->addAction(menuAddEncoding("",		""));
}

void BrowserMainWindow::slotAboutToShowStyles()
{
	m_viewZoomTextOnly->setChecked( QWebSettings::globalSettings()->testAttribute(QWebSettings::ZoomTextOnly) );

	if (!m_stylesMenu)
		return;

	m_stylesMenu->clear();

	QStringList keys = QStyleFactory::keys();
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	QString st = settings.value(QLatin1String("style"), DefaultAppStyle()).toString();
	QActionGroup* alignmentGroup = new QActionGroup(this);
	alignmentGroup->setExclusive(true);
	foreach(QString style, keys)
	{
		QAction* a = m_stylesMenu->addAction(style, this, SLOT(slotStyleChange( )));
		alignmentGroup->addAction(a);
		a->setCheckable(true);
		a->setChecked( st == style);
	}
}

void BrowserMainWindow::setupToolBar()
{
	MenuCommands cmds;

    setUnifiedTitleAndToolBarOnMac(true);
    m_navigationBar = addToolBar(tr("Navigation Bar"));
	m_navigationBar->setObjectName("Navigation Bar");
	
    connect(m_navigationBar->toggleViewAction(), SIGNAL(toggled(bool)),
            this, SLOT(updateToolbarActionText(bool)));

    m_goBackAction = new QAction(this);
	m_goBackAction->setIcon(QIcon(QLatin1String(":arrow_left_green.png"))); 
    m_goBackAction->setMenu(m_historyBackMenu);
    m_tabWidget->addWebAction(m_goBackAction, QWebPage::Back);
    m_navigationBar->addAction(m_goBackAction);

    m_goForwardAction = new QAction(this);
    m_goForwardAction->setIcon(QIcon(QLatin1String(":arrow_right_green.png")));
    m_goForwardAction->setMenu(m_historyForwardMenu);
	m_tabWidget->addWebAction(m_goForwardAction, QWebPage::Forward);
    m_navigationBar->addAction(m_goForwardAction);

	m_homeAction = new QAction(QIcon(QLatin1String(":house.png")), cmds.HomeTitle(), this);
    connect(m_homeAction, SIGNAL(triggered()), this, SLOT(slotHome()));
	m_homeAction->setToolTip(tr("Go to your Home page"));
	m_homeAction->setStatusTip(m_homeAction->toolTip());
    m_navigationBar->addAction(m_homeAction);

	m_bookmarksAction = new QAction(QIcon(QLatin1String(":bookmarks.png")), cmds.BookmarksTitle(), this);
    connect(m_bookmarksAction, SIGNAL(triggered()), this, SLOT(slotShowBookmarksDialog()));
	m_bookmarksAction->setToolTip(tr("Show all Bookmarks"));
	m_bookmarksAction->setStatusTip(  m_bookmarksAction->toolTip() );
    m_navigationBar->addAction(m_bookmarksAction); 

    m_addBookmarkAction = new QAction(QIcon(QLatin1String(":add2.png")), cmds.AddBookTitle(), this);
    connect(m_addBookmarkAction, SIGNAL(triggered()), this, SLOT(slotAddBookmark()));
	m_addBookmarkAction->setToolTip(tr("Bookmark this page"));
	m_addBookmarkAction->setStatusTip(m_addBookmarkAction->toolTip());
    m_navigationBar->addAction(m_addBookmarkAction);

    m_stopReload = new QAction(this);
	m_reloadIcon = style()->standardIcon(QStyle::SP_BrowserReload);
    m_stopReload->setIcon(m_reloadIcon);
    //m_navigationBar->addAction(m_stopReload);
	m_loadIcon = QIcon(QLatin1String(":loadpage.png"));

	m_keyboardAction = new QAction(QIcon(QLatin1String(":keyboard.png")), cmds.KeyboardTitle(), this);
    connect(m_keyboardAction, SIGNAL(triggered()), this, SLOT(slotVirtualKeyboard()));
	m_keyboardAction->setToolTip(tr("Open on-screen Virtual Keyboard"));
	m_keyboardAction->setStatusTip(m_keyboardAction->toolTip());
    //m_navigationBar->addAction(m_keyboardAction); 

	m_textSizeAction = new QAction(this);
	m_textSizeAction->setText( tr("Change Text Size") );
    m_textSizeAction->setIcon(QIcon(QLatin1String(":textsize.png"))); 
    connect(m_textSizeAction, SIGNAL(triggered()), this, SLOT(slotChangeTextSize()));
	m_textSizeAction->setToolTip(tr("Change web page text size"));
	m_textSizeAction->setStatusTip(m_textSizeAction->toolTip());

	m_sizesMenu = new QMenu(tr("Text Size"), this);

    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	int lastSizing = settings.value(QLatin1String("lastSizing"), 0).toInt();

	m_textSizeLarger = new QAction(cmds.LargerTitle(), this);
	if (showMenuIcons()) m_textSizeLarger->setIcon(QIcon(QLatin1String(":zoom-in-24.png")));
    connect(m_textSizeLarger, SIGNAL(triggered( )), this, SLOT(slotViewTextBigger()));
	m_sizesMenu->addAction( m_textSizeLarger );
	m_textSizeLarger->setCheckable(true);
	m_textSizeLarger->setChecked( lastSizing == 1);

	// Make Text Normal
	m_textSizeNormal = new QAction(cmds.NormalTitle(), this);
    connect(m_textSizeNormal, SIGNAL(triggered()), this, SLOT(slotViewTextNormal()));
	m_sizesMenu->addAction( m_textSizeNormal );
	m_textSizeNormal->setCheckable(true);
	m_textSizeNormal->setChecked( lastSizing == 0);

	// Make Text Smaller
	m_textSizeSmaller = new QAction(cmds.SmallerTitle(), this);
	if (showMenuIcons())  m_textSizeSmaller->setIcon(QIcon(QLatin1String(":zoom-out-24.png")));
    connect(m_textSizeSmaller, SIGNAL(triggered()), this, SLOT(slotViewTextSmaller()));
	m_sizesMenu->addAction( m_textSizeSmaller );
	m_textSizeSmaller->setCheckable(true);
	m_textSizeSmaller->setChecked( lastSizing == -1);

    m_textSizeAction->setMenu(m_sizesMenu);
    //m_navigationBar->addAction(m_textSizeAction); 

	// Compatibility 
	m_compatAction = new QAction(this);
	m_compatAction->setText( cmds.CompatTitle() );
	setCurrentAgentIcon();
    connect(m_compatAction, SIGNAL(triggered()), this, SLOT(slotNextUserAgent()));
	m_compatAction->setToolTip(tr("Compatibility mode with other browsers (UserAgent)"));
	m_compatAction->setStatusTip(m_compatAction->toolTip());

    m_compatAction->setMenu(m_compMenu);
    //m_navigationBar->addAction(m_compatAction); 

	m_styles = new QAction(this);
	m_styles->setText(tr("Change Application Style"));
	m_styles->setStatusTip(m_styles->text());
    m_styles->setIcon(QIcon(QLatin1String(":app_style.png"))); 
    m_styles->setMenu(m_stylesMenu);
    connect(m_styles, SIGNAL(triggered()),
            this, SLOT(slotNextStyle()));
    connect(m_stylesMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowStyles()));
    //connect(m_stylesMenu, SIGNAL(triggered()),
    //        this, SLOT(slotStyleChange()));
    //m_navigationBar->addAction(m_styles);

	m_privateBrowsing = new QAction(this);
	m_privateBrowsing->setText(cmds.PrivateTitle());
	m_privateBrowsing->setIcon(QIcon(QLatin1String(":spy.png")));
	//m_navigationBar->addAction(m_privateBrowsing); // !!!
	connect(m_privateBrowsing, SIGNAL(triggered()), this, SLOT(slotPrivateBrowsing()));
	m_privateBrowsing->setCheckable(true);

	QString tip(tr("Turn on Private Browsing mode"));
	m_privateBrowsing->setStatusTip( tip);
	m_privateBrowsing->setToolTip(tip);
	if (m_privateBrowsingMenu)
		m_privateBrowsing->setChecked(m_privateBrowsingMenu->isChecked());

	m_prefsAction = new QAction(QIcon(QLatin1String(":settings.png")), cmds.PrefsTitle(), this);
    connect(m_prefsAction, SIGNAL(triggered()), this, SLOT(slotPreferences()));
	m_prefsAction->setToolTip(tr("View and edit your preferences and settings"));
	m_prefsAction->setStatusTip(m_prefsAction->toolTip());
    //m_navigationBar->addAction(m_prefsAction); // !!!

	m_inspectAction = new QAction(QIcon(QLatin1String(":inspect.png")), cmds.InspectTitle(), this);
    connect(m_inspectAction, SIGNAL(triggered()), this, SLOT(slotInspectElement()));
	m_inspectAction->setToolTip(tr("Inspect web element, debug JavaScript, profile..."));
	m_inspectAction->setStatusTip(m_inspectAction->toolTip() );
    //m_navigationBar->addAction(m_inspectAction); 

	m_proxyAction = new QAction(QIcon(QLatin1String(":proxy.png")), cmds.ProxyTitle(), this);
	m_proxyAction->setToolTip(tr("Enable web access via Proxy"));
	m_proxyAction->setStatusTip(m_proxyAction->toolTip());
	m_proxyAction->setCheckable(true);
	connect(m_proxyAction, SIGNAL(triggered()), this, SLOT(slotToggleProxy()));
    //m_navigationBar->addAction(m_proxyAction); 

	m_imagesAction = new QAction(QIcon(QLatin1String(":images.png")), cmds.ImagesTitle(), this);
	m_imagesAction->setToolTip(tr("Hide images on a web page"));
	m_imagesAction->setStatusTip(m_imagesAction->toolTip());
	m_imagesAction->setCheckable(true);
	connect(m_imagesAction, SIGNAL(triggered()), this, SLOT(slotToggleImages()));
    //m_navigationBar->addAction(m_imagesAction); 

	m_resetAction = new QAction(QIcon(QLatin1String(":delete.png")), cmds.ResetTitle(), this);
    connect(m_resetAction, SIGNAL(triggered()), this, SLOT(slotResetQtWeb()));
	m_resetAction->setToolTip(tr("Reset QtWeb: clean up caches, cookies, history..."));
	m_resetAction->setStatusTip(m_resetAction->toolTip());
    //m_navigationBar->addAction(m_resetAction); 

    m_toolbarSearch = new ToolbarSearch(m_navigationBar);
    connect(m_toolbarSearch, SIGNAL(search(const QUrl&)), SLOT(loadUrl(const QUrl&)));

   
	m_buttonsBar = new QToolBar(this);
    m_buttonsBar->addAction(m_stopReload);
    m_buttonsBar->addAction(m_keyboardAction); 
    m_buttonsBar->addAction(m_textSizeAction); 
    m_buttonsBar->addAction(m_compatAction); 
    m_buttonsBar->addAction(m_styles);
	m_buttonsBar->addAction(m_privateBrowsing);
	m_buttonsBar->addAction(m_prefsAction);
    m_buttonsBar->addAction(m_inspectAction); 
    m_buttonsBar->addAction(m_proxyAction); 
    m_buttonsBar->addAction(m_imagesAction); 
    m_buttonsBar->addAction(m_resetAction); 
	m_buttonsBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

	//////////////////////////////////////
    m_navSplit = new QSplitter(m_navigationBar);

	QWidget* w1 = new QWidget(m_navigationBar);
	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins ( 0,0,0,0 );
	layout->setSpacing ( 0 );

	layout->addWidget(m_tabWidget->lineEditStack());

	layout->addWidget(m_buttonsBar);

	w1->setLayout(layout);

    m_navSplit->addWidget(w1);


	QWidget* w2 = new QWidget(m_navigationBar);
	layout = new QHBoxLayout;
	layout->setContentsMargins ( 0,0,0,0 );
	layout->addWidget(m_toolbarSearch);
	m_toolbarSearch->setMinimumWidth(80);
	w2->setLayout(layout);

	m_navSplit->addWidget(w2);

	m_navSplit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_tabWidget->lineEditStack()->setMinimumWidth(120);
    m_navSplit->setCollapsible(0, false);
    //m_navSplit->setCollapsible(1, false);
    m_navigationBar->addWidget(m_navSplit);
    int splitterWidth = m_navSplit->width();
    QList<int> sizes;
    sizes << (int)((double)splitterWidth * 0.6) << (int)((double)splitterWidth * 0.18) << (int)((double)splitterWidth * 0.22);
    m_navSplit->setSizes(sizes);
	///////////////////////////////////////////////////

    m_chaseWidget = new ChaseWidget(this);
    m_navigationBar->addWidget(m_chaseWidget);

	checkToolBarButtons();
}

void BrowserMainWindow::checkToolBarButtons()
{
	QSettings settings;
    settings.beginGroup(QLatin1String("AddressBar"));
	bool bShowBack = settings.value(QLatin1String("showGoBack"), true).toBool();
	bool bShowForward = settings.value(QLatin1String("showGoForward"), true).toBool() ;
	bool bShowAddBook = settings.value(QLatin1String("showAddBookmark"), true).toBool() ;
	bool bShowHome = settings.value(QLatin1String("showGoHome"), true).toBool();
	bool bShowRefresh = settings.value(QLatin1String("showRefresh"), true).toBool();
	bool bShowAppStyle = settings.value(QLatin1String("showAppStyle"), true).toBool();
	bool bShowPrivMode = settings.value(QLatin1String("showPrivacyMode"), true).toBool();
	bool bShowPrefs = settings.value(QLatin1String("showPreferences"), true).toBool();
	bool bShowImages = settings.value(QLatin1String("showImages"), false).toBool() ;
	bool bShowProxy = settings.value(QLatin1String("showProxy"), false).toBool() ;
	bool bShowCompatibility = settings.value(QLatin1String("showCompatibility"), true).toBool();
	bool bShowReset = settings.value(QLatin1String("showReset"), false).toBool();
	bool bShowInspect = settings.value(QLatin1String("showInspect"), false).toBool();
	bool bShowTextSize = settings.value(QLatin1String("showTextSize"), false).toBool();
	bool bShowKeyboard = settings.value(QLatin1String("showKeyboard"), false).toBool();
	bool bShowBookmarks = settings.value(QLatin1String("showBookmarks"), false).toBool();

	settings.endGroup();
    settings.beginGroup(QLatin1String("websettings"));
	bool autoload_images = settings.value(QLatin1String("autoLoadImages"), true).toBool();
	m_imagesAction->setChecked(!autoload_images);
    settings.endGroup();

	settings.beginGroup(QLatin1String("proxy"));
	bool use_proxy = settings.value(QLatin1String("enabled"), false).toBool();
	m_proxyAction->setChecked(use_proxy);
    settings.endGroup();

	m_goBackAction->setVisible(bShowBack);
	m_goForwardAction->setVisible(bShowForward);
	m_addBookmarkAction->setVisible(bShowAddBook);
	m_homeAction->setVisible(bShowHome);
	m_prefsAction->setVisible(bShowPrefs);
	m_stopReload->setVisible(bShowRefresh);
	m_styles->setVisible(bShowAppStyle);
	m_privateBrowsing->setVisible(bShowPrivMode);
	m_resetAction->setVisible(bShowReset);
	m_compatAction->setVisible(bShowCompatibility);
	m_imagesAction->setVisible(bShowImages);
	m_proxyAction->setVisible(bShowProxy);

	m_inspectAction->setVisible(bShowInspect); 
	m_inspectAction->setEnabled(  QWebSettings::globalSettings()->testAttribute(QWebSettings::DeveloperExtrasEnabled) ); 
	m_keyboardAction->setVisible(bShowKeyboard); 
	m_textSizeAction->setVisible(bShowTextSize); 
	m_bookmarksAction->setVisible(bShowBookmarks); 
}

void BrowserMainWindow::setLoadIcon()
{
    disconnect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
    disconnect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
	connect(m_stopReload, SIGNAL(triggered()), this, SLOT(slotLoadPage()));

	m_stopReload->setIcon(m_loadIcon);
    m_stopReload->setToolTip(tr("Load the web page typed in the Address bar"));

}

void BrowserMainWindow::slotLoadPage()
{
	if (tabWidget() && tabWidget()->currentLineEdit())
	{
		QString url = tabWidget()->currentLineEdit()->text();
		QString cur = tabWidget()->currentWebView()->url().toString();
		if (url != cur )
		{
			loadPage(url);
		}
	}
}

void BrowserMainWindow::slotShowBookmarksDialog()
{
    BookmarksDialog *dialog = new BookmarksDialog(this);
    connect(dialog, SIGNAL(openUrl(const QUrl&)),
            m_tabWidget, SLOT(loadUrlInCurrentTab(const QUrl&)));
    dialog->show();
}

void BrowserMainWindow::slotAddBookmark()
{
    WebView *webView = currentTab();
    QString url = webView->url().toString();
    QString title = webView->title();

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bUseDefaultLocation = settings.value(QLatin1String("addBookmarkUseDefaultLocation"), false).toBool();
	QString defaultLocation = settings.value(QLatin1String("addBookmarkDefaultLocation"), "").toString();
	AddBookmarkDialog dialog(url, title, (bUseDefaultLocation ? defaultLocation : ""));
	if (bUseDefaultLocation)
	{
		dialog.acceptDefaultLocation();
	}
	else
	{
		dialog.exec();
	}
}

void BrowserMainWindow::slotViewMenubar()
{
	QSettings settings;
    if (menuBar()->isVisible()) 
	{
		settings.beginGroup(QLatin1String("websettings"));
		bool confirm = settings.value(QLatin1String("hide_menu_confirmation"), true).toBool();

		if (confirm)
		{
			QString text = tr("<b>Are you sure you want to hide the Menu Bar?</b><br><br>"
				"It is not recommended for the beginners, who not fluent with browser's shortcuts.<br><br>"
				"To show the Menu Bar - use <b>Ctrl+Shift+M</b> shortcut, or go to Preferences.");
			QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Warning"), text,
					QMessageBox::Yes | QMessageBox::No,  QMessageBox::No);
			if (button == QMessageBox::No) 
				return;
		}
		settings.setValue(QLatin1String("hide_menu_confirmation"), false);
		settings.endGroup();
	
		settings.beginGroup(QLatin1String("general"));
		settings.setValue(QLatin1String("ShowMenu"), false);
		settings.endGroup();

	    updateMenubarActionText(false);
        menuBar()->hide();
    } 
	else 
	{
		settings.beginGroup(QLatin1String("general"));
		settings.setValue(QLatin1String("ShowMenu"), true);
		settings.endGroup();

        updateMenubarActionText(true);
        menuBar()->show();
    }
}

void BrowserMainWindow::slotViewToolbar()
{
    if (m_navigationBar->isVisible()) {
        updateToolbarActionText(false);
        m_navigationBar->close();
    } else {
        updateToolbarActionText(true);
        m_navigationBar->show();
    }
    m_autoSaver->changeOccurred();
}

void BrowserMainWindow::slotViewBookmarksBar()
{
    if (m_bookmarksToolbar->isVisible()) {
        updateBookmarksToolbarActionText(false);
        m_bookmarksToolbar->close();
    } else {
        updateBookmarksToolbarActionText(true);
        m_bookmarksToolbar->show();
    }
    m_autoSaver->changeOccurred();
}

void BrowserMainWindow::updateStatusbarActionText(bool visible)
{
	MenuCommands cmds;
    m_viewStatusBar->setText(!visible ? cmds.StatusShowTitle() : cmds.StatusHideTitle());
}

void BrowserMainWindow::updateMenubarActionText(bool visible)
{
	MenuCommands cmds;
    m_viewMenuBar->setText(!visible ? cmds.MenuShowTitle() : cmds.MenuHideTitle());
}

void BrowserMainWindow::updateToolbarActionText(bool visible)
{
	MenuCommands cmds;
    m_viewToolBar->setText(!visible ? cmds.NavShowTitle() : cmds.NavHideTitle());
}

void BrowserMainWindow::updateBookmarksToolbarActionText(bool visible)
{
	MenuCommands cmds;
    m_viewBookmarkBar->setText(!visible ? cmds.BooksShowTitle() : cmds.BooksHideTitle());
}

void BrowserMainWindow::slotViewStatusbar()
{
    if (statusBar()->isVisible()) {
        updateStatusbarActionText(false);
        statusBar()->close();
    } else {
        updateStatusbarActionText(true);
        statusBar()->show();
    }
    m_autoSaver->changeOccurred();
}

QUrl BrowserMainWindow::guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if (url.isValid())
            return url;
    }

    // Might be a file.
    if (QFile::exists(urlStr)) {
        QFileInfo info(urlStr);
        return QUrl::fromLocalFile(info.absoluteFilePath());
    }

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }
    }

    // Fall back to QUrl's own tolerant parser.
    QUrl url = QUrl(string, QUrl::TolerantMode);

    // finally for cases where the user just types in a hostname add http
    if (url.scheme().isEmpty())
        url = QUrl(QLatin1String("http://") + string, QUrl::TolerantMode);
    return url;
}

void BrowserMainWindow::loadUrl(const QUrl &url)
{
    loadPage(url.toString());
}

void BrowserMainWindow::slotDownloadManager()
{
    BrowserApplication::downloadManager()->show();
}

void BrowserMainWindow::slotTorrents()
{
    BrowserApplication::torrents()->show();
}

void BrowserMainWindow::slotSelectLineEdit()
{
	if (!m_navigationBar->isVisible())
		m_navigationBar->show();
    m_tabWidget->currentLineEdit()->selectAll();
    m_tabWidget->currentLineEdit()->setFocus();
}

void BrowserMainWindow::checkQuitAction()
{
	if (m_dumpActionQuit)
		close();
}

void BrowserMainWindow::checkDumpAction(QWebPage* page)
{
	if (!m_dumpFile.isEmpty() && page)
	{
		QFile f(m_dumpFile);
		try
		{
			if (!f.open(QIODevice::WriteOnly | QIODevice::Text) )
			{
				statusBar()->showMessage(tr("Error opening dump file %1 ...").arg(m_dumpFile));
			}
			else
			{
				QWebFrame *mainframe = page->mainFrame();
				QString html = mainframe->toHtml();
				QTextStream out(&f);
				out << html;
				f.close();
			}
		}
		catch(...)
		{
			statusBar()->showMessage(tr("Unknown error opening dump file %1 ...").arg(m_dumpFile));
		}
		
		checkQuitAction();
	}
}

void BrowserMainWindow::setDumpFile(const QString &dump_file, bool quit)
{
	m_dumpFile = dump_file;
	m_dumpActionQuit = quit;
}

void BrowserMainWindow::slotFileSaveAs()
{
    BrowserApplication::downloadManager()->download(currentTab()->url(), true);
}

void BrowserMainWindow::slotPreferences()
{
    SettingsDialog *s = new SettingsDialog(this);
    s->show();
}

void BrowserMainWindow::slotUpdateStatusbar(const QString &string)
{
    statusBar()->showMessage(string, 2000);
}

void BrowserMainWindow::slotUpdateWindowTitle(const QString &title)
{
	m_title = title;

    if (title.isEmpty()) 
        setWindowTitle(("QtWeb Internet Browser"));
    else 
        setWindowTitle(QString("%1 - QtWeb").arg(title));
}

void BrowserMainWindow::emptyCache()
{
	// REading previous cache state
	int pages = QWebSettings::maximumPagesInCache();

	int cacheMinDeadCapacity = 2 * 1024 * 1024;
	int cacheMaxDead = 6 * 1024 * 1024;
	int totalCapacity = 8 * 1024 * 1024;

	// Disabling cache
	QWebSettings::setMaximumPagesInCache ( 0 )  ;
	QWebSettings::setObjectCacheCapacities( 0, 0, 0);   

	QThread::currentThread()->wait(500);

	// Restoring Cache
	QWebSettings::setObjectCacheCapacities ( cacheMinDeadCapacity, cacheMaxDead, totalCapacity );
	QWebSettings::setMaximumPagesInCache ( pages )  ;

}

void BrowserMainWindow::slotEmptyCache()
{
	QString text = tr("<b>Are you sure you want to empty the cache?</b><br><br>"
		"QtWeb saves the contents of web pages you open in a cache so that it's faster to visit them again.");
	QMessageBox::StandardButton button = QMessageBox::information(this, tr("Information"), text,
		QMessageBox::Ok | QMessageBox::Cancel,
						   QMessageBox::Ok);
	if (button == QMessageBox::Cancel) 
		return;

	emptyCache();
}


void BrowserMainWindow::slotHelp()
{
	ShellExec("QtWeb.chm");
}

void BrowserMainWindow::slotHelpOnline()
{
	loadPage("http://www.qtweb.net/help.php");

}

void BrowserMainWindow::slotAboutApplication()
{
	AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->show();
}

void BrowserMainWindow::slotFileNew()
{
    BrowserApplication::instance()->newMainWindow();
    BrowserMainWindow *mw = BrowserApplication::instance()->mainWindow();
    mw->slotHome();
}

void BrowserMainWindow::slotFileOpen()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open Web Resource"), QString(),
            tr("Web Resources (*.html *.htm *.png *.gif *.jpg *.jpeg *.svg *.svgz );;All files (*.*)"));

    if (file.isEmpty())
        return; 

    loadPage(file);
}

void BrowserMainWindow::slotFilePrint()
{
    if (!currentTab())
        return;
    
	printRequested(currentTab()->page()->mainFrame());
}

#include <QtGui/QPrintPreviewDialog>

void BrowserMainWindow::slotFilePrintPreview()
{
    if (!currentTab())
        return;
    
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter *)), currentTab(), SLOT(print(QPrinter *)));
    if (dialog)
		dialog->exec();
}

void BrowserMainWindow::printRequested(QWebFrame *frame)
{
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return; 
    frame->print(&printer);
}

void BrowserMainWindow::slotToggleProxy()
{
	QSettings settings;
	settings.beginGroup(QLatin1String("proxy"));
	bool use_proxy = settings.value(QLatin1String("enabled"), false).toBool();
	settings.setValue(QLatin1String("enabled"), !use_proxy);
	BrowserApplication::networkAccessManager()->loadSettings();
}

void BrowserMainWindow::slotToggleImages()
{
    QWebSettings *settings = QWebSettings::globalSettings();
	bool load_images = settings->testAttribute(QWebSettings::AutoLoadImages);
    settings->setAttribute(QWebSettings::AutoLoadImages, !load_images);

	QSettings local_settings;
    local_settings.beginGroup(QLatin1String("websettings"));
	bool autoload_images = local_settings.value(QLatin1String("autoLoadImages"), true).toBool();
	local_settings.setValue(QLatin1String("autoLoadImages"), !autoload_images);
}


void BrowserMainWindow::slotPrivateBrowsing()
{
    QWebSettings *settings = QWebSettings::globalSettings();
	QSettings local_settings;
    bool pb = settings->testAttribute(QWebSettings::PrivateBrowsingEnabled);
    if (!pb) 
	{
		if (!m_privateBrowsingMenu->isChecked())
			m_privateBrowsingMenu->setChecked(true);

		if (!m_privateBrowsing->isChecked())
			m_privateBrowsing->setChecked(true);

		local_settings.beginGroup(QLatin1String("websettings"));
		bool confirm = local_settings.value(QLatin1String("privacy_mode_confirmation"), true).toBool();

		if (confirm)
		{
			QString title = tr("You are turning on private browsing");
			QString text = tr("<b>%1</b><br><br>When private browsing in turned on,"
				" webpages are not added to the history,"
				" items are automatically removed from the Downloads window," \
				" new cookies are not stored, current cookies can't be accessed," \
				" site icons wont be stored, user names, passwords and sessions wont be saved, UserAgent wont be sent to the web server " \
				" and searches are not addded to the pop-up menu in the search box." \
				"  Until you close the window, you can still click the Back and Forward buttons" \
				" to return to the webpages you have opened."
				"<br><br>Do you want to see this informational message again?"
				).arg(title);

			QMessageBox::StandardButton button = QMessageBox::information(this, tr("Information"), text,
				QMessageBox::Yes | QMessageBox::No,
								   QMessageBox::Yes);
			if (button == QMessageBox::No) 
			{
			    local_settings.setValue(QLatin1String("privacy_mode_confirmation"), false);
			}
		}
		local_settings.endGroup();
		local_settings.beginGroup(QLatin1String("privacy"));
	    local_settings.setValue(QLatin1String("private_browsing"), true);
		local_settings.endGroup();

        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	    WebPage::setUserAgent( QLatin1String("") );

    } 
	else 
	{
		if (m_privateBrowsingMenu->isChecked())
			m_privateBrowsingMenu->setChecked(false);

		if (m_privateBrowsing->isChecked())
			m_privateBrowsing->setChecked(false);

        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);
		
    	WebPage::setDefaultAgent( );

        QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
        for (int i = 0; i < windows.count(); ++i) {
            BrowserMainWindow *window = windows.at(i);
            window->tabWidget()->clear();
        }

		local_settings.beginGroup(QLatin1String("privacy"));
	    local_settings.setValue(QLatin1String("private_browsing"), false);

    }
}

#include "closeapp.h"
void BrowserMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_tabWidget->count() > 1) 
	{
	    QSettings settings;
		settings.beginGroup(QLatin1String("MainWindow"));
	    bool dontAsk = settings.value(QLatin1String("quitDontAsk"), false).toBool();

		if (!dontAsk)
		{
			CloseApp app;
			int ret = app.exec();
			if (ret == QDialog::Rejected)
			{
				event->ignore();
				return;
			}
		}
    }
	if (currentTab())
	{
		WebView* tab = currentTab();
		qreal ratio = tab->textSizeMultiplier();
	    QSettings settings;
		settings.beginGroup(QLatin1String("websettings"));
		settings.setValue(QLatin1String("ZoomRatio"), ratio);
	}
	if (BrowserApplication::instance()->mainWindows().count() == 1)
	{
		if ( BrowserApplication::existDownloadManager() && BrowserApplication::downloadManager()->activeDownloads()==0 )
			BrowserApplication::downloadManager()->close();

		if ( BrowserApplication::torrents() )
			BrowserApplication::torrents()->close();
	}
    event->accept();
    deleteLater();
}

void BrowserMainWindow::slotEditFind()
{
    if (!currentTab())
        return;

    findWidget->show();
}

void BrowserMainWindow::find(const QString &ttf, bool forward)
{
    bool found = false;
	if (!currentTab())
		return;

    QWebPage::FindFlags options;
    if (!ttf.isEmpty()) {
        if (!forward)
            options |= QWebPage::FindBackward;

        if (findWidget->caseSensitive())
            options |= QWebPage::FindCaseSensitively;

        found = currentTab()->findText(ttf, options);
        findWidget->setTextWrappedVisible(false);

        if (!found) {
            options |= QWebPage::FindWrapsAroundDocument;
            found = currentTab()->findText(ttf, options);
            if (found)
                findWidget->setTextWrappedVisible(true);
        }
    }
    // force highlighting of all other matches, also when empty (clear)
    options = QWebPage::HighlightAllOccurrences;
    if (findWidget->caseSensitive())
        options |= QWebPage::FindCaseSensitively;
    currentTab()->findText(QLatin1String(""), options);
    currentTab()->findText(ttf, options);

    if (!found && ttf.isEmpty())
        found = true;   // the line edit is empty, no need to mark it red...

    if (!findWidget->isVisible())
        findWidget->show();

    findWidget->setPalette(found);
}

void BrowserMainWindow::slotEditFindNext()
{
    find(findWidget->text(), true);
}

void BrowserMainWindow::slotEditFindPrevious()
{
    find(findWidget->text(), false);
}

void BrowserMainWindow::setLastSizing(int value)
{
	QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	settings.setValue(QLatin1String("lastSizing"), value);
	m_textSizeNormal->setChecked( value == 0 );
	m_textSizeSmaller->setChecked( value == -1 );
	m_textSizeLarger->setChecked( value == 1 );
}

void BrowserMainWindow::slotViewTextBigger()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() + 0.1);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, m_viewZoomTextOnly->isChecked());

	setLastSizing( 1 );
}

void BrowserMainWindow::slotViewTextNormal()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(1.0);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, m_viewZoomTextOnly->isChecked());
	setLastSizing( 0 );
}

void BrowserMainWindow::slotViewTextSmaller()
{
    if (!currentTab())
        return;
    currentTab()->setTextSizeMultiplier(currentTab()->textSizeMultiplier() - 0.1);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, m_viewZoomTextOnly->isChecked());
	setLastSizing( -1 );
}

void BrowserMainWindow::slotViewFullScreen(bool makeFullScreen)
{
    if (makeFullScreen) {
        showFullScreen();
    } else {
        if (isMinimized())
            showMinimized();
        else if (isMaximized())
            showMaximized();
        else showNormal();
    }
}

extern bool ShellOpenApp(QString app, QString cmd);

void BrowserMainWindow::slotViewPageSource()
{
#ifdef Q_WS_WIN
	QLatin1String notepad("NOTEPAD.EXE");
#else

#ifdef Q_WS_MAC
	QLatin1String notepad("Open");
    #else
        QLatin1String notepad("gedit");
#endif

#endif

    if (!currentTab())
        return;

    QString markup = currentTab()->page()->mainFrame()->toHtml();

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bExtViewer = settings.value(QLatin1String("useExtViewer"), false).toBool();
	QString extViewer = settings.value(QLatin1String("ExtViewer"), notepad).toString();
    settings.endGroup();


	if (!bExtViewer)
	{
		ViewSourceWindow* w = new ViewSourceWindow(this, markup);
		w->resize(800, 512);
		w->setAttribute(Qt::WA_DeleteOnClose);
		w->show();
	}
	else
	{
		QString temp_file_name = QDir::tempPath() + QDir::separator() + QString("HTML_SRC_QTWEB.HTML");

		QFile file(temp_file_name);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
		 return;

		file.write( markup.toUtf8() );
		file.close();


		if (!ShellOpenApp(extViewer, temp_file_name))
			ShellOpenApp(notepad, temp_file_name);
	}
}

void BrowserMainWindow::slotHome()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    QString home = settings.value(QLatin1String("home"), QLatin1String("http://www.qtweb.net/")).toString();
    loadPage(home);
}

void BrowserMainWindow::slotWebSearch()
{
	if (!m_navigationBar->isVisible())
		m_navigationBar->show();

    m_toolbarSearch->lineEdit()->selectAll();
    m_toolbarSearch->lineEdit()->setFocus();
}

void BrowserMainWindow::slotToggleInspector(bool enable)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, enable);
	QSettings settings;
    settings.beginGroup(QLatin1String("general"));
	settings.setValue(QLatin1String("EnableWebInspector"), enable);
	
	if (enable) {
		m_inspectElement->setEnabled(true);
        int result = QMessageBox::question(this, tr("Web Inspector"),
                                           tr("The web inspector will only work correctly for pages that were loaded after enabling. "
										   "Enabling web inspector slows down web pages loading time, and consumes much more memory to keep resource usage statistics.\n\n"
										   "Enabled state will be saved for the next browser runs until you turn it off.\n\n"
                                           "Do you want to reload all pages?"),
                                           QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {

            m_tabWidget->reloadAllTabs();
        }

    }
	checkToolBarButtons();
}

void BrowserMainWindow::slotSwapFocus()
{
    if (currentTab()->hasFocus())
        m_tabWidget->currentLineEdit()->setFocus();
    else
        currentTab()->setFocus();
}

void BrowserMainWindow::loadPage(const QString &page)
{
    if (!currentTab() || page.isEmpty()) 
        return;

	// Check for Tags for Bookmarks
	if (page.indexOf('.') == -1)
	{
		BookmarksManager *bm = BrowserApplication::bookmarksManager();
		if (bm)
		{
			// if tags are detected - load all urls, and return
			QStringList urls = bm->find_tag_urls(NULL, page);
			if (!urls.isEmpty())
			{
				for(int i = 0; i < urls.size(); i++)
				{
					QString url = urls.at(i);
					if (i == 0)
					{
						m_tabWidget->currentLineEdit()->setText( url );
						m_tabWidget->loadUrlInCurrentTab(QUrl(url));
					}
					else
						m_tabWidget->loadUrl(QUrl(url), TabWidget::NewTab);
				}
				return;
			}
		}
	}

    QUrl url = guessUrlFromString(page);
    m_tabWidget->currentLineEdit()->setText(url.toString());
    m_tabWidget->loadUrlInCurrentTab(url);
}

TabWidget *BrowserMainWindow::tabWidget() const
{
    return m_tabWidget;
}

WebView *BrowserMainWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}

void BrowserMainWindow::slotLoadProgress(int progress)
{
    if (progress < 100 && progress > 0) {
        m_chaseWidget->setAnimated(true);
        disconnect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        disconnect(m_stopReload, SIGNAL(triggered()), m_load, SLOT(trigger()));
        if (m_stopIcon.isNull())
            m_stopIcon = style()->standardIcon(QStyle::SP_BrowserStop);
        m_stopReload->setIcon(m_stopIcon);
        connect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setToolTip(tr("Stop loading the current page"));
		m_stop->setEnabled(true);
		m_reload->setEnabled(false);
    } else {
        m_chaseWidget->setAnimated(false);
        disconnect(m_stopReload, SIGNAL(triggered()), m_load, SLOT(trigger()));
        disconnect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setIcon(m_reloadIcon);
        connect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        m_stopReload->setToolTip(tr("Reload the current page"));
		m_stop->setEnabled(false);
		m_reload->setEnabled(true);
    }
}

void BrowserMainWindow::slotAboutToShowBackMenu()
{
    m_historyBackMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = history->backItems(historyCount).count() - 1; i >= 0; --i) {
        QWebHistoryItem item = history->backItems(history->count()).at(i);
		
        QAction *action = new QAction(this);
        action->setData(-1*(historyCount-i-1));
        QIcon icon = BrowserApplication::instance()->icon(item.url());
        action->setIcon(icon);
		action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        m_historyBackMenu->addAction(action);
    }
}

void BrowserMainWindow::slotAboutToShowForwardMenu()
{
    m_historyForwardMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = 0; i < history->forwardItems(history->count()).count(); ++i) {
        QWebHistoryItem item = history->forwardItems(historyCount).at(i);
        QAction *action = new QAction(this);
        action->setData(historyCount-i);
        QIcon icon = BrowserApplication::instance()->icon(item.url());
        action->setIcon(icon);
        action->setText(item.title().isEmpty() ? item.url().toString() : item.title());
        m_historyForwardMenu->addAction(action);
    }
}

void BrowserMainWindow::slotAboutToShowWindowMenu()
{
	static QAction *downs = NULL, *tors = NULL;
	bool empty = m_windowMenu->isEmpty();
	if (!empty)
	{
		if (downs)
		{
			disconnect(downs, SIGNAL(triggered()), this, SLOT(slotDownloadManager()));
			this->removeAction(downs);
		}
		if (tors) 
		{
			disconnect(tors, SIGNAL(triggered()), this, SLOT(slotTorrents()));
			this->removeAction(tors);
		}
	}
    m_windowMenu->clear();

	// Show Next Tab
	m_windowMenu->addAction(m_tabWidget->nextTabAction());
	this->addAction(m_tabWidget->nextTabAction());

	// Show Prev Tab
	m_windowMenu->addAction(m_tabWidget->previousTabAction());
	this->addAction(m_tabWidget->previousTabAction());

	m_windowMenu->addSeparator();

	MenuCommands cmds;

	// Downloads
    downs = new QAction(cmds.DownsTitle(), this);
	if (showMenuIcons())
	{
		downs->setIcon(QIcon(QLatin1String(":data_ok.png")));
	}
	downs->setShortcuts(cmds.DownsShortcuts());
	connect(downs, SIGNAL(triggered()), this, SLOT(slotDownloadManager()));
	m_windowMenu->addAction(downs); 
	this->addAction(downs);

	// Torrents
    tors = new QAction(cmds.TorrentsTitle(), this);
	if (showMenuIcons())
	{
		tors->setIcon(QIcon(QLatin1String(":/icons/1downarrow.png")));
	}
	tors->setShortcuts(cmds.TorrentsShortcuts());
	connect(tors, SIGNAL(triggered()), this, SLOT(slotTorrents()));
	m_windowMenu->addAction(tors); 
	this->addAction(tors);


    m_windowMenu->addSeparator();
    QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
    for (int i = 0; i < windows.count(); ++i) {
        BrowserMainWindow *window = windows.at(i);
        QAction *action = m_windowMenu->addAction(window->windowTitle(), this, SLOT(slotShowWindow()));
        action->setData(i);
        action->setCheckable(true);
        if (window == this)
            action->setChecked(true);
    }
}

void BrowserMainWindow::slotShowWindow()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QVariant v = action->data();
        if (v.canConvert<int>()) {
            int offset = qvariant_cast<int>(v);
            QList<BrowserMainWindow*> windows = BrowserApplication::instance()->mainWindows();
            windows.at(offset)->activateWindow();
            windows.at(offset)->currentTab()->setFocus();
        }
    }
}

void BrowserMainWindow::slotOpenActionUrl(QAction *action)
{
    int offset = action->data().toInt();
    QWebHistory *history = currentTab()->history();
    if (offset < 0)
        history->goToItem(history->backItems(-1*offset).first()); // back
    else if (offset > 0)
        history->goToItem(history->forwardItems(history->count() - offset + 1).back()); // forward
 }

void BrowserMainWindow::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}


void BrowserMainWindow::slotNextStyle()
{
    QSettings settings;

	settings.beginGroup(QLatin1String("MainWindow"));
	QString cur_style = settings.value(QLatin1String("style"), DefaultAppStyle()).toString();

	QStringList styles = QStyleFactory::keys();
	if (styles.size() <= 0 )
		return;

	int current = -1;
	for(int i = 0; i < styles.size(); i++)
	{
		if (cur_style == styles.at(i))
		{
			current = i;
			break;
		}
	}

	if (current == -1 || (current == styles.size() - 1))
		current = -1;
	
	QString style = styles.at(current + 1);


	QApplication::setStyle(QStyleFactory::create(style));
	settings.setValue(QLatin1String("style"), style);
}

void BrowserMainWindow::slotAboutToShowToolsMenu()
{
	bool enabled = QWebSettings::globalSettings()->testAttribute(QWebSettings::DeveloperExtrasEnabled);
	if (m_enableInspector)
		m_enableInspector->setChecked(enabled);
	if (m_inspectElement)
		m_inspectElement->setEnabled( enabled );
}

void BrowserMainWindow::slotFullCleanUpOnQuit()
{
	bool reset_on_quit = false;
	if (!BrowserApplication::resetOnQuit())
	{
		QString title = tr("You are quitting the application with full reset");
		QString text = tr("<b>%1</b><br><br>Full reset will restore all settings to the state as if QtWeb never run on this machine.<br><br>" \
			" No traces of browsing will be left locally:<br><br>" \
			" - Custom Bookmarks will be deleted<br>" \
			" - Cookies will be cleaned up<br>" \
			" - Browsing history will be removed<br>" \
			" - Searches will be cleaned up<br>" \
			" - Saved user names and passwords will be deleted<br>" \
			" - Caches (web pages and icons) will be emptied and removed<br>" \
			" - Downloads (if any) will be deleted together with downloading history<br>" \
			" - All custom settings will be reset to defaults<br>" \
			" - All registry settings, created directories and files will be cleaned up<br>" \
			"<br><br>Are you sure you want to quit application with full reset?<br>"
			).arg(title);

		QMessageBox::StandardButton button = QMessageBox::information(this, tr("Information"), text,
			QMessageBox::Yes | QMessageBox::No,
							   QMessageBox::Yes);

		reset_on_quit = (button == QMessageBox::Yes);
	}

	BrowserApplication::setResetOnQuit(reset_on_quit);
	m_fullCleanUpOnQuitMenu->setChecked(reset_on_quit);

	if (reset_on_quit)
	{
		{
			QSettings settings;
			settings.beginGroup(QLatin1String("MainWindow"));
			settings.setValue(QLatin1String("quitDontAsk"), true);
			settings.setValue(QLatin1String("onStartup"), 0);
			settings.endGroup();
		}

		close();
	}
}


void BrowserMainWindow::slotEncodingChange()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	m_currentEncoding = action->statusTip();

	WebView *view = currentTab();
	if (!view)
		return;

	view->reload();

}

void BrowserMainWindow::slotAboutToShowPrivacyMenu()
{
	bool cache = BrowserApplication::networkAccessManager() && BrowserApplication::networkAccessManager()->cache();
	m_emptyCache->setEnabled( cache );

	m_fullCleanUpOnQuitMenu->setChecked(BrowserApplication::resetOnQuit());

	QWebSettings* defaultSettings = QWebSettings::globalSettings();
    m_disableJavaScript->setChecked(!defaultSettings->testAttribute(QWebSettings::JavascriptEnabled));
	m_disableImages->setChecked(!defaultSettings->testAttribute(QWebSettings::AutoLoadImages));
	m_disablePlugIns->setChecked(!defaultSettings->testAttribute(QWebSettings::PluginsEnabled));
	m_disablePopUps->setChecked(!defaultSettings->testAttribute(QWebSettings::JavascriptCanOpenWindows));

	
	QSettings settings;
	settings.beginGroup(QLatin1String("cookies"));
	QByteArray value = settings.value(QLatin1String("acceptCookies"), QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
	m_disableCookies->setChecked( value == "AcceptNever" );
	settings.endGroup();

	settings.beginGroup(QLatin1String("websettings"));
	m_disableUserAgent->setChecked(settings.value(QLatin1String("customUserAgent"), false).toBool() && 
			settings.value(QLatin1String("UserAgent"), "").toString().trimmed().length() == 0);
	settings.endGroup();

	settings.beginGroup(QLatin1String("proxy"));
    m_enableProxy->setChecked( settings.value(QLatin1String("enabled"), false).toBool()); 
	settings.endGroup();
}

void BrowserMainWindow::slotDisableJavaScript()
{
	bool enabled = !m_disableJavaScript->isChecked();
	QWebSettings* defaultSettings = QWebSettings::globalSettings();
	defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, enabled);
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("enableJavascript"), enabled);
}

void BrowserMainWindow::slotDisableImages()
{
	bool enabled = !m_disableImages->isChecked();
	QWebSettings* defaultSettings = QWebSettings::globalSettings();
	defaultSettings->setAttribute(QWebSettings::AutoLoadImages, enabled);
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("autoLoadImages"), enabled);
	checkToolBarButtons();
}

void BrowserMainWindow::slotDisablePlugIns()
{
	bool enabled = !m_disablePlugIns->isChecked();
	QWebSettings* defaultSettings = QWebSettings::globalSettings();
	defaultSettings->setAttribute(QWebSettings::PluginsEnabled, enabled);
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("enablePlugins"), enabled);
}

void BrowserMainWindow::slotDisableCookies()
{
	bool enabled = !m_disableCookies->isChecked();
	QSettings settings;
	settings.beginGroup(QLatin1String("cookies"));
	settings.setValue(QLatin1String("acceptCookies"), (enabled ? QLatin1String("AcceptOnlyFromSitesNavigatedTo"): QLatin1String("AcceptNever")));
    BrowserApplication::cookieJar()->loadSettings();
}

void BrowserMainWindow::slotDisableUserAgent()
{
	bool disabled = m_disableUserAgent->isChecked();
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("customUserAgent"), disabled);
	if (disabled)
	{
		QString current_agent = settings.value(QLatin1String("UserAgent"), "" ).toString();
		if (current_agent.length() > 0 && current_agent.indexOf('/') != -1)
			settings.setValue(QLatin1String("prevUserAgent"), current_agent );

		settings.setValue(QLatin1String("UserAgent"), "");
	}

	WebPage::setDefaultAgent( );
	setCurrentAgentIcon();
}

void BrowserMainWindow::slotEnableProxy()
{
	slotToggleProxy();
	checkToolBarButtons();
}

void BrowserMainWindow::slotDisablePopUps()
{
	bool disabled = m_disablePopUps->isChecked();
	QWebSettings* defaultSettings = QWebSettings::globalSettings();
	defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, !disabled);
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("blockPopups"), disabled);
}

void BrowserMainWindow::slotInspectElement()
{
	WebView* tab = currentTab();
	if (tab)
		tab->slotInspectElement();
}

void BrowserMainWindow::slotChangeTextSize()
{
	QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	int lastSizing = settings.value(QLatin1String("lastSizing"), 0).toInt();
	switch( lastSizing )
	{
		case -1:
			slotViewTextSmaller();
			break;
		case 1:
			slotViewTextBigger();
			break;
		default:
			slotViewTextNormal();
	}
}

QStringList BrowserMainWindow::compatibleUserAgents()
{
	QStringList keys; 
	keys << "Internet Explorer" << "Firefox" << "Opera" << "Safari" << "Chrome" << "Custom" << "QtWeb";
	return keys;
}

void BrowserMainWindow::slotNextUserAgent()
{
	QStringList styles = compatibleUserAgents();
	if (styles.size() <= 0 )
		return;

	QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bCustomAgent = settings.value(QLatin1String("customUserAgent"), false).toBool();
	if (!bCustomAgent) // QtWeb
	{
		setCompatibilityAgent( styles.at(0) );
		return;
	}
	QString userAgent = settings.value(QLatin1String("UserAgent"), "").toString();
	if (userAgent.indexOf('/') != -1 || userAgent.trimmed().length() == 0) // Custom
		userAgent = "Custom";

	int current = -1;
	for(int i = 0; i < styles.size(); i++)
		if (userAgent == styles.at(i))
		{
			current = i;
			break;
		}

	if (current == -1 || (current == styles.size() - 1))
		current = -1;
	
	setCompatibilityAgent( styles.at(current + 1) );
}

void BrowserMainWindow::slotAboutToShowCompatibility()
{
	if (!m_compMenu)
		return;
	m_compMenu->clear();
	
	QString userAgent = currentListedAgent(); 
	
	QStringList keys = compatibleUserAgents(); 
	QActionGroup* alignmentGroup = new QActionGroup(this);
	alignmentGroup->setExclusive(true);
	foreach(QString style, keys)
	{
		QAction* a = m_compMenu->addAction(style, this, SLOT(slotCompatChange( )));
		alignmentGroup->addAction(a);
		a->setCheckable(true);
		a->setChecked( (userAgent == style) );
		if (showMenuIcons()) 
			a->setIcon(QIcon(getAgentIcon( style )));
	}
}

void BrowserMainWindow::slotCompatChange()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString agent = action->text();
	setCompatibilityAgent(agent);
}

void BrowserMainWindow::setCompatibilityAgent(QString agent)
{
	QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bQtWeb = (agent == "QtWeb");
	settings.setValue(QLatin1String("customUserAgent"), !bQtWeb);
	if (!bQtWeb)
	{
		QString userAgent = agent;
		if (userAgent == "Custom")
			userAgent = settings.value(QLatin1String("prevUserAgent"), "" ).toString();

		QString current_agent = settings.value(QLatin1String("UserAgent"), "" ).toString();
		if (current_agent.length() > 0 && current_agent.indexOf('/') != -1)
			settings.setValue(QLatin1String("prevUserAgent"), current_agent );

		settings.setValue(QLatin1String("UserAgent"), userAgent );
	}
	m_compatAction->setIcon(QIcon(getAgentIcon( agent )));

	WebPage::setDefaultAgent();
}

QString BrowserMainWindow::getAgentIcon(QString agent )
{
	QString name;
	if (agent == "QtWeb")
		 name = "logo";
	else if (agent == "Custom")
		 name = "defaulticon";
	else
		name = agent;
	
	return ":" + name + ".png";
}

QString BrowserMainWindow::currentListedAgent()
{
	QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	bool bCustomAgent = settings.value(QLatin1String("customUserAgent"), false).toBool();
	if (!bCustomAgent) // QtWeb
		return "QtWeb";
	
	QString userAgent = settings.value(QLatin1String("UserAgent"), "").toString();
	if (userAgent.indexOf('/') != -1) // Custom
		return "Custom";
	
	return userAgent;
}

void BrowserMainWindow::setCurrentAgentIcon()
{
	if (m_compatAction)
		m_compatAction->setIcon(QIcon(getAgentIcon(currentListedAgent()))); 
}

void BrowserMainWindow::slotZoomTextOnly(bool textOnly)
{
	QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, textOnly);
    m_viewZoomTextOnly->setChecked(textOnly);

    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("zoom_text_only"), textOnly);
}

void BrowserMainWindow::slotFileSavePdf()
{
    if (!currentTab() || ! (currentTab()->page()) || ! (currentTab()->page()->mainFrame()))
        return;
    
	SavePDF dlg(m_title, currentTab()->page()->mainFrame(), this);
	dlg.exec();
}
