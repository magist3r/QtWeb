/*
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
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
#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QDialog>
#include <QSettings>
#include "ui_shortcuts.h"
#include "commands.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
QT_END_NAMESPACE

class Shortcuts : public QDialog, public Ui_ShortcutsDialog
{
	Q_OBJECT

public:
	Shortcuts(QWidget *parent);
	~Shortcuts();
	void accept();
    void reject();

private:
	bool save();

	MenuCommands	m_data;
};

#endif // SHORTCUTS_H
