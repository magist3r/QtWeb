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

#include "commands.h"
#include <QStringList>


MenuCommands::MenuCommands(void)
{
	m_data.beginGroup(QLatin1String("MenuCommands"));
}

MenuCommands::~MenuCommands(void)
{
	m_data.endGroup();
}

#define MAX_COMMANDS 91

int	MenuCommands::GetCommandsCount() const
{
	return  MAX_COMMANDS;
}

enum What { Key, Title, Shorts} ;

QString	MenuCommands::Get(int ind, What w) const
{
	int cur = 0;

	if (cur++ == ind)
		return (w == Key ? FileKey() : (w == Title? FileTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? NewWinKey() : (w == Title? NewWinTitle() : GetStr(NewWinShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OpenFileKey() : (w == Title? OpenFileTitle() : GetStr(OpenFileShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OpenLocKey() : (w == Title? OpenLocTitle() : GetStr(OpenLocShortcuts())));

	if (cur++ == ind)
		return (w == Key ? SaveAsKey() : (w == Title? SaveAsTitle() : GetStr(SaveAsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? SavePdfKey() : (w == Title? SavePdfTitle() : GetStr(SavePdfShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ImportKey() : (w == Title? ImportTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? ImportIEKey() : (w == Title? ImportIETitle() : GetStr(ImportIEShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ImportFFKey() : (w == Title? ImportFFTitle() : GetStr(ImportFFShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ImportHtmlKey() : (w == Title? ImportHtmlTitle() : GetStr(ImportHtmlShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ImportXmlKey() : (w == Title? ImportXmlTitle() : GetStr(ImportXmlShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ExportKey() : (w == Title? ExportTitle() : GetStr(ExportShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PrintKey() : (w == Title? PrintTitle() : GetStr(PrintShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PreviewKey() : (w == Title? PreviewTitle() : GetStr(PreviewShortcuts())));

	if (cur++ == ind)
		return (w == Key ? QuitKey() : (w == Title? QuitTitle() : GetStr(QuitShortcuts())));

	if (cur++ == ind)
		return (w == Key ? EditKey() : (w == Title? EditTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? UndoKey() : (w == Title? UndoTitle() : GetStr(UndoShortcuts())));

	if (cur++ == ind)
		return (w == Key ? RedoKey() : (w == Title? RedoTitle() : GetStr(RedoShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CutKey() : (w == Title? CutTitle() : GetStr(CutShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CopyKey() : (w == Title? CopyTitle() : GetStr(CopyShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PasteKey() : (w == Title? PasteTitle() : GetStr(PasteShortcuts())));

	if (cur++ == ind)
		return (w == Key ? FindKey() : (w == Title? FindTitle() : GetStr(FindShortcuts())));

	if (cur++ == ind)
		return (w == Key ? NextKey() : (w == Title? NextTitle() : GetStr(NextShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PrevKey() : (w == Title? PrevTitle() : GetStr(PrevShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PrefsKey() : (w == Title? PrefsTitle() : GetStr(PrefsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ViewKey() : (w == Title? ViewTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? AppStylesKey() : (w == Title? AppStylesTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? StatusKey() : (w == Title? StatusTitle() : GetStr(StatusShortcuts())));

	if (cur++ == ind)
		return (w == Key ? MenuKey() : (w == Title? MenuTitle() : GetStr(MenuShortcuts())));

	if (cur++ == ind)
		return (w == Key ? TabKey() : (w == Title? TabTitle() : GetStr(TabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? NavKey() : (w == Title? NavTitle() : GetStr(NavShortcuts())));

	if (cur++ == ind)
		return (w == Key ? BooksKey() : (w == Title? BooksTitle() : GetStr(BooksShortcuts())));

//	if (cur++ == ind)
//		return (w == Key ? Key() : (w == Title? Title() : GetStr(Shortcuts())));

	if (cur++ == ind)
		return (w == Key ? StopKey() : (w == Title? StopTitle() : GetStr(StopShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ReloadKey() : (w == Title? ReloadTitle() : GetStr(ReloadShortcuts())));

	if (cur++ == ind)
		return (w == Key ? LargerKey() : (w == Title? LargerTitle() : GetStr(LargerShortcuts())));

	if (cur++ == ind)
		return (w == Key ? NormalKey() : (w == Title? NormalTitle() : GetStr(NormalShortcuts())));

	if (cur++ == ind)
		return (w == Key ? SmallerKey() : (w == Title? SmallerTitle() : GetStr(SmallerShortcuts())));

	if (cur++ == ind)
		return (w == Key ? TextOnlyKey() : (w == Title? TextOnlyTitle() : GetStr(TextOnlyShortcuts())));

	if (cur++ == ind)
		return (w == Key ? EncodeKey() : (w == Title? EncodeTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? SourceKey() : (w == Title? SourceTitle() : GetStr(SourceShortcuts())));

	if (cur++ == ind)
		return (w == Key ? FullKey() : (w == Title? FullTitle() : GetStr(FullShortcuts())));

	if (cur++ == ind)
		return (w == Key ? HistoryKey() : (w == Title? HistoryTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? BackKey() : (w == Title? BackTitle() : GetStr(BackShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ForwKey() : (w == Title? ForwTitle() : GetStr(ForwShortcuts())));

	if (cur++ == ind)
		return (w == Key ? HomeKey() : (w == Title? HomeTitle() : GetStr(HomeShortcuts())));

	if (cur++ == ind)
		return (w == Key ? LastTabKey() : (w == Title? LastTabTitle() : GetStr(LastTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? LastTabsKey() : (w == Title? LastTabsTitle() : GetStr(LastTabsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? SessionKey() : (w == Title? SessionTitle() : GetStr(SessionShortcuts())));

	if (cur++ == ind)
		return (w == Key ? AllHistKey() : (w == Title? AllHistTitle() : GetStr(AllHistShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ClearKey() : (w == Title? ClearTitle() : GetStr(ClearShortcuts())));

	if (cur++ == ind)
		return (w == Key ? BookmarksKey() : (w == Title? BookmarksTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? AllBooksKey() : (w == Title? AllBooksTitle() : GetStr(AllBooksShortcuts())));

	if (cur++ == ind)
		return (w == Key ? AddBookKey() : (w == Title? AddBookTitle() : GetStr(AddBookShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? PrivacyKey() : (w == Title? PrivacyTitle() : ""));
	
	if (cur++ == ind)
		return (w == Key ? PrivateKey() : (w == Title? PrivateTitle() : GetStr(PrivateShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? JavaScriptKey() : (w == Title? JavaScriptTitle() : GetStr(JavaScriptShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? ImagesKey() : (w == Title? ImagesTitle() : GetStr(ImagesShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? CookiesKey() : (w == Title? CookiesTitle() : GetStr(CookiesShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? PlugInsKey() : (w == Title? PlugInsTitle() : GetStr(PlugInsShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? AgentKey() : (w == Title? AgentTitle() : GetStr(AgentShortcuts())));
	
	if (cur++ == ind)
		return (w == Key ? PopUpsKey() : (w == Title? PopUpsTitle() : GetStr(PopUpsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ProxyKey() : (w == Title? ProxyTitle() : GetStr(ProxyShortcuts())));

	if (cur++ == ind)
		return (w == Key ? EmptyKey() : (w == Title? EmptyTitle() : GetStr(EmptyShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ResetKey() : (w == Title? ResetTitle() : GetStr(ResetShortcuts())));

	if (cur++ == ind)
		return (w == Key ? FullResetKey() : (w == Title? FullResetTitle() : GetStr(FullResetShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ToolsKey() : (w == Title? ToolsTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? CompatKey() : (w == Title? CompatTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? SearchKey() : (w == Title? SearchTitle() : GetStr(SearchShortcuts())));

	if (cur++ == ind)
		return (w == Key ? KeyboardKey() : (w == Title? KeyboardTitle() : GetStr(KeyboardShortcuts())));

	if (cur++ == ind)
		return (w == Key ? InspectorKey() : (w == Title? InspectorTitle() : GetStr(InspectorShortcuts())));

	if (cur++ == ind)
		return (w == Key ? InspectKey() : (w == Title? InspectTitle() : GetStr(InspectShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OptionsKey() : (w == Title? OptionsTitle() : GetStr(OptionsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? WindowKey() : (w == Title? WindowTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? NextTabKey() : (w == Title? NextTabTitle() : GetStr(NextTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? PrevTabKey() : (w == Title? PrevTabTitle() : GetStr(PrevTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? NewTabKey() : (w == Title? NewTabTitle() : GetStr(NewTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CloseTabKey() : (w == Title? CloseTabTitle() : GetStr(CloseTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CloseOtherKey() : (w == Title? CloseOtherTitle() : GetStr(CloseOtherShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CloneTabKey() : (w == Title? CloneTabTitle() : GetStr(CloneTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ReloadTabKey() : (w == Title? ReloadTabTitle() : GetStr(ReloadTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? ReloadAllKey() : (w == Title? ReloadAllTitle() : GetStr(ReloadAllShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OpenNewTabKey() : (w == Title? OpenNewTabTitle() : GetStr(OpenNewTabShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OpenAdBlockKey() : (w == Title? OpenAdBlockTitle() : GetStr(OpenAdBlockShortcuts())));

	if (cur++ == ind)
		return (w == Key ? SwapFocusKey() : (w == Title? SwapFocusTitle() : GetStr(SwapFocusShortcuts())));

	if (cur++ == ind)
		return (w == Key ? CopyAddrKey() : (w == Title? CopyAddrTitle() : ""));

	if (cur++ == ind)
		return (w == Key ? DownsKey() : (w == Title? DownsTitle() : GetStr(DownsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? TorrentsKey() : (w == Title? TorrentsTitle() : GetStr(TorrentsShortcuts())));

	if (cur++ == ind)
		return (w == Key ? HelpKey() : (w == Title? HelpTitle() : GetStr(HelpShortcuts())));

	if (cur++ == ind)
		return (w == Key ? OnlineKey() : (w == Title? OnlineTitle() : GetStr(OnlineShortcuts())));

	if (cur++ == ind)
		return (w == Key ? UpdatesKey() : (w == Title? UpdatesTitle() : GetStr(UpdatesShortcuts())));

	if (cur++ == ind)
		return (w == Key ? AboutKey() : (w == Title? AboutTitle() : GetStr(AboutShortcuts())));

	return "";
}

QString	MenuCommands::GetKey(int ind) const
{
	return Get(ind, Key);
}

QString	MenuCommands::GetTitle(int ind) const
{
	return Get(ind, Title);
}


QString	MenuCommands::GetStr(QList<QKeySequence> list) const
{
	QString result = "";
	foreach(QKeySequence s, list)
		result += s.toString() + " ";
	return result.trimmed();
}


bool	MenuCommands::SetShortcuts(int ind, QString shorts)
{
	QStringList list = shorts.split(QChar(' '));
	QString key = Get(ind,Key);
	int i = 0;
    foreach(QString s, list)
	{
		if (!s.trimmed().isEmpty())
		{
			QKeySequence shortcut(s.trimmed());
			if (shortcut.isEmpty())
				return false;
			m_data.setValue(key + QString("_%1").arg(++i), shortcut.toString() );
		}
	}

	QString shortcut = m_data.value(key + QString("_%1").arg(++i) ).toString();
	if (!shortcut.isEmpty())
		m_data.setValue(key + QString("_%1").arg(i), "");

	return true;
}


void	MenuCommands::SetTitle(int ind, QString title) 
{
	m_data.setValue(GetKey(ind), title);
}

QString	MenuCommands::GetShortcutsString(int ind) const
{
	return Get(ind, Shorts);
}


QList<QKeySequence> MenuCommands::loadShortcuts( QString key, QKeySequence default_shortcut ) const
{
	QList<QKeySequence> keys;
	for (int i = 1; i < 100; i++)
	{
		QString shortcut = m_data.value(key + QString("_%1").arg(i) ).toString();
		if (shortcut.isEmpty())
			break;

		keys.append(QKeySequence(shortcut));
	}
	if (keys.size() == 0 && !default_shortcut.isEmpty())
		keys.append(default_shortcut);

	return keys;
}

QList<QKeySequence> MenuCommands::OpenLocShortcuts() const	
{ 
	QList<QKeySequence> keys = loadShortcuts( OpenLocKey() );
	if (keys.size() == 0)
	{
		keys.append(QKeySequence(Qt::ControlModifier + Qt::Key_L));
		keys.append(QKeySequence(Qt::ALT | Qt::Key_D));
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::StatusShortcuts()  const		
{ 
	QList<QKeySequence> keys = loadShortcuts( StatusKey() );
	if (keys.size() == 0)
	{
		keys.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
		keys.append(QKeySequence("Ctrl+/"));
	}
	return  keys; 
}


QList<QKeySequence> MenuCommands::NavShortcuts()  const			
{ 
	QList<QKeySequence> keys = loadShortcuts( NavKey() );
	if (keys.size() == 0)
	{
		keys.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
		keys.append(QKeySequence("Ctrl+|"));
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::StopShortcuts()  const			
{ 
	QList<QKeySequence> keys = loadShortcuts( StopKey() );
	if (keys.size() == 0)
	{
		keys.append(Qt::Key_Escape);
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::NextTabShortcuts() const			
{ 
	QList<QKeySequence> keys = loadShortcuts( NextTabKey() );
	if (keys.size() == 0)
	{
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_Tab));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::PrevTabShortcuts() const			
{ 
	QList<QKeySequence> keys = loadShortcuts( PrevTabKey() );
	if (keys.size() == 0)
	{
		keys.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
		keys.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::ReloadShortcuts() const	
{ 
	QList<QKeySequence> keys = loadShortcuts( ReloadKey() ); 
	if (keys.size() == 0)
	{
		keys.append( QKeySequence(Qt::CTRL | Qt::Key_R) );
		keys.append( QKeySequence::Refresh );
	}
	return  keys; 
}

QList<QKeySequence> MenuCommands::CloseTabShortcuts() const	
{ 
	QList<QKeySequence> keys = loadShortcuts( CloseTabKey() ); 
	if (keys.size() == 0)
	{
		keys.append( QKeySequence(Qt::CTRL | Qt::Key_W) );
		keys.append( QKeySequence::Close );
	}
	return  keys; 
}
