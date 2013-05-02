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

#ifndef MENUCOMMANDS_H
#define MENUCOMMANDS_H

#include <QSettings>
#include <QList>
#include <QKeySequence>
#include <QObject>


class MenuCommands : public QObject
{
    Q_OBJECT

public:
	MenuCommands(void);
	~MenuCommands(void);

	enum What { Key, Title, Shorts} ;

	QString				Get(int ind, What w) const;
	int					GetCommandsCount() const;
	QString				GetKey(int ind) const;
	QString				GetTitle(int ind) const;
	QList<QKeySequence> GetShortcuts(int ind) const;
	QString				GetShortcutsString(int ind) const;
	void				SetTitle(int ind, QString title);
	bool				SetShortcuts(int ind, QString shorts);
	QString				EnumCommands(int& ind ) const;
	QString				GetStr(QList<QKeySequence> )const;
	// Not yet in Menus

	QString				SwapFocusKey()	const		{ return QLatin1String("SwapFocus"); }
	QString				SwapFocusTitle() const		{ return m_data.value(SwapFocusKey(), tr("Swa&p Focus")).toString(); }
	QList<QKeySequence> SwapFocusShortcuts() const	{ return  loadShortcuts( SwapFocusKey(), QKeySequence(Qt::Key_F6) ); }
	
	////// File menu ///////
	QString				FileKey() const				{ return QLatin1String("File"); }
	QString				FileTitle() const			{ return m_data.value(FileKey(), tr("&File")).toString(); }

	QString				NewWinKey()	const		{ return QLatin1String("NewWin"); }
	QString				NewWinTitle() const		{ return m_data.value(NewWinKey(), tr("&New Window")).toString(); }
	QList<QKeySequence> NewWinShortcuts() const	{ return  loadShortcuts( NewWinKey(), QKeySequence::New ); }

	QString				OpenFileKey() const			{ return QLatin1String("OpenFile"); }
	QString				OpenFileTitle() const		{ return m_data.value(OpenFileKey(), tr("&Open File...")).toString(); }
	QList<QKeySequence> OpenFileShortcuts() const	{ return  loadShortcuts( OpenFileKey(), QKeySequence::Open ); }

	QString				OpenLocKey() const			{ return QLatin1String("OpenLoc"); }
	QString				OpenLocTitle() const		{ return m_data.value(OpenLocKey(), tr("Open &Location...")).toString(); }
	QList<QKeySequence> OpenLocShortcuts() const;

	QString				SaveAsKey() const			{ return QLatin1String("SaveAs"); }
	QString				SaveAsTitle() const		{ return m_data.value(SaveAsKey(), tr("&Save As...")).toString(); }
	QList<QKeySequence> SaveAsShortcuts() const	{ return  loadShortcuts( SaveAsKey(), QKeySequence::Save); }

	QString				SavePdfKey() const			{ return QLatin1String("SavePdf"); }
	QString				SavePdfTitle() const		{ return m_data.value(SavePdfKey(), tr("Sa&ve as PDF...")).toString(); }
	QList<QKeySequence> SavePdfShortcuts() const	{ return  loadShortcuts( SavePdfKey() ); }

	QString				ImportKey() const			{ return QLatin1String("Import"); }
	QString				ImportTitle() const		{ return m_data.value(ImportKey(), tr("&Import Bookmarks")).toString(); }

	QString				ImportIEKey() const			{ return QLatin1String("ImportIE"); }
	QString				ImportIETitle() const			{ return m_data.value(ImportIEKey(), tr("Import from &Interner Explorer")).toString(); }
	QList<QKeySequence> ImportIEShortcuts() const		{ return  loadShortcuts( ImportIEKey()); }

	QString				ImportFFKey() const			{ return QLatin1String("ImportMozilla"); }
	QString				ImportFFTitle() const			{ return m_data.value(ImportFFKey(), tr("Import from Mozilla &FireFox")).toString(); }
	QList<QKeySequence> ImportFFShortcuts() const		{ return  loadShortcuts( ImportFFKey()); }

	QString				ImportHtmlKey()	 const		{ return QLatin1String("ImportHTML"); }
	QString				ImportHtmlTitle() const		{ return m_data.value(ImportHtmlKey(), tr("Import from &HTML (Netscape, Safari, Opera, K-Meleon...)")).toString(); }
	QList<QKeySequence> ImportHtmlShortcuts() const	{ return  loadShortcuts( ImportHtmlKey()); }

	QString				ImportXmlKey() const			{ return QLatin1String("ImportXML"); }
	QString				ImportXmlTitle() const		{ return m_data.value(ImportXmlKey(), tr("Import from &XBEL/XML")).toString(); }
	QList<QKeySequence> ImportXmlShortcuts() const		{ return  loadShortcuts( ImportXmlKey()); }

	QString				ExportKey() const				{ return QLatin1String("Export"); }
	QString				ExportTitle() const			{ return m_data.value(ExportKey(), tr("&Export Bookmarks...")).toString(); }
	QList<QKeySequence> ExportShortcuts() const		{ return  loadShortcuts( ExportKey()); }

	QString				PrintKey() const				{ return QLatin1String("Print"); }
	QString				PrintTitle() const			{ return m_data.value(PrintKey(), tr("&Print...")).toString(); }
	QList<QKeySequence> PrintShortcuts() const			{ return  loadShortcuts( PrintKey(), QKeySequence::Print); }

	QString				PreviewKey() const			{ return QLatin1String("Preview"); }
	QString				PreviewTitle() const			{ return m_data.value(PreviewKey(), tr("Print Previe&w...")).toString(); }
	QList<QKeySequence> PreviewShortcuts() const		{ return  loadShortcuts( PreviewKey()); }

	QString				QuitKey() const				{ return QLatin1String("Quit"); }
	QString				QuitTitle() const				{ return m_data.value(QuitKey(), tr("&Quit")).toString(); }
	QList<QKeySequence> QuitShortcuts() const			{ return  loadShortcuts( QuitKey(), QKeySequence(Qt::CTRL | Qt::Key_Q)); }

	// Edit menu
	QString				EditKey() const				{ return QLatin1String("Edit"); }
	QString				EditTitle() const				{ return m_data.value(EditKey(), tr("&Edit")).toString(); }

	QString				UndoKey() const				{ return QLatin1String("Undo"); }
	QString				UndoTitle() const				{ return m_data.value(UndoKey(), tr("&Undo")).toString(); }
	QList<QKeySequence> UndoShortcuts() const			{ return  loadShortcuts( UndoKey(), QKeySequence::Undo); }

	QString				RedoKey() const				{ return QLatin1String("Redo"); }
	QString				RedoTitle() const				{ return m_data.value(RedoKey(), tr("&Redo")).toString(); }
	QList<QKeySequence> RedoShortcuts() const			{ return  loadShortcuts( RedoKey(), QKeySequence::Redo); }

	QString				CutKey() const				{ return QLatin1String("Cut"); }
	QString				CutTitle() const				{ return m_data.value(CutKey(), tr("Cu&t")).toString(); }
	QList<QKeySequence> CutShortcuts() const			{ return  loadShortcuts( CutKey(), QKeySequence::Cut); }

	QString				CopyKey() const				{ return QLatin1String("Copy"); }
	QString				CopyTitle() const				{ return m_data.value(CopyKey(), tr("&Copy")).toString(); }
	QList<QKeySequence> CopyShortcuts() const			{ return  loadShortcuts( CopyKey(), QKeySequence::Copy); }

	QString				PasteKey() const				{ return QLatin1String("Paste"); }
	QString				PasteTitle() const			{ return m_data.value(PasteKey(), tr("&Paste")).toString(); }
	QList<QKeySequence> PasteShortcuts() const			{ return  loadShortcuts( PasteKey(), QKeySequence::Paste); }

	QString				FindKey() const				{ return QLatin1String("Find"); }
	QString				FindTitle() const				{ return m_data.value(FindKey(), tr("&Find")).toString(); }
	QList<QKeySequence> FindShortcuts() const			{ return  loadShortcuts( FindKey(), QKeySequence::Find); }

	QString				NextKey() const				{ return QLatin1String("Next"); }
	QString				NextTitle() const				{ return m_data.value(NextKey(), tr("Find &Next")).toString(); }
	QList<QKeySequence> NextShortcuts() const			{ return  loadShortcuts( NextKey(), QKeySequence::FindNext); }

	QString				PrevKey() const				{ return QLatin1String("Prev"); }
	QString				PrevTitle() const				{ return m_data.value(PrevKey(), tr("Find &Previous")).toString(); }
	QList<QKeySequence> PrevShortcuts() const			{ return  loadShortcuts( PrevKey(), QKeySequence::FindPrevious); }

	QString				PrefsKey() const			{ return QLatin1String("Prefs"); }
	QString				PrefsTitle() const			{ return m_data.value(PrefsKey(), tr("&Preferences")).toString(); }
	QList<QKeySequence> PrefsShortcuts() const		{ return loadShortcuts( PrefsKey(), QKeySequence(Qt::CTRL | Qt::Key_Comma) ); }

	// View Menu
	QString				ViewKey() const				{ return QLatin1String("View"); }
	QString				ViewTitle() const				{ return m_data.value(ViewKey(), tr("&View")).toString(); }

	QString				AppStylesKey() const			{ return QLatin1String("AppStyles"); }
	QString				AppStylesTitle() const		{ return m_data.value(AppStylesKey(), tr("&Application Styles")).toString(); }

	QString				StatusKey() const				{ return QLatin1String("StatusBar"); }
	QString				StatusTitle() const			{ return m_data.value(StatusKey(), tr("&Status Bar")).toString(); }
	QString				StatusShowTitle() const		{ return tr("Show") + " " + StatusTitle();  }
	QString				StatusHideTitle() const		{ return tr("Hide") + " " + StatusTitle();  }
	QList<QKeySequence> StatusShortcuts() const;

	QString				MenuKey() const				{ return QLatin1String("MenuBar"); }
	QString				MenuTitle() const			{ return m_data.value(MenuKey(), tr("&Menu Bar")).toString(); }
	QString				MenuShowTitle() const			{ return tr("Show") + " " + MenuTitle();  }
	QString				MenuHideTitle() const			{ return tr("Hide") + " " + MenuTitle();  }
	QList<QKeySequence> MenuShortcuts() const			{ return  loadShortcuts( MenuKey(), QKeySequence("Ctrl+Shift+M")); }

	QString				TabKey() const				{ return QLatin1String("TabBar"); }
	QString				TabTitle() const			{ return m_data.value(TabKey(), tr("&Tab Bar")).toString(); }
	QString				TabShowTitle() const			{ return tr("Show") + " " + TabTitle();  }
	QString				TabHideTitle() const			{ return tr("Hide") + " " + TabTitle(); }
	QList<QKeySequence> TabShortcuts() const			{ return  loadShortcuts( TabKey(), QKeySequence("Ctrl+Shift+T")); }

	QString				NavKey() const				{ return QLatin1String("NavigationBar"); }
	QString				NavTitle() const			{ return m_data.value(NavKey(), tr("&Navigation Bar")).toString(); }
	QString				NavShowTitle() const		{ return tr("Show") + " " + NavTitle();  }
	QString				NavHideTitle() const		{ return tr("Hide") + " " + NavTitle();  }
	QList<QKeySequence> NavShortcuts() const;

	QString				BooksKey() const			{ return QLatin1String("BookmarksBar"); }
	QString				BooksTitle() const			{ return m_data.value(BooksKey(), tr("&Bookmarks Bar")).toString(); }
	QString				BooksShowTitle() const		{ return tr("Show") + " " + BooksTitle();  }
	QString				BooksHideTitle() const		{ return tr("Hide") + " " + BooksTitle();  }
	QList<QKeySequence> BooksShortcuts() const			{ return  loadShortcuts( BooksKey(), QKeySequence("Ctrl+Shift+B")); }

	QString				StopKey() const				{ return QLatin1String("Stop"); }
	QString				StopTitle() const				{ return m_data.value(StopKey(), tr("St&op")).toString(); }
	QList<QKeySequence> StopShortcuts() const;

	QString				ReloadKey() const				{ return QLatin1String("Reload"); }
	QString				ReloadTitle() const			{ return m_data.value(ReloadKey() , tr("Reload &Page")).toString(); }
	QList<QKeySequence> ReloadShortcuts() const;//		{ return  loadShortcuts( ReloadKey(), QKeySequence::Refresh); }

	QString				LargerKey() const				{ return QLatin1String("Larger"); }
	QString				LargerTitle() const			{ return m_data.value(LargerKey() , tr("Make Text &Larger")).toString(); }
	QList<QKeySequence> LargerShortcuts() const		{ return  loadShortcuts( LargerKey(), QKeySequence(Qt::CTRL | Qt::Key_Plus)); }

	QString				NormalKey() const				{ return QLatin1String("Normal"); }
	QString				NormalTitle() const			{ return m_data.value(NormalKey() , tr("Ma&ke Text Normal")).toString(); }
	QList<QKeySequence> NormalShortcuts() const		{ return  loadShortcuts( NormalKey(), QKeySequence(Qt::CTRL | Qt::Key_0)); }

	QString				SmallerKey() const			{ return QLatin1String("Smaller"); }
	QString				SmallerTitle() const			{ return m_data.value(SmallerKey() , tr("Make Te&xt Smaller")).toString(); }
	QList<QKeySequence> SmallerShortcuts() const		{ return  loadShortcuts( SmallerKey(), QKeySequence(Qt::CTRL | Qt::Key_Minus)); }

	QString				TextOnlyKey() const			{ return QLatin1String("TextOnly"); }
	QString				TextOnlyTitle() const			{ return m_data.value(TextOnlyKey() , tr("&Zoom Text Only")).toString(); }
	QList<QKeySequence> TextOnlyShortcuts() const		{ return  loadShortcuts( TextOnlyKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Z)); }

	QString				EncodeKey() const				{ return QLatin1String("Encoding"); }
	QString				EncodeTitle() const			{ return m_data.value(EncodeKey() , tr("Text &Encoding")).toString(); }

	QString				SourceKey() const				{ return QLatin1String("Source"); }
	QString				SourceTitle() const			{ return m_data.value(SourceKey() , tr("Page So&urce")).toString(); }
	QList<QKeySequence> SourceShortcuts() const		{ return  loadShortcuts( SourceKey(), QKeySequence("Ctrl+Alt+U")); }

	QString				FullKey() const				{ return QLatin1String("Full"); }
	QString				FullTitle() const				{ return m_data.value(FullKey() , tr("&Full Screen")).toString(); }
	QList<QKeySequence> FullShortcuts() const			{ return  loadShortcuts( FullKey(), Qt::Key_F11); }

	// History Menu
	QString				HistoryKey() const			{ return QLatin1String("History"); }
	QString				HistoryTitle() const			{ return m_data.value(HistoryKey() , tr("Hi&story")).toString(); }

	QString				BackKey() const				{ return QLatin1String("Back"); }
	QString				BackTitle() const				{ return m_data.value(BackKey() , tr("&Back")).toString(); }
	QList<QKeySequence> BackShortcuts() const			{ return  loadShortcuts( BackKey(), QKeySequence::Back); }

	QString				ForwKey() const				{ return QLatin1String("Forward"); }
	QString				ForwTitle() const				{ return m_data.value(ForwKey() , tr("&Forward")).toString(); }
	QList<QKeySequence> ForwShortcuts() const			{ return  loadShortcuts( ForwKey(), QKeySequence::Forward); }

	QString				HomeKey() const				{ return QLatin1String("Home"); }
	QString				HomeTitle() const				{ return m_data.value(HomeKey() , tr("&Home")).toString(); }
	QList<QKeySequence> HomeShortcuts() const			{ return  loadShortcuts( HomeKey(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H)); }

	QString				LastTabKey() const			{ return QLatin1String("LastTab"); }
	QString				LastTabTitle() const			{ return m_data.value(LastTabKey() , tr("Restore Last &Closed Tab")).toString(); }
	QList<QKeySequence> LastTabShortcuts() const		{ return  loadShortcuts( LastTabKey(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C)); }

	QString				LastTabsKey() const				{ return QLatin1String("LastTabs"); }
	QString				LastTabsTitle() const			{ return m_data.value(LastTabsKey() , tr("Restore Closed &Tabs")).toString(); }
	QList<QKeySequence> LastTabsShortcuts() const		{ return loadShortcuts( LastTabsKey()); }

	QString				SessionKey() const			{ return QLatin1String("Session"); }
	QString				SessionTitle() const			{ return m_data.value(SessionKey() , tr("&Restore Last Session")).toString(); }
	QList<QKeySequence> SessionShortcuts() const		{ return  loadShortcuts( SessionKey(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R)); }

	QString				AllHistKey() const			{ return QLatin1String("AllHistory"); }
	QString				AllHistTitle() const			{ return m_data.value(AllHistKey() , tr("Show &All History")).toString(); }
	QList<QKeySequence> AllHistShortcuts() const		{ return  loadShortcuts( AllHistKey(), QKeySequence(Qt::CTRL | Qt::Key_H)); }

	QString				ClearKey() const				{ return QLatin1String("Clear"); }
	QString				ClearTitle() const			{ return m_data.value(ClearKey() , tr("Clea&r History")).toString(); }
	QList<QKeySequence> ClearShortcuts() const		{ return  loadShortcuts( ClearKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_H)); }

	// Bookmarks Menu
	QString				BookmarksKey() const			{ return QLatin1String("Bookmarks"); }
	QString				BookmarksTitle() const		{ return m_data.value(BookmarksKey() , tr("&Bookmarks")).toString(); }

	QString				AllBooksKey() const			{ return QLatin1String("AllBookmarks"); }
	QString				AllBooksTitle() const			{ return m_data.value( AllBooksKey() , tr("Show All &Bookmarks")).toString(); }
	QList<QKeySequence> AllBooksShortcuts() const		{ return  loadShortcuts( AllBooksKey(), QKeySequence(Qt::CTRL | Qt::Key_B) ); }

	QString				AddBookKey() const			{ return QLatin1String("AddBookmark"); }
	QString				AddBookTitle() const			{ return m_data.value( AddBookKey() , tr("A&dd Bookmark...")).toString(); }
	QList<QKeySequence> AddBookShortcuts() const		{ return  loadShortcuts( AddBookKey(), QKeySequence(Qt::CTRL | Qt::Key_D) ); }

	// Privacy Menu
	QString				PrivacyKey() const			{ return QLatin1String("Privacy"); }
	QString				PrivacyTitle() const			{ return m_data.value(PrivacyKey() , tr("&Privacy")).toString(); }

	QString				PrivateKey() const			{ return QLatin1String("PrivateBrowsing"); }
	QString				PrivateTitle() const			{ return m_data.value( PrivateKey() , tr("&Private Browsing...")).toString(); }
	QList<QKeySequence> PrivateShortcuts() const		{ return loadShortcuts( PrivateKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_X) ); }

	QString				JavaScriptKey() const			{ return QLatin1String("JavaScript"); }
	QString				JavaScriptTitle() const		{ return m_data.value( JavaScriptKey() , tr("Disable &JavaScript")).toString(); }
	QList<QKeySequence> JavaScriptShortcuts() const	{ return loadShortcuts( JavaScriptKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_J) ); }

	QString				ImagesKey() const				{ return QLatin1String("Images"); }
	QString				ImagesTitle() const			{ return m_data.value( JavaScriptKey() , tr("Disable &Images")).toString(); }
	QList<QKeySequence> ImagesShortcuts() const		{ return loadShortcuts( JavaScriptKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I) ); }

	QString				CookiesKey() const			{ return QLatin1String("Cookies"); }
	QString				CookiesTitle() const			{ return m_data.value( CookiesKey() , tr("Disable &Cookies")).toString(); }
	QList<QKeySequence> CookiesShortcuts() const		{ return loadShortcuts( CookiesKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C) ); }

	QString				PlugInsKey() const			{ return QLatin1String("PlugIns"); }
	QString				PlugInsTitle() const			{ return m_data.value( PlugInsKey() , tr("Disable Plu&g-Ins")).toString(); }
	QList<QKeySequence> PlugInsShortcuts() const		{ return loadShortcuts( PlugInsKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_G) ); }

	QString				AgentKey() const				{ return QLatin1String("UserAgent"); }
	QString				AgentTitle() const			{ return m_data.value( AgentKey() , tr("Disable User&Agent")).toString(); }
	QList<QKeySequence> AgentShortcuts() const		{ return loadShortcuts( AgentKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_A) ); }

	QString				PopUpsKey() const				{ return QLatin1String("PopUps"); }
	QString				PopUpsTitle() const			{ return m_data.value( PopUpsKey() , tr("Disa&ble Pop-Ups")).toString(); }
	QList<QKeySequence> PopUpsShortcuts() const		{ return loadShortcuts( PopUpsKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_P) ); }

	QString				ProxyKey() const				{ return QLatin1String("Proxy"); }
	QString				ProxyTitle() const			{ return m_data.value( ProxyKey() , tr("Enable Prox&y")).toString(); }
	QList<QKeySequence> ProxyShortcuts() const		{ return loadShortcuts( ProxyKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Y) ); }

	QString				EmptyKey() const				{ return QLatin1String("EmptyCache"); }
	QString				EmptyTitle() const			{ return m_data.value( EmptyKey() , tr("E&mpty Cache...")).toString(); }
	QList<QKeySequence> EmptyShortcuts() const		{ return loadShortcuts( EmptyKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M) ); }

	QString				ResetKey() const				{ return QLatin1String("Reset"); }
	QString				ResetTitle() const			{ return m_data.value( ResetKey() , tr("&Reset QtWeb...")).toString(); }
	QList<QKeySequence> ResetShortcuts() const		{ return loadShortcuts( ResetKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R) ); }

	QString				FullResetKey() const			{ return QLatin1String("FullReset"); }
	QString				FullResetTitle() const		{ return m_data.value( FullResetKey() , tr("Quit with &Full Reset...")).toString(); }
	QList<QKeySequence> FullResetShortcuts() const	{ return loadShortcuts( FullResetKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F) ); }

	// Tools Menu
	QString				ToolsKey() const				{ return QLatin1String("Tools"); }
	QString				ToolsTitle() const			{ return m_data.value(ToolsKey() , tr("&Tools")).toString(); }

	QString				CompatKey() const			{ return QLatin1String("Compatible"); } 
	QString				CompatTitle() const			{ return m_data.value(CompatKey(), tr("&Compatibility")).toString(); }

	QString				SearchKey() const				{ return QLatin1String("Search"); }
	QString				SearchTitle() const			{ return m_data.value( SearchKey() , tr("&Web Search")).toString(); }
	QList<QKeySequence> SearchShortcuts() const		{ return loadShortcuts( SearchKey(), QKeySequence(Qt::CTRL | Qt::Key_G) ); }

	QString				KeyboardKey() const			{ return QLatin1String("Keyboard"); }
	QString				KeyboardTitle() const			{ return m_data.value( KeyboardKey() , tr("Virtual &Keyboard")).toString(); }
	QList<QKeySequence> KeyboardShortcuts() const		{ return loadShortcuts( KeyboardKey(), QKeySequence(Qt::CTRL | Qt::Key_K) ); }

	QString				InspectorKey() const				{ return QLatin1String("Inspector"); }
	QString				InspectorTitle() const			{ return m_data.value( InspectorKey() , tr("&Enable Web Inspector")).toString(); }
	QList<QKeySequence> InspectorShortcuts() const		{ return loadShortcuts( InspectorKey(), QKeySequence(Qt::CTRL | Qt::Key_E)  ); }

	QString				InspectKey() const				{ return QLatin1String("Inspect"); }
	QString				InspectTitle() const			{ return m_data.value( InspectKey() , tr("&Inspect Element")).toString(); }
	QList<QKeySequence> InspectShortcuts() const		{ return loadShortcuts( InspectKey(), QKeySequence(Qt::CTRL | Qt::Key_I) ); }

	QString				OptionsKey() const				{ return QLatin1String("Options"); }
	QString				OptionsTitle() const			{ return m_data.value(OptionsKey(), tr("&Options...")).toString(); }
	QList<QKeySequence> OptionsShortcuts() const		{ return loadShortcuts( OptionsKey(), QKeySequence(Qt::Key_F2) ); }

	// Window Menu
	QString				WindowKey() const				{ return QLatin1String("Window"); }
	QString				WindowTitle() const			{ return m_data.value(WindowKey() , tr("&Window")).toString(); }

	QString				NextTabKey() const			{ return QLatin1String("NextTab"); }
	QString				NextTabTitle() const			{ return m_data.value( NextTabKey() , tr("Show &Next Tab")).toString(); }
	QList<QKeySequence> NextTabShortcuts() const;

	QString				PrevTabKey() const			{ return QLatin1String("PrevTab"); }
	QString				PrevTabTitle() const			{ return m_data.value( PrevTabKey() , tr("Show &Previous Tab")).toString(); }
	QList<QKeySequence> PrevTabShortcuts() const;

	QString				NewTabKey() const				{ return QLatin1String("NewTab"); }
	QString				NewTabTitle() const			{ return m_data.value( NewTabKey() , tr("New &Tab")).toString(); }
	QList<QKeySequence> NewTabShortcuts() const		{ return loadShortcuts( NewTabKey(), QKeySequence::AddTab ); }

	QString				CloseTabKey() const			{ return QLatin1String("CloseTab"); }
	QString				CloseTabTitle() const			{ return m_data.value( CloseTabKey() , tr("&Close Tab")).toString(); }
	QList<QKeySequence> CloseTabShortcuts() const;		//{ return loadShortcuts( CloseTabKey(), QKeySequence::Close ); }

	QString				CloseOtherKey() const			{ return QLatin1String("CloseOtherTabs"); }
	QString				CloseOtherTitle() const		{ return m_data.value( CloseOtherKey() , tr("Close &Other Tabs")).toString(); }
	QList<QKeySequence> CloseOtherShortcuts() const	{ return loadShortcuts( CloseOtherKey() ); }

	QString				CloneTabKey() const			{ return QLatin1String("CloneTab"); }
	QString				CloneTabTitle() const			{ return m_data.value( CloneTabKey() , tr("C&lone Tab")).toString(); }
	QList<QKeySequence> CloneTabShortcuts() const		{ return loadShortcuts( CloneTabKey()); }

	QString				ReloadTabKey() const			{ return QLatin1String("ReloadTab"); }
	QString				ReloadTabTitle() const		{ return m_data.value( ReloadTabKey() , tr("Reload Tab")).toString(); }
	QList<QKeySequence> ReloadTabShortcuts() const	{ return loadShortcuts( ReloadTabKey(), QKeySequence::Refresh); }

	QString				ReloadAllKey() const			{ return QLatin1String("ReloadAll"); }
	QString				ReloadAllTitle() const		{ return m_data.value( ReloadAllKey() , tr("Reload All Tabs")).toString(); }
	QList<QKeySequence> ReloadAllShortcuts() const	{ return loadShortcuts( ReloadAllKey() ); }

	QString				OpenNewTabKey() const			{ return QLatin1String("OpenNewTab"); }
	QString				OpenNewTabTitle() const		{ return m_data.value( OpenNewTabKey() , tr("Open in New Tab")).toString(); }
	QList<QKeySequence> OpenNewTabShortcuts() const	{ return loadShortcuts( OpenNewTabKey() ); }

	QString				OpenAdBlockKey() const			{ return QLatin1String("AdBlock"); }
	QString				OpenAdBlockTitle() const		{ return m_data.value( OpenAdBlockKey() , tr("Ad Block...")).toString(); }
	QList<QKeySequence> OpenAdBlockShortcuts() const	{ return loadShortcuts( OpenAdBlockKey() ); }

	QString				CopyAddrKey() const			{ return QLatin1String("CopyAddr"); }
	QString				CopyAddrTitle() const		{ return m_data.value( CopyAddrKey() , tr("Copy Address")).toString(); }


	QString				DownsKey() const				{ return QLatin1String("Downloads"); }
	QString				DownsTitle() const			{ return m_data.value( DownsKey() , tr("&Downloads")).toString(); }
	QList<QKeySequence> DownsShortcuts() const		{ return loadShortcuts( DownsKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L) ); }

	QString				TorrentsKey() const				{ return QLatin1String("Torrents"); }
	QString				TorrentsTitle() const			{ return m_data.value( TorrentsKey() , tr("&Torrents")).toString(); }
	QList<QKeySequence> TorrentsShortcuts() const		{ return loadShortcuts( TorrentsKey(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_T) ); }

	// Help Menu
	QString				HelpKey() const				{ return QLatin1String("Help"); }
	QString				HelpTitle() const				{ return m_data.value(HelpKey() , tr("&Help")).toString(); }
	QList<QKeySequence> HelpShortcuts() const			{ return  loadShortcuts( HelpKey(), QKeySequence( Qt::Key_F1 )); }

	QString				OnlineKey() const				{ return QLatin1String("Online"); }
	QString				OnlineTitle() const			{ return m_data.value(OnlineKey() , tr("&Online Help")).toString(); }
	QList<QKeySequence> OnlineShortcuts() const		{ return  loadShortcuts( OnlineKey()); }

	QString				UpdatesKey() const			{ return QLatin1String("Updates"); }
	QString				UpdatesTitle() const			{ return m_data.value(UpdatesKey() , tr("&Check for Updates")).toString(); }
	QList<QKeySequence> UpdatesShortcuts() const		{ return  loadShortcuts( UpdatesKey()); }

	QString				AboutKey() const				{ return QLatin1String("About"); }
	QString				AboutTitle() const			{ return m_data.value(AboutKey() , tr("&About QtWeb")).toString(); }
	QList<QKeySequence> AboutShortcuts() const		{ return  loadShortcuts( AboutKey()); }

protected:
	QList<QKeySequence> loadShortcuts( QString key, QKeySequence default_shortcut = QKeySequence() ) const;

	QSettings m_data;
};

#endif // MENUCOMMANDS_H