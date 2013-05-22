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

TabShortcut::TabShortcut(int tab, const QKeySequence &key, QWidget *parent)
    : QShortcut(key, parent)
    , m_tab(tab)
{
}

int TabShortcut::tab()
{
    return m_tab;
}
TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
    , m_viewTabBarAction(0)
    , m_showTabBarWhenOneTab(true)
{
	setElideMode(Qt::ElideRight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);
    setUsesScrollButtons(true);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(contextMenuRequested(const QPoint &)));

    QString alt = QLatin1String("Ctrl+%1");
    for (int i = 0; i < 10; ++i) {
        int key = i + 1;
        TabShortcut *tabShortCut = new TabShortcut(i, alt.arg(key), this);
        connect(tabShortCut, SIGNAL(activated()), this, SLOT(selectTabAction()));
    }

    setTabsClosable(true);

    m_viewTabBarAction = new QAction(this);
    updateViewToolBarAction();
    connect(m_viewTabBarAction, SIGNAL(triggered()),
            this, SLOT(viewTabBar()));

    setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setMovable(true);
}

QAction *TabBar::viewTabBarAction() const
{
    return m_viewTabBarAction;
}

bool TabBar::showTabBarWhenOneTab() const
{
    return m_showTabBarWhenOneTab;
}

void TabBar::setShowTabBarWhenOneTab(bool enabled)
{
    m_showTabBarWhenOneTab = enabled;
    updateVisibility();
}

QTabBar::ButtonPosition TabBar::freeSide()
{
    QTabBar::ButtonPosition side = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this);
    side = (side == QTabBar::LeftSide) ? QTabBar::RightSide : QTabBar::LeftSide;
    return side;
}

void TabBar::tabInserted(int position)
{
    Q_UNUSED(position);
    updateVisibility();
}

void TabBar::tabRemoved(int position)
{
    Q_UNUSED(position);
    updateVisibility();
}

void TabBar::updateViewToolBarAction()
{
    bool show = showTabBarWhenOneTab();
    if (count() > 1)
        show = true;
    MenuCommands cmds;
	m_viewTabBarAction->setText(!show ? cmds.TabShowTitle() : cmds.TabHideTitle() );
}

void TabBar::viewTabBar()
{
    setShowTabBarWhenOneTab(!showTabBarWhenOneTab());
    updateViewToolBarAction();
}

void TabBar::updateVisibility()
{
    setVisible((count()) > 1 || m_showTabBarWhenOneTab);
    m_viewTabBarAction->setEnabled(count() == 1);
    updateViewToolBarAction();
}

void TabBar::selectTabAction()
{
    int index = qobject_cast<TabShortcut*>(sender())->tab();
    setCurrentIndex(index);
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    QMenu menu;

	MenuCommands cmds;
	QAction *add = new QAction(cmds.NewTabTitle(), this);
    add->setShortcuts( cmds.NewTabShortcuts() );
    connect(add, SIGNAL(triggered()), this, SIGNAL(newTab()));
	menu.addAction(add);

    int index = tabAt(position);
    if (-1 != index) 
	{
        QAction *action = new QAction(cmds.CloneTabTitle(), this);
		action->setShortcuts( cmds.CloneTabShortcuts() );
		connect(action, SIGNAL(triggered()), this, SLOT(cloneTab()));
		menu.addAction(action);
    
		action->setData(index);

        menu.addSeparator();

		action = new QAction(cmds.CloseTabTitle(), this);
		action->setShortcuts( cmds.CloseTabShortcuts() );
		connect(action, SIGNAL(triggered()), this, SLOT(closeTab()));
		menu.addAction(action);
        action->setData(index);


		action = new QAction(cmds.CloseOtherTitle(), this);
		action->setShortcuts( cmds.CloseOtherShortcuts() );
		connect(action, SIGNAL(triggered()), this, SLOT(closeOtherTabs()));
		menu.addAction(action);
        action->setData(index);

        menu.addSeparator();

		action = new QAction(cmds.ReloadTabTitle(), this);
		action->setShortcuts( cmds.ReloadTabShortcuts() );
		connect(action, SIGNAL(triggered()), this, SLOT(reloadTab()));
		menu.addAction(action);

        action->setData(index);
    } else {
        menu.addSeparator();
    }

	QAction* action = new QAction(cmds.ReloadAllTitle(), this);
	action->setShortcuts( cmds.ReloadAllShortcuts() );
	connect(action, SIGNAL(triggered()), this, SIGNAL(reloadAllTabs()));
	menu.addAction(action);

	menu.exec(QCursor::pos());
}

void TabBar::cloneTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit cloneTab(index);
    }
}

void TabBar::closeTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeTab(index);
    }
}

void TabBar::closeOtherTabs()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeOtherTabs(index);
    }
}

void TabBar::reloadTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit reloadTab(index);
    }
}


void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
        // Remove the line below when QTabWidget does not have a one pixel frame
        && event->pos().y() < (y() + height())) {
        emit newTab();
        return;
    }
    QTabBar::mouseDoubleClickEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        int index = tabAt(event->pos());
        if (index != -1) {
			{
				if (count() == 1)
				{
					emit newTab();
				}
				emit closeTab(index);
			}
        } else {
            QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
            if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty())
                emit loadUrl(url); 
        }
    }

    QTabBar::mouseReleaseEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QTabBar::mousePressEvent(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int diffX = event->pos().x() - m_dragStartPos.x();
        int diffY = event->pos().y() - m_dragStartPos.y();
        if ((event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance()
            && diffX < 3 && diffX > -3
            && diffY < -10) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QList<QUrl> urls;
            int index = tabAt(event->pos());
            QUrl url = tabData(index).toUrl();
            urls.append(url);
            mimeData->setUrls(urls);
            mimeData->setText(tabText(index));
            mimeData->setData(QLatin1String("action"), "tab-reordering");
            drag->setMimeData(mimeData);
            drag->exec();
        }
    }
    QTabBar::mouseMoveEvent(event);
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize sizeHint = QTabBar::tabSizeHint(index);
    QFontMetrics fm = fontMetrics();
    return sizeHint.boundedTo(QSize(fm.width(QLatin1Char('M')) * 30, sizeHint.height()));
}

WebActionMapper::WebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent)
    : QObject(parent)
    , m_currentParent(0)
    , m_root(root)
    , m_webAction(webAction)
{
    if (!m_root)
        return;
    connect(m_root, SIGNAL(triggered()), this, SLOT(rootTriggered()));
    connect(root, SIGNAL(destroyed(QObject *)), this, SLOT(rootDestroyed()));
    root->setEnabled(false);
}

void WebActionMapper::rootDestroyed()
{
    m_root = 0;
}

void WebActionMapper::currentDestroyed()
{
    updateCurrent(0);
}

void WebActionMapper::addChild(QAction *action)
{
    if (!action)
        return;
    connect(action, SIGNAL(changed()), this, SLOT(childChanged()));
}

QWebPage::WebAction WebActionMapper::webAction() const
{
    return m_webAction;
}

void WebActionMapper::rootTriggered()
{
    if (m_currentParent) {
        QAction *gotoAction = m_currentParent->action(m_webAction);
        gotoAction->trigger();
    }
}

void WebActionMapper::childChanged()
{
    if (QAction *source = qobject_cast<QAction*>(sender())) {
        if (m_root
            && m_currentParent
            && source->parent() == m_currentParent) {
            m_root->setChecked(source->isChecked());
            m_root->setEnabled(source->isEnabled());
        }
    }
}

void WebActionMapper::updateCurrent(QWebPage *currentParent)
{
    if (m_currentParent)
        disconnect(m_currentParent, SIGNAL(destroyed(QObject *)),
                   this, SLOT(currentDestroyed()));

    m_currentParent = currentParent;
    if (!m_root)
        return;
    if (!m_currentParent) {
        m_root->setEnabled(false);
        m_root->setChecked(false);
        return;
    }
    QAction *source = m_currentParent->action(m_webAction);
    m_root->setChecked(source->isChecked());
    m_root->setEnabled(source->isEnabled());
    connect(m_currentParent, SIGNAL(destroyed(QObject *)),
            this, SLOT(currentDestroyed()));
}

