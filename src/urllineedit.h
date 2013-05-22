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

#ifndef URLLINEEDIT_H
#define URLLINEEDIT_H

#include <QtCore/QUrl>
#include <QLabel>
#include <QtGui/QWidget>
#include <QtGui/QStyleOptionFrame>
#include "exlineedit.h"

class WebView;

class UrlIconLabel : public QLabel
{
    Q_OBJECT

public:
    UrlIconLabel(QWidget *parent);
	void checkToolTip();

    WebView *m_webView;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QPoint m_dragStartPos;

};


class UrlLineEdit : public ExLineEdit
{
    Q_OBJECT

public:
    UrlLineEdit(QWidget *parent = 0);
    void setWebView(WebView *webView);

protected:
    void paintEvent(QPaintEvent *event);
    void focusOutEvent(QFocusEvent *event);

private slots:
    void webViewUrlChanged(const QUrl &url);
    void webViewIconChanged();
	void loadUrl(QString);
	void editingURL(const QString&);

private:
    QLinearGradient generateGradient(const QColor &color) const;
    WebView *m_webView;
    UrlIconLabel *m_iconLabel;
    QColor m_defaultBaseColor;

};


#endif // URLLINEEDIT_H

