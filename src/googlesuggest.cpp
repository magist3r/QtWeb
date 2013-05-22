/****************************************************************************
**
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

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QSettings>

#include "googlesuggest.h"

#ifdef Q_WS_WIN
	#include "windows.h"
#endif


#define GSUGGEST_URL "http://google.com/complete/search?output=toolbar&q=%1"

GSuggestCompletion::GSuggestCompletion(QLineEdit *parent): QObject(parent), editor(parent)
{
    popup = new QTreeWidget;
    popup->setColumnCount(2);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//popup->setMaximumWidth( parent->width() );

    popup->header()->hide();
    popup->installEventFilter(this);
    popup->setMouseTracking(true);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(doneCompletion()));

    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));

    connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));

    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    if (settings.value(QLatin1String("enabled"), false).toBool()) 
	{
		QNetworkProxy pxy;
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
            pxy = QNetworkProxy::Socks5Proxy;
        else
            pxy = QNetworkProxy::HttpProxy;
        pxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        pxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        pxy.setUser(settings.value(QLatin1String("userName")).toString());
        pxy.setPassword(settings.value(QLatin1String("password")).toString());
	    networkManager.setProxy(pxy);
    }
	else
    if (settings.value(QLatin1String("autoProxy"), false).toBool()) 
	{

#ifdef Q_WS_WIN
		QNetworkProxy pxy;
		HKEY hKey;
		wchar_t key[256];
		wcscpy(key, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\");
		if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey))
		{
			DWORD dwEnabled = FALSE; DWORD dwBufSize = sizeof(dwEnabled);
			if (RegQueryValueExW(hKey, L"ProxyEnable", NULL, NULL, (LPBYTE)&dwEnabled, &dwBufSize ) == ERROR_SUCCESS
				&& dwEnabled)
			{
				memset(key,0,sizeof(key)); dwBufSize = sizeof(key) / sizeof(TCHAR);
				if (RegQueryValueExW(hKey, L"ProxyServer", NULL, NULL, (LPBYTE)&key, &dwBufSize ) == ERROR_SUCCESS)
				{
					pxy = QNetworkProxy::HttpProxy;
					QStringList lst = QString::fromWCharArray(key).split(':');
					if (lst.size() > 0)
					{
						pxy.setHostName(lst.at(0));
						pxy.setPort( lst.size() > 1 ? lst.at(1).toInt() : 1080 );
						networkManager.setProxy(pxy);
					}
				}
			}
			RegCloseKey(hKey);
		}
#endif
	}

	settings.endGroup();
}

GSuggestCompletion::~GSuggestCompletion()
{
    delete popup;
}

bool GSuggestCompletion::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            doneCompletion();
            consumed = true;

        case Qt::Key_Escape:
            editor->setFocus();
            popup->hide();
            consumed = true;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            editor->setFocus();
            editor->event(ev);
            popup->hide();
            break;
        }

        return consumed;
    }

    return false;
}

void GSuggestCompletion::showCompletion(const QStringList &choices, const QStringList &hits)
{

    if (choices.isEmpty() || choices.count() != hits.count())
        return;

    const QPalette &pal = editor->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    popup->setUpdatesEnabled(false);
    popup->clear();
    for (int i = 0; i < choices.count(); ++i) {
        QTreeWidgetItem * item;
        item = new QTreeWidgetItem(popup);
        item->setText(0, choices[i]);
        item->setText(1, hits[i]);
        item->setTextAlignment(1, Qt::AlignRight);
        item->setTextColor(1, color);
    }
    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->resizeColumnToContents(1);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * qMin(7, choices.count()) + 3;
    popup->resize(popup->width(), h);

    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->setFocus();
    popup->show();
}

void GSuggestCompletion::doneCompletion()
{
    timer->stop();
    popup->hide();
    editor->setFocus();
    QTreeWidgetItem *item = popup->currentItem();
    if (item) {
        editor->setText(item->text(0));
        QKeyEvent *e;
        e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(editor, e);
        e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
        QApplication::postEvent(editor, e);
    }
}

void GSuggestCompletion::preventSuggest()
{
    timer->stop();
}

void GSuggestCompletion::autoSuggest()
{

    QString str = editor->text();
	// Skip local searches by now !!!
	if (str.toAscii() != str)
		return;

    QString url = QString(GSUGGEST_URL).arg(str);
    networkManager.get(QNetworkRequest(QString(url)));
}

void GSuggestCompletion::handleNetworkData(QNetworkReply *networkReply)
{
    QUrl url = networkReply->url();
    if (!networkReply->error()) {
        QStringList choices;
        QStringList hits;

        QString response(networkReply->readAll());
        QXmlStreamReader xml(response);
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement)
                if (xml.name() == "suggestion") {
                    QStringRef str = xml.attributes().value("data");
                    choices << str.toString();
                }
            if (xml.tokenType() == QXmlStreamReader::StartElement)
                if (xml.name() == "num_queries") {
                    QStringRef str = xml.attributes().value("int");
                    hits << str.toString();
                }
        }

        showCompletion(choices, hits);
    }

    networkReply->deleteLater();
}
