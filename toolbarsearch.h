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

#ifndef TOOLBARSEARCH_H
#define TOOLBARSEARCH_H

#include "searchlineedit.h"

QT_BEGIN_NAMESPACE
class QUrl;
class QAction;
class QStringListModel;
QT_END_NAMESPACE

class AutoSaver;
class GSuggestCompletion;

class ToolbarSearch : public SearchLineEdit
{
    Q_OBJECT

signals:
    void search(const QUrl &url);

public:
    ToolbarSearch(QWidget *parent = 0);
    ~ToolbarSearch();

public slots:
    void clear();
    void searchNow();
    void useGoogle();
    void useBing();
    void useYahoo();
    void useCuil();

	void addSearch();
    void useSearch();

private slots:
    void save();
    void aboutToShowMenu();
    void triggeredMenuAction(QAction *action);

private:
    void load();
	void addSearches();
	void checkGoogleSuggest(bool google);

    AutoSaver *m_autosaver;
    int m_maxSavedSearches;
    QStringListModel *m_stringListModel;
    GSuggestCompletion *m_completer;

};

#endif // TOOLBARSEARCH_H

