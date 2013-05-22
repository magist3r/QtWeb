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

#include "passwords.h"
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QHeaderView>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>

static bool s_show_passwords = false;

extern QString DecryptPassword(QString str);
extern QString EncryptPassword(QString str);


Passwords::Passwords(QWidget *parent)
	: QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);

	s_show_passwords = false;

	PasswordsModel *model = new PasswordsModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), PasswordsTable, SLOT(removeOne()));
    connect(removeAllButton, SIGNAL(clicked()), PasswordsTable, SLOT(removeAll()));
    connect(buttonShowPasswords, SIGNAL(clicked()), this, SLOT(showPasswords()));
    connect(buttonMaster, SIGNAL(clicked()), this, SLOT(masterPassword()));
    m_proxyModel->setSourceModel(model);
    PasswordsTable->verticalHeader()->hide();
    PasswordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    PasswordsTable->setModel(m_proxyModel);
    PasswordsTable->setAlternatingRowColors(true);
    PasswordsTable->setTextElideMode(Qt::ElideMiddle);
    PasswordsTable->setShowGrid(false);
    PasswordsTable->setSortingEnabled(true);
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height()/3;
    PasswordsTable->verticalHeader()->setDefaultSectionSize(height);
    PasswordsTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < model->columnCount(); ++i){
        int header = PasswordsTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QString(40, 'X'));
            break;
        case 1:
            header = fm.width(QString(12, 'X'));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        PasswordsTable->horizontalHeader()->resizeSection(i, header);
    }
    PasswordsTable->horizontalHeader()->setStretchLastSection(true);

}

Passwords::~Passwords()
{

}

void Passwords::showPasswords()
{
	s_show_passwords = !s_show_passwords;
	buttonShowPasswords->hide();
	PasswordsTable->update();
	PasswordsTable->selectAll();
	PasswordsTable->clearSelection();
}

void Passwords::masterPassword() 
{
	QSettings settings;
	settings.beginGroup(QLatin1String("AutoComplete"));

	QString master = settings.value(QLatin1String("Master")).toString();

    MasterPassword *dialog = new MasterPassword(this, master);
    
	if (dialog->exec() == QDialog::Accepted)
	{
		if ( master.length() > 0 && DecryptPassword(master) != dialog->txtOld->text())
		{
			QMessageBox::warning(this, tr("Warning"), tr("Old password doesn't match the one you've typed!<br>New password has not been set."));
		}
		else
		{
			if (dialog->txtNew->text() != dialog->txtConfirm->text())
			{
				QMessageBox::warning(this, tr("Warning"), tr("Passwords do not match!<br>New password has not been set."));
			}
			else
			{
				QMessageBox::warning(this, tr("Success"), tr("New master password has been set."));
				settings.setValue(QLatin1String("Master"), EncryptPassword(dialog->txtNew->text()));
			}
		}
	}
	delete dialog;

}

MasterPassword::MasterPassword(QWidget *parent, const QString& master) : QDialog(parent), m_master(master)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
	if (master.length() == 0)
		txtOld->setEnabled(false);
}


PasswordsModel::PasswordsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
	m_data.beginGroup("AutoComplete");

	QString master = m_data.value(QLatin1String("Master")).toString();
	if ( master.length() > 0)
	{
		QString pwd = QInputDialog::getText((QWidget*)parent, tr("Master Password"), tr("Type a master password:"), QLineEdit::Password);
		if (pwd.length() == 0 || EncryptPassword(pwd) != master)
		{
			QMessageBox::warning((QWidget*)parent, tr("Warning"), tr("Invalid password!<br>No information will be displayed."));
			m_data.beginGroup(QLatin1String("fkgr"));
		}
	}
}

PasswordsModel::~PasswordsModel()
{
	m_data.endGroup();
}

QVariant PasswordsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        QFont font;
        font.setPointSize(10);
        QFontMetrics fm(font);
        int height = fm.height() + fm.height()/3;
        int width = fm.width(headerData(section, orientation, Qt::DisplayRole).toString());
        return QSize(width, height);
    }

    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section) {
            case 0:
                return tr("Web URL");
            case 1:
                return tr("User Name");
            case 2:
                return tr("Password");
            default:
                return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant PasswordsModel::data(const QModelIndex &index, int role) const
{
	QStringList hosts = m_data.childGroups();
    if (index.row() < 0 || index.row() >= hosts.size())
        return QVariant();

    switch (role) 
	{
		case Qt::DisplayRole:
		case Qt::EditRole: 
		{
			QString host = hosts.at(index.row());
	        switch (index.column()) 
			{
				case 0:
					{
						return QString(QByteArray::fromPercentEncoding( host.toUtf8() ));
					}
				case 1:
					{
						QString user_control = m_data.value(host + "/form_username_control" ).toString();
						QString username = m_data.value(host + "/" + user_control ).toString();
						return QString(QByteArray::fromPercentEncoding( username.toUtf8() ));
					}
				case 2:
					{
						if (s_show_passwords)
						{
							QString pwd_control = m_data.value(host + "/form_password_control" ).toString();
							QString pwd = DecryptPassword(m_data.value(host + "/" + pwd_control ).toString());
							return pwd; //QString(QByteArray::fromPercentEncoding( pwd.toUtf8() ));
						}
						else
							return QVariant();
					}
			}
		}
		case Qt::FontRole:
		{
			QFont font;
			font.setPointSize(10);
			return font;
		}
	}

    return QVariant();
}

int PasswordsModel::columnCount(const QModelIndex &parent) const
{
	return (parent.isValid()) ? 0 : 3;
}

int PasswordsModel::rowCount(const QModelIndex &parent) const
{
	int size = m_data.childGroups().size();
    return (parent.isValid())  ? 0 : size;
}

bool PasswordsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) 
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QStringList lst = m_data.childGroups();
    for (int i = lastRow; i >= row; --i) 
	{
		m_data.remove( lst.at(i) );
    }
    endRemoveRows();
    return true;
}
