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
#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <QDialog>
#include <QSettings>
#include "ui_passwords.h"
#include "ui_master.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
QT_END_NAMESPACE



class PasswordsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PasswordsModel(QObject *parent = 0);
    ~PasswordsModel();
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
	QSettings	m_data;

};

class MasterPassword : public QDialog, public Ui_MasterDialog
{
	Q_OBJECT

public:
	MasterPassword(QWidget *parent, const QString& master);

private:
	QString m_master;

};

class Passwords : public QDialog, public Ui_PasswordsDialog
{
	Q_OBJECT

public:
	Passwords(QWidget *parent);
	~Passwords();

private slots:
    void showPasswords();
    void masterPassword();

private:
    QSortFilterProxyModel *m_proxyModel;
};

#endif // PASSWORDS_H
