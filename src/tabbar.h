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

#ifndef TABBAR_H
#define TABBAR_H

#include <QtGui/QTabBar>

class QUrl;

#include <QtGui/QShortcut>
/*
    Tab bar with a few more features such as a context menu and shortcuts
 */
class TabBar : public QTabBar
{
    Q_OBJECT

signals:
    void newTab();
    void cloneTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void reloadTab(int index);
    void reloadAllTabs();
#if QT_VERSION < 0x040500
    void tabMoveRequested(int fromIndex, int toIndex);
#endif    
    void loadUrl(const QUrl &url);

public:
    TabBar(QWidget *parent = 0);

	bool showTabBarWhenOneTab() const;
    void setShowTabBarWhenOneTab(bool enabled);
    QAction *viewTabBarAction() const;

#if QT_VERSION >= 0x040500
    QTabBar::ButtonPosition freeSide();
#endif

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void tabInserted(int position);
    void tabRemoved(int position);
    QSize tabSizeHint(int index) const;

private slots:
    void selectTabAction();
    void cloneTab();
    void closeTab();
    void reloadTab();
    void contextMenuRequested(const QPoint &position);
    void closeOtherTabs();
    void updateViewToolBarAction();
    void viewTabBar();

private:
    void updateVisibility();
    QList<QShortcut*> m_tabShortcuts;
    friend class TabWidget;

    QPoint m_dragStartPos;
    int m_dragCurrentIndex;

    QAction *m_viewTabBarAction;
    bool m_showTabBarWhenOneTab;
};

#include <QtWebKit/QWebPage>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

class QLabel;
class WebView;
/*!
    A proxy object that connects a single browser action
    to one child webpage action at a time.

    Example usage: used to keep the main window stop action in sync with
    the current tabs webview's stop action.
 */
class WebActionMapper : public QObject
{
    Q_OBJECT

public:
    WebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent);
    QWebPage::WebAction webAction() const;
    void addChild(QAction *action);
    void updateCurrent(QWebPage *currentParent);

private slots:
    void rootTriggered();
    void childChanged();
    void rootDestroyed();
    void currentDestroyed();

private:
    QWebPage *m_currentParent;
    QAction *m_root;
    QWebPage::WebAction m_webAction;
};

#include <qshortcut.h>

/*
     Shortcut to switch directly to a tab by index
 */
class TabShortcut : public QShortcut
{
    Q_OBJECT

public:
    int tab();
    TabShortcut(int tab, const QKeySequence &key, QWidget *parent);

private:
    int m_tab;
};

#endif // TABBAR_H

