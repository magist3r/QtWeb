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

#include "shortcuts.h"
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QHeaderView>
#include <QSettings>
#include <QMessageBox>


Shortcuts::Shortcuts(QWidget *parent)
	: QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	//ShortcutsTable->verticalHeader()->hide();
	ShortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ShortcutsTable->setAlternatingRowColors(true);
    ShortcutsTable->setTextElideMode(Qt::ElideMiddle);
    ShortcutsTable->setShowGrid(false);
    ShortcutsTable->setSortingEnabled(false);
    //ShortcutsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
	
	ShortcutsTable->setRowCount( m_data.GetCommandsCount() );
	ShortcutsTable->setColumnCount(2);

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height()/3;
    ShortcutsTable->verticalHeader()->setDefaultSectionSize(height);
    ShortcutsTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < 2; ++i){
        int header = ShortcutsTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QString(30, 'X'));
            break;
        case 1:
            header = fm.width(QString(40, 'X'));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        ShortcutsTable->horizontalHeader()->resizeSection(i, header);
    }

    ShortcutsTable->horizontalHeader()->setStretchLastSection(true);

    ShortcutsTable->setEditTriggers(QAbstractItemView::DoubleClicked
                                | QAbstractItemView::SelectedClicked);
    ShortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

	QStringList headerLabels;
    headerLabels << tr("Menu Command") << tr("Assigned Shortcut(s)");
    ShortcutsTable->setHorizontalHeaderLabels(headerLabels);

	for (int i = 0; i < m_data.GetCommandsCount(); i++)
	{
		ShortcutsTable->setItem(i, 0, new QTableWidgetItem(m_data.GetTitle(i)));
		ShortcutsTable->setItem(i, 1, new QTableWidgetItem(m_data.GetShortcutsString(i)));
	}
}

Shortcuts::~Shortcuts()
{
}

bool Shortcuts::save()
{
	for (int i = 0; i < m_data.GetCommandsCount(); i++)
	{
		QTableWidgetItem* menu = ShortcutsTable->item(i, 0);
		if ( m_data.GetTitle(i) != menu->text().trimmed())
		{	
			m_data.SetTitle(i, menu->text().trimmed());
		}

		QTableWidgetItem* shorts = ShortcutsTable->item(i, 1);
		if ( m_data.GetShortcutsString(i) != shorts->text().trimmed())
		{
			if (!m_data.SetShortcuts(i, shorts->text().trimmed()))
			{
		        QMessageBox::warning(0, tr("Invalid shortcut assigned"),
					tr("Error assigning shortcut(s) for the menu command: %1.\n\nPlease check the shortcut(s): %2"
               ).arg(menu->text()).arg(shorts->text().trimmed()));
				return false;
			}
		}

	}

	return true;
}

void Shortcuts::accept()
{
    if (save())
	    QDialog::accept();
}

void Shortcuts::reject()
{
    QDialog::reject();
}
