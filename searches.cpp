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

#include "searches.h"
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QHeaderView>
#include <QSettings>
#include "ui_search.h"

Searches::Searches(QWidget *parent)
	: QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);

	editButton->hide();

	m_model = new SearchesModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), SearchesTable, SLOT(removeOne()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addSearch()));
    connect(buttonAddEbay, SIGNAL(clicked()), this, SLOT(addEbay()));
    connect(buttonAddAsk, SIGNAL(clicked()), this, SLOT(addAsk()));
    //connect(editButton, SIGNAL(clicked()), this, SLOT(editSearch()));
    m_proxyModel->setSourceModel(m_model);
    SearchesTable->verticalHeader()->hide();
    SearchesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    SearchesTable->setModel(m_proxyModel);
    SearchesTable->setAlternatingRowColors(true);
    SearchesTable->setTextElideMode(Qt::ElideMiddle);
    SearchesTable->setShowGrid(false);
    SearchesTable->setSortingEnabled(true);
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height()/3;
    SearchesTable->verticalHeader()->setDefaultSectionSize(height);
    SearchesTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < m_model->columnCount(); ++i){
        int header = SearchesTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QString(10, 'X'));
            break;
        case 1:
            header = fm.width(QString(60, 'X'));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        SearchesTable->horizontalHeader()->resizeSection(i, header);
    }
    SearchesTable->horizontalHeader()->setStretchLastSection(true);
}

Searches::~Searches()
{

}

SearchesModel::SearchesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
	m_data.beginGroup(QLatin1String("SearchProviders"));
	m_exclude.beginGroup(QLatin1String("ExcludeSearchProviders"));
	if (m_exclude.value( SEARCH_GOOGLE ).toString().isEmpty())
		m_data.setValue( SEARCH_GOOGLE, QUERY_GOOGLE );
	if (m_exclude.value( SEARCH_BING ).toString().isEmpty())
		m_data.setValue( SEARCH_BING, QUERY_BING );
	if (m_exclude.value( SEARCH_YAHOO ).toString().isEmpty())
		m_data.setValue( SEARCH_YAHOO, QUERY_YAHOO );
	if (m_exclude.value( SEARCH_CUIL ).toString().isEmpty())
		m_data.setValue( SEARCH_CUIL, QUERY_CUIL );
}

SearchesModel::~SearchesModel()
{
	m_data.endGroup();
}

QVariant SearchesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
	QStringList providers = m_data.allKeys();
    if (index.row() < 0 || index.row() >= providers.size())
        return QVariant();

    switch (role) 
	{
		case Qt::DisplayRole:
		case Qt::EditRole: 
		{
			QString provider = providers.at(index.row());
	        switch (index.column()) 
			{
				case 0:
						return provider;
				case 1:
						return m_data.value(provider).toString();
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

int SearchesModel::columnCount(const QModelIndex &parent) const
{
	return (parent.isValid()) ? 0 : 2;
}

int SearchesModel::rowCount(const QModelIndex &parent) const
{
	int size = m_data.allKeys().size();
    return (parent.isValid())  ? 0 : size;
}

bool SearchesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) 
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);

	QStringList providers = m_data.allKeys();

    for (int i = lastRow; i >= row; --i) 
	{
		if (providers.at(i) == SEARCH_GOOGLE)
			m_exclude.setValue(SEARCH_GOOGLE, "x");
		if (providers.at(i) == SEARCH_YAHOO)
			m_exclude.setValue(SEARCH_YAHOO, "x");
		if (providers.at(i) == SEARCH_BING)
			m_exclude.setValue(SEARCH_BING, "x");
		if (providers.at(i) == SEARCH_CUIL)
			m_exclude.setValue(SEARCH_CUIL, "x");

		m_data.remove( providers.at(i) );
    }
    endRemoveRows();
    return true;
}

void SearchesModel::addSearch(QString name, QString value)
{
	if (value.isNull() || name.isNull())
		return;

	if (value.toLower().indexOf("http://") != 0 )
		value = "http://" + value;

	m_data.setValue(name, value);
	
	reset();
	
}


void Searches::addSearch()
{
	QString name, value;
    
	if (showSearchDialog(name, value)) 
	{
		if (m_model)
			m_model->addSearch(name, value);
	}
}

void Searches::addEbay()
{
	if (m_model)
		m_model->addSearch(QLatin1String("eBay"), QLatin1String("shop.ebay.com/?_nkw="));
}

void Searches::addAsk()
{
	if (m_model)
		m_model->addSearch(QLatin1String("Ask"), QLatin1String("www.ask.com/web?q="));
}

bool Searches::showSearchDialog(QString& name, QString& value)
{
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::SearchDialog searchDialog;
    searchDialog.setupUi(&dialog);

    searchDialog.iconLabel->setText(QString());
	searchDialog.iconLabel->setPixmap(QIcon(":defaulticon.png").pixmap(32,32));

	if (dialog.exec() == QDialog::Accepted) 
	{
		name = searchDialog.searchName->text();
		value = searchDialog.searchURL->text();
		return true;
	}

	return false;
}
