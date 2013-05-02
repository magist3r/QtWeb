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

#include "exlineedit.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "tabwidget.h"
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


ClearButton::ClearButton(QWidget *parent)
  : QAbstractButton(parent)
{
    setCursor(Qt::ArrowCursor);
    setToolTip(tr("Clear"));
    setVisible(false);
    setFocusPolicy(Qt::NoFocus);
}

void ClearButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    int height = this->height();

    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor color = palette().color(QPalette::Midlight);
    painter.setBrush(isDown()
                     ? palette().color(QPalette::Mid)
                     : palette().color(QPalette::Midlight));
    painter.setPen(painter.brush().color());
    int size = width();
    int offset = size / 5;
    int radius = size - offset * 2;
    painter.drawEllipse(offset, offset, radius, radius);

    painter.setPen(palette().color(QPalette::Base));
    int border = offset * 2;
    painter.drawLine(border, border, width() - border, height - border);
    painter.drawLine(border, height - border, width() - border, border);
}

void ClearButton::textChanged(const QString &text)
{
    setVisible(!text.isEmpty());
}

LineEdit::LineEdit(QWidget *parent, bool fix_url) :
	QLineEdit( parent)
	, m_fix_url(fix_url)
{

}

void LineEdit::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (!hasSelectedText())
			selectAll();
	}

	QLineEdit::mouseReleaseEvent(event);
}

void LineEdit::keyPressEvent ( QKeyEvent * event )
{
	if ( event->key() == Qt::Key_Return && m_fix_url)
	{
		if (event->modifiers() & Qt::ControlModifier)
		{
			QString t = text().trimmed();
			if (t.length() > 0)
			{
				if (t.toLower().indexOf("http://") != 0)
				{
					t = "http://" + t;
				}
				if (t.toLower().indexOf("www.") != 7)
				{
					t.insert(7, "www.");
				}
				
				int ind = t.indexOf('.', 12);
				if ( ind == -1)
				{
					t += ".com/";
				}
				else
					if (ind == t.length() - 1)
						t += "com/";

				setText( t );
			}
		}
	}

	// ALT+ENTER - open link in a new tab
	if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::AltModifier))
	{
		BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
		mainWindow->tabWidget()->loadUrlNewTab(  BrowserMainWindow::guessUrlFromString( text() ) );
		clear();
		return;
	}

	QLineEdit::keyPressEvent(event);
}


////////////////////////////////////////
ExLineEdit::ExLineEdit(QWidget *parent, bool fix_url)
    : QWidget(parent)
    , m_leftWidget(0)
    , m_lineEdit(new LineEdit(this, fix_url))
    , m_clearButton(0)
{
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(m_lineEdit->sizePolicy());
    setBackgroundRole(m_lineEdit->backgroundRole());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    QPalette p = m_lineEdit->palette();
    setPalette(p);

    // line edit
    m_lineEdit->setFrame(false);
    m_lineEdit->setFocusProxy(this);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    QPalette clearPalette = m_lineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    m_lineEdit->setPalette(clearPalette);

    // clearButton
    m_clearButton = new ClearButton(this);
	connect(m_lineEdit, SIGNAL(textChanged(const QString&)),
            m_clearButton, SLOT(textChanged(const QString&)));

	connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(cleared()));
}

void ExLineEdit::cleared()
{
	m_lineEdit->clear();
	if (m_lineEdit && m_lineEdit->completer())
	{
		m_lineEdit->completer()->setCompletionPrefix( "http://" ); 
		m_lineEdit->completer()->complete();
	}
	m_lineEdit->clear();
}

void ExLineEdit::setLeftWidget(QWidget *widget)
{
    m_leftWidget = widget;
}

QWidget *ExLineEdit::leftWidget() const
{
    return m_leftWidget;
}

void ExLineEdit::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(m_leftWidget);
    updateGeometries();
    QWidget::resizeEvent(event);
}

void ExLineEdit::updateGeometries()
{
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int height = rect.height();
    int width = rect.width();

    int m_leftWidgetHeight = m_leftWidget->height();
    m_leftWidget->setGeometry(rect.x() + 1,          rect.y() + (height - m_leftWidgetHeight)/2,
                              m_leftWidget->width(), m_leftWidget->height());

    int clearButtonWidth = this->height();
    m_lineEdit->setGeometry(m_leftWidget->x() + m_leftWidget->width(),        0,
                            width - clearButtonWidth - m_leftWidget->width(), this->height());

    m_clearButton->setGeometry(this->width() - clearButtonWidth, 0,
                               clearButtonWidth, this->height());
}

void ExLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (m_lineEdit->isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrameV2::None;
}

QSize ExLineEdit::sizeHint() const
{
    m_lineEdit->setFrame(true);
    QSize size = m_lineEdit->sizeHint();
    m_lineEdit->setFrame(false);
    return size;
}

void ExLineEdit::focusInEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);
    QWidget::focusInEvent(event);
}

void ExLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);

    if (m_lineEdit->completer()) {
        connect(m_lineEdit->completer(), SIGNAL(activated(QString)),
                         m_lineEdit, SLOT(setText(QString)));

		connect(m_lineEdit->completer(), SIGNAL(activated(QString)),
                         this, SLOT(isCompleted(QString)));

        connect(m_lineEdit->completer(), SIGNAL(highlighted(QString)),
                         m_lineEdit, SLOT(_q_completionHighlighted(QString)));
    }
    QWidget::focusOutEvent(event);
}

void ExLineEdit::isCompleted(QString q)
{
	emit completed(q);
}

void ExLineEdit::keyPressEvent(QKeyEvent *event)
{
    m_lineEdit->event(event);

    QWidget::keyPressEvent(event);
}

bool ExLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        m_lineEdit->event(event);
    return QWidget::event(event);
}

void ExLineEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
}



