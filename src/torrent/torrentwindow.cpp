/****************************************************************************
**
 * Copyright (C) 2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtWidgets>
#include <QTreeWidget>
#include <QItemDelegate>

#include "addtorrentdialog.h"
#include "torrentwindow.h"
#include "ratecontroller.h"
#include "torrentclient.h"

// TorrentView extends QTreeWidget to allow drag and drop.
class TorrentView : public QTreeWidget
{
    Q_OBJECT
public:
    TorrentView(QWidget *parent = 0);

signals:
    void fileDropped(const QString &fileName);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};

// TorrentViewDelegate is used to draw the progress bars.
class TorrentViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    inline TorrentViewDelegate(TorrentWindow *mainWindow) : QItemDelegate(mainWindow) {}

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        if (index.column() != 2) {
            QItemDelegate::paint(painter, option, index);
            return;
        }

        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        int progress = qobject_cast<TorrentWindow *>(parent())->clientForRow(index.row())->progress();
        progressBarOption.progress = progress < 0 ? 0 : progress;
        progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
};

TorrentWindow::TorrentWindow(QWidget *parent)
    : QMainWindow(parent), quitDialog(0), saveChanges(false)
{
    // Initialize some static strings
    QStringList headers;
    headers << tr("Torrent") << tr("Peers/Seeds") << tr("Progress")
            << tr("Down rate") << tr("Up rate") << tr("Status");

    // Main torrent list
    torrentView = new TorrentView(this);
    torrentView->setItemDelegate(new TorrentViewDelegate(this));
    torrentView->setHeaderLabels(headers);
    torrentView->setSelectionBehavior(QAbstractItemView::SelectRows);
    torrentView->setAlternatingRowColors(true);
    torrentView->setRootIsDecorated(false);
    setCentralWidget(torrentView);

    // Set header resize modes and initial section sizes
    QFontMetrics fm = fontMetrics();
    QHeaderView *header = torrentView->header();
    header->resizeSection(0, fm.width("typical-name-for-a-torrent.torrent"));
    header->resizeSection(1, fm.width(headers.at(1) + "  "));
    header->resizeSection(2, fm.width(headers.at(2) + "  "));
    header->resizeSection(3, qMax(fm.width(headers.at(3) + "  "), fm.width(" 1234.0 KB/s ")));
    header->resizeSection(4, qMax(fm.width(headers.at(4) + "  "), fm.width(" 1234.0 KB/s ")));
    header->resizeSection(5, qMax(fm.width(headers.at(5) + "  "), fm.width(tr("Downloading") + "  ")));

    // Create common actions
    QAction *newTorrentAction = new QAction(QIcon(":/icons/bottom.png"), tr("Add &new torrent"), this);
    pauseTorrentAction = new QAction(QIcon(":/icons/player_pause.png"), tr("&Pause torrent"), this);
    removeTorrentAction = new QAction(QIcon(":/icons/player_stop.png"), tr("&Remove torrent"), this);
    
    // Top toolbar
    QToolBar *topBar = new QToolBar(tr("Tools"));
    addToolBar(Qt::TopToolBarArea, topBar);
    topBar->setMovable(false);
    topBar->addAction(newTorrentAction);
    topBar->addAction(removeTorrentAction);
    topBar->addAction(pauseTorrentAction);
    topBar->addSeparator();
    downActionTool = topBar->addAction(QIcon(":/icons/1downarrow.png"), tr("Move down"));
    upActionTool = topBar->addAction(QIcon(":/icons/1uparrow.png"), tr("Move up"));

    // Bottom toolbar
    QToolBar *bottomBar = new QToolBar(tr("Rate control"));
    addToolBar(Qt::BottomToolBarArea, bottomBar);
    bottomBar->setMovable(false);
    downloadLimitSlider = new QSlider(Qt::Horizontal);
    downloadLimitSlider->setRange(0, 1000);
    bottomBar->addWidget(new QLabel(tr("Max download:")));
    bottomBar->addWidget(downloadLimitSlider);
    bottomBar->addWidget((downloadLimitLabel = new QLabel(tr("0 KB/s"))));
    downloadLimitLabel->setFixedSize(QSize(fm.width(tr("99999 KB/s")), fm.lineSpacing()));
    bottomBar->addSeparator();
    uploadLimitSlider = new QSlider(Qt::Horizontal);
    uploadLimitSlider->setRange(0, 1000);
    bottomBar->addWidget(new QLabel(tr("Max upload:")));
    bottomBar->addWidget(uploadLimitSlider);
    bottomBar->addWidget((uploadLimitLabel = new QLabel(tr("0 KB/s"))));
    uploadLimitLabel->setFixedSize(QSize(fm.width(tr("99999 KB/s")), fm.lineSpacing()));

    // Set up connections
    connect(torrentView, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActionsEnabled()));
    connect(torrentView, SIGNAL(fileDropped(const QString &)),
            this, SLOT(acceptFileDrop(const QString &)));
    connect(uploadLimitSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setUploadLimit(int)));
    connect(downloadLimitSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setDownloadLimit(int)));
    connect(newTorrentAction, SIGNAL(triggered()),
            this, SLOT(addTorrent()));
    connect(pauseTorrentAction, SIGNAL(triggered()),
            this, SLOT(pauseTorrent()));
    connect(removeTorrentAction, SIGNAL(triggered()),
            this, SLOT(removeTorrent()));
    connect(upActionTool, SIGNAL(triggered(bool)),
            this, SLOT(moveTorrentUp()));
    connect(downActionTool, SIGNAL(triggered(bool)),
            this, SLOT(moveTorrentDown()));

    // Load settings and start
    setWindowTitle(tr("Torrents"));
    setActionsEnabled();
    QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);
}

TorrentWindow::~TorrentWindow()
{
    if (jobs.isEmpty())
        return;

    // Save upload / download numbers.
    saveSettings();
    saveChanges = false;

    quitDialog = new QProgressDialog(tr("Disconnecting from trackers"), tr("Abort"), 0, jobsToStop, this);

    // Stop all clients, remove the rows from the view and wait for
    // them to signal that they have stopped.
    jobsToStop = 0;
    jobsStopped = 0;
    foreach (Job job, jobs) {
        ++jobsToStop;
        TorrentClient *client = job.client;
        client->disconnect();
        connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
        client->stop();
        delete torrentView->takeTopLevelItem(0);
    }

    if (jobsToStop > jobsStopped)
        quitDialog->exec();
    quitDialog->deleteLater();
    quitDialog = 0;
}

QSize TorrentWindow::sizeHint() const
{
    const QHeaderView *header = torrentView->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = fontMetrics().width(tr("Downloading") + "  ");
    for (int i = 0; i < header->count() - 1; ++i)
        width += header->sectionSize(i);

    return QSize(width, QMainWindow::sizeHint().height())
        .expandedTo(QApplication::globalStrut());
}

const TorrentClient *TorrentWindow::clientForRow(int row) const
{
    // Return the client at the given row.
    return jobs.at(row).client;
}

int TorrentWindow::rowOfClient(TorrentClient *client) const
{
    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    foreach (Job job, jobs) {
        if (job.client == client)
            return row;
        ++row;
    }
    return -1;
}

void TorrentWindow::loadSettings()
{
    // Load base settings (last working directory, upload/download limits).
    QSettings settings;

    settings.beginGroup(QLatin1String("downloadmanager"));
    QString downloadDirectory = settings.value(QLatin1String("downloadDirectory")).toString();

    settings.endGroup();

    settings.beginGroup("Torrents");
    lastDirectory = settings.value("LastDirectory").toString();
    if (lastDirectory.isEmpty())
        if (!downloadDirectory.isEmpty())
            lastDirectory = downloadDirectory;
        else
            lastDirectory = QDir::currentPath();
    int up = settings.value("UploadLimit").toInt();
    int down = settings.value("DownloadLimit").toInt();
    uploadLimitSlider->setValue(up ? up : 170);
    downloadLimitSlider->setValue(down ? down : 550);

    // Resume all previous downloads.
    int size = settings.beginReadArray("Torrents");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QByteArray resumeState = settings.value("resumeState").toByteArray();
        QString fileName = settings.value("sourceFileName").toString();
        QString dest = settings.value("destinationFolder").toString();

        if (addTorrent(fileName, dest, resumeState)) {
            TorrentClient *client = jobs.last().client;
            client->setDownloadedBytes(settings.value("downloadedBytes").toLongLong());
            client->setUploadedBytes(settings.value("uploadedBytes").toLongLong());
        }
    }
}

bool TorrentWindow::addTorrent(const QString& fileName)
{
    if (fileName.isEmpty())
        return false;
    lastDirectory = QFileInfo(fileName).absolutePath();

    // Show the "Add Torrent" dialog.
    AddTorrentDialog *addTorrentDialog = new AddTorrentDialog(this);
    addTorrentDialog->setTorrent(fileName);
    addTorrentDialog->deleteLater();
    if (!addTorrentDialog->exec())
        return false;

    // Add the torrent to our list of downloads
    addTorrent(fileName, addTorrentDialog->destinationFolder());
    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(1000, this, SLOT(saveSettings()));
    }
    return true;
}

bool TorrentWindow::addTorrent()
{
    // Show the file dialog, let the user select what torrent to start downloading.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a torrent file"),
                                                    lastDirectory,
                                                    tr("Torrents (*.torrent);;"
                                                    " All files (*.*)"));

    return addTorrent(fileName);
}

void TorrentWindow::removeTorrent()
{
    // Find the row of the current item, and find the torrent client
    // for that row.
    int row = torrentView->indexOfTopLevelItem(torrentView->currentItem());
    TorrentClient *client = jobs.at(row).client;

    // Stop the client.
    client->disconnect();
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    client->stop();

    // Remove the row from the view.
    delete torrentView->takeTopLevelItem(row);
    jobs.removeAt(row);
    setActionsEnabled();

    saveChanges = true;
    saveSettings();
}

void TorrentWindow::torrentStopped()
{
    // Schedule the client for deletion.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    client->deleteLater();

    // If the quit dialog is shown, update its progress.
    if (quitDialog) {
        if (++jobsStopped == jobsToStop)
            quitDialog->close();
    }
}

void TorrentWindow::torrentError(TorrentClient::Error)
{
    // Delete the client.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString fileName = jobs.at(row).torrentFileName;
    jobs.removeAt(row);

    // Display the warning.
    QMessageBox::warning(this, tr("Error"),
                         tr("An error occurred while downloading %0: %1")
                         .arg(fileName)
                         .arg(client->errorString()));

    delete torrentView->takeTopLevelItem(row);
    client->deleteLater();
}

bool TorrentWindow::addTorrent(const QString &fileName, const QString &destinationFolder,
                            const QByteArray &resumeState)
{
    // Check if the torrent is already being downloaded.
    foreach (Job job, jobs) {
        if (job.torrentFileName == fileName && job.destinationDirectory == destinationFolder) {
            QMessageBox::warning(this, tr("Already downloading"),
                                 tr("The torrent file %1 is already being downloaded.").arg(fileName));
            return false;
        }
    }

    // Create a new torrent client and attempt to parse the torrent data.
    TorrentClient *client = new TorrentClient(this);
    if (!client->setTorrent(fileName)) {
        delete client;
        return false;
    }
    client->setDestinationFolder(destinationFolder);
    client->setDumpedState(resumeState);

    // Setup the client connections.
    connect(client, SIGNAL(stateChanged(TorrentClient::State)), this, SLOT(updateState(TorrentClient::State)));
    connect(client, SIGNAL(peerInfoUpdated()), this, SLOT(updatePeerInfo()));
    connect(client, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
    connect(client, SIGNAL(downloadRateUpdated(int)), this, SLOT(updateDownloadRate(int)));
    connect(client, SIGNAL(uploadRateUpdated(int)), this, SLOT(updateUploadRate(int)));
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    connect(client, SIGNAL(error(TorrentClient::Error)), this, SLOT(torrentError(TorrentClient::Error)));

    // Add the client to the list of downloading jobs.
    Job job;
    job.client = client;
    job.torrentFileName = fileName;
    job.destinationDirectory = destinationFolder;
    jobs << job;

    // Create and add a row in the torrent view for this download.
    QTreeWidgetItem *item = new QTreeWidgetItem(torrentView);

    QString baseFileName = QFileInfo(fileName).fileName();
    if (baseFileName.toLower().endsWith(".torrent"))
        baseFileName.remove(baseFileName.size() - 8);

    item->setText(0, baseFileName);
    item->setToolTip(0, tr("Torrent: %1<br>Destination: %2")
                     .arg(baseFileName).arg(destinationFolder));
    item->setText(1, tr("0/0"));
    item->setText(2, "0");
    item->setText(3, "0.0 KB/s");
    item->setText(4, "0.0 KB/s");
    item->setText(5, tr("Idle"));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(1, Qt::AlignHCenter);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
    client->start();
    return true;
}

void TorrentWindow::saveSettings()
{
    if (!saveChanges)
      return;
    saveChanges = false;

    // Prepare and reset the settings
    QSettings settings;
    settings.beginGroup("Torrents");
    settings.remove("");

    settings.setValue("LastDirectory", lastDirectory);
    settings.setValue("UploadLimit", uploadLimitSlider->value());
    settings.setValue("DownloadLimit", downloadLimitSlider->value());

    // Store data on all known torrents
    settings.beginWriteArray("Torrents");
    for (int i = 0; i < jobs.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("sourceFileName", jobs.at(i).torrentFileName);
        settings.setValue("destinationFolder", jobs.at(i).destinationDirectory);
        settings.setValue("uploadedBytes", jobs.at(i).client->uploadedBytes());
        settings.setValue("downloadedBytes", jobs.at(i).client->downloadedBytes());
        settings.setValue("resumeState", jobs.at(i).client->dumpedState());
    }
    settings.endArray();
    settings.sync();
}

void TorrentWindow::updateState(TorrentClient::State)
{
    // Update the state string whenever the client's state changes.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    if (item) {
        item->setToolTip(0, tr("Torrent: %1<br>Destination: %2<br>State: %3")
                         .arg(jobs.at(row).torrentFileName)
                         .arg(jobs.at(row).destinationDirectory)
                         .arg(client->stateString()));

        item->setText(5, client->stateString());
    }
    setActionsEnabled();
}

void TorrentWindow::updatePeerInfo()
{
    // Update the number of connected, visited, seed and leecher peers.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    item->setText(1, tr("%1/%2").arg(client->connectedPeerCount())
                  .arg(client->seedCount()));
}

void TorrentWindow::updateProgress(int percent)
{
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    // Update the progressbar.
    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    if (item)
        item->setText(2, QString::number(percent));
}

void TorrentWindow::setActionsEnabled()
{
    // Find the view item and client for the current row, and update
    // the states of the actions.
    QTreeWidgetItem *item = 0;
    if (!torrentView->selectedItems().isEmpty())
        item = torrentView->selectedItems().first();
    TorrentClient *client = item ? jobs.at(torrentView->indexOfTopLevelItem(item)).client : 0;
    bool pauseEnabled = client && ((client->state() == TorrentClient::Paused)
                                       ||  (client->state() > TorrentClient::Preparing));

    removeTorrentAction->setEnabled(item != 0);
    pauseTorrentAction->setEnabled(item != 0 && pauseEnabled);

    if (client && client->state() == TorrentClient::Paused) {
        pauseTorrentAction->setIcon(QIcon(":/icons/player_play.png"));
        pauseTorrentAction->setText(tr("Resume torrent"));
    } else {
        pauseTorrentAction->setIcon(QIcon(":/icons/player_pause.png"));
        pauseTorrentAction->setText(tr("Pause torrent"));
    }

    int row = torrentView->indexOfTopLevelItem(item);
    upActionTool->setEnabled(item && row != 0);
    downActionTool->setEnabled(item && row != jobs.size() - 1);
}

void TorrentWindow::updateDownloadRate(int bytesPerSecond)
{
    // Update the download rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    torrentView->topLevelItem(row)->setText(3, num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void TorrentWindow::updateUploadRate(int bytesPerSecond)
{
    // Update the upload rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    torrentView->topLevelItem(row)->setText(4, num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void TorrentWindow::pauseTorrent()
{
    // Pause or unpause the current torrent.
    int row = torrentView->indexOfTopLevelItem(torrentView->currentItem());
    TorrentClient *client = jobs.at(row).client;
    client->setPaused(client->state() != TorrentClient::Paused);
    setActionsEnabled();
}

void TorrentWindow::moveTorrentUp()
{
    QTreeWidgetItem *item = torrentView->currentItem();
    int row = torrentView->indexOfTopLevelItem(item);
    if (row == 0)
        return;

    Job tmp = jobs.at(row - 1);
    jobs[row - 1] = jobs[row];
    jobs[row] = tmp;

    QTreeWidgetItem *itemAbove = torrentView->takeTopLevelItem(row - 1);
    torrentView->insertTopLevelItem(row, itemAbove);
    setActionsEnabled();
}

void TorrentWindow::moveTorrentDown()
{
    QTreeWidgetItem *item = torrentView->currentItem();
    int row = torrentView->indexOfTopLevelItem(item);
    if (row == jobs.size() - 1)
        return;

    Job tmp = jobs.at(row + 1);
    jobs[row + 1] = jobs[row];
    jobs[row] = tmp;

    QTreeWidgetItem *itemAbove = torrentView->takeTopLevelItem(row + 1);
    torrentView->insertTopLevelItem(row, itemAbove);
    setActionsEnabled();
}

static int rateFromValue(int value)
{
    int rate = 0;
    if (value >= 0 && value < 250) {
        rate = 1 + int(value * 0.124);
    } else if (value < 500) {
        rate = 32 + int((value - 250) * 0.384);
    } else if (value < 750) {
        rate = 128 + int((value - 500) * 1.536);
    } else {
        rate = 512 + int((value - 750) * 6.1445);
    }
    return rate;
}

void TorrentWindow::setUploadLimit(int value)
{
    int rate = rateFromValue(value);
    uploadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setUploadLimit(rate * 1024);
}

void TorrentWindow::setDownloadLimit(int value)
{
    int rate = rateFromValue(value);
    downloadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setDownloadLimit(rate * 1024);
}

void TorrentWindow::acceptFileDrop(const QString &fileName)
{
    // Create and show the "Add Torrent" dialog.
    AddTorrentDialog *addTorrentDialog = new AddTorrentDialog;
    lastDirectory = QFileInfo(fileName).absolutePath();
    addTorrentDialog->setTorrent(fileName);
    addTorrentDialog->deleteLater();
    if (!addTorrentDialog->exec())
        return;

    // Add the torrent to our list of downloads.
    addTorrent(fileName, addTorrentDialog->destinationFolder());
    saveSettings();
}

void TorrentWindow::closeEvent(QCloseEvent *)
{
    ;
}

TorrentView::TorrentView(QWidget *parent)
    : QTreeWidget(parent)
{
    setAcceptDrops(true);
}

void TorrentView::dragMoveEvent(QDragMoveEvent *event)
{
    // Accept file actions with a '.torrent' extension.
    QUrl url(event->mimeData()->text());
    if (url.isValid() && url.scheme().toLower() == "file"
            && url.path().toLower().endsWith(".torrent"))
        event->acceptProposedAction();
}

void TorrentView::dropEvent(QDropEvent *event)
{
    // Accept drops if the file has a '.torrent' extension and it
    // exists.
    QString fileName = QUrl(event->mimeData()->text()).path();
    if (QFile::exists(fileName) && fileName.toLower().endsWith(".torrent"))
        emit fileDropped(fileName);
}

#include "torrentwindow.moc"
