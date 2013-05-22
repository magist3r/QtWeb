/****************************************************************************
**
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
** Copyright (C) 2008-2008 Trolltech ASA. All rights reserved.
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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>
#include <qtoolbutton.h>

#include <QtGui/QUndoCommand>

/*!
    Bookmark manager, owner of the bookmarks, loads, saves and basic tasks
  */
class AutoSaver;
class BookmarkNode;
class BookmarksModel;
class BookmarksManager : public QObject
{
    Q_OBJECT

signals:
    void entryAdded(BookmarkNode *item);
    void entryRemoved(BookmarkNode *parent, int row, BookmarkNode *item);
    void entryChanged(BookmarkNode *item);

public:
    BookmarksManager(QObject *parent = 0);
    ~BookmarksManager();

    void addBookmark(BookmarkNode *parent, BookmarkNode *node, int row = -1);
    void removeBookmark(BookmarkNode *node);
    void setTitle(BookmarkNode *node, const QString &newTitle);
    void setUrl(BookmarkNode *node, const QString &newUrl);
    void setTags(BookmarkNode *node, const QString &newTags);
    void changeExpanded();

    BookmarkNode *bookmarks();
    BookmarkNode *menu();
    BookmarkNode *toolbar();
    BookmarkNode *find_folder(BookmarkNode *start_node, const QString& title);
	QStringList find_tag_urls(BookmarkNode *start_node, const QString& tag);

    BookmarksModel *bookmarksModel();
    QUndoStack *undoRedoStack() { return &m_commands; };

public slots:
    void importBookmarks();
	void importFromIE();
	void importFromMozilla();
	void importFromHTML();
    void exportBookmarks();

private slots:
    void save() const;

private:
    void load();

    bool m_loaded;
    AutoSaver *m_saveTimer;
    BookmarkNode *m_bookmarkRootNode;
    BookmarksModel *m_bookmarkModel;
    QUndoStack m_commands;

    friend class RemoveBookmarksCommand;
    friend class ChangeBookmarkCommand;
};

class RemoveBookmarksCommand : public QUndoCommand
{

public:
    RemoveBookmarksCommand(BookmarksManager *m_bookmarkManagaer, BookmarkNode *parent, int row);
    ~RemoveBookmarksCommand();
    void undo();
    void redo();

protected:
    int m_row;
    BookmarksManager *m_bookmarkManagaer;
    BookmarkNode *m_node;
    BookmarkNode *m_parent;
    bool m_done;
};

class InsertBookmarksCommand : public RemoveBookmarksCommand
{

public:
    InsertBookmarksCommand(BookmarksManager *m_bookmarkManagaer,
        BookmarkNode *parent, BookmarkNode *node, int row);
    void undo() { RemoveBookmarksCommand::redo(); }
    void redo() { RemoveBookmarksCommand::undo(); }

};

enum ChangeBookmarkType{
	changedURL,
	changedTitle,
	changedTags
};

class ChangeBookmarkCommand : public QUndoCommand
{

public:
    ChangeBookmarkCommand(BookmarksManager *m_bookmarkManagaer,
        BookmarkNode *node, const QString &newValue, ChangeBookmarkType type);
    void undo();
    void redo();

private:
    BookmarksManager *m_bookmarkManagaer;
    ChangeBookmarkType m_type;
    QString m_oldValue;
    QString m_newValue;
    BookmarkNode *m_node;
};

/*!
    BookmarksModel is a QAbstractItemModel wrapper around the BookmarkManager
  */
#include <QtGui/QIcon>
class BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT

public slots:
    void entryAdded(BookmarkNode *item);
    void entryRemoved(BookmarkNode *parent, int row, BookmarkNode *item);
    void entryChanged(BookmarkNode *item);

public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        UrlRole = Qt::UserRole + 2,
        UrlStringRole = Qt::UserRole + 3,
        SeparatorRole = Qt::UserRole + 4
    };

    BookmarksModel(BookmarksManager *bookmarkManager, QObject *parent = 0);
    inline BookmarksManager *bookmarksManager() const { return m_bookmarksManager; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index= QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Qt::DropActions supportedDropActions () const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    bool dropMimeData(const QMimeData *data,
        Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    BookmarkNode *node(const QModelIndex &index) const;
    QModelIndex index(BookmarkNode *node) const;
	void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder ) {;} 

private:

    bool m_endMacro;
    BookmarksManager *m_bookmarksManager;
};

// Menu that is dynamically populated from the bookmarks
#include "modelmenu.h"
class BookmarksMenu : public ModelMenu
{
    Q_OBJECT

signals:
    void openUrl(const QUrl &url);

public:
     BookmarksMenu(QWidget *parent = 0);
     void setInitialActions(QList<QAction*> actions);

protected:
    bool prePopulated();

private slots:
    void activated(const QModelIndex &index);

private:
    BookmarksManager *m_bookmarksManager;
    QList<QAction*> m_initialActions;
};

/*
    Proxy model that filters out the bookmarks so only the folders
    are left behind.  Used in the add bookmark dialog combobox.
 */
#include <QtGui/QSortFilterProxyModel>
class AddBookmarkProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    AddBookmarkProxyModel(QObject * parent = 0);
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

/*!
    Add bookmark dialog
 */
#include "ui_addbookmarkdialog.h"
class AddBookmarkDialog : public QDialog, public Ui_AddBookmarkDialog
{
    Q_OBJECT

public:
    AddBookmarkDialog(const QString &url, const QString &title, const QString& default_folder, QWidget *parent = 0, BookmarksManager *bookmarkManager = 0);
    void acceptDefaultLocation();

private slots:
    void accept();

private:
    QString m_url;
	QString m_default_folder;
    BookmarksManager *m_bookmarksManager;
    AddBookmarkProxyModel *m_proxyModel;
};

#include "ui_bookmarks.h"
class TreeProxyModel;
class BookmarksDialog : public QDialog, public Ui_BookmarksDialog
{
    Q_OBJECT

signals:
    void openUrl(const QUrl &url);

public:
    BookmarksDialog(QWidget *parent = 0, BookmarksManager *manager = 0);
    ~BookmarksDialog();

private slots:
    void customContextMenuRequested(const QPoint &pos);
    void open();
    void newFolder();
    void sortBookmarks();

private:
    void expandNodes(BookmarkNode *node);
    bool saveExpandedNodes(const QModelIndex &parent);

    BookmarksManager *m_bookmarksManager;
    BookmarksModel *m_bookmarksModel;
    TreeProxyModel *m_proxyModel;
};

#include <QtGui/QToolBar>
#include <tabwidget.h>

class BookmarksToolBar : public QToolBar
{
    Q_OBJECT

signals:
    void openUrl(const QUrl &url, TabWidget::Tab tab, const QString &title);
    void openUrl(const QUrl &url);

public:
    BookmarksToolBar(BookmarksModel *model, QWidget *parent = 0);
    void setRootIndex(const QModelIndex &index);
    QModelIndex rootIndex() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void triggered(QAction *action);
    void activated(const QModelIndex &index);
    void build();
    void deleteBookmark(const QUrl &url, const QString &title);
    void renameBookmark(const QUrl &url, const QString &title, const QString &new_title);
	void addFolder(const QString &title, const QString &new_folder);

private:
    BookmarksModel *m_bookmarksModel;
    QPersistentModelIndex m_root;
};


#include "tabwidget.h"

class BookmarkToolButton : public QToolButton
{
    Q_OBJECT

signals:
    void openBookmark(const QUrl &url, TabWidget::Tab tab, const QString &title);
    void deleteBookmark(const QUrl &url, const QString &title);
    void renameBookmark(const QUrl &url, const QString &title, const QString &new_title);
    void addFolder(const QString &title, const QString &new_folder);

public slots:
	void contextMenuRequested(const QPoint &);
	void deleteBookmark();
	void openBookmark();
	void openBookmarkNewTab();
	void renameBookmark();
	void addFolder();

public:
    BookmarkToolButton(QUrl url, QWidget *parent = 0);
    QUrl url() const;

protected:
    void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QUrl m_url;
	QPoint dragStartPosition;

};

#endif // BOOKMARKS_H
