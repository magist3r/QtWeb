/*
 * Copyright (C) 2008-2009 Alexei Chaloupov <alexei.chaloupov@gmail.com>
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

#include "certificateinfo.h"
#include "ui_certificateinfo.h"
#include <QDateTime>
#include <QSslSocket>
#include <QSslCipher>

#define SECURE_SSL_PORT 443

CertificateInfo::CertificateInfo(QString host, QWidget *parent)
    : QDialog(parent), m_host(host)
{
    form = new Ui_CertificateInfo;
    form->setupUi(this);

    connect(form->certificationPathView, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateCertificateInfo(int)));
	
	for (int i = 0; i < 5; i++)
		form->certificationPathView->addItem( "");
	form->certificationPathView->addItem( tr("Loading certificate info for ") + host + " ...");

    m_socket = new QSslSocket(this);

    connect(m_socket, SIGNAL(encrypted()), this, SLOT(socketEncrypted()));
	connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)),this, SLOT(sslErrors(QList<QSslError>)));

    m_socket->connectToHostEncrypted(m_host, SECURE_SSL_PORT);

}

CertificateInfo::~CertificateInfo()
{
    delete form;
}

void CertificateInfo::socketEncrypted()
{
    QSslCipher ciph = m_socket->sessionCipher();
    QString cipher = QString("%1, %2 (%3/%4)").arg(ciph.authenticationMethod())
                     .arg(ciph.name()).arg(ciph.usedBits()).arg(ciph.supportedBits());;

	form->lblSecured->setText( tr("Encryption: ") + cipher );

	setCertificateChain(m_socket->peerCertificateChain());
}

void CertificateInfo::sslErrors(const QList<QSslError> &)
{
    m_socket->ignoreSslErrors();
}

void CertificateInfo::setCertificateChain(const QList<QSslCertificate> &chain)
{
    this->chain = chain;

    form->certificationPathView->clear();

    for (int i = 0; i < chain.size(); ++i) {
        const QSslCertificate &cert = chain.at(i);
        form->certificationPathView->addItem(tr("%1%2 (%3)").arg(!i ? QString() : tr("Issued by: "))
                                             .arg(cert.subjectInfo(QSslCertificate::Organization))
                                             .arg(cert.subjectInfo(QSslCertificate::CommonName)));
    }

    form->certificationPathView->setCurrentRow(0);
}

void CertificateInfo::updateCertificateInfo(int index)
{
    form->certificateInfoView->clear();
    if (index >= 0 && index < chain.size()) {
        const QSslCertificate &cert = chain.at(index);
        QStringList lines;
		lines << tr("Issued to:   %1").arg(cert.subjectInfo(QSslCertificate::Organization))
			  << tr("Issued by:  %1").arg(cert.issuerInfo(QSslCertificate::Organization))
			  << tr("Valid from:  %1 to %2").arg(cert.effectiveDate().date().toString("MMM dd, yyyy")).arg(cert.expiryDate().date().toString("MMM dd, yyyy"))
			  << tr("")
			  << tr("Organization: %1").arg(cert.subjectInfo(QSslCertificate::Organization))
              << tr("Subunit: %1").arg(cert.subjectInfo(QSslCertificate::OrganizationalUnitName))
              << tr("Country: %1").arg(cert.subjectInfo(QSslCertificate::CountryName))
              << tr("Locality: %1").arg(cert.subjectInfo(QSslCertificate::LocalityName))
              << tr("State/Province: %1").arg(cert.subjectInfo(QSslCertificate::StateOrProvinceName))
              << tr("Common Name: %1").arg(cert.subjectInfo(QSslCertificate::CommonName))
              << QString()
              << tr("Issuer Organization: %1").arg(cert.issuerInfo(QSslCertificate::Organization))
              << tr("Issuer Unit Name: %1").arg(cert.issuerInfo(QSslCertificate::OrganizationalUnitName))
              << tr("Issuer Country: %1").arg(cert.issuerInfo(QSslCertificate::CountryName))
              << tr("Issuer Locality: %1").arg(cert.issuerInfo(QSslCertificate::LocalityName))
              << tr("Issuer State/Province: %1").arg(cert.issuerInfo(QSslCertificate::StateOrProvinceName))
              << tr("Issuer Common Name: %1").arg(cert.issuerInfo(QSslCertificate::CommonName));
        foreach (QString line, lines)
            form->certificateInfoView->addItem(line);
    } else {
        form->certificateInfoView->clear();
    }
}
