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

#include "bookmarks.h"

#include "autosaver.h"
#include "browserapplication.h"
#include "history.h"
#include "xbel.h"
#include "bookmarksimport.h"

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QInputDialog>

#include <QtWebKit/QWebSettings>

#include <QtCore/QDebug>

BookmarksManager::BookmarksManager(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
    , m_saveTimer(new AutoSaver(this))
    , m_bookmarkRootNode(0)
    , m_bookmarkModel(0)
{
    connect(this, SIGNAL(entryAdded(BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryRemoved(BookmarkNode *, int, BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryChanged(BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
}

BookmarksManager::~BookmarksManager()
{
    m_saveTimer->saveIfNeccessary();
}

void BookmarksManager::changeExpanded()
{
    m_saveTimer->changeOccurred();
}

void BookmarksManager::load()
{
    if (m_loaded)
        return;
    m_loaded = true;

    QString dir = BrowserApplication::dataLocation();
    QString bookmarkFile = dir + QLatin1String("/bookmarks.xbel");
    if (!QFile::exists(bookmarkFile))
#ifdef WINPE
		bookmarkFile = QLatin1String(":defaultbookmarks_winpe.xbel");
#else
		bookmarkFile = QLatin1String(":defaultbookmarks.xbel");
#endif

    XbelReader reader;
    m_bookmarkRootNode = reader.read(bookmarkFile);
    if (reader.error() != QXmlStreamReader::NoError) {
        QMessageBox::warning(0, tr("Loading Bookmark"),
            tr("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
    }

    BookmarkNode *toolbar = 0;
    BookmarkNode *menu = 0;
    QList<BookmarkNode*> others;
    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i) 
	{
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);
        if (node->type() == BookmarkNode::Folder) 
		{
            // Automatically convert
            if (node->title == ("Bookmarks Bar") && !toolbar) 
			{
                node->title = tr("Bookmarks Bar"); 
            }

            if (node->title == tr("Bookmarks Bar") && !toolbar) 
			{
                toolbar = node;
            }

            // Automatically convert
            if (node->title == ("Bookmarks Menu") && !menu) 
			{
                node->title = tr("Bookmarks Menu");
            }
            if (node->title == tr("Bookmarks Menu") && !menu) 
			{
                menu = node;
            }
        } 
		else 
		{
            others.append(node);
        }
	    m_bookmarkRootNode->remove(node);
    }
    
	Q_ASSERT(m_bookmarkRootNode->children().count() == 0);
    if (!toolbar) {
        toolbar = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        toolbar->title = tr("Bookmarks Bar"); 
    } else 
	{
        m_bookmarkRootNode->add(toolbar);
    }

    if (!menu) 
	{
        menu = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        menu->title = tr("Bookmarks Menu");
    } else 
	{
        m_bookmarkRootNode->add(menu);
    }
	
    for (int i = 0; i < others.count(); ++i)
        menu->add(others.at(i));
}

QString bookmarkWriteName( const BookmarkNode * node)
{
	if (!node)
		return "";

        if (node->type() == BookmarkNode::Folder)
	{
		BookmarksManager* bm = BrowserApplication::bookmarksManager();
		if (bm && bm->menu() == node)
			return "Bookmarks Menu";
		if (bm && bm->toolbar() == node)
			return "Bookmarks Bar";
	}

	return node->title;
}

void BookmarksManager::save() const
{
    if (!m_loaded)
        return;

    XbelWriter writer;
    QString dir = BrowserApplication::dataLocation();
    QString bookmarkFile = dir + QLatin1String("/bookmarks.xbel");
    if (!writer.write(bookmarkFile, m_bookmarkRootNode))
        qWarning() << "BookmarkManager: error saving to" << bookmarkFile;
}

void BookmarksManager::addBookmark(BookmarkNode *parent, BookmarkNode *node, int row)
{
    if (!m_loaded)
        return;
    Q_ASSERT(parent);
    InsertBookmarksCommand *command = new InsertBookmarksCommand(this, parent, node, row);
    m_commands.push(command);
}

void BookmarksManager::removeBookmark(BookmarkNode *node)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    BookmarkNode *parent = node->parent();
    int row = parent->children().indexOf(node);
    RemoveBookmarksCommand *command = new RemoveBookmarksCommand(this, parent, row);
    m_commands.push(command);
}

void BookmarksManager::setTitle(BookmarkNode *node, const QString &newTitle)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
        ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newTitle, changedTitle);
    m_commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode *node, const QString &newUrl)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newUrl, changedURL);
    m_commands.push(command);
}

void BookmarksManager::setTags(BookmarkNode *node, const QString &newTags)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newTags, changedTags);
    m_commands.push(command);
}

BookmarkNode *BookmarksManager::bookmarks()
{
    if (!m_loaded)
        load();
    return m_bookmarkRootNode;
}

BookmarkNode *BookmarksManager::menu()
{
    if (!m_loaded)
        load();

    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i) {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);
        if (node->title == tr("Bookmarks Menu"))
            return node;
    }
    Q_ASSERT(false);
    return 0;
}

BookmarkNode *BookmarksManager::toolbar()
{
    if (!m_loaded)
        load();

    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i) {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);
        if (node->title == tr("Bookmarks Bar")) 
            return node;
    }
    Q_ASSERT(false);
    return 0;
}
QStringList BookmarksManager::find_tag_urls(BookmarkNode *start_node, const QString& tag)
{
	if (!m_loaded)
        load();

    if (!start_node)
		start_node = m_bookmarkRootNode;

	QStringList tags;

	for (int i = start_node->children().count() - 1; i >= 0; --i) {
        BookmarkNode *node = start_node->children().at(i);
		if (node)
		{
			if (node->type() == BookmarkNode::Bookmark)
			{
				QStringList node_tags = node->tags.split(QRegExp("\\W+"), QString::SkipEmptyParts);
				foreach(QString node_tag, node_tags)
				{
					if (tag.toLower() == node_tag.toLower())
					{
						tags.append(node->url);
						break;
					}
				}
			}
			if (node->type() == BookmarkNode::Folder)
			{
				tags += find_tag_urls(node, tag);
			}
		}
    }

	return tags;
}

BookmarkNode *BookmarksManager::find_folder(BookmarkNode *start_node,const QString& title)
{
   if (!m_loaded)
        load();

    if (!start_node)
		start_node = m_bookmarkRootNode;

    for (int i = start_node->children().count() - 1; i >= 0; --i) {
        BookmarkNode *node = start_node->children().at(i);
		if (node && node->title == title && node->type() == BookmarkNode::Folder)
            return node;
		if (node && node->type() == BookmarkNode::Folder)
		{
			BookmarkNode *found_node = find_folder(node, title);
			if (found_node)
				return found_node;
		}
    }
    return 0;
}

BookmarksModel *BookmarksManager::bookmarksModel()
{
    if (!m_bookmarkModel)
        m_bookmarkModel = new BookmarksModel(this, this);
    return m_bookmarkModel;
}

void BookmarksManager::importFromIE()
{
    BookmarkNode *importRootNode = BookmarksImport::importFromIE();
	if (importRootNode)
	{
		importRootNode->setType(BookmarkNode::Folder);
		importRootNode->title = (tr("Imported %1 from Internet Explorer").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
		addBookmark(menu(), importRootNode);
		QMessageBox::information(0, tr("Importing Bookmarks"), 
			tr("Successfully imported Microsoft Internet Explorer favorites."));
	}
}

void BookmarksManager::importFromMozilla()
{
	QString path = BookmarksImport::mozillaPath();
	if (path.isEmpty())
	{
		QMessageBox::warning(0, tr("Importing Bookmarks"), 
			tr("Mozilla FireFox local profile location is not found.<br>Please find and import BOOKMARKS.HTML file manually."));
		return;
	}
    BookmarkNode *importRootNode = BookmarksImport::importFromHtml( path );
	if (importRootNode)
	{
		importRootNode->setType(BookmarkNode::Folder);
		importRootNode->title = (tr("Imported %1 from Mozilla FireFox").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
		addBookmark(menu(), importRootNode);
		QMessageBox::information(0, tr("Importing Bookmarks"), 
			tr("Successfully imported Mozilla FireFox bookmarks."));
	}
}

void BookmarksManager::importFromHTML()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"),
                                                     QString(),
                                                     tr("Netscape HTML Bookmarks (*.html;*.htm)"));
    if (fileName.isEmpty())
        return;

    BookmarkNode *importRootNode = BookmarksImport::importFromHtml( fileName );

	if (importRootNode)
	{
	    importRootNode->setType(BookmarkNode::Folder);
		importRootNode->title = (tr("Imported %1 from HTML").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
		addBookmark(menu(), importRootNode);
		QMessageBox::information(0, tr("Importing Bookmarks"), 
			tr("Successfully imported bookmarks from Netscape defined HTML file."));
	}
}

void BookmarksManager::importBookmarks()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"),
                                                     QString(),
                                                     tr("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelReader reader;
    BookmarkNode *importRootNode = reader.read(fileName);
    if (reader.error() != QXmlStreamReader::NoError) {
        QMessageBox::warning(0, tr("Loading Bookmark"),
            tr("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
    }

    importRootNode->setType(BookmarkNode::Folder);
    importRootNode->title = (tr("Imported %1").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
    addBookmark(menu(), importRootNode);
}

void BookmarksManager::exportBookmarks()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save File"),
                                QString("%1 Bookmarks.xbel").arg(QCoreApplication::applicationName()),
                                QString("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelWriter writer;
    if (!writer.write(fileName, m_bookmarkRootNode))
        QMessageBox::critical(0, tr("Export error"), tr("error saving bookmarks"));
}

RemoveBookmarksCommand::RemoveBookmarksCommand(BookmarksManager *m_bookmarkManagaer, BookmarkNode *parent, int row)
    : QUndoCommand(BookmarksManager::tr("Remove Bookmark"))
    , m_row(row)
    , m_bookmarkManagaer(m_bookmarkManagaer)
    , m_node(parent->children().value(row))
    , m_parent(parent)
    , m_done(false)
{
}

RemoveBookmarksCommand::~RemoveBookmarksCommand()
{
    if (m_done && !m_node->parent()) {
        delete m_node;
    }
}

void RemoveBookmarksCommand::undo()
{
    m_parent->add(m_node, m_row);
    emit m_bookmarkManagaer->entryAdded(m_node);
    m_done = false;
}

void RemoveBookmarksCommand::redo()
{
    m_parent->remove(m_node);
    emit m_bookmarkManagaer->entryRemoved(m_parent, m_row, m_node);
    m_done = true;
}

InsertBookmarksCommand::InsertBookmarksCommand(BookmarksManager *m_bookmarkManagaer,
                BookmarkNode *parent, BookmarkNode *node, int row)
    : RemoveBookmarksCommand(m_bookmarkManagaer, parent, row)
{
    setText(BookmarksManager::tr("Insert Bookmark"));
    m_node = node;
}

ChangeBookmarkCommand::ChangeBookmarkCommand(BookmarksManager *m_bookmarkManagaer, BookmarkNode *node,
                        const QString &newValue, ChangeBookmarkType type)
    : QUndoCommand()
    , m_bookmarkManagaer(m_bookmarkManagaer)
    , m_type(type)
    , m_newValue(newValue)
    , m_node(node)
{
    if (m_type == changedTitle) {
        m_oldValue = m_node->title;
        setText(BookmarksManager::tr("Name Change"));
    } 
        if (m_type == changedURL)
	{
        m_oldValue = m_node->url;
        setText(BookmarksManager::tr("Address Change"));
	}
	else {
        m_oldValue = m_node->tags;
        setText(BookmarksManager::tr("Tags Change"));
    }
}

void ChangeBookmarkCommand::undo()
{
    if (m_type == changedTitle)
        m_node->title = m_oldValue;
	else
    if (m_type == changedTags)
        m_node->tags = m_oldValue;
    else
        m_node->url = m_oldValue;
    emit m_bookmarkManagaer->entryChanged(m_node);
}

void ChangeBookmarkCommand::redo()
{
    if (m_type == changedTitle)
        m_node->title = m_newValue;
	else
    if (m_type == changedTags)
        m_node->tags = m_newValue;
    else
        m_node->url = m_newValue;
    emit m_bookmarkManagaer->entryChanged(m_node);
}

BookmarksModel::BookmarksModel(BookmarksManager *bookmarkManager, QObject *parent)
    : QAbstractItemModel(parent)
    , m_endMacro(false)
    , m_bookmarksManager(bookmarkManager)
{
    connect(bookmarkManager, SIGNAL(entryAdded(BookmarkNode *)),
            this, SLOT(entryAdded(BookmarkNode *)));
    connect(bookmarkManager, SIGNAL(entryRemoved(BookmarkNode *, int, BookmarkNode *)),
            this, SLOT(entryRemoved(BookmarkNode *, int, BookmarkNode *)));
    connect(bookmarkManager, SIGNAL(entryChanged(BookmarkNode *)),
            this, SLOT(entryChanged(BookmarkNode *)));
}

QModelIndex BookmarksModel::index(BookmarkNode *node) const
{
    BookmarkNode *parent = node->parent();
    if (!parent)
        return QModelIndex();
    return createIndex(parent->children().indexOf(node), 0, node);
}

void BookmarksModel::entryAdded(BookmarkNode *item)
{
    Q_ASSERT(item && item->parent());
    int row = item->parent()->children().indexOf(item);
    BookmarkNode *parent = item->parent();
    // item was already added so remove beore beginInsertRows is called
    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void BookmarksModel::entryRemoved(BookmarkNode *parent, int row, BookmarkNode *item)
{
    // item was already removed, re-add so beginRemoveRows works
    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void BookmarksModel::entryChanged(BookmarkNode *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

bool BookmarksModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    BookmarkNode *bookmarkNode = node(parent);
    for (int i = row + count - 1; i >= row; --i) {
        BookmarkNode *node = bookmarkNode->children().at(i);
        if (node == m_bookmarksManager->menu()
            || node == m_bookmarksManager->toolbar())
            continue;

        m_bookmarksManager->removeBookmark(node);
    }
    if (m_endMacro) {
        m_bookmarksManager->undoRedoStack()->endMacro();
        m_endMacro = false;
    }
    return true;
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Title");
            case 1: return tr("Address");
            case 2: return tr("Tag");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const BookmarkNode *bookmarkNode = node(index);
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (bookmarkNode->type() == BookmarkNode::Separator) {
            switch (index.column()) {
            case 0: return QString(50, 0xB7);
            case 1: return QString();
            case 2: return QString();
            }
        }

        switch (index.column()) {
        case 0: return bookmarkNode->title;
        case 1: return bookmarkNode->url;
        case 2: return bookmarkNode->tags;
        }
        break;
    case BookmarksModel::UrlRole:
        return QUrl(bookmarkNode->url);
        break;
    case BookmarksModel::UrlStringRole:
        return bookmarkNode->url;
        break;
    case BookmarksModel::TypeRole:
        return bookmarkNode->type();
        break;
    case BookmarksModel::SeparatorRole:
        return (bookmarkNode->type() == BookmarkNode::Separator);
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (bookmarkNode->type() == BookmarkNode::Folder)
                return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            return BrowserApplication::instance()->icon(bookmarkNode->url);
        }
    }

    return QVariant();
}

int BookmarksModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 3;
}

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_bookmarksManager->bookmarks()->children().count();

    const BookmarkNode *item = static_cast<BookmarkNode*>(parent.internalPointer());
    return item->children().count();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    BookmarkNode *parentNode = node(parent);
    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex BookmarksModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    BookmarkNode *itemNode = node(index);
    BookmarkNode *parentNode = (itemNode ? itemNode->parent() : 0);
    if (!parentNode || parentNode == m_bookmarksManager->bookmarks())
        return QModelIndex();

    // get the parent's row
    BookmarkNode *grandParentNode = parentNode->parent();
    int parentRow = grandParentNode->children().indexOf(parentNode);
    Q_ASSERT(parentRow >= 0);
    return createIndex(parentRow, 0, parentNode);
}

bool BookmarksModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    const BookmarkNode *parentNode = node(parent);
    return (parentNode->type() == BookmarkNode::Folder);
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    BookmarkNode *bookmarkNode = node(index);

    if (bookmarkNode != m_bookmarksManager->menu()
        && bookmarkNode != m_bookmarksManager->toolbar()) {
        flags |= Qt::ItemIsDragEnabled;
        if (bookmarkNode->type() != BookmarkNode::Separator)
            flags |= Qt::ItemIsEditable;
    }
    if (hasChildren(index))
        flags |= Qt::ItemIsDropEnabled;
    return flags;
}

Qt::DropActions BookmarksModel::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}

#define MIMETYPE QLatin1String("application/bookmarks.xbel")

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types << MIMETYPE;
    return types;
}

QMimeData *BookmarksModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    foreach (QModelIndex index, indexes) {
        if (index.column() != 0 || !index.isValid())
            continue;
        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        XbelWriter writer;
        const BookmarkNode *parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }
    mimeData->setData(MIMETYPE, data);
    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData *data,
     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(MIMETYPE)
        || column > 0)
        return false;

    QByteArray ba = data->data(MIMETYPE);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    QUndoStack *undoStack = m_bookmarksManager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Bookmarks"));

    while (!stream.atEnd()) {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        XbelReader reader;
        BookmarkNode *rootNode = reader.read(&buffer);
        QList<BookmarkNode*> children = rootNode->children();
        for (int i = 0; i < children.count(); ++i) {
            BookmarkNode *bookmarkNode = children.at(i);
            rootNode->remove(bookmarkNode);
            row = qMax(0, row);
            BookmarkNode *parentNode = node(parent);
            m_bookmarksManager->addBookmark(parentNode, bookmarkNode, row);
            m_endMacro = true;
        }
        delete rootNode;
    }
    return true;
}

bool BookmarksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
        return false;

    BookmarkNode *item = node(index);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.column() == 0) {
            m_bookmarksManager->setTitle(item, value.toString());
            break;
        }
        if (index.column() == 1) {
            m_bookmarksManager->setUrl(item, value.toString());
            break;
        }
        if (index.column() == 2) {
            m_bookmarksManager->setTags(item, value.toString());
            break;
        }
        return false;
    case BookmarksModel::UrlRole:
        m_bookmarksManager->setUrl(item, value.toUrl().toString());
        break;
    case BookmarksModel::UrlStringRole:
        m_bookmarksManager->setUrl(item, value.toString());
        break;
    default:
        break;
        return false;
    }

    return true;
}

BookmarkNode *BookmarksModel::node(const QModelIndex &index) const
{
    BookmarkNode *itemNode = static_cast<BookmarkNode*>(index.internalPointer());
    if (!itemNode)
        return m_bookmarksManager->bookmarks();
    return itemNode;
}


AddBookmarkProxyModel::AddBookmarkProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

int AddBookmarkProxyModel::columnCount(const QModelIndex &parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddBookmarkProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->hasChildren(idx);
}

AddBookmarkDialog::AddBookmarkDialog(const QString &url, const QString &title, const QString& default_folder, QWidget *parent, BookmarksManager *bookmarkManager)
    : QDialog(parent)
    , m_url(url)
	, m_default_folder(default_folder)
    , m_bookmarksManager(bookmarkManager)
{
    setWindowFlags(Qt::Sheet);
    if (!m_bookmarksManager)
        m_bookmarksManager = BrowserApplication::bookmarksManager();
    setupUi(this);
    QTreeView *view = new QTreeView(this);
    m_proxyModel = new AddBookmarkProxyModel(this);
    BookmarksModel *model = m_bookmarksManager->bookmarksModel();
    m_proxyModel->setSourceModel(model);
    view->setModel(m_proxyModel);
    view->expandAll();
    view->header()->setStretchLastSection(true);
    view->header()->hide();
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setIndentation(10);
    location->setModel(m_proxyModel);
    view->show();
    location->setView(view);
    BookmarkNode *menu = m_bookmarksManager->toolbar();
    QModelIndex idx = m_proxyModel->mapFromSource(model->index(menu));
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	if (!m_default_folder.isEmpty())
	{
		BookmarkNode* folder = m_bookmarksManager->find_folder(NULL, m_default_folder);
		if (folder)
		{
			QModelIndex ix = m_proxyModel->mapFromSource(model->index(folder));
			if (ix.isValid())
				idx = ix;
		}
		else
		{
			settings.setValue(QLatin1String("addBookmarkDefaultLocation"), "");
			settings.setValue(QLatin1String("addBookmarkUseDefaultLocation"), false);
		}
	}
    view->setCurrentIndex(idx);
    location->setCurrentIndex(idx.row());
    name->setText(title);
	chkSetDefault->setChecked(settings.value(QLatin1String("addBookmarkUseDefaultLocation"), false).toBool());
}

void AddBookmarkDialog::accept()
{
    QModelIndex index = location->view()->currentIndex();
    index = m_proxyModel->mapToSource(index);
    if (!index.isValid())
        index = m_bookmarksManager->bookmarksModel()->index(0, 0);
    BookmarkNode *parent = m_bookmarksManager->bookmarksModel()->node(index);

    for (int i = parent->children().count() - 1; i >= 0; --i) 
	{
        BookmarkNode *node = parent->children().at(i);
		if (node && (node->title == name->text() || node->url == m_url) )
		{
			QMessageBox::StandardButton button = QMessageBox::information(this, tr("Duplicate bookmark"), 
				tr("Bookmark having the same title or url already exists in your folder.<br>Are you sure to add another one?"),
				QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
			if (button == QMessageBox::Cancel) 
				return;
		}
    }

	BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark);
    bookmark->url = m_url;
    bookmark->title = name->text();

    m_bookmarksManager->addBookmark(parent, bookmark);

	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("addBookmarkUseDefaultLocation"), chkSetDefault->isChecked());
	settings.setValue(QLatin1String("addBookmarkDefaultLocation"), parent->title);

    QDialog::accept();
}

void AddBookmarkDialog::acceptDefaultLocation()
{
	accept();
}

BookmarksMenu::BookmarksMenu(QWidget *parent)
    : ModelMenu(parent)
    , m_bookmarksManager(0)
{
    connect(this, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(activated(const QModelIndex &)));
    setMaxRows(-1);
    setHoverRole(BookmarksModel::UrlStringRole);
    setSeparatorRole(BookmarksModel::SeparatorRole);
}

void BookmarksMenu::activated(const QModelIndex &index)
{
    emit openUrl(index.data(BookmarksModel::UrlRole).toUrl());
}

bool BookmarksMenu::prePopulated()
{
    m_bookmarksManager = BrowserApplication::bookmarksManager();
    setModel(m_bookmarksManager->bookmarksModel());
    setRootIndex(m_bookmarksManager->bookmarksModel()->index(1, 0));
    // initial actions
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
    if (!m_initialActions.isEmpty())
        addSeparator();
    createMenu(model()->index(0, 0), 1, this);
    return true;
}

void BookmarksMenu::setInitialActions(QList<QAction*> actions)
{
    m_initialActions = actions;
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
}

BookmarksDialog::BookmarksDialog(QWidget *parent, BookmarksManager *manager)
    : QDialog(parent)
{
    m_bookmarksManager = manager;
    if (!m_bookmarksManager)
        m_bookmarksManager = BrowserApplication::bookmarksManager();
    setupUi(this);

    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    tree->setTextElideMode(Qt::ElideMiddle);
    m_bookmarksModel = m_bookmarksManager->bookmarksModel();
    m_proxyModel = new TreeProxyModel(this);
	m_proxyModel->setSortRole(Qt::DisplayRole);

    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), tree, SLOT(removeOne()));
    m_proxyModel->setSourceModel(m_bookmarksModel);
    tree->setModel(m_proxyModel);
    tree->setDragDropMode(QAbstractItemView::InternalMove);
    tree->setExpanded(m_proxyModel->index(0, 0), true);
    tree->setAlternatingRowColors(true);
    QFontMetrics fm(font());
    int header = fm.width(QLatin1Char('m')) * 35;
    tree->header()->resizeSection(0, header);
    header = fm.width(QLatin1Char('m')) * 35;
    tree->header()->resizeSection(1, header);
    header = fm.width(QLatin1Char('m')) * 5;
    tree->header()->resizeSection(2, header);
    tree->header()->setStretchLastSection(true);
    connect(tree, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(open()));
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuRequested(const QPoint &)));
    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(newFolder()));
    connect(buttonSort, SIGNAL(clicked()),
            this, SLOT(sortBookmarks()));
    expandNodes(m_bookmarksManager->bookmarks());
    setAttribute(Qt::WA_DeleteOnClose);

	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	checkOneClickEnabled->setChecked(settings.value(QLatin1String("addBookmarkUseDefaultLocation"), false).toBool());
}

BookmarksDialog::~BookmarksDialog()
{
	QSettings settings;
	settings.beginGroup(QLatin1String("websettings"));
	settings.setValue(QLatin1String("addBookmarkUseDefaultLocation"), checkOneClickEnabled->isChecked());

    if (saveExpandedNodes(tree->rootIndex()))
        m_bookmarksManager->changeExpanded();
}

bool BookmarksDialog::saveExpandedNodes(const QModelIndex &parent)
{
    bool changed = false;
    for (int i = 0; i < m_proxyModel->rowCount(parent); ++i) {
        QModelIndex child = m_proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex = m_proxyModel->mapToSource(child);
        BookmarkNode *childNode = m_bookmarksModel->node(sourceIndex);
        bool wasExpanded = childNode->expanded;
        if (tree->isExpanded(child)) {
            childNode->expanded = true;
            changed |= saveExpandedNodes(child);
        } else {
            childNode->expanded = false;
        }
        changed |= (wasExpanded != childNode->expanded);
    }
    return changed;
}

void BookmarksDialog::expandNodes(BookmarkNode *node)
{
    for (int i = 0; i < node->children().count(); ++i) {
        BookmarkNode *childNode = node->children()[i];
        if (childNode->expanded) {
            QModelIndex idx = m_bookmarksModel->index(childNode);
            idx = m_proxyModel->mapFromSource(idx);
            tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void BookmarksDialog::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QModelIndex index = tree->indexAt(pos);
    index = index.sibling(index.row(), 0);
    if (index.isValid() && !tree->model()->hasChildren(index)) {
        menu.addAction(tr("Open"), this, SLOT(open()));
        menu.addSeparator();
    }
    menu.addAction(tr("Delete"), tree, SLOT(removeOne()));
    menu.exec(QCursor::pos());
}

void BookmarksDialog::open()
{
    QModelIndex index = tree->currentIndex();
    if (!index.parent().isValid())
        return;
    emit openUrl(index.sibling(index.row(), 1).data(BookmarksModel::UrlRole).toUrl());
}

void BookmarksDialog::sortBookmarks()
{
	static bool order = false;
	tree->setSortingEnabled ( true );
	tree->sortByColumn(0, order ? Qt::DescendingOrder : Qt::AscendingOrder );
	order = !order;
}

void BookmarksDialog::newFolder()
{
    QModelIndex currentIndex = tree->currentIndex();
    QModelIndex idx = currentIndex;
    if (idx.isValid() && !idx.model()->hasChildren(idx))
        idx = idx.parent();
    if (!idx.isValid())
	{
		QModelIndex topmenu = m_bookmarksModel->index( m_bookmarksManager->menu() );
        idx = topmenu;
	}
	else
		idx = m_proxyModel->mapToSource(idx);
    BookmarkNode *parent = m_bookmarksManager->bookmarksModel()->node(idx);
    BookmarkNode *node = new BookmarkNode(BookmarkNode::Folder);
    node->title = tr("New Folder");
    m_bookmarksManager->addBookmark(parent, node, currentIndex.row() + 1);
}

BookmarksToolBar::BookmarksToolBar(BookmarksModel *model, QWidget *parent)
    : QToolBar(tr("Bookmarks Bar"), parent)
    , m_bookmarksModel(model)
{
	setObjectName("Bookmarks Bar");
    connect(this, SIGNAL(actionTriggered(QAction*)), this, SLOT(triggered(QAction*)));
    setRootIndex(model->index(0, 0));
    connect(m_bookmarksModel, SIGNAL(modelReset()), this, SLOT(build()));
    connect(m_bookmarksModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(build()));
    connect(m_bookmarksModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(build()));
    connect(m_bookmarksModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(build()));
    setAcceptDrops(true);
	setIconSize( QSize(16,16));
}

void BookmarksToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
        event->acceptProposedAction();
    QToolBar::dragEnterEvent(event);
}

void BookmarksToolBar::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
	{	
		QList<QUrl> urls = mimeData->urls();
		int row = -1;
		BookmarkNode * del_node = NULL;
		QModelIndex parentIndex = m_root;
		if( mimeData->hasText()) 
		{
			QAction *action = actionAt(event->pos());
			QString dropText;
			if (action)
				dropText = action->text();

			if (dropText.isEmpty())
			{
				QWidget* wid = widgetForAction ( action );
				if (wid)
					dropText = ((QToolButton*)wid)->text();
			}
		
			for (int i = 0; i < m_bookmarksModel->rowCount(m_root); ++i) 
			{
				QModelIndex idx = m_bookmarksModel->index(i, 0, m_root);
				QString title = idx.data().toString();
				if (title == dropText) 
				{
					row = i;
					if (m_bookmarksModel->hasChildren(idx)) 
					{
						parentIndex = idx;
						row = -1;
					}
				}
				if (mimeData->text() == title)
				{
					del_node = m_bookmarksModel->node(idx);
				}
			}
		}
        BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark);
        bookmark->url = urls.at(0).toString();
		bookmark->title = mimeData->hasText() ? mimeData->text() : urls.at(0).toString();

        BookmarksManager *bookmarksManager = m_bookmarksModel->bookmarksManager();
        BookmarkNode *parent = m_bookmarksModel->node(parentIndex);

		if (del_node )
		{
			bookmarksManager->removeBookmark(del_node);
		}

        bookmarksManager->addBookmark(parent, bookmark, row);

        event->acceptProposedAction();
    }
    QToolBar::dropEvent(event);
}


void BookmarksToolBar::setRootIndex(const QModelIndex &index)
{
    m_root = index;
    build();
}

QModelIndex BookmarksToolBar::rootIndex() const
{
    return m_root;
}

void BookmarksToolBar::build()
{
    clear();
    for (int i = 0; i < m_bookmarksModel->rowCount(m_root); ++i) {
        QModelIndex idx = m_bookmarksModel->index(i, 0, m_root);
        if (m_bookmarksModel->hasChildren(idx)) {
            QToolButton *button = new QToolButton(this);
            button->setPopupMode(QToolButton::InstantPopup);
            button->setArrowType(Qt::DownArrow);
            button->setText(idx.data().toString());
            ModelMenu *menu = new ModelMenu(this);
            connect(menu, SIGNAL(activated(const QModelIndex &)),
                    this, SLOT(activated(const QModelIndex &)));
            menu->setModel(m_bookmarksModel);
            menu->setRootIndex(idx);
			menu->setMaxRows(100);
            menu->addAction(new QAction(menu));
            button->setMenu(menu);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            QAction *a = addWidget(button);
            a->setText(idx.data().toString());
        } else {
            //QAction *action = addAction(idx.data().toString());
            //action->setData(idx.data(BookmarksModel::UrlRole));
			QUrl url = idx.data(BookmarksModel::UrlRole).toUrl();
            BookmarkToolButton *button = new BookmarkToolButton(url, this);
            button->setText(idx.data().toString());
            connect(button, SIGNAL(openBookmark(const QUrl &, TabWidget::Tab, const QString &)),
                    this, SIGNAL(openUrl(const QUrl &, TabWidget::Tab, const QString &)));
		    connect(button, SIGNAL(deleteBookmark(const QUrl &, const QString & )),
					this, SLOT(deleteBookmark(const QUrl &, const QString &)));
		    connect(button, SIGNAL(renameBookmark(const QUrl &, const QString &, const QString & )),
					this, SLOT(renameBookmark(const QUrl &, const QString &, const QString &)));
		    connect(button, SIGNAL(addFolder(const QString &, const QString & )),
					this, SLOT(addFolder(const QString &, const QString &)));

            addWidget(button);
        }
    }
}

void BookmarksToolBar::deleteBookmark(const QUrl &url, const QString &title)
{
	for (int i = 0; i < m_bookmarksModel->rowCount(m_root); ++i) 
	{
		QModelIndex idx = m_bookmarksModel->index(i, 0, m_root);
		QString t = idx.data().toString();
		if ( title == t )
		{
			BookmarkNode* node = m_bookmarksModel->node(idx);
			BookmarksManager *bookmarksManager = m_bookmarksModel->bookmarksManager();
			if (node && bookmarksManager)
			{
				bookmarksManager->removeBookmark(node);
				break;
			}
		}
	}
}

void BookmarksToolBar::renameBookmark(const QUrl &url, const QString &title, const QString &new_title)
{
	for (int i = 0; i < m_bookmarksModel->rowCount(m_root); ++i) 
	{
		QModelIndex idx = m_bookmarksModel->index(i, 0, m_root);
		QString t = idx.data().toString();
		if ( title == t )
		{
			BookmarkNode* node = m_bookmarksModel->node(idx);
			BookmarksManager *bookmarksManager = m_bookmarksModel->bookmarksManager();
			if (node && bookmarksManager)
			{
				bookmarksManager->setTitle(node, new_title);
				break;
			}
		}
	}
}

void BookmarksToolBar::addFolder(const QString &title, const QString &new_folder)
{
	for (int i = 0; i < m_bookmarksModel->rowCount(m_root); ++i) 
	{
		QModelIndex idx = m_bookmarksModel->index(i, 0, m_root);
		QString t = idx.data().toString();
		if ( title == t )
		{
			BookmarkNode* node = m_bookmarksModel->node(idx);
			BookmarksManager *bookmarksManager = m_bookmarksModel->bookmarksManager();
			if (node && bookmarksManager)
			{
				BookmarkNode *f = new BookmarkNode(BookmarkNode::Folder, bookmarksManager->toolbar());
				f->title = new_folder;
				bookmarksManager->addBookmark(bookmarksManager->toolbar(), f, i);
				break;
			}
		}
	}
}

void BookmarksToolBar::triggered(QAction *action)
{
    QVariant v = action->data();
    if (v.canConvert<QUrl>()) {
        emit openUrl(v.toUrl(), TabWidget::CurrentTab, tr("Loading..."));
    }
}

void BookmarksToolBar::activated(const QModelIndex &index)
{
	QUrl url = index.data(BookmarksModel::UrlRole).toUrl();
        emit openUrl(url, TabWidget::CurrentTab, tr("Loading..."));
}




BookmarkToolButton::BookmarkToolButton(QUrl url, QWidget *parent)
    : QToolButton(parent)
    , m_url(url)
	, dragStartPosition(0,0)
{
	QIcon icon(QLatin1String(":/add2.png"));
	
	icon = BrowserApplication::instance()->icon(url);
	
	setIcon(icon);
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setAcceptDrops(false);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(contextMenuRequested(const QPoint &)));

}

void BookmarkToolButton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
	QToolButton::mousePressEvent(event);
}

void BookmarkToolButton::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
         return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
         return;

	 QDrag *drag = new QDrag(this);
     QMimeData *mimeData = new QMimeData;

	 QList<QUrl> lst;
	 lst << m_url;
     mimeData->setUrls( lst );
	 mimeData->setText( text() );
	 mimeData->setProperty( "move", QVariant(TRUE));
     drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
	QToolButton::mousePressEvent(event);
}


void BookmarkToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    QToolButton::mouseReleaseEvent(event);
    if (hitButton(event->pos())) {
        if (event->button() == Qt::MidButton)
            emit openBookmark(url(), TabWidget::NewTab, text());
        if (event->button() == Qt::LeftButton) {
            TabWidget::Tab openLocation =
                (event->modifiers() & Qt::ControlModifier)
                ? TabWidget::NewTab
                : TabWidget::CurrentTab;
            emit openBookmark(url(), openLocation, text());
        }
    }
}

QUrl BookmarkToolButton::url() const
{
    return m_url;
}

void BookmarkToolButton::contextMenuRequested(const QPoint &pt)
{
    QMenu menu;
    menu.addAction(tr("Open"), this, SLOT(openBookmark()));
    menu.addAction(tr("Open New Tab"), this, SLOT(openBookmarkNewTab()));
    menu.addSeparator();
    menu.addAction(tr("Add Folder"), this, SLOT(addFolder()));
    menu.addSeparator();
    menu.addAction(tr("Rename"), this, SLOT(renameBookmark()));
    menu.addAction(tr("Delete"), this, SLOT(deleteBookmark()));
    menu.exec(QCursor::pos());
}

void BookmarkToolButton::deleteBookmark()
{
    emit deleteBookmark(url(), text());
}

void BookmarkToolButton::openBookmark()
{
    emit openBookmark(url(), TabWidget::CurrentTab, text());
}

void BookmarkToolButton::openBookmarkNewTab()
{
	emit openBookmark(url(), TabWidget::NewTab, text());
}

void BookmarkToolButton::renameBookmark()
{
	QString txt = QInputDialog::getText(this, tr("Rename bookmark"), tr("Type a new bookmark title:"), QLineEdit::Normal, text());
	if (txt.length() > 0)
	{
		emit renameBookmark(url(), text(), txt);
	}
}

void BookmarkToolButton::addFolder()
{
	QString txt = QInputDialog::getText(this, tr("New Folder"), tr("Type a new folder name:"), QLineEdit::Normal);
	if (txt.length() > 0)
	{
		emit addFolder(text(), txt);
	}
}
