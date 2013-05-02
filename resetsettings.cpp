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

#include "resetsettings.h"
#include "toolbarsearch.h"
#include "browsermainwindow.h"
#include "browserapplication.h"
#include "history.h"
#include "downloadmanager.h"
#include "cookiejar.h"
#include <QSettings>
#include <QWebSettings>
#include "networkaccessmanager.h"

#define CLEAR_HISTORY	"clear_history"
#define EMPTY_CACHE		"empty_cache"
#define CLEAR_DOWNLOADS	"clear_downloads"
#define CLEAR_COOKIES	"clear_cookies"
#define CLEAR_ICONS		"clear_icons"
#define CLEAR_PWDS		"clear_passwords"
#define CLEAR_SEARCHES	"clear_searches"
#define CLOSE_WINDOWS	"close_windows"
#define CLEAR_SSL		"clear_approved_ssl"
#define RESET_SETTINGS	"reset_settings_to_defaults"

ResetSettings::ResetSettings(ToolbarSearch* ts, QWidget *parent )
	: QDialog(parent), m_toolbarSearch(ts)
{
	setWindowFlags(Qt::Sheet);
	setupUi(this);
	connect(pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	QSettings settings;
    settings.beginGroup(QLatin1String("ResetSettings"));

	chkClearHistory->setChecked( settings.value(CLEAR_HISTORY, true).toBool() );
	chkEmptyCache->setChecked( settings.value(EMPTY_CACHE, true).toBool() );
	chkClearDownloads->setChecked( settings.value(CLEAR_DOWNLOADS, true).toBool() );
	chkClearCookies->setChecked( settings.value(CLEAR_COOKIES, true).toBool() );
	chkClearIcons->setChecked( settings.value(CLEAR_ICONS, true).toBool() );
	chkClearPasswords->setChecked( settings.value(CLEAR_PWDS, true).toBool() );
	chkClearSearches->setChecked( settings.value(CLEAR_SEARCHES, true).toBool() );
	chkClearSSL->setChecked( settings.value(CLEAR_SSL, true).toBool() );
	chkCloseWindows->setChecked( settings.value(CLOSE_WINDOWS, true).toBool() );
	chkResetSettings->setChecked( settings.value(RESET_SETTINGS, true).toBool() );
}

ResetSettings::~ResetSettings()
{

}

void ResetSettings::accept()
{

	if (chkClearHistory->isChecked())
	{
		BrowserApplication::historyClear();
	}

	if (chkEmptyCache->isChecked())
	{
		BrowserApplication::emptyCaches();
	}

	if (chkClearDownloads->isChecked())
	{
		BrowserApplication::clearDownloads();
	}

	if (chkClearCookies->isChecked())
	{
		BrowserApplication::clearCookies();
	}

	if (chkClearIcons->isChecked())
	{
		BrowserApplication::clearIcons();
	}

	if (chkClearPasswords->isChecked())
	{
		BrowserApplication::clearPasswords();
	}

	if (chkClearSSL->isChecked())
	{
		BrowserApplication::clearSSL();
	}

	if (chkClearSearches->isChecked())
	{
		BrowserApplication::clearSearches();
	}

	if (chkCloseWindows->isChecked())
	{
		BrowserApplication::closeExtraWindows();
	}

	if (chkResetSettings->isChecked())
	{

		BrowserApplication::resetSettings( true );

	}

	{
		QSettings settings;
		settings.beginGroup(QLatin1String("ResetSettings"));
		settings.setValue(CLEAR_HISTORY, chkClearHistory->isChecked());
		settings.setValue(EMPTY_CACHE, chkEmptyCache->isChecked());
		settings.setValue(CLEAR_DOWNLOADS, chkClearDownloads->isChecked());
		settings.setValue(CLEAR_COOKIES, chkClearCookies->isChecked());
		settings.setValue(CLEAR_ICONS, chkClearIcons->isChecked());
		settings.setValue(CLEAR_PWDS, chkClearPasswords->isChecked());
		settings.setValue(CLEAR_SEARCHES, chkClearSearches->isChecked());
		settings.setValue(CLEAR_SSL, chkClearSSL->isChecked());
		settings.setValue(CLOSE_WINDOWS, chkCloseWindows->isChecked());
		settings.setValue(RESET_SETTINGS, chkResetSettings->isChecked());
	}

    QDialog::accept();
}
