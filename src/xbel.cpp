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
** Copyright (C) 2005-2008 Trolltech ASA. All rights reserved.
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

#include "xbel.h"

#include <QtCore/QFile>

BookmarkNode::BookmarkNode(BookmarkNode::Type type, BookmarkNode *parent) :
     expanded(false)
   , m_parent(parent)
   , m_type(type)
{
    if (parent)
        parent->add(this);
}

BookmarkNode::~BookmarkNode()
{
    if (m_parent)
        m_parent->remove(this);
    qDeleteAll(m_children);
    m_parent = 0;
    m_type = BookmarkNode::Root;
}

bool BookmarkNode::operator==(const BookmarkNode &other)
{
    if (url != other.url
        || title != other.title
        || desc != other.desc
        || expanded != other.expanded
        || m_type != other.m_type
        || m_children.count() != other.m_children.count()
		|| tags != other.tags)
        return false;

    for (int i = 0; i < m_children.count(); ++i)
        if (!((*(m_children[i])) == (*(other.m_children[i]))))
            return false;

    return true;
}

BookmarkNode::Type BookmarkNode::type() const
{
    return m_type;
}

void BookmarkNode::setType(Type type)
{
    m_type = type;
}

QList<BookmarkNode *> BookmarkNode::children() const
{
    return m_children;
}

BookmarkNode *BookmarkNode::parent() const
{
    return m_parent;
}

void BookmarkNode::add(BookmarkNode *child, int offset)
{
    Q_ASSERT(child->m_type != Root);
    if (child->m_parent)
        child->m_parent->remove(child);
    child->m_parent = this;
    if (-1 == offset)
        offset = m_children.size();
    m_children.insert(offset, child);
}

void BookmarkNode::remove(BookmarkNode *child)
{
    child->m_parent = 0;
    m_children.removeAll(child);
}


XbelReader::XbelReader()
{
}

BookmarkNode *XbelReader::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        return new BookmarkNode(BookmarkNode::Root);
    }
    file.open(QFile::ReadOnly);
    return read(&file);
}

BookmarkNode *XbelReader::read(QIODevice *device)
{
    BookmarkNode *root = new BookmarkNode(BookmarkNode::Root);
    setDevice(device);
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            QString version = attributes().value(QLatin1String("version")).toString();
            if (name() == QLatin1String("xbel")
                && (version.isEmpty() || version == QLatin1String("1.0"))) {
                readXBEL(root);
            } else {
                raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
            }
        }
    }
    return root;
}

void XbelReader::readXBEL(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("xbel"));

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("folder"))
                readFolder(parent);
            else if (name() == QLatin1String("bookmark"))
                readBookmarkNode(parent);
            else if (name() == QLatin1String("separator"))
                readSeparator(parent);
            else
                skipUnknownElement();
        }
    }
}

void XbelReader::readFolder(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("folder"));

    BookmarkNode *folder = new BookmarkNode(BookmarkNode::Folder, parent);
    folder->expanded = (attributes().value(QLatin1String("folded")) == QLatin1String("no"));

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("title"))
                readTitle(folder);
            else if (name() == QLatin1String("desc"))
                readDescription(folder);
            else if (name() == QLatin1String("folder"))
                readFolder(folder);
            else if (name() == QLatin1String("bookmark"))
                readBookmarkNode(folder);
            else if (name() == QLatin1String("separator"))
                readSeparator(folder);
            else
                skipUnknownElement();
        }
    }
}

void XbelReader::readTitle(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("title"));
    parent->title = readElementText();
}

void XbelReader::readDescription(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("desc"));
    parent->desc = readElementText();
}

void XbelReader::readInfo(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("info"));
    while (!atEnd()) 
	{
        readNext();
        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("metadata"))
			{
				parent->tags = attributes().value(QLatin1String("ShortcutURL")).toString();
				readElementText();
			}
            else
                skipUnknownElement();
        }
    }
	//parent->tags  = readElementText();
}

void XbelReader::readSeparator(BookmarkNode *parent)
{
    new BookmarkNode(BookmarkNode::Separator, parent);
    // empty elements have a start and end element
    readNext();
}

void XbelReader::readBookmarkNode(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("bookmark"));
    BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark, parent);
    bookmark->url = attributes().value(QLatin1String("href")).toString();
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("title"))
                readTitle(bookmark);
            else if (name() == QLatin1String("desc"))
                readDescription(bookmark);
            else if (name() == QLatin1String("info"))
                readInfo(bookmark);
            else
                skipUnknownElement();
        }
    }
    if (bookmark->title.isEmpty())
        bookmark->title = QObject::tr("Unknown title");
}

void XbelReader::skipUnknownElement()
{
    Q_ASSERT(isStartElement());

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            skipUnknownElement();
    }
}


XbelWriter::XbelWriter()
{
    setAutoFormatting(true);
}

bool XbelWriter::write(const QString &fileName, const BookmarkNode *root)
{
    QFile file(fileName);
    if (!root || !file.open(QFile::WriteOnly))
        return false;
    return write(&file, root);
}

bool XbelWriter::write(QIODevice *device, const BookmarkNode *root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    if (root->type() == BookmarkNode::Root) {
        for (int i = 0; i < root->children().count(); ++i)
            writeItem(root->children().at(i));
    } else {
        writeItem(root);
    }

    writeEndDocument();
    return true;
}

extern QString bookmarkWriteName( const BookmarkNode * );

void XbelWriter::writeItem(const BookmarkNode *parent)
{
    switch (parent->type()) {
    case BookmarkNode::Folder:
        writeStartElement(QLatin1String("folder"));
        writeAttribute(QLatin1String("folded"), parent->expanded ? QLatin1String("no") : QLatin1String("yes"));
        writeTextElement(QLatin1String("title"), bookmarkWriteName(parent));
        for (int i = 0; i < parent->children().count(); ++i)
            writeItem(parent->children().at(i));
        writeEndElement();
        break;
    case BookmarkNode::Bookmark:
        writeStartElement(QLatin1String("bookmark"));
        if (!parent->url.isEmpty())
            writeAttribute(QLatin1String("href"), parent->url);
        writeTextElement(QLatin1String("title"), parent->title);
        if (!parent->desc.isEmpty())
            writeAttribute(QLatin1String("desc"), parent->desc);
        if (!parent->tags.isEmpty())
		{
			writeStartElement(QLatin1String("info"));
			writeStartElement(QLatin1String("metadata"));
            writeAttribute(QLatin1String("owner"), QLatin1String("Mozilla"));
            writeAttribute(QLatin1String("ShortcutURL"), parent->tags);
			writeEndElement();
			writeEndElement();
		}
		writeEndElement();
        break;
    case BookmarkNode::Separator:
        writeEmptyElement(QLatin1String("separator"));
        break;
    default:
        break;
    }
}

