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

#include "settings.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "passwords.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "webview.h"
#include "webpage.h"

#include <QtCore/QSettings>
#include <QtGui>
#include <QtWebKit>
#include <QSysInfo>
#include <QInputDialog>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(exceptionsButton, SIGNAL(clicked()), this, SLOT(showExceptions()));
    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));
    connect(restoreToDefaultButton, SIGNAL(clicked()), this, SLOT(restoreHomeToDefault()));
    connect(cookiesButton, SIGNAL(clicked()), this, SLOT(showCookies()));
    connect(standardFontButton, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(fixedFontButton, SIGNAL(clicked()), this, SLOT(chooseFixedFont()));
    connect(comboBoxStyle, SIGNAL(currentIndexChanged(int)), this,   SLOT(setAppStyle(int)));
    connect(showPasswordsButton, SIGNAL(clicked()), this, SLOT(showPasswords()));
    connect(chkUserAgent, SIGNAL(stateChanged ( int ) ), this, SLOT(useUserAgent(int)));
    connect(buttonSearchProviders, SIGNAL(clicked() ), this, SLOT( showSearchProviders() ) );
    connect(buttonEditShortcuts, SIGNAL(clicked() ), this, SLOT( editShortcuts() ) );
    connect(btnExtView, SIGNAL(clicked() ), this, SLOT( chooseExtViewer() ) );
    connect(btnStylePath, SIGNAL(clicked() ), this, SLOT( chooseStylePath() ) );

    connect(proxyAuto, SIGNAL(stateChanged ( int ) ), this, SLOT(setAutoProxy(int)));
    connect(proxySupport, SIGNAL(toggled( bool ) ), this, SLOT(setProxyEnabled(bool)));

    connect(btnAddAd, SIGNAL(clicked() ), this, SLOT( addBlockAd() ) );
    connect(btnEditAd, SIGNAL(clicked() ), this, SLOT( editBlockAd() ) );
    connect(btnDelAd, SIGNAL(clicked() ), this, SLOT( removeBlockAd() ) );
    connect(btnDelAds, SIGNAL(clicked() ), this, SLOT( removeBlockAds() ) );

    connect(btnBlockMostAds, SIGNAL(clicked() ), this, SLOT( blockMostAds() ) );
    connect(btnBlockMostCnts, SIGNAL(clicked() ), this, SLOT( blockMostCnts() ) );

    connect(btnAddAdEx, SIGNAL(clicked() ), this, SLOT( addBlockAdEx() ) );
    connect(btnEditAdEx, SIGNAL(clicked() ), this, SLOT( editBlockAdEx() ) );
    connect(btnDelAdEx, SIGNAL(clicked() ), this, SLOT( removeBlockAdEx() ) );

    loadDefaults();
    loadFromSettings();
    
    fontChanged = false;
    connect(tbGoBack, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbGoForward, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbAddBook, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbHome, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbRefresh, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbAppStyle, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbPrivMode, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbPrefs, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbImages, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbProxy, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbCompatibility, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbReset, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbInspect, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbBookmarks, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbTextSize, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbVirtKeyb, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
    connect(tbJavaScript, SIGNAL(clicked()), this, SLOT( checkAddressBarButtons() ));
}

extern QString DefaultDownloadPath(bool create);

void SettingsDialog::useUserAgent(int state)
{
    if (state == Qt::Checked)
    {
        comboAgents->setEnabled(true);
    }
    else
    {
        comboAgents->setEnabled(false);
    }
}

void SettingsDialog::setAutoProxy(int state)
{
    if (state == Qt::Checked)
        proxySupport->setChecked( false );
}

void SettingsDialog::setProxyEnabled(bool state)
{
    if (state )
        proxyAuto->setChecked( false );
}


QString DefaultAppStyle()
{
#ifdef Q_WS_WIN
    return QLatin1String("Windows .NET");
#else
    #ifdef Q_WS_MAC
        return QLatin1String("Macintosh (aqua)");
    #else
        return QLatin1String("GTK+");
    #endif
#endif

 }

void SettingsDialog::loadDefaults()
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    standardFont = QFont(standardFontFamily, standardFontSize);
    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    comboMainMenu->setCurrentIndex(0);

    downloadsLocation->setText( DefaultDownloadPath( false )  );

    enableJavascript->setChecked(defaultSettings->testAttribute(QWebSettings::JavascriptEnabled));
    enablePlugins->setChecked(defaultSettings->testAttribute(QWebSettings::PluginsEnabled));
    blockPopups->setChecked( ! (defaultSettings->testAttribute(QWebSettings::JavascriptCanOpenWindows)) );
    autoLoadImages->setChecked(defaultSettings->testAttribute(QWebSettings::AutoLoadImages));

    enableDiskCache->setChecked(false);
    enableLocalStorage->setChecked(false);

    chkSavePasswords->setChecked(false);
    checkBoxDeleteDownloads->setChecked(false);

    newTabAction->setCurrentIndex(0);

    comboBoxAV->setCurrentIndex(0);

    mouseweelClick->setCurrentIndex(1);

    comboBoxStyle->addItems(QStyleFactory::keys());
    int ind = comboBoxStyle->findText(DefaultAppStyle());
    if (ind >= 0)
        comboBoxStyle->setCurrentIndex(ind);
    
    chkUserStyleSheet->setChecked(false);
    chkUserAgent->setChecked(false);
    comboAgents->setEditText("");

    chkExtViewer->setChecked(false);

    tbGoBack->setChecked( true );
    tbGoForward->setChecked( true );
    tbAddBook->setChecked( true );
    tbHome->setChecked( true );
    tbRefresh->setChecked( true );
    tbAppStyle->setChecked( true );
    tbPrivMode->setChecked( true );
    tbPrefs->setChecked( true );
    tbImages->setChecked( false );
    tbProxy->setChecked( false );
    proxyExcept->setChecked(false);
    tbCompatibility->setChecked( true );
    tbReset->setChecked( false );
    tbJavaScript->setChecked( false );

#ifndef Q_WS_WIN
    proxyAuto->setVisible(false);
#endif

    tbInspect->setChecked( false );
    tbVirtKeyb->setChecked( false );
    tbBookmarks->setChecked( false );
    tbTextSize->setChecked( false );

    expireHistory->setCurrentIndex(1);

    chkBlockAds->setChecked(false);
    listAds->clear();

    chkBlockAdsEx->setChecked(false);
    listAdEx->clear();
}

QString defaultHome = QLatin1String("http://www.qtweb.net/");

void SettingsDialog::loadFromSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));

    bool bDownloadAudioVideo = settings.value(QLatin1String("downloadAudioVideo"), (bool)(comboBoxAV->currentIndex()) ).toBool();
    comboBoxAV->setCurrentIndex( bDownloadAudioVideo ? 1 : 0);

    QString style = settings.value(QLatin1String("style"), DefaultAppStyle()).toString();

     int ind = comboBoxStyle->findText(style);
     if (ind >= 0)
        comboBoxStyle->setCurrentIndex(ind);

     m_last_style = comboBoxStyle->currentText();

    homeLineEdit->setText(settings.value(QLatin1String("home"), defaultHome).toString());
    startupAction->setCurrentIndex(settings.value(QLatin1String("onStartup"), 0).toInt());
    newTabAction->setCurrentIndex(settings.value(QLatin1String("newTabAction"), 0).toInt());

    mouseweelClick->setCurrentIndex(settings.value(QLatin1String("mouseweelClickAction"), 1).toInt());

    settings.endGroup();

    settings.beginGroup(QLatin1String("history"));
    int historyExpire = settings.value(QLatin1String("historyExpire")).toInt();
    int idx = 0;
    switch (historyExpire) {
    case 1: idx = 0; break;
    case 7: idx = 1; break;
    case 14: idx = 2; break;
    case 30: idx = 3; break;
    case 365: idx = 4; break;
    case -1: idx = 5; break;
    default:
        idx = 1;
    }
    expireHistory->setCurrentIndex(idx);
    settings.endGroup();

    settings.beginGroup(QLatin1String("downloadmanager"));
    QString downloadDirectory = settings.value(QLatin1String("downloadDirectory"), downloadsLocation->text()).toString();
    downloadsLocation->setText(downloadDirectory);
    bool full_cleanup = settings.value(QLatin1String("full_cleanup"), false).toBool();
    checkBoxDeleteDownloads->setChecked(full_cleanup);
    bool alwaysPromptForFileName = settings.value(QLatin1String("askForFileName"), false).toBool();
    if (alwaysPromptForFileName)
        checkBoxAsk->setChecked(true);

    settings.endGroup();

    settings.beginGroup(QLatin1String("general"));
    openLinksIn->setCurrentIndex(settings.value(QLatin1String("openLinksIn"), openLinksIn->currentIndex()).toInt());

    bool bEnableInspector = settings.value(QLatin1String("EnableWebInspector"), false).toBool();
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, bEnableInspector);

    bool bHideIcons = settings.value(QLatin1String("hideMenuIcons"), false).toBool();
    bool bShowMenu = settings.value(QLatin1String("ShowMenu"), true).toBool();

    if (!bShowMenu)
        comboMainMenu->setCurrentIndex(2);
    else
        if (bHideIcons)
            comboMainMenu->setCurrentIndex(1);
        else
            comboMainMenu->setCurrentIndex(0);

    QString language = settings.value(QLatin1String("Language"), "").toString();
    if (!language.isEmpty())
    {
        language = "(" + language + ")";
        for(int i = 0; i < comboLangs->count(); i++)
        {
            if (comboLangs->itemText(i).contains(language, Qt::CaseInsensitive) )
            {
                comboLangs->setCurrentIndex(i);
                break;
            }
        }
    }
    connect(comboLangs, SIGNAL(currentIndexChanged(int)), this,   SLOT(warnLangChange(int)));

    settings.endGroup();

    // Appearance
    settings.beginGroup(QLatin1String("websettings"));
    fixedFont = settings.value(QLatin1String("fixedFont"), fixedFont).value<QFont>();
    standardFont = settings.value(QLatin1String("standardFont"), standardFont).value<QFont>();

    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    enableJavascript->setChecked(settings.value(QLatin1String("enableJavascript"), enableJavascript->isChecked()).toBool());
    enablePlugins->setChecked(settings.value(QLatin1String("enablePlugins"), enablePlugins->isChecked()).toBool());

    autoLoadImages->setChecked(settings.value(QLatin1String("autoLoadImages"), autoLoadImages->isChecked()).toBool());
    blockPopups->setChecked(settings.value(QLatin1String("blockPopups"), blockPopups->isChecked()).toBool());

    enableDiskCache->setChecked(settings.value(QLatin1String("enableDiskCache"), enableDiskCache->isChecked()).toBool());
    enableLocalStorage->setChecked(settings.value(QLatin1String("enableLocalStorage"), enableLocalStorage->isChecked()).toBool());

    chkSavePasswords->setChecked( settings.value(QLatin1String("savePasswords"), chkSavePasswords->isChecked()).toBool() );

    userStyleSheet->setText(settings.value(QLatin1String("userStyleSheet")).toUrl().toString());
    chkUserStyleSheet->setChecked( settings.value(QLatin1String("customUserStyleSheet"), chkUserStyleSheet->isChecked() ).toBool() );

    chkUserAgent->setChecked( settings.value(QLatin1String("customUserAgent"), chkUserAgent->isChecked()).toBool());
    comboAgents->setEditText(settings.value(QLatin1String("UserAgent")).toString());

    chkExtViewer->setChecked( settings.value(QLatin1String("useExtViewer"), chkExtViewer->isChecked()).toBool());

#ifdef Q_WS_WIN
        QLatin1String extViewer("NOTEPAD.EXE");
#else

#ifdef Q_WS_MAC
        QLatin1String extViewer("Open");
#else
        QLatin1String extViewer("gedit");
#endif

#endif
    txtExtViewer->setText(settings.value(QLatin1String("ExtViewer"), extViewer).toString());

    settings.endGroup();

    // Privacy
    settings.beginGroup(QLatin1String("cookies"));

    CookieJar *jar = BrowserApplication::cookieJar();
    QByteArray value = settings.value(QLatin1String("acceptCookies"), QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    CookieJar::AcceptPolicy acceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                        CookieJar::AcceptOnlyFromSitesNavigatedTo :
                        static_cast<CookieJar::AcceptPolicy>(acceptPolicyEnum.keyToValue(value));
    switch(acceptCookies) {
    case CookieJar::AcceptAlways:
        acceptCombo->setCurrentIndex(0);
        break;
    case CookieJar::AcceptNever:
        acceptCombo->setCurrentIndex(1);
        break;
    case CookieJar::AcceptOnlyFromSitesNavigatedTo:
        acceptCombo->setCurrentIndex(2);
        break;
    }

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("Expire")).toByteArray();
    QMetaEnum keepPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("KeepPolicy"));
    CookieJar::KeepPolicy keepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                        CookieJar::KeepUntilExpire :
                        static_cast<CookieJar::KeepPolicy>(keepPolicyEnum.keyToValue(value));
    switch(keepCookies) {
    case CookieJar::KeepUntilExpire:
        keepUntilCombo->setCurrentIndex(0);
        break;
    case CookieJar::KeepUntilExit:
        keepUntilCombo->setCurrentIndex(1);
        break;
    case CookieJar::KeepUntilTimeLimit:
        keepUntilCombo->setCurrentIndex(2);
        break;
    }
    settings.endGroup();

    // Proxy
    settings.beginGroup(QLatin1String("proxy"));
    proxySupport->setChecked(settings.value(QLatin1String("enabled"), false).toBool());
    proxyType->setCurrentIndex(settings.value(QLatin1String("type"), 0).toInt());
    proxyHostName->setText(settings.value(QLatin1String("hostName")).toString());
    proxyPort->setValue(settings.value(QLatin1String("port"), 1080).toInt());
    proxyUserName->setText(settings.value(QLatin1String("userName")).toString());
    proxyPassword->setText(settings.value(QLatin1String("password")).toString());
    proxyExcept->setChecked(settings.value(QLatin1String("useExceptions"), false).toBool());
    proxyExceptions->setText(settings.value(QLatin1String("exceptions")).toString());
    proxyAuto->setChecked(settings.value(QLatin1String("autoProxy"), false).toBool());

    settings.endGroup();

    settings.beginGroup(QLatin1String("AddressBar"));
    tbGoBack->setChecked( settings.value(QLatin1String("showGoBack"), tbGoBack->isChecked()).toBool() );
    tbGoForward->setChecked( settings.value(QLatin1String("showGoForward"), tbGoForward->isChecked()).toBool() );
    tbAddBook->setChecked( settings.value(QLatin1String("showAddBookmark"), tbAddBook->isChecked()).toBool() );
    tbHome->setChecked( settings.value(QLatin1String("showGoHome"), tbHome->isChecked()).toBool() );
    tbRefresh->setChecked( settings.value(QLatin1String("showRefresh"), tbRefresh->isChecked()).toBool() );
    tbAppStyle->setChecked( settings.value(QLatin1String("showAppStyle"), tbAppStyle->isChecked()).toBool() );
    tbPrivMode->setChecked( settings.value(QLatin1String("showPrivacyMode"), tbPrivMode->isChecked()).toBool() );
    tbPrefs->setChecked( settings.value(QLatin1String("showPreferences"), tbPrefs->isChecked()).toBool() );
    tbImages->setChecked( settings.value(QLatin1String("showImages"), tbImages->isChecked()).toBool() );
    tbProxy->setChecked( settings.value(QLatin1String("showProxy"), tbProxy->isChecked()).toBool() );
    tbCompatibility->setChecked( settings.value(QLatin1String("showCompatibility"), tbCompatibility->isChecked()).toBool() );
    tbReset->setChecked( settings.value(QLatin1String("showReset"), tbReset->isChecked()).toBool() );

    tbVirtKeyb->setChecked( settings.value(QLatin1String("showKeyboard"), tbVirtKeyb->isChecked()).toBool() );
    tbInspect->setChecked( settings.value(QLatin1String("showInspect"), tbInspect->isChecked()).toBool() );
    tbTextSize->setChecked( settings.value(QLatin1String("showTextSize"), tbTextSize->isChecked()).toBool() );
    tbBookmarks->setChecked( settings.value(QLatin1String("showBookmarks"), tbBookmarks->isChecked()).toBool() );

    tbJavaScript->setChecked( settings.value(QLatin1String("showDisableJavaScript"), tbJavaScript->isChecked()).toBool() );

    settings.endGroup();
    settings.beginGroup(QLatin1String("AdBlock"));
    chkBlockAds->setChecked(settings.value(QLatin1String("useAdBlock"), chkBlockAds->isChecked()).toBool());
    foreach(QString ad, settings.allKeys())
        if (ad != QLatin1String("useAdBlock"))
            listAds->addItem(ad );
    settings.endGroup();

    settings.beginGroup(QLatin1String("AdBlockEx"));
    chkBlockAdsEx->setChecked(settings.value(QLatin1String("useAdBlockEx"), chkBlockAdsEx->isChecked()).toBool());
    foreach(QString ad, settings.allKeys())
        if (ad != QLatin1String("useAdBlockEx"))
            listAdEx->addItem(ad );
    settings.endGroup();
}

void SettingsDialog::saveToSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));

    bool bDownloadAudioVideo = (comboBoxAV->currentIndex() > 0);
    settings.setValue(QLatin1String("downloadAudioVideo"), bDownloadAudioVideo );

    settings.setValue(QLatin1String("mouseweelClickAction"), mouseweelClick->currentIndex());

    QString style = comboBoxStyle->currentText();
    settings.setValue(QLatin1String("style"), style);
    QApplication::setStyle(QStyleFactory::create(style));

    settings.setValue(QLatin1String("home"), homeLineEdit->text());
    settings.setValue(QLatin1String("onStartup"), startupAction->currentIndex());
    settings.setValue(QLatin1String("newTabAction"), newTabAction->currentIndex());
    settings.endGroup();

    settings.beginGroup(QLatin1String("general"));
    settings.setValue(QLatin1String("openLinksIn"), openLinksIn->currentIndex());
    switch(comboMainMenu->currentIndex())
    {
        case 1:
            settings.setValue(QLatin1String("hideMenuIcons"), true);
            settings.setValue(QLatin1String("ShowMenu"), true);
            break;
        case 2:
            settings.setValue(QLatin1String("hideMenuIcons"), false);
            settings.setValue(QLatin1String("ShowMenu"), false);
            break;
        default:
            settings.setValue(QLatin1String("hideMenuIcons"), false);
            settings.setValue(QLatin1String("ShowMenu"), true);
    }

    QString lang = comboLangs->currentText();
    // Get the encoding name in brackets    
    if (lang.indexOf('(') != -1)
        lang = lang.mid(lang.indexOf('(') + 1);
    if (lang.indexOf(')') != -1)
        lang = lang.mid(0, lang.indexOf(')'));

    settings.setValue(QLatin1String("Language"), lang);

    settings.endGroup();

    settings.beginGroup(QLatin1String("downloadmanager"));
    settings.setValue(QLatin1String("downloadDirectory"), downloadsLocation->text());
    settings.setValue(QLatin1String("full_cleanup"), checkBoxDeleteDownloads->isChecked());
    settings.setValue(QLatin1String("askForFileName"), checkBoxAsk->isChecked());
    settings.endGroup();

    settings.beginGroup(QLatin1String("history"));
    int historyExpire = expireHistory->currentIndex();
    int idx = -1;
    switch (historyExpire) {
    case 0: idx = 1; break;
    case 1: idx = 7; break;
    case 2: idx = 14; break;
    case 3: idx = 30; break;
    case 4: idx = 365; break;
    case 5: idx = -1; break;
    }
    settings.setValue(QLatin1String("historyExpire"), idx);
    settings.endGroup();

    // Appearance
    settings.beginGroup(QLatin1String("websettings"));
    settings.setValue(QLatin1String("fixedFont"), fixedFont);
    settings.setValue(QLatin1String("standardFont"), standardFont);
    settings.setValue(QLatin1String("enableJavascript"), enableJavascript->isChecked());
    settings.setValue(QLatin1String("enablePlugins"), enablePlugins->isChecked());
    settings.setValue(QLatin1String("autoLoadImages"), autoLoadImages->isChecked());
    settings.setValue(QLatin1String("blockPopups"), blockPopups->isChecked());
    settings.setValue(QLatin1String("savePasswords"), chkSavePasswords->isChecked());
    settings.setValue(QLatin1String("enableDiskCache"), enableDiskCache->isChecked());
    settings.setValue(QLatin1String("enableLocalStorage"), enableLocalStorage->isChecked());


    QString userStyleSheetString = userStyleSheet->text();
    if (QFile::exists(userStyleSheetString))
        settings.setValue(QLatin1String("userStyleSheet"), QUrl::fromLocalFile(userStyleSheetString));
    else
        settings.setValue(QLatin1String("userStyleSheet"), QUrl(userStyleSheetString));

    settings.setValue(QLatin1String("customUserStyleSheet"), chkUserStyleSheet->isChecked());

    settings.setValue(QLatin1String("useExtViewer"), chkExtViewer->isChecked());
    settings.setValue(QLatin1String("ExtViewer"), txtExtViewer->text());
    settings.setValue(QLatin1String("customUserAgent"), chkUserAgent->isChecked());

    QString current_agent = settings.value(QLatin1String("UserAgent"), "" ).toString();
    settings.setValue(QLatin1String("UserAgent"), comboAgents->currentText());
    if (comboAgents->currentText() != current_agent)
    {
        if (current_agent.length() > 0 && current_agent.indexOf('/') != -1 )
            settings.setValue(QLatin1String("prevUserAgent"), current_agent );

        if (BrowserApplication::instance()->mainWindow())
            BrowserApplication::instance()->mainWindow()->setCurrentAgentIcon();
    }

    settings.endGroup();
    WebPage::setUserAgent();

    //Privacy
    settings.beginGroup(QLatin1String("cookies"));

    CookieJar::KeepPolicy keepCookies;
    switch(acceptCombo->currentIndex()) {
    default:
    case 0:
        keepCookies = CookieJar::KeepUntilExpire;
        break;
    case 1:
        keepCookies = CookieJar::KeepUntilExit;
        break;
    case 2:
        keepCookies = CookieJar::KeepUntilTimeLimit;
        break;
    }
    CookieJar *jar = BrowserApplication::cookieJar();
    QMetaEnum acceptPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(keepCookies)));

    CookieJar::KeepPolicy keepPolicy;
    switch(keepUntilCombo->currentIndex()) {
        default:
    case 0:
        keepPolicy = CookieJar::KeepUntilExpire;
        break;
    case 1:
        keepPolicy = CookieJar::KeepUntilExit;
        break;
    case 2:
        keepPolicy = CookieJar::KeepUntilTimeLimit;
        break;
    }

    QMetaEnum keepPolicyEnum = jar->staticMetaObject.enumerator(jar->staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(keepPolicy)));

    settings.endGroup();

    // proxy
    settings.beginGroup(QLatin1String("proxy"));
    settings.setValue(QLatin1String("enabled"), proxySupport->isChecked());
    settings.setValue(QLatin1String("type"), proxyType->currentIndex());
    settings.setValue(QLatin1String("hostName"), proxyHostName->text());
    settings.setValue(QLatin1String("port"), proxyPort->text());
    settings.setValue(QLatin1String("userName"), proxyUserName->text());
    settings.setValue(QLatin1String("password"), proxyPassword->text());
    settings.setValue(QLatin1String("useExceptions"), proxyExcept->isChecked());
    settings.setValue(QLatin1String("Exceptions"), proxyExceptions->text());
    settings.setValue(QLatin1String("autoProxy"), proxyAuto->isChecked());
    settings.endGroup();

    settings.beginGroup(QLatin1String("AdBlock"));
    settings.remove("");
    settings.setValue(QLatin1String("useAdBlock"), chkBlockAds->isChecked());
    for (int i = 0; i < listAds->count(); i++)
        settings.setValue( listAds->item(i)->text(), "" );
    settings.endGroup();

    settings.beginGroup(QLatin1String("AdBlockEx"));
    settings.remove("");
    settings.setValue(QLatin1String("useAdBlockEx"), chkBlockAdsEx->isChecked());
    for (int i = 0; i < listAdEx->count(); i++)
        settings.setValue( listAdEx->item(i)->text(), "" );
    settings.endGroup();


    BrowserApplication::instance()->loadSettings();
    BrowserApplication::networkAccessManager()->loadSettings();
    BrowserApplication::cookieJar()->loadSettings();
    BrowserApplication::historyManager()->loadSettings();

    if (BrowserApplication::instance()->mainWindow() && 
        BrowserApplication::instance()->mainWindow()->currentTab())
    {
        BrowserApplication::instance()->mainWindow()->checkToolBarButtons();

        if (BrowserApplication::instance()->mainWindow()->currentTab()->textSizeMultiplier() != 1.0)
            BrowserApplication::instance()->mainWindow()->currentTab()->setTextSizeMultiplier(1.0);

        if (fontChanged)
            BrowserApplication::instance()->mainWindow()->currentTab()->reload();
    }

}

void SettingsDialog::checkAddressBarButtons()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("AddressBar"));
    settings.setValue(QLatin1String("showGoBack"), tbGoBack->isChecked());
    settings.setValue(QLatin1String("showGoForward"), tbGoForward->isChecked());
    settings.setValue(QLatin1String("showAddBookmark"), tbAddBook->isChecked() );
    settings.setValue(QLatin1String("showGoHome"), tbHome->isChecked() );
    settings.setValue(QLatin1String("showRefresh"), tbRefresh->isChecked());
    settings.setValue(QLatin1String("showAppStyle"), tbAppStyle->isChecked() );
    settings.setValue(QLatin1String("showPrivacyMode"), tbPrivMode->isChecked());
    settings.setValue(QLatin1String("showPreferences"), tbPrefs->isChecked());
    settings.setValue(QLatin1String("showImages"), tbImages->isChecked());
    settings.setValue(QLatin1String("showProxy"), tbProxy->isChecked());
    settings.setValue(QLatin1String("showCompatibility"), tbCompatibility->isChecked());
    settings.setValue(QLatin1String("showReset"), tbReset->isChecked());

    settings.setValue(QLatin1String("showInspect"), tbInspect->isChecked());
    settings.setValue(QLatin1String("showTextSize"), tbTextSize->isChecked());
    settings.setValue(QLatin1String("showKeyboard"), tbVirtKeyb->isChecked());
    settings.setValue(QLatin1String("showBookmarks"), tbBookmarks->isChecked());

    settings.setValue(QLatin1String("showDisableJavascript"), tbJavaScript->isChecked());
    settings.endGroup();

    BrowserApplication::instance()->mainWindow()->checkToolBarButtons();
}

void SettingsDialog::accept()
{
    saveToSettings();
    QDialog::accept();
}

void SettingsDialog::showCookies()
{
    CookiesDialog *dialog = new CookiesDialog(BrowserApplication::cookieJar(), this);
    dialog->exec();
}

void SettingsDialog::showExceptions()
{
    CookiesExceptionsDialog *dialog = new CookiesExceptionsDialog(BrowserApplication::cookieJar(), this);
    dialog->exec();
}

void SettingsDialog::chooseFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, standardFont, this);
    if ( ok ) {
        standardFont = font;
        standardLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
        fontChanged = true;
    }
}

void SettingsDialog::chooseFixedFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fixedFont, this);
    if ( ok ) {
        fixedFont = font;
        fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
        fontChanged = true;
    }
}

void SettingsDialog::setHomeToCurrentPage()
{
    BrowserMainWindow *mw = static_cast<BrowserMainWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
        homeLineEdit->setText(webView->url().toString());
}

void SettingsDialog::restoreHomeToDefault()
{
    homeLineEdit->setText(defaultHome);
}

void SettingsDialog::warnLangChange(int)
{
    QMessageBox::information(this, "QtWeb", tr("The new language will be applied after QtWeb restarts"));
}

void SettingsDialog::setAppStyle(int index)
{
    QString style = comboBoxStyle->currentText();
    QApplication::setStyle(QStyleFactory::create(style));
}

void SettingsDialog::reject()
{
    if (m_last_style != comboBoxStyle->currentText())
    {
        QApplication::setStyle(QStyleFactory::create(m_last_style));
    }

    QDialog::reject();
}


void SettingsDialog::showPasswords()
{
    Passwords *dialog = new Passwords(this);
    dialog->exec();
    delete dialog;
}

#include "searches.h"

void SettingsDialog::showSearchProviders()
{
    Searches *dialog = new Searches(this);
    dialog->exec();
    delete dialog;
}

#include "shortcuts.h"

void SettingsDialog::editShortcuts()
{
    Shortcuts *dialog = new Shortcuts(this);
    dialog->exec();
    delete dialog;
}

void SettingsDialog::chooseExtViewer()
{
#ifdef Q_WS_WIN
    QString filter(tr("Applications (*.exe);;All files (*.*)"));
#else
    QString filter = tr("All files (*.*)");
#endif

    QString file = QFileDialog::getOpenFileName(this, tr("External Web Page Source Viewer"), QString(),filter);

    if (file.isEmpty())
        return;

    txtExtViewer->setText(QDir::toNativeSeparators(file));
}

void SettingsDialog::chooseStylePath()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Defaut Style Sheet:"), QString(),
            tr("Cascading Style Sheets (*.CSS);;All files (*.*)"));

    if (file.isEmpty())
        return;

    userStyleSheet->setText(QDir::toNativeSeparators(file));
}

void SettingsDialog::addBlockAd()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add AdBlock"),
        tr("Enter the pattern to block:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        if (!text.startsWith(QChar('*')))
            text = QChar('*') + text;
        if (!text.endsWith(QChar('*')))
            text += QChar('*');

        listAds->addItem(text);
    }
}

void SettingsDialog::editBlockAd()
{
    if (listAds->currentRow() == -1)
        return;

    QString text = listAds->item(listAds->currentRow())->text();

    bool ok;
    text = QInputDialog::getText(this, tr("Edit AdBlock"),
        tr("Enter the pattern to block:"), QLineEdit::Normal, text, &ok);
    if (ok && !text.isEmpty())
        listAds->currentItem()->setText(text);
}

void SettingsDialog::removeBlockAd()
{
    if (listAds->currentRow() != -1)
        delete listAds->takeItem( listAds->currentRow() );
}

void SettingsDialog::removeBlockAds()
{
    listAds->clear();
}

void SettingsDialog::addBlockAdEx()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add exception"),
        tr("Enter URL or pattern to unblock:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
        listAdEx->addItem(text);
}

void SettingsDialog::editBlockAdEx()
{
    if (listAdEx->currentRow() == -1)
        return;

    QString text = listAdEx->item(listAdEx->currentRow())->text();

    bool ok;
    text = QInputDialog::getText(this, tr("Edit exception"),
        tr("Enter URL or pattern to unblock:"), QLineEdit::Normal, text, &ok);
    if (ok && !text.isEmpty())
        listAdEx->currentItem()->setText(text);
}

void SettingsDialog::removeBlockAdEx()
{
    if (listAdEx->currentRow() != -1)
        delete listAdEx->takeItem( listAdEx->currentRow() );
}

void SettingsDialog::addBlockItems(const QLatin1String& filename, QListWidget* listview)
{
    QFile file(filename);
    bool isOpened = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString all = QString(QLatin1String(file.readAll()));
    file.close();
    QStringList lst = all.split("\n");

    foreach(QString l, lst)
    {
        QList<QListWidgetItem *> found = listview->findItems(l, Qt::MatchExactly);
        if (found.size() == 0)
            listview->addItem(l);
    }
}

void SettingsDialog::blockMostAds()
{
    addBlockItems(QLatin1String(":/BlockAds.txt"), listAds);
}

void SettingsDialog::blockMostCnts()
{
    addBlockItems(QLatin1String(":/BlockCounters.txt"), listAds);
}
