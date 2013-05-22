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

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "ui_downloads.h"
#include "ui_downloaditem.h"

#include <QtNetwork/QNetworkReply>

#include <QtCore/QFile>
#include <QtCore/QTime>

class DownloadItem : public QWidget, public Ui_DownloadItem
{
    Q_OBJECT

signals:
    void statusChanged();

public:
    DownloadItem(QNetworkReply *reply = 0, bool requestFileName = false, QWidget *parent = 0);
    bool downloading() const;
    bool downloadedSuccessfully() const;
	bool initSuccess() {return m_success_init; }
	void setSizeDate(qint64 sz, QDateTime m); 
	void setOutputTitle();

    QUrl m_url;

    QFile m_output;
    QNetworkReply *m_reply;
	bool   m_must_be_deleted;
	bool   m_to_delete;

protected:
	void mouseDoubleClickEvent ( QMouseEvent * event );

public slots:
    void open();

private slots:
    void stop();
    void tryAgain();
    void remove();

    void downloadReadyRead();
    void error(QNetworkReply::NetworkError code);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void metaDataChanged();
    void finished();

private:
    bool getFileName();
    void init();
    void updateInfoLabel();
    QString dataString(int size) const;
	bool checkAddTorrent();
    QString saveFileName(const QString &directory) const;

    bool   m_requestFileName;
    qint64 m_bytesReceived;
    QTime  m_downloadTime;
	bool   m_success_init;
	bool   m_finished;
};

class AutoSaver;
class DownloadModel;
QT_BEGIN_NAMESPACE
class QFileIconProvider;
QT_END_NAMESPACE

QString dirDownloads(bool create_dir);

class DownloadManager : public QDialog, public Ui_DownloadDialog
{
    Q_OBJECT
    Q_PROPERTY(RemovePolicy removePolicy READ removePolicy WRITE setRemovePolicy)
    Q_ENUMS(RemovePolicy)

public:
    enum RemovePolicy {
        Never,
        Exit,
        SuccessFullDownload
    };

    DownloadManager(QWidget *parent = 0);
    ~DownloadManager();
    int activeDownloads() const;
    void load();
	void addItem(const QUrl& url, QString filename, bool done);

    RemovePolicy removePolicy() const;
    void setRemovePolicy(RemovePolicy policy);

public slots:
	void openItem(const QModelIndex& index);
    void download(const QNetworkRequest &request, bool requestFileName = true);
    inline void download(const QUrl &url, bool requestFileName = false)
        { download(QNetworkRequest(url), requestFileName); }
    void handleUnsupportedContent(QNetworkReply *reply, bool requestFileName = false);
	void cleanup_list();
    void cleanup_full();
	void open_downloads();

	private slots:
    void save() const;
    void updateRow();

private:
    void addItem(DownloadItem *item);
    void updateItemCount();

    AutoSaver *m_autoSaver;
    DownloadModel *m_model;
    QNetworkAccessManager *m_manager;
    QFileIconProvider *m_iconProvider;
    QList<DownloadItem*> m_downloads;
    RemovePolicy m_removePolicy;
    friend class DownloadModel;
};

class DownloadModel : public QAbstractListModel
{
    friend class DownloadManager;
    Q_OBJECT

public:
    DownloadModel(DownloadManager *downloadManager, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
    DownloadManager *m_downloadManager;

};

#endif // DOWNLOADMANAGER_H

