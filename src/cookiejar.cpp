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

#include "cookiejar.h"

#include "browserapplication.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtCore/QUrl>

#include <QCompleter>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include <QWebSettings>

#include <QtCore/QDebug>

#include <QNetworkCookie>

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for(quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}
QT_END_NAMESPACE

CookieJar::CookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
    , m_loaded(false)
    , m_acceptCookies(AcceptOnlyFromSitesNavigatedTo)
{
}

CookieJar::~CookieJar()
{
    if (m_keepCookies == KeepUntilExit)
        clear();
    if (!BrowserApplication::resetOnQuit())
        save();
}

void CookieJar::clear(bool reset)
{
    setAllCookies(QList<QNetworkCookie>());
    if (reset) {
        m_exceptions_allow.clear();
        m_exceptions_allowForSession.clear();
        m_exceptions_block.clear();
    }
    save();
    emit cookiesChanged();
}

void CookieJar::load()
{
    if (m_loaded)
        return;
    // load cookies and exceptions
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieSettings(BrowserApplication::dataLocation() + QLatin1String("/cookies"), QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    m_exceptions_block = cookieSettings.value(QLatin1String("block")).toStringList();
    m_exceptions_allow = cookieSettings.value(QLatin1String("allow")).toStringList();
    m_exceptions_allowForSession = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());

    loadSettings();
}

void CookieJar::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QByteArray value = settings.value(QLatin1String("acceptCookies"),
                        QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    m_acceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                        AcceptOnlyFromSitesNavigatedTo :
                        static_cast<AcceptPolicy>(acceptPolicyEnum.keyToValue(value));

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("KeepUntilExpire")).toByteArray();
    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    m_keepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                        KeepUntilExpire :
                        static_cast<KeepPolicy>(keepPolicyEnum.keyToValue(value));

    if (m_keepCookies == KeepUntilExit)
        setAllCookies(QList<QNetworkCookie>());

    m_loaded = true;
    emit cookiesChanged();
}

void CookieJar::save()
{
    if (!m_loaded)
        return;
    purgeOldCookies();
    QString directory = BrowserApplication::dataLocation();
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    if (!QFile::exists(directory)) {
        QDir dir;
        dir.mkpath(directory);
    }
    QSettings cookieSettings(directory + QLatin1String("/cookies"), QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }

    if (!cookies.isEmpty())
        cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));

    if (!m_exceptions_block.isEmpty() || !m_exceptions_allow.isEmpty() || !m_exceptions_allowForSession.isEmpty()) {
        cookieSettings.beginGroup(QLatin1String("Exceptions"));
        if (!m_exceptions_block.isEmpty()) cookieSettings.setValue(QLatin1String("block"), m_exceptions_block);
        if (!m_exceptions_allow.isEmpty()) cookieSettings.setValue(QLatin1String("allow"), m_exceptions_allow);
        if (!m_exceptions_allowForSession.isEmpty()) cookieSettings.setValue(QLatin1String("allowForSession"), m_exceptions_allowForSession);
        cookieSettings.endGroup();
    }

    // save cookie settings
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(m_acceptCookies)));

    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(m_keepCookies)));
    settings.endGroup();
}

void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);
    emit cookiesChanged();
}

void CookieJar::changed()
{
    save();
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!m_loaded)
        that->load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        QList<QNetworkCookie> noCookies;
        return noCookies;
    }

    return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!m_loaded)
        load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return false;

    QString host = url.host();
    bool eBlock = qBinaryFind(m_exceptions_block.begin(), m_exceptions_block.end(), host) != m_exceptions_block.end();
    bool eAllow = qBinaryFind(m_exceptions_allow.begin(), m_exceptions_allow.end(), host) != m_exceptions_allow.end();
    bool eAllowSession = qBinaryFind(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end(), host) != m_exceptions_allowForSession.end();

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (m_acceptCookies != AcceptNever);

    if ((acceptInitially && !eBlock)
        || (!acceptInitially && (eAllow || eAllowSession))) {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach(QNetworkCookie cookie, cookieList) {
            QList<QNetworkCookie> lst;
            if (m_keepCookies == KeepUntilTimeLimit
                && !cookie.isSessionCookie()
                && cookie.expirationDate() > soon) {
                    cookie.setExpirationDate(soon);
            }
            lst += cookie;
            if (QNetworkCookieJar::setCookiesFromUrl(lst, url)) {
                addedCookies = true;
            } else {
                // finally force it in if wanted
                if (m_acceptCookies == AcceptAlways) {
                    QList<QNetworkCookie> cookies = allCookies();
                    cookies += cookie;
                    setAllCookies(cookies);
                    addedCookies = true;
                }
            }
        }
    }

    if (addedCookies) {
        save();
        emit cookiesChanged();
    }
    return addedCookies;
}

CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_acceptCookies;
}

void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_acceptCookies)
        return;
    m_acceptCookies = policy;
    save();
}

CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_keepCookies;
}

void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_keepCookies)
        return;
    m_keepCookies = policy;
    save();
}

QStringList CookieJar::blockedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_block;
}

QStringList CookieJar::allowedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allow;
}

QStringList CookieJar::allowForSessionCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allowForSession;
}

void CookieJar::setBlockedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_block = list;
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    save();
}

void CookieJar::setAllowedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allow = list;
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    save();
}

void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allowForSession = list;
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());
    save();
}

CookieModel::CookieModel(CookieJar *cookieJar, QObject *parent)
    : QAbstractTableModel(parent)
    , m_cookieJar(cookieJar)
{
    connect(m_cookieJar, SIGNAL(cookiesChanged()), this, SLOT(cookiesChanged()));
    m_cookieJar->load();
}

QVariant CookieModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return tr("Website");
            case 1:
                return tr("Name");
            case 2:
                return tr("Path");
            case 3:
                return tr("Secure");
            case 4:
                return tr("Expires");
            case 5:
                return tr("Contents");
            default:
                return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant CookieModel::data(const QModelIndex &index, int role) const
{
    QList<QNetworkCookie> lst;
    if (m_cookieJar)
        lst = m_cookieJar->allCookies();
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        QNetworkCookie cookie = lst.at(index.row());
        switch (index.column()) {
            case 0:
                return cookie.domain();
            case 1:
                return cookie.name();
            case 2:
                return cookie.path();
            case 3:
                return cookie.isSecure();
            case 4:
                return cookie.expirationDate();
            case 5:
                return cookie.value();
        }
        }
    case Qt::FontRole:{
        QFont font;
        font.setPointSize(10);
        return font;
        }
    }

    return QVariant();
}

int CookieModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 6;
}

int CookieModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_cookieJar->allCookies().count();
}

bool CookieModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<QNetworkCookie> lst = m_cookieJar->allCookies();
    for (int i = lastRow; i >= row; --i) {
        lst.removeAt(i);
    }
    m_cookieJar->setAllCookies(lst);
    m_cookieJar->changed();
    
    endRemoveRows();
    return true;
}

void CookieModel::cookiesChanged()
{
    beginResetModel();
    endResetModel();
}

CookiesDialog::CookiesDialog(CookieJar *cookieJar, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    CookieModel *model = new CookieModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), cookiesTable, SLOT(removeOne()));
    connect(removeAllButton, SIGNAL(clicked()), cookiesTable, SLOT(removeAll()));
    m_proxyModel->setSourceModel(model);
    cookiesTable->verticalHeader()->hide();
    cookiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cookiesTable->setModel(m_proxyModel);
    cookiesTable->setAlternatingRowColors(true);
    cookiesTable->setTextElideMode(Qt::ElideMiddle);
    cookiesTable->setShowGrid(false);
    cookiesTable->setSortingEnabled(true);
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height()/3;
    cookiesTable->verticalHeader()->setDefaultSectionSize(height);
    cookiesTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < model->columnCount(); ++i){
        int header = cookiesTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QLatin1String("averagehost.domain.com"));
            break;
        case 1:
            header = fm.width(QLatin1String("_session_id"));
            break;
        case 4:
            header = fm.width(QDateTime::currentDateTime().toString(Qt::LocalDate));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        cookiesTable->horizontalHeader()->resizeSection(i, header);
    }
    cookiesTable->horizontalHeader()->setStretchLastSection(true);
}



CookieExceptionsModel::CookieExceptionsModel(CookieJar *cookiejar, QObject *parent)
    : QAbstractTableModel(parent)
    , m_cookieJar(cookiejar)
{
    m_allowedCookies = m_cookieJar->allowedCookies();
    m_blockedCookies = m_cookieJar->blockedCookies();
    m_sessionCookies = m_cookieJar->allowForSessionCookies();
}

QVariant CookieExceptionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        QFont font;
        font.setPointSize(10);
        QFontMetrics fm(font);
        int height = fm.height() + fm.height()/3;
        int width = fm.width(headerData(section, orientation, Qt::DisplayRole).toString());
        return QSize(width, height);
    }

    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Website");
            case 1:
                return tr("Status");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant CookieExceptionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        int row = index.row();
        if (row < m_allowedCookies.count()) {
            switch (index.column()) {
                case 0:
                    return m_allowedCookies.at(row);
                case 1:
                    return tr("Allow");
            }
        }
        row = row - m_allowedCookies.count();
        if (row < m_blockedCookies.count()) {
            switch (index.column()) {
                case 0:
                    return m_blockedCookies.at(row);
                case 1:
                    return tr("Block");
            }
        }
        row = row - m_blockedCookies.count();
        if (row < m_sessionCookies.count()) {
            switch (index.column()) {
                case 0:
                    return m_sessionCookies.at(row);
                case 1:
                    return tr("Allow for Session");
            }
        }
        }
    case Qt::FontRole:{
        QFont font;
        font.setPointSize(10);
        return font;
        }
    }
    return QVariant();
}

int CookieExceptionsModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}

int CookieExceptionsModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_allowedCookies.count() + m_blockedCookies.count() + m_sessionCookies.count();
}

bool CookieExceptionsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    for (int i = lastRow; i >= row; --i) {
        if (i < m_allowedCookies.count()) {
            m_allowedCookies.removeAt(row);
            continue;
        }
        i = i - m_allowedCookies.count();
        if (i < m_blockedCookies.count()) {
            m_blockedCookies.removeAt(row);
            continue;
        }
        i = i - m_blockedCookies.count();
        if (i < m_sessionCookies.count()) {
            m_sessionCookies.removeAt(row);
            continue;
        }
    }
    m_cookieJar->setAllowedCookies(m_allowedCookies);
    m_cookieJar->setBlockedCookies(m_blockedCookies);
    m_cookieJar->setAllowForSessionCookies(m_sessionCookies);
    endRemoveRows();
    return true;
}

CookiesExceptionsDialog::CookiesExceptionsDialog(CookieJar *cookieJar, QWidget *parent)
    : QDialog(parent)
    , m_cookieJar(cookieJar)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    connect(removeButton, SIGNAL(clicked()), exceptionTable, SLOT(removeOne()));
    connect(removeAllButton, SIGNAL(clicked()), exceptionTable, SLOT(removeAll()));
    exceptionTable->verticalHeader()->hide();
    exceptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    exceptionTable->setAlternatingRowColors(true);
    exceptionTable->setTextElideMode(Qt::ElideMiddle);
    exceptionTable->setShowGrid(false);
    exceptionTable->setSortingEnabled(true);
    m_exceptionsModel = new CookieExceptionsModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_exceptionsModel);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    exceptionTable->setModel(m_proxyModel);

    CookieModel *cookieModel = new CookieModel(cookieJar, this);
    domainLineEdit->setCompleter(new QCompleter(cookieModel, domainLineEdit));

    connect(domainLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(blockButton, SIGNAL(clicked()), this, SLOT(block()));
    connect(allowButton, SIGNAL(clicked()), this, SLOT(allow()));
    connect(allowForSessionButton, SIGNAL(clicked()), this, SLOT(allowForSession()));

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height()/3;
    exceptionTable->verticalHeader()->setDefaultSectionSize(height);
    exceptionTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < m_exceptionsModel->columnCount(); ++i){
        int header = exceptionTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QLatin1String("averagebiglonghost.domain.com"));
            break;
        case 1:
            header = fm.width(QLatin1String("Allow for Session"));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        exceptionTable->horizontalHeader()->resizeSection(i, header);
    }
}

void CookiesExceptionsDialog::textChanged(const QString &text)
{
    bool enabled = !text.isEmpty();
    blockButton->setEnabled(enabled);
    allowButton->setEnabled(enabled);
    allowForSessionButton->setEnabled(enabled);
}

void CookiesExceptionsDialog::block()
{
    if (domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->beginResetModel();
    m_exceptionsModel->m_blockedCookies.append(domainLineEdit->text());
    m_cookieJar->setBlockedCookies(m_exceptionsModel->m_blockedCookies);
    m_exceptionsModel->endResetModel();
}

void CookiesExceptionsDialog::allow()
{
    if (domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->beginResetModel();
    m_exceptionsModel->m_allowedCookies.append(domainLineEdit->text());
    m_cookieJar->setAllowedCookies(m_exceptionsModel->m_allowedCookies);
    m_exceptionsModel->endResetModel();
}

void CookiesExceptionsDialog::allowForSession()
{
    if (domainLineEdit->text().isEmpty())
        return;
    m_exceptionsModel->beginResetModel();
    m_exceptionsModel->m_sessionCookies.append(domainLineEdit->text());
    m_cookieJar->setAllowForSessionCookies(m_exceptionsModel->m_sessionCookies);
    m_exceptionsModel->endResetModel();
}

