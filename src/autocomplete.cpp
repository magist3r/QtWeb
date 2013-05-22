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

#include "autocomplete.h"
#include <QUrl>
#include <QMap>
#include <QDomElement>
#include <QSettings>
#include <QWebFrame>
#include <QWebSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QByteArray>

QString EncryptPassword(QString str)
{
        QByteArray xor1(QString("X!2$6*9(SKiasb+!v<.qF58_qwe~QsRTYvdeTYb").toUtf8());
	QString HEX("0123456789ABCDEF");
	wchar_t arr[256];
	memset(arr,0,sizeof(arr));
	int size = str.toWCharArray(arr);

	QString out;

        for (int i = 0; i < size && i < xor1.size(); i++)
	{
                arr[i] = arr[i] ^ xor1.at(i);

		for (int j=0; j<4; j++)
		{
			int m = arr[i] % 16;
			out.append( HEX[ m ]);
			arr[i] = arr[i] >> 4;
		}

	}
	
	
	return out;
}

QString DecryptPassword(QString str)
{
        QByteArray xor1 (QString("X!2$6*9(SKiasb+!v<.qF58_qwe~QsRTYvdeTYb").toUtf8());
	wchar_t arr[256];
	memset(arr,0,sizeof(arr));

	int cnt = 0;
	for(int i = 0; i < str.size(); i+=4)
	{
		for (int j = 0; j < 4 ; j++)
		{
			char c = str[i + j].toAscii();
			int v = 0;
			if (c >= '0' && c <= '9')
				v = c - '0';
			else
			if (c >= 'A' && c <= 'F')
				v = 10 + c - 'A';

			int val = ( v << (j * 4));
			arr[cnt] = arr[cnt] + val;
		}
                arr[cnt] = arr[cnt] ^ xor1[cnt];
		cnt++;
	}


	return QString::fromWCharArray(arr);
}


AutoComplete::AutoComplete(QObject *parent)
	: QObject(parent)
{

}

QString AutoComplete::formUrl(QUrl url)
{
	if (url.isEmpty())
		return "";

	QString u = url.scheme() + "://" + url.host() + url.path();
	u = QUrl::toPercentEncoding(u);
	return u;
}

void AutoComplete::setFormHtml(QUrl url, QString data  )
{
	m_forms[formUrl(url)] = data;
}

void AutoComplete::setFormData(QUrl url, QString data  )
{
	m_forms_data[formUrl(url)] = data;
}

bool AutoComplete::evaluate(QUrl form_url)
{
	QString u = formUrl(form_url);

	if (!m_forms.contains(u) || !m_forms_data.contains(u))
		return false;
	
	QString m_form = m_forms[u];
	QString m_data = m_forms_data[u];

	// do not process AutoComplete if empty
	if (m_form.isEmpty() || m_data.isEmpty())
		return false;

	// Populate form data
	QMap<QString, QString> data_map;
	QUrl url( "?" + m_data );
	QPair<QString, QString> item;
	foreach(item, url.queryItems())
	{
		QString name = item.first;
		QString value = item.second;
		value = value.replace('+', ' ');
		data_map[name.toLower()] = value;
	}

	if (data_map.size() == 0)
		return false;

	// parse Form
	QString html = m_form.toLower();
	int form_ind = html.indexOf("<form");
	bool pwd_found = false;
	bool name_found = false;
	QString pwd_name, user_name;
	int current_form = -1;
	QMap<QString, QString> exclude_map;
	while (form_ind != -1)
	{	
		current_form++;

		QString form = html.mid(form_ind);
		int end = form.indexOf("<form", form_ind + 5);
		if (end != -1)
			form = form.left( end );

		exclude_map.clear();
		// form - pure form
		int inp_ind = form.indexOf("<input");
		while (inp_ind != -1)
		{
			QString inp = form.mid(inp_ind);
			int end = inp.indexOf(">");
			if (end != -1)
			{
				inp = inp.left(end+1);
				inp.insert(inp.length() - 1, '/');

				QDomDocument d;
				d.setContent(inp);
				QDomElement de = d.documentElement();
				if (!de.isNull())
				{
					QDomAttr dn = de.attributeNode("name");
					QDomAttr dt = de.attributeNode("type");

					if (!dn.isNull())
					{
						QString name = dn.value();
						QString type = dt.value();

						if (!name.isNull() )
						{
							if (! data_map.contains(name) )
								goto next_form;

							if (type == "password")
							{
								pwd_name = name;
								pwd_found = true;
							}
							if (!name_found && (type == "text" || type.isNull()))
							{
								name_found = true;
								user_name = name;
							}

							if (!(type == "text" || type == "password" || type.isNull()))
							{
								exclude_map[name] = type;
							}

						}
					}
				}
			}
			inp_ind = form.indexOf("<input", inp_ind + 7);
		}

		// password is found and all elements matched
		if (pwd_found)
			break;

next_form:
		form_ind = html.indexOf("<form", form_ind + 5);
	}

	QWebSettings *globalSettings = QWebSettings::globalSettings();
	if (! globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) 
	{
		// Store data somewhere
		if (pwd_found && current_form != -1 && !pwd_name.isEmpty())
		{
			QSettings settings;
			settings.beginGroup(QLatin1String("websettings"));
			if (! settings.value(QLatin1String("savePasswords"), false).toBool())
				return false;

			settings.endGroup();
			settings.beginGroup("AutoComplete/" + u);
			QString i("%1");
			settings.setValue( "form_index_number", i.arg(current_form));
			settings.setValue( "form_password_control", pwd_name);
			settings.setValue( "form_username_control", user_name);

			bool bFirst = true;
			foreach(item, url.queryItems())
			{
				QString name = item.first;
				QString lower_name = name.toLower();
				if (exclude_map.contains( lower_name ))
					continue;

				QString value = item.second;
				if (bFirst &&  lower_name != pwd_name)
				{
					settings.setValue( "form_first_control", lower_name);
					bFirst = false;
				}

				value = value.replace('+', ' ');
				if (lower_name == pwd_name)
				{
					value = QUrl::fromPercentEncoding(QByteArray(value.toUtf8()));
					settings.setValue( name, EncryptPassword( value ) );
				}
				else
				{
					settings.setValue( name, value);
				}
			}

			settings.endGroup();
		}
	}

	return true;
}


bool AutoComplete::complete( QWebFrame * frame)
{
	if (!frame || frame->url().isEmpty())
		return false;

	QString url = formUrl(frame->url());

	QSettings settings;
	settings.beginGroup("AutoComplete");
	QString master = settings.value(QLatin1String("Master")).toString();
	settings.endGroup();
	settings.beginGroup("AutoComplete/" + url);
	QMap<QString, QString> keys;
	QString pwd_ctl;
	foreach(QString key, settings.childKeys())
	{
		keys[key] = settings.value(key).toString();
		if (key == "form_password_control")
			pwd_ctl = keys[key];
	}

	if (pwd_ctl.length() > 0)
	{
		if (master.length() > 0)
		{
			static bool passed = false;
			static bool started = false;
			if (!passed)
			{
				if (started)
					return false;

				started = true;
				QString pwd = QInputDialog::getText( 0, tr("Master Password"), tr("Type a master password:"), QLineEdit::Password);
				if ( pwd != DecryptPassword(master))
				{
					QMessageBox::warning(0, tr("Warning"), tr("Invalid password"));
					started = false;
					return false;
				}
				started = false;
				passed = true;
			}
		}
	}

	QString form_index = keys["form_index_number"];
	foreach(QString key, settings.childKeys() )
	{
		if (!form_index.isEmpty() && key != "form_index_number" && key != "form_password_control" 
			&& key != "form_first_control" && key != "form_username_control" )
		{
			QByteArray arr = QByteArray::fromPercentEncoding( keys[key].toUtf8() );
			QString val( arr );
			if (key.toLower() == pwd_ctl)
				val = DecryptPassword(val);

			QString java = "document.forms[" + form_index + "]." + key + ".value='" + val + "';";
			frame->evaluateJavaScript(java);
		}
	}

	return true;
}
