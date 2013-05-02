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

#include "tabwidget.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "history.h"
#include "urllineedit.h"
#include "webview.h"
#include "webpage.h"
#include "commands.h"

#include <QtGui/QClipboard>
#include <QtGui/QCompleter>
#include <QtGui/QListView>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QStackedWidget>
#include <QtGui/QStyle>
#include <QtGui/QToolButton>
#include <QFile>
#include <QtCore/QDebug>
#include <QBuffer>
#include <QLabel>
#include <QMovie>
#include <QSettings>
#include <QStyle>


TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_recentlyClosedTabsAction(0)
    , m_newTabAction(0)
    , m_closeTabAction(0)
    , m_nextTabAction(0)
    , m_previousTabAction(0)
    , m_recentlyClosedTabsMenu(0)
    , m_lineEdits(0)
	, m_prevSelectedTab(-1)
	, m_prevSelectedTabMark(-1)
    , m_tabBar(new TabBar(this))
{
	setElideMode(Qt::ElideRight);
	
    //new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, SLOT(openLastTab()));

    connect(m_tabBar, SIGNAL(loadUrl(const QUrl&)),
            this, SLOT(loadUrlNewTab(const QUrl&)));

    connect(m_tabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));
    connect(m_tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(moveTab(int, int)));
    setTabBar(m_tabBar);

    setDocumentMode(true);

	MenuCommands cmds;

	// Actions
    QSettings settings;
    settings.beginGroup(QLatin1String("general"));
	bool hide_icons = settings.value(QLatin1String("hideMenuIcons"), false).toBool();
    settings.endGroup();	

	// New Tab
    m_newTabAction = new QAction(cmds.NewTabTitle(), this);
	m_newTabAction->setIcon(QIcon(QLatin1String(":addtab.png")));
	m_newTabAction->setIconVisibleInMenu(!hide_icons);
    m_newTabAction->setShortcuts( cmds.NewTabShortcuts() );
    connect(m_newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));
	this->addAction(m_newTabAction);

	// Close Tab
    m_closeTabAction = new QAction(cmds.CloseTabTitle(), this);
	m_closeTabAction->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    m_closeTabAction->setShortcuts(cmds.CloseTabShortcuts());
    m_closeTabAction->setIconVisibleInMenu(!hide_icons);
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeTab()));
	this->addAction(m_closeTabAction);

	// Show Next Tab
    m_nextTabAction = new QAction(cmds.NextTabTitle(), this);
    m_nextTabAction->setShortcuts(cmds.NextTabShortcuts());
    connect(m_nextTabAction, SIGNAL(triggered()), this, SLOT(nextTab()));

	// Show Prev Tab
    m_previousTabAction = new QAction(cmds.PrevTabTitle(), this);
    m_previousTabAction->setShortcuts(cmds.PrevTabShortcuts());
    connect(m_previousTabAction, SIGNAL(triggered()), this, SLOT(previousTab()));

    m_recentlyClosedTabsMenu = new QMenu(this);
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction *)),this, SLOT(aboutToShowRecentTriggeredAction(QAction *)));

	m_recentlyClosedTabsAction = new QAction(cmds.LastTabsTitle(), this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

    m_tabBar->setTabsClosable(true);
    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    m_tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

	// corner buttons
    QToolButton *addTabButton = new QToolButton(this);
    addTabButton->setDefaultAction(m_newTabAction);
    addTabButton->setAutoRaise(true);
    addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCornerWidget(addTabButton, Qt::TopLeftCorner);

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    m_lineEdits = new QStackedWidget(this);
}

void TabWidget::openLastTab()
{
    if (m_recentlyClosedTabs.isEmpty())
        return;
    QUrl url = m_recentlyClosedTabs.takeFirst();
    loadUrl(url, NewTab);
    m_recentlyClosedTabsAction->setEnabled(!m_recentlyClosedTabs.isEmpty());
}

void TabWidget::loadUrl(const QUrl &url, Tab type, const QString &title)
{
    WebView *webView;
    if (NewTab == type) {
        webView = newTab();
        if (count() == 1)
            webView = this->webView(0);
    } else {
        webView = currentWebView();
    }

    if (webView) {
        webView->loadUrl(url, title);
        webView->setFocus();
    }
}

// When index is -1 index chooses the current tab
void TabWidget::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WebView *tab = qobject_cast<WebView*>(widget))
        tab->reload();
}

void TabWidget::clear()
{
    // clear the recently closed tabs
    m_recentlyClosedTabs.clear();
    // clear the line edit history
    for (int i = 0; i < m_lineEdits->count(); ++i) {
        QLineEdit *qLineEdit = lineEdit(i);
        qLineEdit->setText(qLineEdit->text());
    }
}

void TabWidget::moveTab(int fromIndex, int toIndex)
{
    QWidget *lineEdit = m_lineEdits->widget(fromIndex);
    m_lineEdits->removeWidget(lineEdit);
    m_lineEdits->insertWidget(toIndex, lineEdit);

}

void TabWidget::addWebAction(QAction *action, QWebPage::WebAction webAction)
{
    if (!action)
        return;
    m_actions.append(new WebActionMapper(action, webAction, this));
}

void TabWidget::currentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;
	
	m_prevSelectedTab = m_prevSelectedTabMark;
	m_prevSelectedTabMark = currentIndex();

    Q_ASSERT(m_lineEdits->count() == count());

    WebView *oldWebView = this->webView(m_lineEdits->currentIndex());
    if (oldWebView) {
        disconnect(oldWebView, SIGNAL(statusBarMessage(const QString&)),
                this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                this, SIGNAL(loadProgress(int)));
    }

    connect(webView, SIGNAL(statusBarMessage(const QString&)),
            this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(webView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
            this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));

    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    m_lineEdits->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());
    if (webView->url().isEmpty())
        m_lineEdits->currentWidget()->setFocus();
    else
        webView->setFocus();
}

QAction *TabWidget::newTabAction() const
{
    return m_newTabAction;
}

QAction *TabWidget::closeTabAction() const
{
    return m_closeTabAction;
}

QAction *TabWidget::recentlyClosedTabsAction() const
{
    return m_recentlyClosedTabsAction;
}

QAction *TabWidget::nextTabAction() const
{
    return m_nextTabAction;
}

QAction *TabWidget::previousTabAction() const
{
    return m_previousTabAction;
}

QWidget *TabWidget::lineEditStack() const
{
    return m_lineEdits;
}

QLineEdit *TabWidget::currentLineEdit() const
{
    return lineEdit(m_lineEdits->currentIndex());
}

WebView *TabWidget::currentWebView() const
{
    return webView(currentIndex());
}

QLineEdit *TabWidget::lineEdit(int index) const
{
    UrlLineEdit *urlLineEdit = qobject_cast<UrlLineEdit*>(m_lineEdits->widget(index));
    if (urlLineEdit)
        return urlLineEdit->lineEdit();
    return 0;
}

WebView *TabWidget::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebView *webView = qobject_cast<WebView*>(widget)) {
        return webView;
    } else {
        // optimization to delay creating the first webview
        if (count() == 1) 
		{
			/*TabWidget *that = const_cast<TabWidget*>(this);
            that->setUpdatesEnabled(false);
            that->newTab();
            that->closeTab(0);
            that->setUpdatesEnabled(true);
            return currentWebView();*/

            TabWidget *that = const_cast<TabWidget*>(this);
            that->setUpdatesEnabled(false);
            UrlLineEdit *currentLocationBar = qobject_cast<UrlLineEdit*>(m_lineEdits->widget(0));
            bool giveBackFocus = currentLocationBar->hasFocus();
            m_lineEdits->removeWidget(currentLocationBar);
            m_lineEdits->addWidget(new QWidget());
            that->newTab();
            that->closeTab(0);
            QWidget *newEmptyLineEdit = m_lineEdits->widget(0);
            m_lineEdits->removeWidget(newEmptyLineEdit);
            newEmptyLineEdit->deleteLater();
            m_lineEdits->addWidget(currentLocationBar);
            currentLocationBar->setWebView(currentWebView());
            if (giveBackFocus)
                currentLocationBar->setFocus();
            that->setUpdatesEnabled(true);
            return currentWebView();
        }
    }
    return 0;
}

int TabWidget::webViewIndex(WebView *webView) const
{
    int index = indexOf(webView);
    return index;
}

int TabWidget::addNewTab(WebView *view)
{
	int index = addTab(view, tr("about:blank"));
	if (index == 0)
		return index;

	QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	int newTabAction = settings.value(QLatin1String("newTabAction"), 0).toInt();

	if (newTabAction == 2 || newTabAction == 1)
	{
		return index;
	}

	QFile file(QLatin1String(":/Welcome.html"));
	bool isOpened = file.open(QIODevice::ReadOnly | QIODevice::Text);
	Q_ASSERT(isOpened);
	QString html = QString(QLatin1String(file.readAll()));

	QPixmap pix= style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(32,32);

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    if (pix.save(&imageBuffer, "PNG")) 
	{
        html.replace(QLatin1String("LOGO_BINARY_DATA_HERE"),
                     QString(QLatin1String(imageBuffer.buffer().toBase64())));
    }
	view->setHtml( html );
 
	return  index;
}

WebView *TabWidget::newTab(bool makeCurrent)
{
    // line edit
    UrlLineEdit *urlLineEdit = new UrlLineEdit;
    QLineEdit *lineEdit = urlLineEdit->lineEdit();
    HistoryCompletionModel *completionModel = new HistoryCompletionModel(this);
    completionModel->setSourceModel(BrowserApplication::historyManager()->historyFilterModel());
    QCompleter * lineEditCompleter = new QCompleter(completionModel, this);
    QAbstractItemView *popup = lineEditCompleter->popup();
    QListView *listView = qobject_cast<QListView*>(popup);
    if (listView)
	{
        listView->setUniformItemSizes(true);
		listView->setSpacing(2);
		listView->setMinimumHeight(240);//800);//(listView->height() < 200 ? 200 : (listView->height() > 800? 800 : listView->height()) );
	}
    lineEdit->setCompleter(lineEditCompleter);
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    m_lineEdits->addWidget(urlLineEdit);
    m_lineEdits->setSizePolicy(lineEdit->sizePolicy());

    // optimization to delay creating the more expensive WebView, history, etc
    if (count() == 0) {
        QWidget *emptyWidget = new QWidget;
        QPalette p = emptyWidget->palette();
        p.setColor(QPalette::Window, palette().color(QPalette::Base));
        emptyWidget->setPalette(p);
        emptyWidget->setAutoFillBackground(true);
        disconnect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));
        //addNewTab(emptyWidget);
        addTab(emptyWidget, tr("Blank Tab"));
        connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));
        return 0;
    }

    // webview
    WebView *webView = new WebView;
    urlLineEdit->setWebView(webView);
    connect(webView, SIGNAL(loadStarted()),
            this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadFinished(bool)),
            this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(loadFinished(bool)),
            this, SLOT(webViewLoadFinished(bool)));
    connect(webView, SIGNAL(iconChanged()),
            this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)),
            this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView->page(), SIGNAL(windowCloseRequested()),
            this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(geometryChangeRequested(const QRect &)),
            this, SIGNAL(geometryChangeRequested(const QRect &)));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)),
            this, SIGNAL(printRequested(QWebFrame *)));
    connect(webView->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)),
            this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)),
            this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
    connect(webView->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)),
            this, SIGNAL(toolBarVisibilityChangeRequested(bool)));
    addNewTab(webView);

	if (makeCurrent)
        setCurrentWidget(webView);

    // webview actions
    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->addChild(webView->page()->action(mapper->webAction()));
    }

    if (count() == 1)
        currentChanged(currentIndex());
    emit tabsChanged();

	// move focus to address bar to type a new site
	currentLineEdit()->selectAll();
    currentLineEdit()->setFocus();

	QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
	int newTabAction = settings.value(QLatin1String("newTabAction"), 0).toInt();
	if (newTabAction == 1)
	{
	    QString home = settings.value(QLatin1String("home"), QLatin1String("http://www.qtweb.net/")).toString();
	    loadUrlInCurrentTab(home);
	}

    return webView;
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i) {
        QWidget *tabWidget = widget(i);
        if (WebView *tab = qobject_cast<WebView*>(tabWidget)) 
		{
            tab->reload();
        }
    }
}

void TabWidget::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender())) {
        emit loadPage(lineEdit->text());
        if (m_lineEdits->currentWidget() == lineEdit)
            currentWebView()->setFocus();
    }
}

void TabWidget::windowCloseRequested()
{
    WebPage *webPage = qobject_cast<WebPage*>(sender());
    WebView *webView = qobject_cast<WebView*>(webPage->view());
    int index = webViewIndex(webView);
    if (index >= 0) {
        if (count() == 1)
            webView->webPage()->mainWindow()->close();
        else
            closeTab(index);
    }
}

void TabWidget::prevSelectedTab()
{
	if (m_prevSelectedTab == -1)
		return;

	if (m_prevSelectedTab >= 0 && m_prevSelectedTab < count())
	{
		int prev = currentIndex();
		setCurrentIndex(m_prevSelectedTab);
		m_prevSelectedTab = prev;
	}
}

void TabWidget::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

// When index is -1 index chooses the current tab
void TabWidget::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
	if (webView(index)->url().isEmpty())
		return;

    WebView *tab = newTab(false);
    tab->setUrl(webView(index)->url());
}

// When index is -1 index chooses the current tab
void TabWidget::closeTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WebView *tab = webView(index)) {
        if (tab->isModified()) {
            QMessageBox closeConfirmation(tab);
            closeConfirmation.setWindowFlags(Qt::Sheet);
            closeConfirmation.setWindowTitle(tr("Do you really want to close this page?"));
            closeConfirmation.setInformativeText(tr("You have modified this page and when closing it you would lose the modification.\n"
                                                     "Do you really want to close this page?\n"));
            closeConfirmation.setIcon(QMessageBox::Question);
            closeConfirmation.addButton(QMessageBox::Yes);
            closeConfirmation.addButton(QMessageBox::No);
            closeConfirmation.setEscapeButton(QMessageBox::No);
            if (closeConfirmation.exec() == QMessageBox::No)
                return;
        }
        hasFocus = tab->hasFocus();

        m_recentlyClosedTabsAction->setEnabled(true);
        m_recentlyClosedTabs.prepend(tab->url());
        if (m_recentlyClosedTabs.size() >= TabWidget::m_recentlyClosedTabsSize)
            m_recentlyClosedTabs.removeLast();
    }
    QWidget *lineEdit = m_lineEdits->widget(index);
    m_lineEdits->removeWidget(lineEdit);
    lineEdit->deleteLater();
    QWidget *webView = widget(index);
    removeTab(index);
    webView->deleteLater();
    emit tabsChanged();
    if (hasFocus && count() > 0)
        currentWebView()->setFocus();
    if (count() == 0)
        emit lastTabClosed();
}

QLabel *TabWidget::animationLabel(int index, bool addMovie)
{
    if (-1 == index)
        return 0;
    QTabBar::ButtonPosition side = m_tabBar->freeSide();
    QLabel *loadingAnimation = qobject_cast<QLabel*>(m_tabBar->tabButton(index, side));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel(this);
    }
    if (addMovie && !loadingAnimation->movie()) {
        QMovie *movie = new QMovie(QLatin1String(":loading.gif"), QByteArray(), loadingAnimation);
        loadingAnimation->setMovie(movie);
        movie->start();
    }
    m_tabBar->setTabButton(index, side, 0);
    m_tabBar->setTabButton(index, side, loadingAnimation);
    return loadingAnimation;
}

void TabWidget::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    QLabel *label = animationLabel(index, true);
    if (label->movie())
        label->movie()->start();
}

void TabWidget::webViewIconChanged()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) 
	{
        QIcon icon =  BrowserApplication::instance()->icon(webView->url());
		if (!icon.isNull())
		{
			QLabel *label = animationLabel(index, false);
			QMovie *movie = label->movie();
			delete movie;
			label->setMovie(0);
			label->setPixmap(icon.pixmap(16, 16));
			setElideMode(Qt::ElideRight);
		}
    }
}

void TabWidget::webViewLoadFinished(bool ok)
{
	WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) 
	{
        QLabel *label = animationLabel(index, true);
        if (label->movie())
            label->movie()->stop();
    }
    webViewIconChanged();
}

void TabWidget::webViewTitleChanged(const QString &title)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    QString tabTitle = title;
    if (-1 != index) {
        if (title.isEmpty())
            tabTitle = webView->url().toString();
        tabTitle.replace(QLatin1Char('&'), QLatin1String("&&"));
        setTabText(index, tabTitle);
		setTabToolTip(index, tabTitle);
    }

	if (currentIndex() == index)
        emit setCurrentTitle(tabTitle);

	BrowserApplication::historyManager()->updateHistoryItem(webView->url(), tabTitle);
}

void TabWidget::webViewUrlChanged(const QUrl &url)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) 
	{
		m_tabBar->setTabData(index, url);
    }
    emit tabsChanged();
}

void TabWidget::aboutToShowRecentTabsMenu()
{
    m_recentlyClosedTabsMenu->clear();
    for (int i = 0; i < m_recentlyClosedTabs.count(); ++i) {
        QAction *action = new QAction(m_recentlyClosedTabsMenu);
        action->setData(m_recentlyClosedTabs.at(i));
        QIcon icon = BrowserApplication::instance()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText(m_recentlyClosedTabs.at(i).toString());
        m_recentlyClosedTabsMenu->addAction(action);
    }
}

void TabWidget::aboutToShowRecentTriggeredAction(QAction *action)
{
    QUrl url = action->data().toUrl();
    loadUrlInCurrentTab(url);
}

void TabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        newTab();
        return;
    }
    QTabWidget::mouseDoubleClickEvent(event);
}

void TabWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!childAt(event->pos())) {
        m_tabBar->contextMenuRequested(event->pos());
        return;
    }
    QTabWidget::contextMenuEvent(event);
}

void TabWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && !childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            WebView *webView = newTab();
            webView->setUrl(url);
        }
    }
}

void TabWidget::loadUrlInCurrentTab(const QUrl &url)
{
    WebView *webView = currentWebView();
    if (webView) {
        webView->loadUrl(url);
        webView->setFocus();
    }
}

void TabWidget::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void TabWidget::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}



static const qint32 TabWidgetMagic = 0xaa;
QByteArray TabWidget::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(TabWidgetMagic);
    stream << qint32(version);

    QStringList tabs;
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = qobject_cast<WebView*>(widget(i))) {
            tabs.append(tab->url().toString());
        } else {
            tabs.append(QString::null);
        }
    }
    stream << tabs;
    stream << currentIndex();
    return data;
}

bool TabWidget::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != TabWidgetMagic || v != version)
        return false;

    QStringList openTabs;
    stream >> openTabs;

    for (int i = 0; i < openTabs.count(); ++i) {
        if (i != 0)
            newTab();
        loadPage(openTabs.at(i));
    }

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);

    return true;
}
