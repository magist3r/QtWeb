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

#include "urllineedit.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "searchlineedit.h"
#include "webview.h"
#include "certificateinfo.h"

#include <QtCore/QEvent>

#include <QtGui/QApplication>
#include <QtGui/QCompleter>
#include <QtGui/QFocusEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionFrameV2>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>


UrlIconLabel::UrlIconLabel(QWidget *parent)
    : QLabel(parent)
    , m_webView(0)
{
    setMinimumWidth(16);
    setMinimumHeight(16);

	checkToolTip();
}

void UrlIconLabel::checkToolTip()
{
	if (m_webView && !m_webView->url().isEmpty() && m_webView->url().scheme() == "https")
	{
		setToolTip("<p style='white-space:pre'>" + tr("Secure connection to <b>") + m_webView->url().host() + 
			tr("</b> is encrypted via SSL <p>Click the lock icon for the certificate and encryption details.")); 
	}
}

void UrlIconLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
	{
		m_dragStartPos = event->pos();

		if (m_webView && !m_webView->url().isEmpty() && m_webView->url().scheme() == "https")
		{
			CertificateInfo *info = new CertificateInfo(m_webView->url().host(), this);
			info->exec();
			info->deleteLater();
			return;
		}
		
		UrlLineEdit* lineEdit = (UrlLineEdit*)parent();
		if (lineEdit && lineEdit->lineEdit()->completer()) 
		{
			lineEdit->lineEdit()->completer()->complete();
		}		
	}
    QLabel::mousePressEvent(event);
}

void UrlIconLabel::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
	}

    QLabel::mouseReleaseEvent(event);
}

void UrlIconLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton
        && (event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance()
         && m_webView) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(m_webView->url().toString());
        QList<QUrl> urls;
        urls.append(m_webView->url());
        mimeData->setUrls(urls);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}

UrlLineEdit::UrlLineEdit(QWidget *parent)
    : ExLineEdit(parent, true)
    , m_webView(0)
    , m_iconLabel(0)
{
    // icon
    m_iconLabel = new UrlIconLabel(this);
    m_iconLabel->resize(16, 16);
    setLeftWidget(m_iconLabel);
    m_defaultBaseColor = palette().color(QPalette::Base);

	connect(m_lineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(editingURL(const QString&)));

    webViewIconChanged();
}

void UrlLineEdit::editingURL(const QString& )
{
	BrowserApplication::instance()->mainWindow()->setLoadIcon();
}

void UrlLineEdit::setWebView(WebView *webView)
{
    Q_ASSERT(!m_webView);
    m_webView = webView;
    m_iconLabel->m_webView = webView;
    connect(webView, SIGNAL(urlChanged(const QUrl &)),
        this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView, SIGNAL(loadFinished(bool)),
        this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(iconChanged()),
        this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(loadProgress(int)),
        this, SLOT(update()));

	// AC: to avoid blue non-completed
	connect(webView, SIGNAL(loadFinished(bool)),
        this, SLOT(update()));

	connect(this, SIGNAL(completed(QString)), this, SLOT(loadUrl(QString)));
}

void UrlLineEdit::loadUrl(QString url)
{
	if (m_webView && m_webView == BrowserApplication::instance()->mainWindow()->currentTab())
	{
		QUrl u = BrowserMainWindow::guessUrlFromString(url);
		m_webView->loadUrl( u.toString() );
	}
}

void UrlLineEdit::webViewUrlChanged(const QUrl &url)
{
    m_lineEdit->setText( url.toString() );
    m_lineEdit->setCursorPosition(0);
    QIcon icon = BrowserApplication::instance()->icon(m_webView->url());
    m_iconLabel->setPixmap( icon.pixmap(16,16) );
}

void UrlLineEdit::webViewIconChanged()
{
	if (m_webView)
	{
		QIcon icon = BrowserApplication::instance()->icon(m_webView->url()); 
		if (!icon.isNull())
		{
			QPixmap pixmap(icon.pixmap(16, 16));
			m_iconLabel->setPixmap(pixmap);
			m_iconLabel->checkToolTip();
		}
	}
}

QLinearGradient UrlLineEdit::generateGradient(const QColor &color) const
{
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_defaultBaseColor);
    gradient.setColorAt(0.15, color.lighter(120));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(0.85, color.lighter(120));
    gradient.setColorAt(1, m_defaultBaseColor);
    return gradient;
}

void UrlLineEdit::focusOutEvent(QFocusEvent *event)
{
    //if (m_lineEdit->text().isEmpty() && m_webView)
    //    m_lineEdit->setText(m_webView->url().toString()); // AC: otherwise context menu re-populates the EditBox

    ExLineEdit::focusOutEvent(event);
}

void UrlLineEdit::paintEvent(QPaintEvent *event)
{
    QPalette p = palette();
    if (m_webView && m_webView->url().scheme() == QLatin1String("https")) 
	{
		if (m_webView->sslErrors())
		{
			// Light Red
			p.setBrush(QPalette::Base, generateGradient(QColor(250, 171, 191)));
		}
		else
		{
			// Light Green
			p.setBrush(QPalette::Base, generateGradient(QColor(0x90, 0xEE, 0x90)));
		}
    } else {
        p.setBrush(QPalette::Base, m_defaultBaseColor);
    }
    setPalette(p);
    ExLineEdit::paintEvent(event);

    QPainter painter(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
	backgroundRect.setWidth(backgroundRect.width() - m_clearButton->width());
    if (m_webView && !hasFocus()) {
        int progress = m_webView->progress();
        QColor loadingColor = QColor(116, 192, 250);
        painter.setBrush(generateGradient(loadingColor));
        painter.setPen(Qt::transparent);
        int mid = backgroundRect.width() / 100 * progress;
        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);
    }
}


