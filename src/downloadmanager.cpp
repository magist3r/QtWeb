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
** Copyright (C) 2008-2008 Trolltech ASA. All rights reserved.
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

#include "downloadmanager.h"

#include "browserapplication.h"
#include "networkaccessmanager.h"

#include <math.h>

#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QFileIconProvider>
#include <QtGui/QDesktopServices>
#include <QMessageBox>
#include <QtCore/QDebug>
#include <QtCore/QProcess>

#include <QtWebKit/QWebSettings>
#include "torrent/torrentwindow.h"

QString DefaultDownloadPath(bool create_dir)
{
    try
    {
#ifdef WINPE
        return QDir::currentPath().left(2);
#else
        return BrowserApplication::downloadsLocation(create_dir);
#endif

    }
    catch(...)
    {
#ifdef WINPE
        return "X:"; // AC: Reference to the root of the Virtual drive is set by default - it always exists!
#else
        return "C:"; // AC: Reference to the root of the Virtual drive is set by default - it always exists!
#endif
    }
}

/*!
    DownloadItem is a widget that is displayed in the download manager list.
    It moves the data from the QNetworkReply into the QFile as well
    as update the information/progressbar and report errors.
 */
DownloadItem::DownloadItem(QNetworkReply *reply, bool requestFileName, QWidget *parent)
    : QWidget(parent)
    , m_reply(reply)
    , m_requestFileName(requestFileName)
    , m_bytesReceived(0)
    , m_to_delete(false)
    , m_finished(false)
{
    setupUi(this);
    QPalette p = downloadInfoLabel->palette();
    p.setColor(QPalette::Text, Qt::darkGray);
    downloadInfoLabel->setPalette(p);
    progressBar->setMaximum(0);
    tryAgainButton->hide();
    stopButton->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
    tryAgainButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

    connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));
    connect(tryAgainButton, SIGNAL(clicked()), this, SLOT(tryAgain()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(remove()));
    
    m_success_init = false;
    m_must_be_deleted = true;

    if (!requestFileName) {
        QSettings settings;
        settings.beginGroup(QLatin1String("downloadmanager"));
        m_requestFileName = settings.value(QLatin1String("askForFileName"), false).toBool();
    }

    init();
}

void DownloadItem::init()
{
    if (!m_reply)
        return;

    m_finished = false;

    // attach to the m_reply
    m_url = m_reply->url();
    m_reply->setParent(this);

    connect(m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgress(qint64, qint64)));
    connect(m_reply, SIGNAL(metaDataChanged()),
            this, SLOT(metaDataChanged()));
    connect(m_reply, SIGNAL(finished()),
            this, SLOT(finished()));

    // reset info
    downloadInfoLabel->clear();
    progressBar->setValue(0);
    m_success_init = getFileName();

    // start timer for the download estimation
    m_downloadTime.start();

    if (m_reply->error() != QNetworkReply::NoError) 
    {
        error(m_reply->error());
        finished();
    }
    else
    {
        if (m_finished)
        {
            finished();
        }
    }
}

void DownloadItem::setOutputTitle()
{
    QString out = QFileInfo(m_output).fileName();

    fileNameLabel->setText( out );
    fileNameLabel->setToolTip(  QDir::toNativeSeparators( m_output.fileName()) );
}

bool DownloadItem::getFileName()
{
    QString downloadDirectory = dirDownloads(true);

    if (!downloadDirectory.isEmpty() && (downloadDirectory.right(1) != QDir::separator() ))
        downloadDirectory += QDir::separator();

    QString defaultFileName = saveFileName(downloadDirectory);
    QString fileName = defaultFileName;
    if (m_requestFileName) 
    {
        fileName = QFileDialog::getSaveFileName((QWidget*)parent(), tr("Save File"), defaultFileName); 
        if (fileName.isEmpty()) {
            m_reply->close();
            fileNameLabel->setText(tr("Download canceled: %1").arg(QFileInfo(defaultFileName).fileName()));
            return false;
        }
    }

    m_output.setFileName(QDir::toNativeSeparators(fileName));
    
    setOutputTitle();

    if (m_requestFileName) 
    {
        downloadReadyRead();
    }

    return true;
}

QString DownloadItem::saveFileName(const QString &directory) const
{
    // Move this function into QNetworkReply to also get file name sent from the server
    QString path;
    if (m_reply->hasRawHeader("Content-Disposition")) {
        QString value = QString::fromUtf8(m_reply->rawHeader("Content-Disposition"));
        int pos = value.indexOf(QLatin1String("filename="));
        if (pos != -1) {
            QString name = value.mid(pos + 9);
            if (name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char('"')))
                name = name.mid(1, name.size() - 2);
            path = name;
        }
    }
    
    if (path.isEmpty())
        path = m_url.path();

    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();

    if (baseName.isEmpty()) {
        baseName = QLatin1String("unnamed_download");
        qDebug() << "DownloadManager:: downloading unknown file:" << m_url;
    }
    QString name = directory + baseName + QLatin1Char('.') + endName;

    if (QFile::exists(name)) {
        // already exists, don't overwrite
        int i = 1;
        do {
            name = directory + baseName + QLatin1Char('-') + QString::number(i++) + QLatin1Char('.') + endName;
        } while (QFile::exists(name));
    }
    return name;
}


void DownloadItem::stop()
{
    setUpdatesEnabled(false);
    stopButton->setEnabled(false);
    stopButton->hide();
    tryAgainButton->setEnabled(true);
    tryAgainButton->show();
    setUpdatesEnabled(true);
    m_reply->abort();
    deleteButton->setVisible(true);
}

void DownloadItem::mouseDoubleClickEvent ( QMouseEvent * event )
{
    open();
}

void DownloadItem::open()
{
    if (checkAddTorrent())
        return;

    QFileInfo info(m_output);
    if ( tryAgainButton->isVisible() || stopButton->isVisible())
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
}

void DownloadItem::tryAgain()
{
    if (!tryAgainButton->isEnabled())
        return;

    tryAgainButton->setEnabled(false);
    tryAgainButton->setVisible(false);
    stopButton->setEnabled(true);
    stopButton->setVisible(true);
    deleteButton->setVisible(false);
    progressBar->setVisible(true);

    QNetworkReply *r = BrowserApplication::networkAccessManager()->get(QNetworkRequest(m_url));
    if (m_reply)
        m_reply->deleteLater();
    if (m_output.exists())
        m_output.remove();
    m_reply = r;
    init();
    emit statusChanged();
}

void DownloadItem::remove()
{
    m_to_delete = true;
    emit statusChanged();
}

void DownloadItem::downloadReadyRead()
{
    if (m_requestFileName && m_output.fileName().isEmpty())
        return;
    if (!m_output.isOpen()) {
        // in case someone else has already put a file there
        if (!m_requestFileName)
            getFileName();
        if (!m_output.open(QIODevice::WriteOnly)) {
            downloadInfoLabel->setText(tr("Error opening file: %1")
                    .arg(m_output.errorString()));
            stopButton->click();
            emit statusChanged();
            return;
        }
        emit statusChanged();
    }
    if (-1 == m_output.write(m_reply->readAll())) {
        downloadInfoLabel->setText(tr("Error saving: %1")
                .arg(m_output.errorString()));
        stopButton->click();
    }
}

void DownloadItem::error(QNetworkReply::NetworkError)
{
    qDebug() << "DownloadItem::error" << m_reply->errorString() << m_url;
    downloadInfoLabel->setText(tr("Network Error: %1").arg(m_reply->errorString()));
    tryAgainButton->setEnabled(true);
    tryAgainButton->setVisible(true);
}

void DownloadItem::metaDataChanged()
{
    // AC: ???

    /*QVariant locationHeader = m_reply->header(QNetworkRequest::LocationHeader);
    if (locationHeader.isValid()) {
        m_url = locationHeader.toUrl();
        m_reply->deleteLater();
        m_reply = BrowserApplication::networkAccessManager()->get(QNetworkRequest(m_url));
        init();
        return;
    }*/ 

    qDebug() << "DownloadItem::metaDataChanged: not handled.";
}

void DownloadItem::setSizeDate(qint64 sz, QDateTime m) 
{ 
    m_bytesReceived = sz; 
    downloadInfoLabel->setText(dataString(sz) + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font size=-1>" + m.toString("MMM d, yyyy")); 
}

void DownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_bytesReceived = bytesReceived;
    if (bytesTotal == -1) {
        progressBar->setValue(0);
        progressBar->setMaximum(0);
    } else {
        progressBar->setValue(bytesReceived);
        progressBar->setMaximum(bytesTotal);
    }
    updateInfoLabel();
}

void DownloadItem::updateInfoLabel()
{
    if (m_reply->error() != QNetworkReply::NoError)
        return;

    qint64 bytesTotal = progressBar->maximum();
    bool running = !downloadedSuccessfully();

    // update info label
    double speed = m_bytesReceived * 1000.0 / m_downloadTime.elapsed();
    double timeRemaining = ((double)(bytesTotal - m_bytesReceived)) / speed;
    QString timeRemainingString = tr("seconds");
    if (timeRemaining > 60) {
        timeRemaining = timeRemaining / 60;
        timeRemainingString = tr("minutes");
    }
    timeRemaining = floor(timeRemaining);

    // When downloading the eta should never be 0
    if (timeRemaining == 0)
        timeRemaining = 1;

    QString info;
    if (running) {
        QString remaining;
        if (bytesTotal != 0)
            remaining = tr("- %4 %5 remaining")
            .arg(timeRemaining)
            .arg(timeRemainingString);
        info = QString(tr("%1 of %2 (%3/sec) %4"))
            .arg(dataString(m_bytesReceived))
            .arg(bytesTotal == 0 ? QString("?") : dataString(bytesTotal))
            .arg(dataString((int)((int)speed > 0 ? speed : 0)))
            .arg(remaining);
    } else {
        if (m_bytesReceived == bytesTotal)
            info = dataString(m_output.size());
        else
            info = tr("%1 of %2 - Stopped")
                .arg(dataString(m_bytesReceived))
                .arg(dataString(bytesTotal));
    }
    downloadInfoLabel->setText(info);
}

QString DownloadItem::dataString(int size) const
{
    QString unit;
    if (size < 1024) {
        unit = tr("bytes");
    } else if (size < 1024*1024) {
        size /= 1024;
        unit = tr("KB");
    } else {
        size /= 1024*1024;
        unit = tr("MB");
    }
    return QString(QLatin1String("%1 %2")).arg(size).arg(unit);
}

bool DownloadItem::downloading() const
{
    return (progressBar->isVisible());
}

bool DownloadItem::downloadedSuccessfully() const
{
    return (stopButton->isHidden() && tryAgainButton->isHidden());
}

void DownloadItem::finished()
{
    if (m_reply->bytesAvailable() > 0)
        downloadReadyRead();

    m_finished = true;
    progressBar->hide();
    stopButton->setEnabled(false);
    stopButton->hide();
    m_output.close();
    deleteButton->show();
    updateInfoLabel();
    emit statusChanged();

    // Process completed Torrents here
    checkAddTorrent();
}

bool DownloadItem::checkAddTorrent()
{
    if ( downloadedSuccessfully() && m_output.fileName().endsWith( ".torrent", Qt::CaseInsensitive ))
    {
        TorrentWindow* window = BrowserApplication::torrents();
        if (window->addTorrent( m_output.fileName() ))
            window->show();
        return true;
    }
    return false;
}

/*!
    DownloadManager is a Dialog that contains a list of DownloadItems

    It is a basic download manager.  It only downloads the file, doesn't do BitTorrent,
    extract zipped files or anything fancy.
  */
DownloadManager::DownloadManager(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , m_manager(BrowserApplication::networkAccessManager())
    , m_iconProvider(0)
    , m_removePolicy(Never)
{
    setupUi(this);
    downloadsView->setShowGrid(false);
    downloadsView->verticalHeader()->hide();
    downloadsView->horizontalHeader()->hide();
    downloadsView->setAlternatingRowColors(true);
    downloadsView->horizontalHeader()->setStretchLastSection(true);
//  connect(downloadsView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(openItem(const QModelIndex& index)));

    m_model = new DownloadModel(this);
    downloadsView->setModel(m_model);
    connect(cleanupButton, SIGNAL(clicked()), this, SLOT(cleanup_full()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(open_downloads()));
    openButton->setEnabled( true );

#ifdef WINPE
    openButton->hide();
#endif

    load();
 
    this->setWindowTitle(tr("Downloads"));
}

DownloadManager::~DownloadManager()
{
    if (!BrowserApplication::resetOnQuit())
        save();

    if (m_iconProvider)
        delete m_iconProvider;
}

void DownloadManager::openItem(const QModelIndex& index)
{
}

int DownloadManager::activeDownloads() const
{
    int count = 0;
    for (int i = 0; i < m_downloads.count(); ++i) {
        if (m_downloads.at(i)->stopButton->isEnabled())
            ++count;
    }
    return count;
}

void DownloadManager::download(const QNetworkRequest &request, bool requestFileName)
{
    if (request.url().isEmpty())
        return;
    handleUnsupportedContent(m_manager->get(request), requestFileName);
}

void DownloadManager::handleUnsupportedContent(QNetworkReply *reply, bool requestFileName)
{
    if (!reply || reply->url().isEmpty())
        return;
    QVariant header = reply->header(QNetworkRequest::ContentLengthHeader);
    bool ok;
    int size = header.toInt(&ok);
    if (ok && size == 0)
        return;

    qDebug() << "DownloadManager::handleUnsupportedContent" << reply->url() << "requestFileName" << requestFileName;
    DownloadItem *item = new DownloadItem(reply, requestFileName, this);
    
    if (item->initSuccess())
        addItem(item);
    else
        delete item;
}

void DownloadManager::addItem(DownloadItem *item)
{
    connect(item, SIGNAL(statusChanged()), this, SLOT(updateRow()));
    int row = m_downloads.count();
    m_model->beginInsertRows(QModelIndex(), row, row);
    m_downloads.append(item);
    m_model->endInsertRows();
    updateItemCount();
    if (row == 0)
        show();
    downloadsView->setIndexWidget(m_model->index(row, 0), item);
    QIcon icon = style()->standardIcon(QStyle::SP_FileIcon);
    item->fileIcon->setPixmap(icon.pixmap(48, 48));
    downloadsView->setRowHeight(row, item->sizeHint().height());
    this->setVisible(true);
}

void DownloadManager::updateRow()
{
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    int row = m_downloads.indexOf(item);
    if (-1 == row)
        return;
    if (!m_iconProvider)
        m_iconProvider = new QFileIconProvider();
    QIcon icon = m_iconProvider->icon(item->m_output.fileName());
    if (icon.isNull())
        icon = style()->standardIcon(QStyle::SP_FileIcon);
    item->fileIcon->setPixmap(icon.pixmap(48, 48));
    downloadsView->setRowHeight(row, item->minimumSizeHint().height());

    bool remove = false;
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (!item->downloading()
        && globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
    {
        remove = true;
        item->m_must_be_deleted = false;
    }

    if (item->downloadedSuccessfully()
        && removePolicy() == DownloadManager::SuccessFullDownload) {
        remove = true;
    }
    if (remove || item->m_to_delete)
        m_model->removeRow(row);

    cleanupButton->setEnabled(m_downloads.count() - activeDownloads() > 0);
    openButton->setEnabled(m_downloads.count() > 0);
    updateItemCount();
}

DownloadManager::RemovePolicy DownloadManager::removePolicy() const
{
    return m_removePolicy;
}

void DownloadManager::setRemovePolicy(RemovePolicy policy)
{
    if (policy == m_removePolicy)
        return;
    m_removePolicy = policy;
    save();
}

void DownloadManager::save() const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QMetaEnum removePolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    settings.setValue(QLatin1String("removeDownloadsPolicy"), QLatin1String(removePolicyEnum.valueToKey(m_removePolicy)));
    settings.setValue(QLatin1String("size"), size());
    if (m_removePolicy == Exit)
        return;

    for (int i = 0; i < m_downloads.count(); ++i) {
        QString key = QString(QLatin1String("download_%1_")).arg(i);
        settings.setValue(key + QLatin1String("url"), m_downloads[i]->m_url);
        settings.setValue(key + QLatin1String("location"), QFileInfo(m_downloads[i]->m_output).filePath());
        settings.setValue(key + QLatin1String("done"), m_downloads[i]->downloadedSuccessfully());
    }
    int i = m_downloads.count();
    QString key = QString(QLatin1String("download_%1_")).arg(i);
    while (settings.contains(key + QLatin1String("url"))) {
        settings.remove(key + QLatin1String("url"));
        settings.remove(key + QLatin1String("location"));
        settings.remove(key + QLatin1String("done"));
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }
}

void DownloadManager::addItem(const QUrl& url, QString filename, bool done)
{
    DownloadItem *item = new DownloadItem(0, false, this);

    item->m_output.setFileName(filename);
    item->setOutputTitle();

    item->m_url = url;
    item->stopButton->setVisible(false);
    item->stopButton->setEnabled(false);
    item->tryAgainButton->setVisible(!done);
    item->tryAgainButton->setEnabled(!done);
    item->progressBar->setVisible(!done);
    if (QFile::exists(filename))
    {
        QFile f(filename);
        if (f.open(QIODevice::ReadOnly ))
        {
            QFileInfo fi( f );
            item->setSizeDate(f.size(), fi.lastModified());

            f.close();
        }
    }
    addItem(item);

    if (!m_iconProvider)
        m_iconProvider = new QFileIconProvider();
    QIcon icon = m_iconProvider->icon(QDir::toNativeSeparators(item->m_output.fileName()));
    if (icon.isNull())
        icon = style()->standardIcon(QStyle::SP_FileIcon);
    item->fileIcon->setPixmap(icon.pixmap(48, 48));

    cleanupButton->setEnabled(m_downloads.count() - activeDownloads() > 0);
}

void DownloadManager::load()
{
    //m_downloads.clear();
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QSize size = settings.value(QLatin1String("size")).toSize();
    if (size.isValid())
        resize(size);
    QByteArray value = settings.value(QLatin1String("removeDownloadsPolicy"), QLatin1String("Never")).toByteArray();
    QMetaEnum removePolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    m_removePolicy = removePolicyEnum.keyToValue(value) == -1 ?
                        Never :
                        static_cast<RemovePolicy>(removePolicyEnum.keyToValue(value));

    if (!m_iconProvider)
        m_iconProvider = new QFileIconProvider();

    int i = 0;
    QString key = QString(QLatin1String("download_%1_")).arg(i);
    while (settings.contains(key + QLatin1String("url"))) {
        QUrl url = settings.value(key + QLatin1String("url")).toUrl();
        QString fileName = settings.value(key + QLatin1String("location")).toString();
        bool done = settings.value(key + QLatin1String("done"), true).toBool();
        if (!url.isEmpty() && !fileName.isEmpty()) 
        {
            addItem( url, fileName, done );
        }
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }
    cleanupButton->setEnabled(m_downloads.count() - activeDownloads() > 0);
}

QString dirDownloads(bool create_dir)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QString location = QDir::toNativeSeparators(settings.value(QLatin1String("downloadDirectory"), "").toString());

    if (location.isEmpty())
        location = DefaultDownloadPath(create_dir);
    else
    {
        if (QDir::isRelativePath(location))
            location = QCoreApplication::applicationDirPath() + "/" + location;

        if (create_dir) {
            QDir dir(location);
            if (!dir.exists())
                dir.mkpath(location);
        }
    }

    return location;
}

void DownloadManager::open_downloads()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirDownloads(true)));
}


void DownloadManager::cleanup_list()
{
    if (m_downloads.isEmpty())
        return;
    m_model->removeRows(0, m_downloads.count());
    updateItemCount();
    if (m_downloads.isEmpty() && m_iconProvider) {
        delete m_iconProvider;
        m_iconProvider = 0;
    }
    save();
}

void DownloadManager::cleanup_full()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    bool full_cleanup = settings.value(QLatin1String("full_cleanup"), true).toBool();
    bool first_ask = settings.value(QLatin1String("first_ask"), true).toBool();

    if (first_ask)
    {
        int ret = QMessageBox::question(this, tr("Confirmation"),
                           tr("<b>Do you want to remove downloads history and downloads themselves?</b><br><br>"
                           "Click <b>Yes</b> to clean up downloads and history<br>"
                           "Click <b>No</b> to clean up history but keep downloaded files<br>"
                           "Click <b>Cancel</b> to cancel clean up procedure<br><br>"
                           "Your choice will be stored to settings, and used in future cleanups."
                           ),
                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                           QMessageBox::Yes);
        if (ret == QMessageBox::Cancel)
            return;

        full_cleanup = (ret == QMessageBox::Yes);
        settings.setValue(QLatin1String("full_cleanup"), full_cleanup);
        settings.setValue(QLatin1String("first_ask"), false);
    }
    
    settings.endGroup();

    foreach(DownloadItem *item, m_downloads)
    {
        item->m_must_be_deleted = (full_cleanup);
    }

    if (m_downloads.isEmpty())
        return;

    m_model->removeRows(0, m_downloads.count());
    updateItemCount();
    if (m_downloads.isEmpty() && m_iconProvider) {
        delete m_iconProvider;
        m_iconProvider = 0;
    }
    save();
}

void DownloadManager::updateItemCount()
{
    int count = m_downloads.count();
    itemCount->setText(count == 1 ? tr("1 download") : tr("%1 downloads").arg(count));
}

DownloadModel::DownloadModel(DownloadManager *downloadManager, QObject *parent)
    : QAbstractListModel(parent)
    , m_downloadManager(downloadManager)
{
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();
    if (role == Qt::ToolTipRole)
        if (!m_downloadManager->m_downloads.at(index.row())->downloadedSuccessfully())
            return m_downloadManager->m_downloads.at(index.row())->downloadInfoLabel->text();
    return QVariant();
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_downloadManager->m_downloads.count();
}

bool DownloadModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    int lastRow = row + count - 1;
    for (int i = lastRow; i >= row; --i) {
        if (m_downloadManager->m_downloads.at(i)->downloadedSuccessfully()
            || m_downloadManager->m_downloads.at(i)->tryAgainButton->isEnabled()) {
            beginRemoveRows(parent, i, i);
            DownloadItem* item = m_downloadManager->m_downloads.takeAt(i);
            
            if (item && item->m_output.exists())
            {
                if (item->m_must_be_deleted)
                    item->m_output.remove();
            }

            item->deleteLater();
            endRemoveRows();
        }
    }
    m_downloadManager->save();
    return true;
}

