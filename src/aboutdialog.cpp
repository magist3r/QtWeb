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

#include "aboutdialog.h"
#include "browserapplication.h"
#include <QMessageBox>

#include <qdialogbuttonbox.h>
#include "webpage.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle(tr("About") + QLatin1String(" ") + qApp->applicationName());
    logo->setPixmap(QIcon(QLatin1String(":/logo.png")).pixmap(128, 128));
    name->setText(qApp->applicationName() + " " + QApplication::applicationVersion());
    connect(creditsButton, SIGNAL(clicked()), this, SLOT(credits()));

    author->setText(WebPage::getUserAgent());

    int dwBuild = BrowserApplication::getApplicationBuild();
    if (dwBuild > 0)
    {
        QString ver = name->text() + QString(" <font size=-2>(build %1)</font>").arg(dwBuild,3,10,QChar('0'));
        name->setText(ver);
    }
}


void AboutDialog::credits()
{
    QMessageBox::information(this, tr("Credits..."), QString("<b>Nokia Qt Team<br>Apple WebKit Team<br>OpenSSL Project Team</b><br>" \
        "Alex Harder<br>Tony Yu<br>Alex Chaloupov<br>Yury Zimin<br>Olga Volkova<br>") + QChar(0x00C5) +  "ke Engelbrektson<br>Otsubo Kanako<br>Istvan Somlai<br>Hessam Mohamadi<br>Sergei Lopatin");
}
