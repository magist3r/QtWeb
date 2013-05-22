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

#ifndef CHASEWIDGET_H
#define CHASEWIDGET_H

#include <QtGui/QWidget>

#include <QtCore/QSize>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

QT_BEGIN_NAMESPACE
class QHideEvent;
class QShowEvent;
class QPaintEvent;
class QTimerEvent;
QT_END_NAMESPACE

class ChaseWidget : public QWidget
{
    Q_OBJECT
public:
    ChaseWidget(QWidget *parent = 0, QPixmap pixmap = QPixmap(), bool pixmapEnabled = false);

    void setAnimated(bool value);
    void setPixmapEnabled(bool enable);
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    int segmentCount() const;
    QColor colorForSegment(int segment) const;

    int m_segment;
    int m_delay;
    int m_step;
    int m_timerId;
    bool m_animated;
    QPixmap m_pixmap;
    bool m_pixmapEnabled;
};

#endif
