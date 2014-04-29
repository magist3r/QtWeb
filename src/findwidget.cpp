/****************************************************************************
**
** Copyright (C) 2010 Alexei Chaloupov <alexei.chaloupov@gmail.com>
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/
#include "findwidget.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtGui/QHideEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , appPalette(qApp->palette())
{
    installEventFilter(this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);

#ifndef Q_OS_MAC
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing(6);
#endif

    toolClose = setupToolButton(QLatin1String(""),
        QLatin1String(":closetab.png"));
    hboxLayout->addWidget(toolClose);
    connect(toolClose, SIGNAL(clicked()), SLOT(hide()));

    editFind = new QLineEdit(this);
    hboxLayout->addWidget(editFind);
    editFind->setMinimumSize(QSize(150, 0));
    connect(editFind, SIGNAL(textChanged(QString)), this,
        SLOT(textChanged(QString)));
    connect(editFind, SIGNAL(returnPressed()), this, SIGNAL(findNext()));
    connect(editFind, SIGNAL(textChanged(QString)), this, SLOT(updateButtons()));

    toolPrevious = setupToolButton(tr(""), QLatin1String(":previous.png"));
    connect(toolPrevious, SIGNAL(clicked()), this, SIGNAL(findPrevious()));

    hboxLayout->addWidget(toolPrevious);

    toolNext = setupToolButton(tr(""), QLatin1String(":next.png"));
    hboxLayout->addWidget(toolNext);
    connect(toolNext, SIGNAL(clicked()), this, SIGNAL(findNext()));

    checkCase = new QCheckBox(tr("Case Sensitive"), this);
    hboxLayout->addWidget(checkCase);

    labelWrapped = new QLabel(this);
    labelWrapped->setScaledContents(true);
    labelWrapped->setTextFormat(Qt::RichText);
    labelWrapped->setMinimumSize(QSize(0, 20));
    labelWrapped->setMaximumSize(QSize(105, 20));
    labelWrapped->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
    labelWrapped->setText("<i>" +  tr("Search wrapped"));
    hboxLayout->addWidget(labelWrapped);

    QSpacerItem *spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding,
        QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem);
    setMinimumWidth(minimumSizeHint().width());
    labelWrapped->hide();

    updateButtons();
}

FindWidget::~FindWidget()
{
}

void FindWidget::show()
{
    QWidget::show();
    editFind->selectAll();
    editFind->setFocus(Qt::ShortcutFocusReason);
}

void FindWidget::showAndClear()
{
    show();
    editFind->clear();
}

QString FindWidget::text() const
{
    return editFind->text();
}

bool FindWidget::caseSensitive() const
{
    return checkCase->isChecked();
}

void FindWidget::setPalette(bool found)
{
    QPalette palette = editFind->palette();
    palette.setColor(QPalette::Active, QPalette::Base, found ? Qt::white
        : QColor(255, 102, 102));
    editFind->setPalette(palette);
}

void FindWidget::setTextWrappedVisible(bool visible)
{
    labelWrapped->setVisible(visible);
}

void FindWidget::hideEvent(QHideEvent* event)
{
#if !defined(QT_NO_WEBKIT)
    // TODO: remove this once webkit supports setting the palette
    if (!event->spontaneous())
        qApp->setPalette(appPalette);
#else
    Q_UNUSED(event);
#endif
}

void FindWidget::showEvent(QShowEvent* event)
{
#if !defined(QT_NO_WEBKIT)
    // TODO: remove this once webkit supports setting the palette
    if (!event->spontaneous()) {
        QPalette p = appPalette;
        p.setColor(QPalette::Inactive, QPalette::Highlight,
            p.color(QPalette::Active, QPalette::Highlight));
        p.setColor(QPalette::Inactive, QPalette::HighlightedText,
            p.color(QPalette::Active, QPalette::HighlightedText));
        qApp->setPalette(p);
    }
#else
    Q_UNUSED(event);
#endif
}

void FindWidget::updateButtons()
{
    const bool enable = !editFind->text().isEmpty();
    toolNext->setEnabled(enable);
    toolPrevious->setEnabled(enable);
}

void FindWidget::textChanged(const QString &text)
{
    emit find(text, true);
}

bool FindWidget::eventFilter(QObject *object, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        if ((static_cast<QKeyEvent*>(e))->key() == Qt::Key_Escape) {
            hide();
            emit escapePressed();
        }
    }
    return QWidget::eventFilter(object, e);
}

QToolButton* FindWidget::setupToolButton(const QString &text, const QString &icon)
{
    QToolButton *toolButton = new QToolButton(this);

    toolButton->setText(text);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon(icon));
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    return toolButton;
}

QT_END_NAMESPACE
