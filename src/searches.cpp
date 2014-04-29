/*
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
 * Copyright (C) 2013 Sergei Lopatin <magist3r@gmail.com>
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

#include "searches.h"
#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QHeaderView>
#include <QSettings>

SearchesModel::SearchesModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_searchProviders(new QList< QPair<QString, QString> >)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("SearchProviders"));
    QStringList keys = settings.allKeys();
    if (keys.isEmpty())
        loadDefaultProviders();
    else {
        Q_FOREACH(QString key, keys) {
            m_searchProviders->append(QPair<QString,QString> (key, settings.value(key).toString()));
        }
    }
    settings.endGroup();
}

QVariant SearchesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section) {
            case 0:
                return tr("Provider");
            case 1:
                return tr("Search URL");
            default:
                return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant SearchesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_searchProviders->size())
        return QVariant();

    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        QPair<QString, QString> provider = m_searchProviders->at(index.row());
        switch (index.column())
        {
            case 0:
                return provider.first;
            case 1:
                return provider.second;
        }
    }

    return QVariant();
}

int SearchesModel::columnCount(const QModelIndex &) const
{
    return 2;
}

int SearchesModel::rowCount(const QModelIndex &) const
{
    return m_searchProviders->size();
}

bool SearchesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) 
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);

    for (int i = row; i <= lastRow; ++i)
        m_searchProviders->removeAt(i);

    endRemoveRows();
    return true;
}

bool SearchesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        QPair<QString, QString> pair = m_searchProviders->at(index.row());
        switch (index.column()) {
        case 0:
            pair.first = value.toString();
            break;
        case 1:
            pair.second = value.toString();
            break;
        }
        m_searchProviders->replace(index.row(), pair);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags SearchesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int SearchesModel::addRow(QString name, QString value)
{
    QPair<QString, QString> pair(name, value);
    int oldIndex = m_searchProviders->indexOf(pair);
    if (oldIndex != -1) { // prevent from inserting more than one unique row
        return oldIndex;
    } else {
        beginResetModel();
        m_searchProviders->append(pair);
        int index = m_searchProviders->indexOf(pair);
//        reset();
        endResetModel();
        return index;
    }
}

void SearchesModel::loadDefaultProviders()
{
    beginResetModel();
    m_searchProviders->clear();
    m_searchProviders->append(QPair<QString, QString>("Google", "http://www.google.com/search?q="));
    m_searchProviders->append(QPair<QString, QString>("Yahoo", "http://search.yahoo.com/search?p="));
    m_searchProviders->append(QPair<QString, QString>("Bing", "http://www.bing.com/search?q="));
    m_searchProviders->append(QPair<QString, QString>("Cuil", "http://www.cuil.com/search?q="));
//    reset();
    endResetModel();
}

void SearchesModel::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("SearchProviders"));
    settings.remove("");
    for (int i = 0; i < m_searchProviders->size(); i++) {
        QPair<QString, QString> pair = m_searchProviders->at(i);
        settings.setValue(pair.first, pair.second);
    }
    settings.endGroup();
}

Searches::Searches(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    HintLabel->hide();

    m_model = new SearchesModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addSearchProvider()));
    connect(addButton, SIGNAL(clicked()), HintLabel, SLOT(show()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSelectedRows()));
    connect(restoreButton, SIGNAL(clicked()), m_model, SLOT(loadDefaultProviders()));
    connect(buttonAddEbay, SIGNAL(clicked()), this, SLOT(addEbay()));
    connect(buttonAddAsk, SIGNAL(clicked()), this, SLOT(addAsk()));
    connect(buttonBox, SIGNAL(accepted()), m_model, SLOT(save()));
    m_proxyModel->setSourceModel(m_model);
    SearchesTable->setModel(m_proxyModel);
    SearchesTable->resizeRowsToContents();
}

void Searches::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && m_model)
        removeSelectedRows();

    QDialog::keyPressEvent(event);
}

void Searches::addSearchProvider()
{
    if (m_model) {
        int index = m_model->addRow("", "");
        SearchesTable->setFocus();
        SearchesTable->selectRow(index);
    }
}

void Searches::addEbay()
{
    if (m_model) {
        int index = m_model->addRow(QLatin1String("eBay"), QLatin1String("shop.ebay.com/?_nkw="));
        SearchesTable->setFocus();
        SearchesTable->selectRow(index);
    }
}

void Searches::addAsk()
{
    if (m_model) {
        int index = m_model->addRow(QLatin1String("Ask"), QLatin1String("www.ask.com/web?q="));
        SearchesTable->setFocus();
        SearchesTable->selectRow(index);
    }
}

void Searches::removeSelectedRows()
{
    if (m_model && SearchesTable->selectionModel()) {
        QModelIndexList list = SearchesTable->selectionModel()->selectedRows();
        QList<int> indexes;
        for (int i = 0; i < list.size(); i++)
            indexes << list.at(i).row();

        qSort(indexes.begin(), indexes.end());

        for (int j = indexes.size() - 1; j >= 0; j--) // remove rows in reversed order
            m_model->removeRow(indexes.at(j));

        SearchesTable->selectRow(m_model->rowCount() - 1);
    }
}
