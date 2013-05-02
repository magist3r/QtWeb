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
#include "closeapp.h"
#include "qsettings.h"

CloseApp::CloseApp(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect((const QObject*)((ui.buttonBox)->button(QDialogButtonBox::SaveAll)), SIGNAL(clicked()), this, SLOT(saveAll()));
	connect((const QObject*)((ui.buttonBox)->button(QDialogButtonBox::Close)), SIGNAL(clicked()), this, SLOT(closeAll()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

}

CloseApp::~CloseApp()
{
}

void CloseApp::closeAll()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));

	if (ui.checkBoxDontAsk->isChecked())
	{
	    settings.setValue(QLatin1String("quitDontAsk"), true);
	}

    settings.endGroup();

	accept();
}

void CloseApp::saveAll()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    settings.setValue(QLatin1String("onStartup"), 2);

	if (ui.checkBoxDontAsk->isChecked())
	{
	    settings.setValue(QLatin1String("quitDontAsk"), true);
	}

    settings.endGroup();

	accept();
}