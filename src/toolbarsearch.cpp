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

#include "toolbarsearch.h"
#include "autosaver.h"

#include <QtCore/QSettings>
#include <QtCore/QUrl>

#include <QtGui/QCompleter>
#include <QtGui/QMenu>
#include <QtGui/QStringListModel>
#include <QtWebKit/QWebSettings>
#include "searches.h"
#include "googlesuggest.h"

/*
    ToolbarSearch is a very basic search widget that also contains a small history.
    Searches are turned into urls that use Google to perform search
 */
ToolbarSearch::ToolbarSearch(QWidget *parent)
    : SearchLineEdit(parent)
    , m_autosaver(new AutoSaver(this))
    , m_maxSavedSearches(10)
    , m_stringListModel(new QStringListModel(this))
	, m_completer(0)
{
    QMenu *m = menu();
    connect(m, SIGNAL(aboutToShow()), this, SLOT(aboutToShowMenu()));
    connect(m, SIGNAL(triggered(QAction*)), this, SLOT(triggeredMenuAction(QAction*)));

    QCompleter *completer = new QCompleter(m_stringListModel, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    lineEdit()->setCompleter(completer);

    connect(lineEdit(), SIGNAL(returnPressed()), SLOT(searchNow()));
    setInactiveText(DEFAULT_PROVIDER);

    load();

	checkGoogleSuggest( inactiveText() == SEARCH_GOOGLE);
}

ToolbarSearch::~ToolbarSearch()
{
    m_autosaver->saveIfNeccessary();
}

void ToolbarSearch::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("toolbarsearch"));
    settings.setValue(QLatin1String("recentSearches"), m_stringListModel->stringList());
    settings.setValue(QLatin1String("maximumSaved"), m_maxSavedSearches);
    settings.setValue(QLatin1String("searcher"), inactiveText());
    settings.endGroup();
}

void ToolbarSearch::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("toolbarsearch"));
    QStringList list = settings.value(QLatin1String("recentSearches")).toStringList();
    m_maxSavedSearches = settings.value(QLatin1String("maximumSaved"), m_maxSavedSearches).toInt();
    m_stringListModel->setStringList(list);
	QString engine = settings.value(QLatin1String("searcher"), DEFAULT_PROVIDER).toString();
	setInactiveText(engine);
    settings.endGroup();
}

void ToolbarSearch::searchNow()
{
    QString searchText = lineEdit()->text().trimmed();
	
	QStringList lst = searchText.split(QChar(' '));
	QString keyword_provider = "";
	if (lst.size() > 1)
	{
		QSettings settings;
		settings.beginGroup(QLatin1String("SearchProviders"));
		QStringList providers = settings.allKeys();
		QString kw = lst[0].toLower();
		if (providers.size() > 0)
		{
			foreach(QString prov, providers)
			{
				if (kw == prov.toLower())
				{
					keyword_provider = inactiveText();
					setInactiveText(prov);
					break;
				}
			}
		}
		else
		{
			if (kw == SEARCH_GOOGLE.toLower())
			{
				keyword_provider = inactiveText();
				setInactiveText(SEARCH_GOOGLE);
			} else
			if (kw == SEARCH_YAHOO.toLower())
			{
				keyword_provider = inactiveText();
				setInactiveText(SEARCH_YAHOO);
			}else
			if (kw == SEARCH_BING.toLower())
			{
				keyword_provider = inactiveText();
				setInactiveText(SEARCH_BING);
			}else
			if (kw == SEARCH_CUIL.toLower())
			{
				keyword_provider = inactiveText();
				setInactiveText(SEARCH_CUIL);
			}
		}
		if (!keyword_provider.isEmpty())
		{
			searchText = searchText.remove(0, kw.length()).trimmed();
		}
	}

    QStringList newList = m_stringListModel->stringList();
    if (newList.contains(searchText))
        newList.removeAt(newList.indexOf(searchText));
    newList.prepend(searchText);
    if (newList.size() >= m_maxSavedSearches)
        newList.removeLast();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (!globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        m_stringListModel->setStringList(newList);
        m_autosaver->changeOccurred();
    }

	QUrl url;
	if (inactiveText() == SEARCH_GOOGLE)
	{
		url.setUrl(QLatin1String("http://www.google.com/search"));
		url.addQueryItem(QLatin1String("q"), searchText);
		url.addQueryItem(QLatin1String("ie"), QLatin1String("UTF-8"));
		url.addQueryItem(QLatin1String("oe"), QLatin1String("UTF-8"));
	}
	else
	if (inactiveText() == SEARCH_CUIL)
	{
		url.setUrl(QLatin1String("http://www.cuil.com/search"));
		url.addQueryItem(QLatin1String("q"), searchText);
	}
	else
	if (inactiveText() == SEARCH_YAHOO)
	{
		url.setUrl(QLatin1String("http://search.yahoo.com/search"));
		url.addQueryItem(QLatin1String("p"), searchText);
		url.addQueryItem(QLatin1String("ei"), QLatin1String("UTF-8"));
	}
	else
	if (inactiveText() == SEARCH_BING)
	{
		url.setUrl(QLatin1String("http://www.bing.com/search"));
		url.addQueryItem(QLatin1String("q"), searchText);
	}
	else
	{
		QSettings settings;
		settings.beginGroup(QLatin1String("SearchProviders"));
		QStringList providers = settings.allKeys();
		foreach(QString prov, providers)
		{
			if (inactiveText() == prov)
			{
				QString search_url  = settings.value(prov).toString();
				url.setUrl(search_url + searchText );
				break;
			}
		}
		settings.endGroup();
	}

	url.addQueryItem(QLatin1String("client"), QLatin1String("QtWeb"));

	if (!keyword_provider.isEmpty())
	{
		setInactiveText(keyword_provider);
	}

    emit search(url);
}

void ToolbarSearch::checkGoogleSuggest(bool show_google)
{
	if (show_google)
	{
		if (!m_completer)
			m_completer = new GSuggestCompletion( m_lineEdit );
	}
	else
	{
		delete m_completer;
		m_completer = NULL;
	}
}

void ToolbarSearch::useGoogle()
{
	m_autosaver->changeOccurred();
	setInactiveText(SEARCH_GOOGLE);
	checkGoogleSuggest( true );
	if (!lineEdit()->text().isEmpty())
		searchNow();
}

void ToolbarSearch::useBing()
{
	m_autosaver->changeOccurred();
    setInactiveText(SEARCH_BING);
	if (!lineEdit()->text().isEmpty())
		searchNow();
}

void ToolbarSearch::useYahoo()
{
	m_autosaver->changeOccurred();
    setInactiveText(SEARCH_YAHOO);
	checkGoogleSuggest( false );
	if (!lineEdit()->text().isEmpty())
		searchNow();
}

void ToolbarSearch::useCuil()
{
	m_autosaver->changeOccurred();
    setInactiveText(SEARCH_CUIL);
	checkGoogleSuggest( false );
	if (!lineEdit()->text().isEmpty())
		searchNow();
}


void ToolbarSearch::useSearch( )
{
	m_autosaver->changeOccurred();

	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString text = action->statusTip();
    setInactiveText(text);

	checkGoogleSuggest( text == SEARCH_GOOGLE );

	if (!lineEdit()->text().isEmpty())
		searchNow();
}

void ToolbarSearch::addSearches()
{
	QMenu *m = menu();	
	m->addSeparator();

    QAction *recent = m->addAction(tr("Search Providers"));
    recent->setEnabled(false);

	QSettings settings;
	settings.beginGroup(QLatin1String("SearchProviders"));
	QStringList providers = settings.allKeys();
	bool bExists = false;
	foreach(QString prov, providers)
	{
		QString search_url  = settings.value(prov).toString();
		if (!search_url.isEmpty() )
		{
			QAction* cust = m->addAction(prov, this, SLOT(useSearch()));
			QFont fc = cust->font();
			fc.setBold( inactiveText() == prov );
			cust->setFont( fc );
			cust->setStatusTip( prov );
			bExists = true;
		}
	}

	settings.endGroup();

	if (!bExists)
	{
		settings.beginGroup(QLatin1String("ExcludeSearchProviders"));

		if (settings.value(SEARCH_GOOGLE).toString().isEmpty())
		{
			QAction* google = m->addAction(SEARCH_GOOGLE, this, SLOT(useGoogle()));
			QFont fg = google->font();
			fg.setBold( inactiveText() == SEARCH_GOOGLE);
			google->setFont ( fg );
		}

		if (settings.value(SEARCH_YAHOO).toString().isEmpty())
		{
			QAction* yahoo = m->addAction(SEARCH_YAHOO, this, SLOT(useYahoo()));
			QFont fy = yahoo->font();
			fy.setBold( inactiveText() == SEARCH_YAHOO);
			yahoo->setFont( fy );
		}

		if (settings.value(SEARCH_BING).toString().isEmpty())
		{
			QAction* live = m->addAction(SEARCH_BING, this, SLOT(useBing()));
			QFont fl = live->font();
			fl.setBold( inactiveText() == SEARCH_BING );
			live->setFont( fl );
		}

		if (settings.value(SEARCH_CUIL).toString().isEmpty())
		{
			QAction* cuil = m->addAction(SEARCH_CUIL, this, SLOT(useCuil()));
			QFont fc = cuil->font();
			fc.setBold( inactiveText() == SEARCH_CUIL );
			cuil->setFont( fc );
		}

		settings.endGroup();
	}

	QAction* searches = m->addAction(tr("Add..."), this, SLOT(addSearch()));
 }

void ToolbarSearch::addSearch()
{
    Searches *dialog = new Searches(this);
    dialog->exec();
	delete dialog;
}


void ToolbarSearch::aboutToShowMenu()
{
    lineEdit()->selectAll();
    QMenu *m = menu();
    m->clear();
    QStringList list = m_stringListModel->stringList();
    if (list.isEmpty()) {
        m->addAction(tr("No Recent Searches"));
		addSearches();
        return;
    }

    QAction *recent = m->addAction(tr("Recent Searches"));
    recent->setEnabled(false);
    for (int i = 0; i < list.count(); ++i) {
        QString text = list.at(i);
        m->addAction(text)->setData(text);
    }
    m->addSeparator();

    m->addAction(tr("Clear Recent Searches"), this, SLOT(clear()));

   addSearches();
}

void ToolbarSearch::triggeredMenuAction(QAction *action)
{
    QVariant v = action->data();
    if (v.canConvert<QString>()) {
        QString text = v.toString();
        lineEdit()->setText(text);
        searchNow();
    }
}

void ToolbarSearch::clear()
{
    m_stringListModel->setStringList(QStringList());
    m_autosaver->changeOccurred();;
}

