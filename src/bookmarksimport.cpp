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

#include "bookmarksimport.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QApplication>

#ifdef Q_WS_WIN
    #include <shlobj.h>
#endif

QString BookmarksImport::mozillaPath()
{
#ifdef Q_WS_WIN
        wchar_t szTemp[MAX_PATH];
	memset(szTemp, 0 , sizeof(szTemp));
	LPITEMIDLIST pidl;
	if (NOERROR == SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl))
		SHGetPathFromIDList(pidl, szTemp);
	if (szTemp[ wcslen(szTemp)-1] != '\\')
		wcscat(szTemp, L"\\");
	wcscat(szTemp, L"Mozilla\\Firefox\\Profiles\\");
	QDir base(QString::fromWCharArray(szTemp));
	QStringList filter;
	filter << "*.default";
	QStringList subs = base.entryList(  filter );
	if (subs.count() < 1)
		return "";
	
	QString path = base.absolutePath() + "/" + subs[0] + "/bookmarks.html";
	return QFile::exists(path) ? path : "";
#else
        return "";
#endif
}

QString BookmarksImport::ieFavoritesPath()
{
#ifdef Q_WS_WIN
        wchar_t szTemp[MAX_PATH];
	memset(szTemp, 0 , sizeof(szTemp));
	LPITEMIDLIST pidl;
	if (NOERROR == SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidl))
		SHGetPathFromIDList(pidl, szTemp);

	return QString::fromWCharArray(szTemp);
#else
        return "";
#endif

}

void BookmarksImport::ImportInternetExplorerFavorites(QString path, BookmarkNode* root)
{
	QDir base(path);
	QFileInfoList subs = base.entryInfoList();
	foreach(QFileInfo fi, subs)
	{
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

		QString fn = fi.fileName();
		if (fn == "." || fn == "..")
			continue;

		QString fp = fi.filePath();
		if (fi.isDir())
		{
			BookmarkNode * subfolder = new BookmarkNode(BookmarkNode::Folder, root);
			subfolder->title = fn;
			ImportInternetExplorerFavorites(fp, subfolder);
		}
		else
		{
			QFile f(fp);
			if (f.open(QIODevice::ReadOnly | QIODevice::Text) )
			{
				QString baseurl, url;
				while (!f.atEnd()) 
				{
					 QByteArray line = f.readLine();
					 QString ln(line);
					 if (ln.indexOf("BASEURL=") == 0)
					 {
						 baseurl = ln.mid(8);
						 QChar last = baseurl.at(baseurl.length() - 1);
						 if (last == '\n')
							baseurl = baseurl.left(baseurl.length() - 1);
					 }
					 if (ln.indexOf("URL=") == 0)
					 {
						 url = ln.mid(4);
						 QChar last = url.at(url.length() - 1);
						 if (last == '\n')
							url = url.left(url.length() - 1);

						 break;
					 }
				}
				f.close();
				if (url.isEmpty())
					url = baseurl;
				if (!url.isEmpty())
				{
					BookmarkNode * link = new BookmarkNode(BookmarkNode::Bookmark, root);
					link->url = url;
					if (fn.indexOf(".url") == fn.length() - 4)
						fn = fn.left(fn.length() - 4);
					link->title = fn;
				}
			}
		}
	}
}

BookmarkNode *BookmarksImport::importFromIE()
{
	QString path = ieFavoritesPath();
	if (path.isEmpty())
		return NULL;

        BookmarkNode* root = new BookmarkNode();

#ifdef Q_WS_WIN

        CoInitialize( 0 );
	
	ImportInternetExplorerFavorites( path, root);
#endif

	return root;
}

void  ParseHtmlBookmarks( QString& books , BookmarkNode* root)
{
	QString lowBooks = books.toLower();

	int endFolder	= lowBooks.indexOf("</dl>");
	int startLink	= lowBooks.indexOf("<dt><a");
	int startFolder	= lowBooks.indexOf("<dt><h3");
	
	while (endFolder != -1 || startLink != -1 || startFolder != -1)
	{
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

		if (endFolder != -1 && 
			(endFolder < startLink || startLink == -1) &&
			(endFolder < startFolder || startFolder == -1) ) 
		{
			books = books.mid(endFolder + 4);
			return;	// Done with a folder <DL></DL>
		}

		if (startLink != -1 && 
			(startLink < endFolder || endFolder == -1) &&
			(startLink < startFolder || startFolder == -1) ) 
		{
			QString url, title, tags;
			// Next is link
			int ind = lowBooks.indexOf("href=\"", startLink);
			if (ind != -1)
			{
				QString url_full = books.mid(ind + 6);
				int quote = url_full.indexOf("\"");
				if (quote != -1)
				{
					url = url_full.left(quote);
				}

				if (url.toLower().indexOf("http://") == 0 || url.toLower().indexOf("https://") == 0 || url.toLower().indexOf("ftp://") == 0 )
				{

					int endA = url_full.indexOf(">");
					if (endA != -1)
					{
						title = url_full.mid(endA + 1);

						tags = "";
						int startTags = url_full.toLower().indexOf("shortcuturl=\"");
						if (startTags != -1 && startTags < endA)
						{
							QString tags_full = url_full.mid(startTags + 13);
							int quote = tags_full.indexOf("\"");
							if (quote != -1)
								tags = tags_full.left(quote);
						}

						int startCloseA = title.toLower().indexOf("</a>");
						if (startCloseA != -1)
						{
							title = title.left(startCloseA);
							BookmarkNode * link = new BookmarkNode(BookmarkNode::Bookmark, root);
							link->url = url;
							link->title = title;
							link->tags = tags;
						}
					}
				}
			}

			lowBooks = lowBooks.mid(startLink + 10);
			books = books.mid(startLink + 10);

		}

		if (startFolder != -1 && 
			(startFolder < endFolder || endFolder == -1) &&
			(startFolder < startLink || startLink == -1) ) 
		{
			// Next is Folder
			QString folder_title;
			QString title_full = books.mid(startFolder + 7);
			int endA = title_full.indexOf(">");
			if (endA != -1)
			{
				folder_title = title_full.mid(endA + 1);
				int startCloseA = folder_title.toLower().indexOf("</h3>");
				if (startCloseA != -1)
					folder_title = folder_title.left(startCloseA);
			}

			books = books.mid(startFolder + 10);

			BookmarkNode * subfolder = new BookmarkNode(BookmarkNode::Folder, root);
			subfolder->title = folder_title;
			ParseHtmlBookmarks(books , subfolder);

			lowBooks = books.toLower();

		}


		endFolder	= lowBooks.indexOf("</dl>");
		startLink	= lowBooks.indexOf("<dt><a");
		startFolder	= lowBooks.indexOf("<dt><h3");
	}

}

#include <QTextCodec>

BookmarkNode *BookmarksImport::importFromHtml( QString path )
{
	BookmarkNode* root = new BookmarkNode();


	QFile f(path);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text) )
		return NULL;

	bool bIsNetscape = false;
	// check format
	while (!f.atEnd()) 
	{
		 QByteArray line = f.readLine();
		 QString ln(line);
		 ln = ln.toLower();
		 if (ln.indexOf("netscape-bookmark-file-1")  != -1)
		 {
			 bIsNetscape = true;
			 break;
		 }
	}

	if (!bIsNetscape)
	{
		// Format not supported
		f.close();
		QMessageBox::warning(0, QObject::tr("Importing Bookmarks"), 
			QObject::tr("HTML format is not supported.<br>Please make sure that HTML file type is NETSCAPE-Bookmark-file-1."));
		return NULL;
	}

	QString books;
	QByteArray ba;
	bool bUtf8 = false;
	while (!f.atEnd()) 
	{
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

		 QByteArray line = f.readLine();
		 QString ln = QString(line);
		 books += ln;
		 ba += line;
		 // <META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=UTF-8">
		 // check for UTF-8
		 ln = ln.toLower();
		 if ( (ln.indexOf(QLatin1String("<meta")) != -1) && 
			  (ln.indexOf(QLatin1String("utf-8")) != -1) )
		 {
			 bUtf8 = true;
		 }
	}
	f.close();

	if (bUtf8)
	{
		QTextCodec *codec = QTextCodec::codecForName( "utf-8" );
		if (codec)	
		{
			QTextDecoder *decoder = codec->makeDecoder();
			if (decoder)
			{
				books = decoder->toUnicode(ba);
			}
		}
	}

	ParseHtmlBookmarks( books , root);

	return root;
}


