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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtGui/QDialog>
#include "ui_settings.h"

class SettingsDialog : public QDialog, public Ui_Settings
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
    void accept();
    void reject();

private slots:
    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();

    void setHomeToCurrentPage();
	void restoreHomeToDefault();
    void showCookies();
    void showExceptions();
    void showPasswords();
	void showSearchProviders();
	void editShortcuts();

	void chooseExtViewer();
	void chooseStylePath();
    void chooseFont();
    void chooseFixedFont();
	void setAppStyle(int);
	void useUserAgent(int);
	void warnLangChange(int);
	void setAutoProxy(int);
	void setProxyEnabled(bool);

	void checkAddressBarButtons();

	void addBlockAd();
	void editBlockAd();
	void removeBlockAd();
	void removeBlockAds();
	void addBlockAdEx();
	void editBlockAdEx();
	void removeBlockAdEx();
	void blockMostAds();
	void blockMostCnts();

protected:
	void addBlockItems(const QLatin1String&, QListWidget* );

private:
    QFont standardFont;
    QFont fixedFont;
	QString m_last_style;
	bool  fontChanged;
};

#endif // SETTINGS_H

