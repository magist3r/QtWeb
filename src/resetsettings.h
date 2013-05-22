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
#ifndef RESETSETTINGS_H
#define RESETSETTINGS_H

#include <QDialog>
#include "ui_resetsettings.h"

class ToolbarSearch;

class ResetSettings : public QDialog, public Ui_ResetSettings
{
	Q_OBJECT

private:
	ToolbarSearch* m_toolbarSearch;

public:
	ResetSettings(ToolbarSearch*, QWidget *parent = 0);
	~ResetSettings();

public slots:
    void accept();
};

#endif // RESETSETTINGS_H
