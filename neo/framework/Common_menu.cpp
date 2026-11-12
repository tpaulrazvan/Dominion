/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "Common_local.h"
#if defined( DOMINION ) // OLD GUIs - Refresh overwritten save previews
#include "../renderer/Image.h"
#endif

/*
==============
idCommonLocal::InitializeMPMapsModes
==============
*/
void idCommonLocal::InitializeMPMapsModes()
{

	const char** gameModes = NULL;
	const char** gameModesDisplay = NULL;
	int numModes = game->GetMPGameModes( &gameModes, &gameModesDisplay );
	mpGameModes.SetNum( numModes );
	for( int i = 0; i < numModes; i++ )
	{
		mpGameModes[i] = gameModes[i];
	}
	mpDisplayGameModes.SetNum( numModes );
	for( int i = 0; i < numModes; i++ )
	{
		mpDisplayGameModes[i] = gameModesDisplay[i];
	}
	int numMaps = declManager->GetNumDecls( DECL_MAPDEF );
	mpGameMaps.Clear();
	for( int i = 0; i < numMaps; i++ )
	{
		const idDeclEntityDef* mapDef = static_cast<const idDeclEntityDef*>( declManager->DeclByIndex( DECL_MAPDEF, i ) );
		uint32 supportedModes = 0;
		for( int j = 0; j < numModes; j++ )
		{
			if( mapDef->dict.GetBool( gameModes[j], false ) )
			{
				supportedModes |= BIT( j );
			}
		}
		if( supportedModes != 0 )
		{
			mpMap_t& mpMap = mpGameMaps.Alloc();
			mpMap.mapFile = mapDef->GetName();
			mpMap.mapName = mapDef->dict.GetString( "name", mpMap.mapFile );
			mpMap.supportedModes = supportedModes;
		}
	}
}

/*
==============
idCommonLocal::OnStartHosting
==============
*/
void idCommonLocal::OnStartHosting( idMatchParameters& parms )
{
	if( ( parms.matchFlags & MATCH_REQUIRE_PARTY_LOBBY ) == 0 )
	{
		return; // This is the party lobby or a SP match
	}

	// If we were searching for a random match but didn't find one, we'll need to select parameters now
	if( parms.gameMap < 0 )
	{
		if( parms.gameMode < 0 )
		{
			// Random mode means any map will do
			parms.gameMap = idLib::frameNumber % mpGameMaps.Num();
		}
		else
		{
			// Select a map which supports the chosen mode
			idList<int> supportedMaps;
			uint32 supportedMode = BIT( parms.gameMode );
			for( int i = 0; i < mpGameMaps.Num(); i++ )
			{
				if( mpGameMaps[i].supportedModes & supportedMode )
				{
					supportedMaps.Append( i );
				}
			}
			if( supportedMaps.Num() == 0 )
			{
				// We don't have any maps which support the chosen mode...
				parms.gameMap = idLib::frameNumber % mpGameMaps.Num();
				parms.gameMode = -1;
			}
			else
			{
				parms.gameMap = supportedMaps[ idLib::frameNumber % supportedMaps.Num() ];
			}
		}
	}
	if( parms.gameMode < 0 )
	{
		uint32 supportedModes = mpGameMaps[parms.gameMap].supportedModes;
		int8 supportedModeList[32] = {};
		int numSupportedModes = 0;
		for( int i = 0; i < 32; i++ )
		{
			if( supportedModes & BIT( i ) )
			{
				supportedModeList[numSupportedModes] = i;
				numSupportedModes++;
			}
		}
		parms.gameMode = supportedModeList[( idLib::frameNumber / mpGameMaps.Num() ) % numSupportedModes ];
	}
	parms.mapName = mpGameMaps[parms.gameMap].mapFile;
	parms.numSlots = session->GetTitleStorageInt( "MAX_PLAYERS_ALLOWED", 4 );
}

/*
==============
idCommonLocal::StartMainMenu
==============
*/
void idCommonLocal::StartMenu( bool playIntro )
{
	if( game && game->Shell_IsActive() )
	{
		return;
	}

#if defined( RENDERDEMOS )
	if( readDemo )
	{
		// if we're playing a demo, esc kills it
		UnloadMap();
	}
#endif

	if( game )
	{
		game->Shell_Show( true );
		game->Shell_SyncWithSession();
	}

	console->Close();

}

/*
===============
idCommonLocal::ExitMenu
===============
*/
void idCommonLocal::ExitMenu()
{
	if( game )
	{
		game->Shell_Show( false );
	}
}

/*
==============
idCommonLocal::MenuEvent

Executes any commands returned by the gui
==============
*/
bool idCommonLocal::MenuEvent( const sysEvent_t* event )
{

	if( session->GetSignInManager().ProcessInputEvent( event ) )
	{
		return true;
	}

	if( game && game->Shell_IsActive() )
	{
		return game->Shell_HandleGuiEvent( event );
	}

	if( game )
	{
		return game->HandlePlayerGuiEvent( event );
	}

	return false;
}

/*
=================
idCommonLocal::GuiFrameEvents
=================
*/
void idCommonLocal::GuiFrameEvents()
{
	if( game )
	{
		game->Shell_SyncWithSession();
	}
}

#if defined( DOMINION ) // OLD GUIs - Menu
#include "../d3xp/Game_local.h"
#include "Session_menu.h"
#include "../ui/Rectangle.h"
#include "../ui/DeviceContext.h"
#include "../ui/UserInterfaceLocal.h"
#include "../ui/ListGUI.h"

// idSessionMenu retains the vanilla Session_menu.cpp function order and GUI
// protocol. BFG's idSessionLocal is a network session; the shell owns this small
// menu controller instead. Backend adaptations are kept in helpers below.
// OLD GUIs - Menu return: BFG rebuilds the shell after maps, but the startup intro
// belongs to the process startup, rather than each new menu controller.
static bool oldGuiStartupIntroPending = true;
static bool oldGuiStoryIntroPending = false;
static idCVar com_machineSpec( "com_machineSpec", "3", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "original menu quality preset", 0, 3 );
static idCVar gui_configServerRate( "gui_configServerRate", "0", CVAR_GUI | CVAR_ARCHIVE | CVAR_ROM | CVAR_INTEGER, "original menu connection preset" );

/*
===============
idSessionMenu::GetActiveMenu
===============
*/
idUserInterface *idSessionMenu::GetActiveMenu()
{
	return active ? guiActive : NULL;
}

/*
===============
idSessionMenu::StartMenu
===============
*/
void idSessionMenu::StartMenu( bool playIntro )
{
	SetGUI( guiMainMenu );
	guiMainMenu->HandleNamedEvent( playIntro ? "playIntro" : "noIntro" );
	console->Close();
}
/*
===============
idSessionMenu::SetGUI
===============
*/
void idSessionMenu::SetGUI( idUserInterfaceLocal *gui )
{
	com_frameTime = Sys_Milliseconds();
	if ( guiActive != NULL && guiActive != gui )
	{
		guiActive->Activate( false, com_frameTime );
	}
	guiActive = gui;
	if ( guiActive != NULL )
	{
		guiActive->SetKeyBindingNames();
		guiActive->StateChanged( com_frameTime, false );
		// OLD GUIs - Menu return: like vanilla/dhewm3 SetGUI, evaluate the initial
		// timeline before activation and before noIntro applies the final menu state.
		sysEvent_t event = {};
		event.evType = SE_NONE;
		guiActive->HandleEvent( &event, com_frameTime, NULL );
		const idStr command = guiActive->Activate( true, com_frameTime );
		DispatchCommand( command );
	}
}

/*
===============
idSessionMenu::ExitMenu
===============
*/
void idSessionMenu::ExitMenu()
{
	Activate( false );
}

/*
===============
idSessionMenu::SetSaveGameGuiVars
===============
*/
void idSessionMenu::SetSaveGameGuiVars()
{
	com_frameTime = Sys_Milliseconds();
	const saveGameDetailsList_t &entries = session->GetSaveGameManager().GetEnumeratedSavegames();
	saveGameDetails.Clear();
	for ( int i = 0; i < entries.Num(); ++i )
	{
		saveGameDetails.Append( entries[i] );
	}
	// Keep vanilla's newest-first list ordering.
	for ( int i = 1; i < saveGameDetails.Num(); ++i )
	{
		for ( int j = i; j > 0 && saveGameDetails[j].date > saveGameDetails[j - 1].date; --j )
		{
			idSaveGameDetails swap = saveGameDetails[j - 1];
			saveGameDetails[j - 1] = saveGameDetails[j];
			saveGameDetails[j] = swap;
		}
	}
	idUserInterfaceLocal *guis[] = { guiMainMenu, guiRestartMenu };
	for ( int g = 0; g < 2; ++g )
	{
		if ( guis[g] == NULL )
		{
			continue;
		}
		for ( int i = 0; i < saveGameDetails.Num(); ++i )
		{
			const idStr title = saveGameDetails[i].descriptors.GetString( "legacySaveName", saveGameDetails[i].GetMapName() );
			// OLD GUIs - Save list columns: BFG's formatter uses spaces for SWF, not GUI tabs.
			idStr date = Sys_TimeStampToStr( saveGameDetails[i].date );
			date.Replace( " ", "\t" );
			guis[g]->SetStateString( va( "loadgame_item_%d", i ), va( "%s\t%s", title.c_str(), date.c_str() ) );
		}
		guis[g]->DeleteStateVar( va( "loadgame_item_%d", saveGameDetails.Num() ) );
		guis[g]->SetStateInt( "loadgame_sel_0", -1 );
		guis[g]->SetStateString( "loadgame_shot", "guis/assets/blankLevelShot" );
		guis[g]->StateChanged( com_frameTime, false );
	}
}

/*
===============
idSessionMenu::SetModsMenuGuiVars
===============
*/
void idSessionMenu::SetModsMenuGuiVars()
{
	modsList.Clear();
	// Mod folders are siblings of the selected base folder, not virtual files inside the active mod.
	idStrList roots;
	roots.AddUnique( cvarSystem->GetCVarString( "fs_savepath" ) );
	roots.AddUnique( cvarSystem->GetCVarString( "fs_basepath" ) );
	for ( int r = 0; r < roots.Num(); ++r )
	{
		idStrList folders;
		Sys_ListFiles( roots[r], "/", folders );
		for ( int i = 0; i < folders.Num(); ++i )
		{
			if ( folders[i] == "." || folders[i] == ".." || folders[i].Icmp( fileSystem->GetBaseGameDir() ) == 0
			   )
			{
				continue;
			}
			const idStr path = roots[r] + "/" + folders[i];
			idFileLocal description( fileSystem->OpenExplicitFileRead( path + "/description.txt" ) );
			idStrList packages;
			Sys_ListFiles( path, ".pk4", packages );
			if ( description == NULL && packages.Num() == 0 )
			{
				continue;
			}
			const int index = modsList.AddUnique( folders[i] );
			idStr title = folders[i];
			if ( description != NULL && description->Length() > 0 )
			{
				idTempArray<char> text( description->Length() + 1 );
				text[description->Length()] = '\0';
				description->Read( text.Ptr(), description->Length() );
				title = text.Ptr();
			}
			guiMainMenu->SetStateString( va( "modsList_item_%d", index ), title );
		}
	}
	guiMainMenu->DeleteStateVar( va( "modsList_item_%d", modsList.Num() ) );
	guiMainMenu->SetStateInt( "modsList_sel_0", -1 );
}

/*
===============
idSessionMenu::SetMainMenuSkin
===============
*/
void idSessionMenu::SetMainMenuSkin( idUserInterfaceLocal *gui )
{
	// OLD GUIs - Skin buttons: main and pause menus both use mod_validSkins order.
	if( gui == NULL )
	{
		gui = guiMainMenu;
	}
	// skins
	idStr str = cvarSystem->GetCVarString( "mod_validSkins" );
	idStr uiSkin = cvarSystem->GetCVarString( "ui_skin" );
	idStr skin;
	int skinId = 1;
	int count = 1;
	while ( str.Length() )
	{
		int n = str.Find( ";" );
		if ( n >= 0 )
		{
			skin = str.Left( n );
			str = str.Right( str.Length() - n - 1 );
		}
		else
		{
			skin = str;
			str = "";
		}
		if ( skin.Icmp( uiSkin ) == 0 )
		{
			skinId = count;
		}
		count++;
	}

	for ( int i = 0; i < count; i++ )
	{
		gui->SetStateInt( va( "skin%i", i + 1 ), 0 );
	}
	gui->SetStateInt( va( "skin%i", skinId ), 1 );
}

/*
===============
idSessionMenu::SetPbMenuGuiVars
===============
*/
void idSessionMenu::SetPbMenuGuiVars()
{
	// PunkBuster was not included in the GPL engine.
}

/*
===============
idSessionMenu::SetMainMenuGuiVars
===============
*/
void idSessionMenu::SetMainMenuGuiVars()
{
	guiMainMenu->SetStateInt( "inGame", mapSpawned ? ( common->IsMultiplayer() ? 2 : 1 ) : 0 );
	guiMainMenu->SetStateInt( "nightmare", cvarSystem->GetCVarBool( "g_nightmare" ) ? 1 : 0 );
	guiMainMenu->SetStateInt( "com_machineSpec", cvarSystem->GetCVarInteger( "com_machineSpec" ) );
	guiMainMenu->SetStateString( "inetGame", "" );
	guiMainMenu->SetStateInt( "serverlist_sel_0", -1 );
	guiMainMenu->SetStateInt( "serverlist_selid_0", -1 );
	guiMainMenu->SetStateString( "browser_levelshot", "guis/assets/splash/pdtempa" );
	guiMainMenu->SetStateInt( "driver_prompt", 0 );
	ReadVideoSettings();
	guiMainMenu->SetKeyBindingNames();
	guiMainMenu->SetStateString( "str_d3key_state", "Not required" );
	guiMainMenu->SetStateString( "str_xpkey_state", "Not required" );
	SetMainMenuSkin();
	SetModsMenuGuiVars();
}

/*
===============
idSessionMenu::HandleSaveGameMenuCommand
===============
*/
bool idSessionMenu::HandleSaveGameMenuCommand( const idCmdArgs &args, int &index )
{
	const char *command = args.Argv( index - 1 );
	const int choice = guiActive->State().GetInt( "loadgame_sel_0", "-1" );
	if ( !idStr::Icmp( command, "loadGame" ) )
	{
		if ( choice >= 0 && choice < saveGameDetails.Num() )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "loadGame \"%s\"\n", saveGameDetails[choice].slotName.c_str() ) );
		}
		return true;
	}
	if ( !idStr::Icmp( command, "saveGame" ) )
	{
		idStr name = guiActive->State().GetString( "saveGameName" );
		const bool overwrite = index < args.Argc() && idStr::IsNumeric( args.Argv( index ) ) && atoi( args.Argv( index++ ) ) != 0;
		if ( !name.IsEmpty() && mapSpawned )
		{
			idStr slot = name;
			slot.RemoveColors();
			slot.StripFileExtension();
			for ( int i = 0; i < slot.Length(); ++i )
			{
				if ( ( unsigned char )slot[i] >= 128 || strchr( " ',.~!@#$%^&*()[]{}<>\\|/=?+;:-\"", slot[i] ) != NULL )
				{
					slot[i] = '_';
				}
			}
			for ( int i = 0; i < saveGameDetails.Num(); ++i )
			{
				if ( saveGameDetails[i].slotName.Icmp( slot ) == 0 && !overwrite )
				{
					guiActive->HandleNamedEvent( saveGameDetails[i].slotName.Icmp( "autosave" ) == 0 ? "autosaveOverwriteError" : "saveGameOverwrite" );
					return true;
				}
			}
			// Queue a stable command after GUI dispatch; SaveGame can pump frames.
			name.ReplaceChar( '"', '_' );
			name.ReplaceChar( '\n', '_' );
			name.ReplaceChar( '\r', '_' );
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "saveGame \"%s\"\n", name.c_str() ) );
		}
		return true;
	}
	if ( !idStr::Icmp( command, "deleteGame" ) )
	{
		if ( choice >= 0 && choice < saveGameDetails.Num() )
		{
			const idStr name = saveGameDetails[choice].slotName;
			session->DeleteSaveGameAsync( name );
		}
		return true;
	}
	if ( !idStr::Icmp( command, "updateSaveGameInfo" ) )
	{
		UpdateSaveGameInfo();
		return true;
	}
	return false;
}

/*
===============
idSessionMenu::HandleRestartMenuCommands
===============
*/
void idSessionMenu::HandleRestartMenuCommands( const char *menuCommand )
{
	int icmd;
	idCmdArgs args;
	args.TokenizeString( menuCommand, false );
	for ( icmd = 0; icmd < args.Argc(); )
	{
		const char *cmd = args.Argv( icmd++ );
		if ( HandleSaveGameMenuCommand( args, icmd ) )
		{
			continue;
		}
		if ( !idStr::Icmp( cmd, "restart" ) )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "menuRestart\n" );
			return;
		}
		if ( !idStr::Icmp( cmd, "quit" ) )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
			return;
		}
		if ( !idStr::Icmp( cmd, "exec" ) && icmd < args.Argc() )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, idStr( args.Argv( icmd++ ) ) + "\n" );
			continue;
		}
		if ( !idStr::Icmp( cmd, "play" ) && icmd < args.Argc() )
		{
			menuSoundWorld->PlayShaderDirectly( args.Argv( icmd++ ) );
		}
	}
}
/*
===============
idSessionMenu::HandleIntroMenuCommands
===============
*/
void idSessionMenu::HandleIntroMenuCommands( const char *menuCommand )
{
	// execute the command from the menu
	int i;
	idCmdArgs args;

	args.TokenizeString( menuCommand, false );

	for ( i = 0; i < args.Argc(); )
	{
		const char *cmd = args.Argv( i++ );

		if ( !idStr::Icmp( cmd, "startGame" ) )
		{
			menuSoundWorld->ClearAllSoundEmitters();
			ExitMenu();
			continue;
		}

		if ( !idStr::Icmp( cmd, "play" ) )
		{
			if ( args.Argc() - i >= 1 )
			{
				idStr snd = args.Argv( i++ );
				menuSoundWorld->PlayShaderDirectly( snd );
			}
			continue;
		}
	}
}

/*
===============
idSessionMenu::UpdateMPLevelShot
===============
*/
void idSessionMenu::UpdateMPLevelShot()
{
	guiMainMenu->SetStateString( "current_levelshot", MapScreenshot( currentMPMap ) );
}

/*
===============
idSessionMenu::HandleMainMenuCommands
===============
*/
void idSessionMenu::HandleMainMenuCommands( const char *menuCommand )
{
	// execute the command from the menu
	int icmd;
	idCmdArgs args;

	args.TokenizeString( menuCommand, false );

	for ( icmd = 0; icmd < args.Argc(); )
	{
		const char *cmd = args.Argv( icmd++ );
		// OLD GUIs - Multiplayer pause menu: reuse shared controls/video commands.
		if( guiActive == guiMPMenu && HandleMPMenuCommand( cmd, args, icmd ) )
		{
			continue;
		}

		if ( HandleSaveGameMenuCommand( args, icmd ) )
		{
			continue;
		}



		if ( !idStr::Icmp( cmd, "startGame" ) )
		{
			cvarSystem->SetCVarInteger( "g_skill", guiActive->State().GetInt( "skill" ) );
			const char *map = "game/mars_city1";
			if ( icmd < args.Argc() && idStr::Icmp( args.Argv( icmd ), ";" ) != 0 )
			{
				map = args.Argv( icmd++ );
			}
			// BFG recreates the shell during a map load. Defer the load until GUI
			// dispatch returns, then activate the intro on the newly created shell.
			oldGuiStoryIntroPending = guiIntro != NULL;
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "map \"%s\"\n", map ) );
			continue;
		}

		if ( !idStr::Icmp( cmd, "quit" ) )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
			return;
		}

		if ( !idStr::Icmp( cmd, "loadMod" ) )
		{
			int choice = guiActive->State().GetInt( "modsList_sel_0" );
			if ( choice >= 0 && choice < modsList.Num() )
			{
				cvarSystem->SetCVarString( "fs_game", modsList[ choice ] );
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "reloadEngine menu\n" );
			}
		}

		if ( !idStr::Icmp( cmd, "UpdateServers" ) )
		{
			UpdateServerList();
			continue;
		}

		if ( !idStr::Icmp( cmd, "RefreshServers" ) )
		{
			UpdateServerList();
			continue;
		}

		if ( !idStr::Icmp( cmd, "FilterServers" ) )
		{
			UpdateServerListGUI();
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortServerName" ) )
		{
			SetServerSorting( 0 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortGame" ) )
		{
			SetServerSorting( 1 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortPlayers" ) )
		{
			SetServerSorting( 2 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortPing" ) )
		{
			SetServerSorting( 3 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortGameType" ) )
		{
			SetServerSorting( 4 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "sortMap" ) )
		{
			SetServerSorting( 5 );
			continue;
		}

		if ( !idStr::Icmp( cmd, "serverList" ) )
		{
			UpdateServerInfo();
			continue;
		}

		if ( !idStr::Icmp( cmd, "LANConnect" ) )
		{
			int sel = guiMainMenu_ServerList->GetSelection( NULL, 0 );
			if ( sel >= 0 && sel < session->NumServers() )
			{
				session->ConnectToServer( sel );
			}
			return;
		}

		if ( !idStr::Icmp( cmd, "MAPScan" ) )
		{
			const char *gametype = cvarSystem->GetCVarString( "si_gameType" );
			if ( gametype == NULL || *gametype == 0 || idStr::Icmp( gametype, "singleplayer" ) == 0 )
			{
				gametype = "Deathmatch";
			}

			guiMainMenu_MapList->Clear();
			guiMainMenu_MapList->SetSelection( 0 );
			for ( int i = 0; i < declManager->GetNumDecls( DECL_MAPDEF ); i++ )
			{
				const idDeclEntityDef *map = static_cast<const idDeclEntityDef *>( declManager->DeclByIndex( DECL_MAPDEF, i ) );
				if ( map->dict.GetBool( gametype ) )
				{
					guiMainMenu_MapList->Add( i, idLocalization::GetString( map->dict.GetString( "name", map->GetName() ) ) );
					if ( !currentMPMap.Icmp( map->GetName() ) )
					{
						guiMainMenu_MapList->SetSelection( guiMainMenu_MapList->Num() - 1 );
					}
				}
			}
			// BFG's si_map is an integer map index. Keep the vanilla path in the
			// menu controller instead of writing a path into that existing CVar.
			int i = guiMainMenu_MapList->GetSelection( NULL, 0 );
			currentMPMap = i >= 0 ? declManager->DeclByIndex( DECL_MAPDEF, i )->GetName() : "";
			UpdateMPLevelShot();
			continue;
		}

		if ( !idStr::Icmp( cmd, "click_mapList" ) )
		{
			int mapNum = guiMainMenu_MapList->GetSelection( NULL, 0 );
			if ( mapNum >= 0 && mapNum < declManager->GetNumDecls( DECL_MAPDEF ) )
			{
				currentMPMap = declManager->DeclByIndex( DECL_MAPDEF, mapNum )->GetName();
			}
			UpdateMPLevelShot();
			continue;
		}

		if ( !idStr::Icmp( cmd, "inetConnect" ) )
		{
			const char *s = guiActive->State().GetString( "inetGame" );
			if ( !s || s[0] == 0 )
			{
				continue;
			}
			netadr_t address;
			if ( Sys_StringToNetAdr( s, &address, true ) )
			{
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "connect %s\n", Sys_NetAdrToString( address ) ) );
			}
			return;
		}

		if ( !idStr::Icmp( cmd, "startMultiplayer" ) )
		{
			// The GUI is original; the network protocol and hosting remain BFG's.
			if ( guiActive->State().GetBool( "dedicated" ) )
			{
				Message( "Dedicated servers are not supported by this BFG session backend.", "Multiplayer" );
				continue;
			}
			if ( currentMPMap.IsEmpty() )
			{
				continue;
			}
			int mode = common->GetModeList().FindIndex( cvarSystem->GetCVarString( "si_gameType" ) );
			if ( mode >= 0 )
			{
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va( "menuStartMultiplayer \"%s\" %d\n", currentMPMap.c_str(), mode ) );
			}
			return;
		}

		if ( !idStr::Icmp( cmd, "mpSkin" ) )
		{
			idStr skin;
			if ( args.Argc() - icmd >= 1 )
			{
				skin = args.Argv( icmd++ );
				cvarSystem->SetCVarString( "ui_skin", skin );
				SetPlayerSkin( skin );
				SetMainMenuSkin();
			}
			continue;
		}

		if ( !idStr::Icmp( cmd, "close" ) )
		{
			// if we aren't in a game, the menu can't be closed
			if ( mapSpawned )
			{
				ExitMenu();
			}
			continue;
		}

		if ( !idStr::Icmp( cmd, "resetdefaults" ) )
		{
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "exec default.cfg" );
			guiActive->SetKeyBindingNames();
			continue;
		}


		if ( !idStr::Icmp( cmd, "bind" ) )
		{
			if ( args.Argc() - icmd >= 2 )
			{
				int key = atoi( args.Argv( icmd++ ) );
				idStr bind = args.Argv( icmd++ );
				if ( idKeyInput::NumBinds( bind ) >= 2 && !idKeyInput::KeyIsBoundTo( key, bind ) )
				{
					idKeyInput::UnbindBinding( bind );
				}
				idKeyInput::SetBinding( key, bind );
				guiActive->SetKeyBindingNames();
			}
			continue;
		}

		if ( !idStr::Icmp( cmd, "play" ) )
		{
			if ( args.Argc() - icmd >= 1 )
			{
				idStr snd = args.Argv( icmd++ );
				int channel = 1;
				if ( snd.Length() == 1 && idStr::IsNumeric( snd ) && icmd < args.Argc() )
				{
					channel = atoi( snd );
					snd = args.Argv( icmd++ );
				}
				menuSoundWorld->PlayShaderDirectly( snd, channel );

			}
			continue;
		}

		if ( !idStr::Icmp( cmd, "music" ) )
		{
			if ( args.Argc() - icmd >= 1 )
			{
				idStr snd = args.Argv( icmd++ );
				menuSoundWorld->PlayShaderDirectly( snd, 2 );
			}
			continue;
		}

		// triggered from mainmenu or mpmain
		if ( !idStr::Icmp( cmd, "sound" ) )
		{
			idStr vcmd;
			if ( args.Argc() - icmd >= 1 )
			{
				vcmd = args.Argv( icmd++ );
			}
			if ( !vcmd.Icmp( "eax" ) )
			{
				cvarSystem->SetCVarBool( "s_useEAXReverb", false );
				Message( idLocalization::GetString( "#str_07232" ), idLocalization::GetString( "#str_07231" ) );
			}
			else if ( !vcmd.Length() || !vcmd.Icmp( "speakers" ) || !vcmd.Icmp( "drivar" ) )
			{
				// BFG uses the output device's speaker configuration.
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "s_restart\n" );
			}
			continue;
		}

		if ( !idStr::Icmp( cmd, "video" ) )
		{
			idStr vcmd;
			if ( args.Argc() - icmd >= 1 )
			{
				vcmd = args.Argv( icmd++ );
			}

			int oldSpec = com_machineSpec.GetInteger();

			if ( idStr::Icmp( vcmd, "low" ) == 0 )
			{
				com_machineSpec.SetInteger( 0 );
			}
			else if ( idStr::Icmp( vcmd, "medium" ) == 0 )
			{
				com_machineSpec.SetInteger( 1 );
			}
			else  if ( idStr::Icmp( vcmd, "high" ) == 0 )
			{
				com_machineSpec.SetInteger( 2 );
			}
			else  if ( idStr::Icmp( vcmd, "ultra" ) == 0 )
			{
				com_machineSpec.SetInteger( 3 );
			}
			else if ( idStr::Icmp( vcmd, "recommended" ) == 0 )
			{
				SetMachineSpec();
			}

			if ( oldSpec != com_machineSpec.GetInteger() )
			{
				guiActive->SetStateInt( "com_machineSpec", com_machineSpec.GetInteger() );
				guiActive->StateChanged( com_frameTime, false );
				ExecMachineSpec();
			}

			if ( idStr::Icmp( vcmd, "restart" )  == 0 )
			{
				guiActive->HandleNamedEvent( "cvar write render" );
				ApplyVideoSettings();
			}

			continue;
		}

		if ( !idStr::Icmp( cmd, "clearBind" ) )
		{
			if ( args.Argc() - icmd >= 1 )
			{
				idKeyInput::UnbindBinding( args.Argv( icmd++ ) );
				guiActive->SetKeyBindingNames();
			}
			continue;
		}

		// FIXME: obsolete
		if ( !idStr::Icmp( cmd, "chatdone" ) )
		{
			idStr temp = guiActive->State().GetString( "chattext" );
			temp += "\r";
			guiActive->SetStateString( "chattext", "" );
			continue;
		}

		if ( !idStr::Icmp ( cmd, "exec" ) )
		{

			//Backup the language so we can restore it after defaults.
			idStr lang = cvarSystem->GetCVarString( "sys_lang" );

			if ( icmd >= args.Argc() )
			{
				continue;
			}
			const idStr executable = args.Argv( icmd++ );
			cmdSystem->BufferCommandText( executable.Icmp( "cvar_restart" ) == 0 ? CMD_EXEC_NOW : CMD_EXEC_APPEND, executable + "\n" );
			if ( idStr::Icmp( "cvar_restart", args.Argv( icmd - 1 ) ) == 0 )
			{
				cmdSystem->BufferCommandText( CMD_EXEC_NOW, "exec default.cfg" );
				SetMachineSpec();

				//Make sure that any r_brightness changes take effect
				float bright = cvarSystem->GetCVarFloat( "r_brightness" );
				cvarSystem->SetCVarFloat( "r_brightness", 0.0f );
				cvarSystem->SetCVarFloat( "r_brightness", bright );

				// BFG persists these player options through archived CVars.
				cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );

				guiActive->SetStateInt( "com_machineSpec", com_machineSpec.GetInteger() );

				//Restore the language
				cvarSystem->SetCVarString( "sys_lang", lang );

			}
			continue;
		}

		if ( !idStr::Icmp ( cmd, "loadBinds" ) )
		{
			guiActive->SetKeyBindingNames();
			continue;
		}

		if ( !idStr::Icmp( cmd, "systemCvars" ) )
		{
			ReadVideoSettings();
			guiActive->HandleNamedEvent( "cvar read render" );
			guiActive->HandleNamedEvent( "cvar read sound" );
			continue;
		}

		if ( !idStr::Icmp( cmd, "SetCDKey" ) )
		{
			Message( "This GPL engine does not require a CD key.", "CD Key" );
			continue;
		}

		if ( !idStr::Icmp( cmd, "CheckUpdate" ) )
		{
			Message( "The original Doom 3 update service is not used by Dominion.", "Updates" );
			continue;
		}

		if ( !idStr::Icmp( cmd, "CheckUpdate2" ) )
		{
			Message( "The original Doom 3 update service is not used by Dominion.", "Updates" );
			continue;
		}

		if ( !idStr::Icmp( cmd, "checkKeys" ) )
		{
			// The GPL engine does not require retail CD-key authentication.
			continue;
		}

		// triggered from mainmenu or mpmain
		if ( !idStr::Icmp( cmd, "punkbuster" ) )
		{
			if ( icmd < args.Argc() && idStr::Icmp( args.Argv( icmd ), ";" ) != 0 )
			{
				icmd++;
			}
			SetPbMenuGuiVars();
			continue;
		}
	}
}

// ---------------------------------------------------------------------------
// DOMINION/BFG integration. The original command handlers above retain their
// vanilla names and order so changes can be compared with the 2004 source.
// ---------------------------------------------------------------------------
static bool OldGui_FileExists( const char *name, const char *directory )
{
	idFileLocal file( fileSystem->OpenFileRead( name, false, directory ) );
	return file != NULL;
}

bool Session_LoadMenu( const char *swfName, bool mapSpawned, idSoundWorld *menuSoundWorld,
                       idSWF *&swf, idSessionMenu *&legacy )
{
	swf = NULL;
	legacy = NULL;
	if ( !mapSpawned )
	{
		oldGuiStoryIntroPending = false;
	}
	idStrList directories;
	fileSystem->GetGameDirectories( directories );
	idStr source = swfName;
	if ( source.Icmpn( "swf/", 4 ) != 0 )
	{
		source = "swf/" + source;
	}
	source.SetFileExtension( ".swf" );
	idStr binary = "generated/" + source;
	binary.SetFileExtension( ".bswf" );
	idStr json = source;
	json.SetFileExtension( ".json" );
	for ( int i = 0; i < directories.Num(); ++i )
	{
		const char *directory = directories[i];
		if ( OldGui_FileExists( binary, directory ) || OldGui_FileExists( json, directory ) || OldGui_FileExists( source, directory ) )
		{
			swf = new idSWF( swfName, menuSoundWorld, false, false, directory );
			if ( swf->IsLoaded() )
			{
				oldGuiStoryIntroPending = false;
				common->Printf( "Menu: %s/%s (SWF)\n", directory, source.c_str() );
				return true;
			}
			delete swf;
			swf = NULL;
		}
		// Do not initialize any original-menu state when this directory has no
		// GUI. The next directory may contain the normal BFG SWF menu.
		idStr localized = va( "guis/%s/mainmenu.gui", cvarSystem->GetCVarString( "sys_lang" ) );
		const bool mpMenu = mapSpawned && common->IsMultiplayer() &&
			( OldGui_FileExists( "guis/mpmain.gui", directory ) || OldGui_FileExists( va( "guis/%s/mpmain.gui", cvarSystem->GetCVarString( "sys_lang" ) ), directory ) );
		if ( !mpMenu && !OldGui_FileExists( "guis/mainmenu.gui", directory ) && !OldGui_FileExists( localized, directory ) )
		{
			continue;
		}
		legacy = new idSessionMenu( mapSpawned, menuSoundWorld );
		if ( legacy->Initialize( directory ) )
		{
			common->Printf( "Menu: %s/guis/%s (original GUI)\n", directory, mpMenu ? "mpmain.gui" : "mainmenu.gui" );
			return true;
		}
		delete legacy;
		legacy = NULL;
	}
	common->Warning( "No %s menu SWF or guis/mainmenu.gui in the active game directories", swfName );
	return false;
}

idSessionMenu::idSessionMenu( bool inGame_, idSoundWorld *soundWorld_ ) :
	com_frameTime( Sys_Milliseconds() ), active( false ), mapSpawned( inGame_ ), activateIntro( false ),
	searchingServers( false ), serverSort( 0 ), menuSoundWorld( soundWorld_ ),
	guiMainMenu( NULL ), guiRestartMenu( NULL ), guiIntro( NULL ), guiMsg( NULL ),
	guiActive( NULL ), guiMsgRestore( NULL ), guiMainMenu_MapList( NULL ), guiMainMenu_ServerList( NULL )
{
}

idSessionMenu::~idSessionMenu()
{
	if ( searchingServers )
	{
		session->CancelListServers();
	}
	if ( guiMainMenu_ServerList != NULL )
	{
		guiMainMenu_ServerList->Shutdown();
		uiManager->FreeListGUI( guiMainMenu_ServerList );
	}
	if ( guiMainMenu_MapList != NULL )
	{
		guiMainMenu_MapList->Shutdown();
		uiManager->FreeListGUI( guiMainMenu_MapList );
	}
	uiManager->DeAlloc( guiMainMenu );
	uiManager->DeAlloc( guiMPMenu ); // OLD GUIs - Multiplayer pause menu
	uiManager->DeAlloc( guiRestartMenu );
	uiManager->DeAlloc( guiIntro );
	uiManager->DeAlloc( guiMsg );
}

idUserInterfaceLocal *idSessionMenu::LoadGui( const char *path, const char *directory )
{
	idStrList directories;
	if ( directory != NULL )
	{
		directories.Append( directory );
	}
	else
	{
		fileSystem->GetGameDirectories( directories );
	}
	for ( int i = 0; i < directories.Num(); ++i )
	{
		idUserInterfaceLocal *gui = new( TAG_OLD_UI ) idUserInterfaceLocal();
		if ( gui->InitMenuFromFile( path, directories[i] ) )
		{
			return gui;
		}
		delete gui;
	}
	return NULL;
}

bool idSessionMenu::Initialize( const char *directory )
{
	InitializeCompatibilityCVars();
	guiMainMenu = LoadGui( "guis/mainmenu.gui", directory );
	// OLD GUIs - Multiplayer pause menu: it is a separate native desktop.
	if( mapSpawned && common->IsMultiplayer() )
	{
		guiMPMenu = LoadGui( "guis/mpmain.gui", directory );
		if( guiMPMenu == NULL )
		{
			guiMPMenu = LoadGui( "guis/mpmain.gui" );
		}
	}
	if ( guiMainMenu == NULL && guiMPMenu == NULL )
	{
		return false;
	}
	guiRestartMenu = LoadGui( "guis/restart.gui" );
	guiIntro = LoadGui( "guis/intro.gui" );
	guiMsg = LoadGui( "guis/msg.gui" );
	if( guiMainMenu != NULL )
	{
		guiMainMenu_MapList = uiManager->AllocListGUI();
		guiMainMenu_MapList->Config( guiMainMenu, "mapList" );
		guiMainMenu_ServerList = uiManager->AllocListGUI();
		guiMainMenu_ServerList->Config( guiMainMenu, "serverList" );
		SetMainMenuGuiVars();
	}
	guiActive = guiMPMenu != NULL ? guiMPMenu : guiMainMenu;
	if ( mapSpawned && oldGuiStoryIntroPending )
	{
		oldGuiStoryIntroPending = false;
		if ( guiIntro != NULL )
		{
			guiActive = guiIntro;
			active = true;
			activateIntro = true;
		}
	}
	return true;
}

void idSessionMenu::PlayIntro( bool play )
{
	if( guiMainMenu == NULL || ( mapSpawned && common->IsMultiplayer() ) )
	{
		return;
	}
	guiMainMenu->HandleNamedEvent( play ? "playIntro" : "noIntro" );
}

void idSessionMenu::Activate( bool show )
{
	com_frameTime = Sys_Milliseconds();
	if ( show == active )
	{
		return;
	}
	active = show;
	if ( !show )
	{
		if ( guiActive != NULL )
		{
			guiActive->Activate( false, com_frameTime );
		}
		return;
	}
	// OLD GUIs - Multiplayer pause menu: preserve the game behind the native overlay.
	if( mapSpawned && common->IsMultiplayer() && guiMPMenu != NULL )
	{
		oldGuiStartupIntroPending = false;
		SetMPMenuGuiVars();
		SetGUI( guiMPMenu );
		return;
	}
	SetMainMenuGuiVars();
	idPlayer *player = gameLocal.GetLocalPlayer();
	SetGUI( mapSpawned && player != NULL && player->health <= 0 && !common->IsMultiplayer() && guiRestartMenu != NULL ? guiRestartMenu : guiMainMenu );
	if ( guiActive == guiMainMenu )
	{
		PlayIntro( oldGuiStartupIntroPending && !mapSpawned );
	}
	oldGuiStartupIntroPending = false;
	// Use DOMINION's save manager while keeping the original GUI's list protocol.
	session->EnumerateSaveGamesAsync();
	SetSaveGameGuiVars();
}

bool idSessionMenu::HandleEvent( const sysEvent_t *event )
{
	com_frameTime = Sys_Milliseconds();
	if ( !active || guiActive == NULL )
	{
		return false;
	}
	const idStr command = guiActive->HandleEvent( event, com_frameTime, NULL );
	if ( !command.IsEmpty() )
	{
		DispatchCommand( command );
	}
	else if ( event->evType == SE_KEY && event->evValue2 && event->evValue >= K_F1 && event->evValue <= K_F12 )
	{
		idKeyInput::ExecKeyBinding( event->evValue );
	}
	return true;
}

void idSessionMenu::Update()
{
	com_frameTime = Sys_Milliseconds();
	if ( !active || guiActive == NULL )
	{
		return;
	}
	if ( activateIntro )
	{
		activateIntro = false;
		SetGUI( guiIntro );
	}
	// Vanilla GUI timelines can return commands without mouse/keyboard input.
	sysEvent_t event = {};
	event.evType = SE_NONE;
	HandleEvent( &event );
	if ( active && guiActive != NULL )
	{
		// OLD GUIs - Multiplayer pause menu: native mpmain.gui is an overlay.
		const bool multiplayerOverlay = guiMPMenu != NULL && ( guiActive == guiMPMenu || guiMsgRestore == guiMPMenu );
		if( !multiplayerOverlay )
		{
			renderSystem->SetColor( colorBlack );
			renderSystem->DrawStretchPic( 0, 0, renderSystem->GetVirtualWidth(), renderSystem->GetVirtualHeight(), 0, 0, 1, 1, declManager->FindMaterial( "_white" ) );
		}
		if ( guiActive == guiMsg && guiMsgRestore != NULL )
		{
			guiMsgRestore->Redraw( com_frameTime, false );
		}
		guiActive->Redraw( com_frameTime, false );
	}
}

void idSessionMenu::UpdateSaveGameInfo()
{
	const int choice = guiActive->State().GetInt( "loadgame_sel_0", "-1" );
	if ( choice < 0 || choice >= saveGameDetails.Num() )
	{
		return;
	}
	const idSaveGameDetails &save = saveGameDetails[choice];
	guiActive->SetStateString( "saveGameName", save.descriptors.GetString( "legacySaveName", save.slotName ) );
	guiActive->SetStateString( "saveGameDescription", save.GetMapName() );
	const idStr date = Sys_TimeStampToStr( save.date );
	int tab = date.Find( '\t' );
	if( tab < 0 )
	{
		// OLD GUIs - Save details use the same space-separated BFG timestamp.
		tab = date.Find( ' ' );
	}
	guiActive->SetStateString( "saveGameDate", tab >= 0 ? date.Left( tab ) : date );
	guiActive->SetStateString( "saveGameTime", tab >= 0 ? date.Mid( tab + 1, date.Length() ) : idStr( "" ) );
	// OLD GUIs - Save previews: prefer this save's picture, then the original autosave art.
	idStr preview = save.descriptors.GetString( "legacyPreview" );
	if( preview.IsEmpty() && fileSystem->IsDoom2004() && save.slotName.Icmp( "autosave" ) == 0 )
	{
		idStr mapBase = save.descriptors.GetString( "legacyMap" );
		mapBase.StripPath();
		preview = "guis/assets/autosave/" + mapBase;
	}
	if( preview.IsEmpty() || ( preview.CheckExtension( ".tga" ) && fileSystem->GetFileLength( preview ) < 0 ) )
	{
		preview = MapScreenshot( save.descriptors.GetString( "legacyMap" ) );
	}
	else if( preview.CheckExtension( ".tga" ) )
	{
		// Overwriting a slot must update an already-loaded thumbnail too. Reload
		// queues only this small picture; it does not reload the level's textures.
		const idMaterial *material = declManager->FindMaterial( preview );
		if( material->GetNumStages() > 0 && material->GetStage( 0 )->texture.image != NULL )
		{
			material->GetStage( 0 )->texture.image->Reload( true, NULL );
		}
	}
	guiActive->SetStateString( "loadgame_shot", preview );
	guiActive->StateChanged( com_frameTime, false );
	// Newly written thumbnails were not part of the level preload. Submit their
	// deferred texture upload before the menu tries to draw them (vanilla uploads immediately).
	globalImages->LoadDeferredImages();
}

void idSessionMenu::Message( const char *text, const char *title )
{
	common->Printf( "%s: %s\n", title, text );
	if ( guiMsg == NULL )
	{
		return;
	}
	if ( guiActive != guiMsg )
	{
		guiMsgRestore = guiActive;
	}
	guiMsg->SetStateString( "title", title );
	guiMsg->SetStateString( "message", text );
	guiMsg->SetStateInt( "visible_msgbox", 1 );
	guiMsg->SetStateInt( "visible_waitbox", 0 );
	guiMsg->SetStateInt( "visible_entry", 0 );
	guiMsg->SetStateInt( "visible_cdkey", 0 );
	guiMsg->SetStateInt( "visible_mid", 1 );
	guiMsg->SetStateInt( "visible_left", 0 );
	guiMsg->SetStateInt( "visible_right", 0 );
	guiMsg->SetStateString( "mid", idLocalization::GetString( "#str_04339" ) );
	SetGUI( guiMsg );
}

void idSessionMenu::DispatchCommand( const char *command )
{
	if ( command == NULL || command[0] == '\0' || guiActive == NULL )
	{
		return;
	}
	// A command can activate another GUI; keep the returned script string alive.
	idStr text = command;
	if ( guiActive == guiMainMenu || guiActive == guiMPMenu )
	{
		HandleMainMenuCommands( text );
	}
	else if ( guiActive == guiRestartMenu )
	{
		HandleRestartMenuCommands( text );
	}
	else if ( guiActive == guiIntro )
	{
		HandleIntroMenuCommands( text );
	}
	else if ( guiActive == guiMsg )
	{
		idCmdArgs args;
		args.TokenizeString( text, false );
		if ( !idStr::Icmp( args.Argv( 0 ), "mid" ) || !idStr::Icmp( args.Argv( 0 ), "left" ) || !idStr::Icmp( args.Argv( 0 ), "right" ) )
		{
			idUserInterfaceLocal *restore = guiMsgRestore;
			guiMsgRestore = NULL;
			SetGUI( restore != NULL ? restore : guiMainMenu );
		}
	}
}

// The vanilla resolution list is fixed; BFG's r_vidMode indexes the monitor's
// enumerated modes. Translate only when the user changes the original control.
static const int menuVideoModes[][2] =
{
	{ 320, 240 }, { 400, 300 }, { 512, 384 }, { 640, 480 },
	{ 800, 600 }, { 1024, 768 }, { 1152, 864 }, { 1280, 1024 },
	{ 1600, 1200 }, { 1280, 720 }, { 1280, 768 }, { 1280, 800 },
	{ 1280, 960 }, { 1400, 1050 }, { 1440, 900 }, { 1600, 900 },
	{ 1680, 1050 }, { 1920, 1080 }, { 1920, 1200 }, { 2560, 1600 }
};

void idSessionMenu::InitializeCompatibilityCVars()
{
	// Register removed vanilla names before Choice/Edit/Slider PostParse binds
	// them. Existing engine CVars keep their flags, defaults and implementation.
	// Static registration keeps control bindings valid after cvar_restart.
	static idCVar menu_r_mode( "r_mode", "-1", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_r_multiSamples( "r_multiSamples", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_legacyShadows( "gui_legacyShadows", "1", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_legacyFullscreen( "gui_legacyFullscreen", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_legacyVsync( "gui_legacyVsync", "1", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_in_freeLook( "in_freeLook", "1", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_s_numberOfSpeakers( "s_numberOfSpeakers", "2", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_s_useEAXReverb( "s_useEAXReverb", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_s_reverse( "s_reverse", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_s_driver( "s_driver", "default", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_filter_gameType( "gui_filter_gameType", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_filter_password( "gui_filter_password", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_filter_players( "gui_filter_players", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_filter_idle( "gui_filter_idle", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_gui_filter_game( "gui_filter_game", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_name( "si_name", "DOOM Server", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_gameType( "si_gameType", "Deathmatch", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_maxPlayers( "si_maxPlayers", "4", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_warmup( "si_warmup", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_usepass( "si_usepass", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_si_pure( "si_pure", "1", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_g_password( "g_password", "", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_g_mapCycle( "g_mapCycle", "", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_net_clientMaxRate( "net_clientMaxRate", "16000", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_net_serverRemoteConsolePassword( "net_serverRemoteConsolePassword", "", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_net_serverReloadEngine( "net_serverReloadEngine", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_net_serverAllowServerMod( "net_serverAllowServerMod", "0", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	static idCVar menu_ui_skin( "ui_skin", "skins/characters/player/marine_mp", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	// OLD GUIs - Skin buttons: vanilla's five button slots are default, green, blue, red, yellow.
	static idCVar menu_mod_validSkins( "mod_validSkins", "skins/characters/player/marine_mp;skins/characters/player/marine_mp_green;skins/characters/player/marine_mp_blue;skins/characters/player/marine_mp_red;skins/characters/player/marine_mp_yellow", CVAR_GUI | CVAR_ARCHIVE, "original menu compatibility value" );
	// Repair only the incorrect default archived by earlier compatibility builds; retain custom mod lists.
	if( idStr::Icmp( menu_mod_validSkins.GetString(), "skins/characters/player/marine_mp;skins/characters/player/marine_mp_red;skins/characters/player/marine_mp_blue;skins/characters/player/marine_mp_green" ) == 0 )
	{
		menu_mod_validSkins.SetString( menu_mod_validSkins.GetDefaultString() );
	}
	ReadVideoSettings();
}

void idSessionMenu::ReadVideoSettings()
{
	readVideoMode = -1;
	for ( int i = 0; i < sizeof( menuVideoModes ) / sizeof( menuVideoModes[0] ); i++ )
	{
		if ( renderSystem->GetWidth() == menuVideoModes[i][0] && renderSystem->GetHeight() == menuVideoModes[i][1] )
		{
			readVideoMode = i;
			break;
		}
	}
	cvarSystem->SetCVarInteger( "r_mode", readVideoMode );
	readShadows = cvarSystem->GetCVarInteger( "r_shadows" );
	readFullscreen = cvarSystem->GetCVarInteger( "r_fullscreen" );
	readVsync = cvarSystem->GetCVarInteger( "r_swapInterval" );
	cvarSystem->SetCVarBool( "gui_legacyShadows", readShadows != 0 );
	cvarSystem->SetCVarBool( "gui_legacyFullscreen", readFullscreen != 0 );
	cvarSystem->SetCVarBool( "gui_legacyVsync", readVsync != 0 );
	readMultiSamples = 0;
#if ID_MSAA
	const int aa = cvarSystem->GetCVarInteger( "r_antiAliasing" );
	readMultiSamples = aa == ANTI_ALIASING_MSAA_2X ? 2 : aa == ANTI_ALIASING_MSAA_4X ? 4 : 0;
#endif
	cvarSystem->SetCVarInteger( "r_multiSamples", readMultiSamples );
}

void idSessionMenu::ApplyVideoSettings()
{
	const int mode = cvarSystem->GetCVarInteger( "r_mode" );
	if ( mode != readVideoMode && mode >= 0 && mode < sizeof( menuVideoModes ) / sizeof( menuVideoModes[0] ) )
	{
		cvarSystem->SetCVarInteger( "r_vidMode", -1 );
		cvarSystem->SetCVarInteger( "r_customWidth", menuVideoModes[mode][0] );
		cvarSystem->SetCVarInteger( "r_customHeight", menuVideoModes[mode][1] );
	}
	const bool shadows = cvarSystem->GetCVarBool( "gui_legacyShadows" );
	if ( shadows != ( readShadows != 0 ) )
	{
		cvarSystem->SetCVarInteger( "r_shadows", shadows ? 2 : 0 );
	}
	const bool fullscreen = cvarSystem->GetCVarBool( "gui_legacyFullscreen" );
	if ( fullscreen != ( readFullscreen != 0 ) )
	{
		cvarSystem->SetCVarInteger( "r_fullscreen", fullscreen ? 1 : 0 );
	}
	const bool vsync = cvarSystem->GetCVarBool( "gui_legacyVsync" );
	if ( vsync != ( readVsync != 0 ) )
	{
		cvarSystem->SetCVarInteger( "r_swapInterval", vsync ? 1 : 0 );
	}
	const int samples = cvarSystem->GetCVarInteger( "r_multiSamples" );
	if ( samples != readMultiSamples )
	{
#if ID_MSAA
		cvarSystem->SetCVarInteger( "r_antiAliasing", samples >= 4 ? ANTI_ALIASING_MSAA_4X : samples >= 2 ? ANTI_ALIASING_MSAA_2X : 0 );
#else
		if ( samples != 0 )
		{
			Message( "This renderer build does not support MSAA.", "Anti-aliasing" );
		}
		else
		{
			cvarSystem->SetCVarInteger( "r_antiAliasing", 0 );
		}
#endif
	}
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "vid_restart\n" );
}

void idSessionMenu::SetMachineSpec()
{
	// The 2004 hardware classifier does not describe a modern renderer. Use
	// Dominion's normal quality as the recommendation, retaining vanilla levels.
	com_machineSpec.SetInteger( 2 );
}

void idSessionMenu::ExecMachineSpec()
{
	const int spec = com_machineSpec.GetInteger();
	// Original preset resolutions and filtering, with BFG renderer equivalents.
	cvarSystem->SetCVarInteger( "r_mode", spec == 3 ? 5 : spec == 2 ? 4 : 3 );
	cvarSystem->SetCVarInteger( "r_multiSamples", 0 );
	cvarSystem->SetCVarInteger( "r_maxAnisotropicFiltering", spec >= 2 ? 8 : 1 );
	cvarSystem->SetCVarBool( "r_useTrilinearFiltering", true );
	cvarSystem->SetCVarInteger( "s_maxSamples", spec == 0 ? 1 : 5 );
	cvarSystem->SetCVarInteger( "r_shadowMapImageSize", spec == 0 ? 256 : spec == 1 ? 512 : spec == 2 ? 1024 : 2048 );
	guiActive->HandleNamedEvent( "cvar read render" );
}

void idSessionMenu::SetPlayerSkin( const char *skin )
{
	// OLD GUIs - Skin buttons: BFG's skin index has a different order from the classic GUI.
	for( int index = 0; index < gameLocal.mpGame.GetNumSkins(); ++index )
	{
		if( idStr::Icmp( gameLocal.mpGame.GetSkinName( index ), skin ) == 0 )
		{
			cvarSystem->SetCVarInteger( "ui_skinIndex", index );
			return;
		}
	}
}

idStr idSessionMenu::MapScreenshot( const char *map ) const
{
	if ( map != NULL && map[0] != '\0' )
	{
		idStr shot = va( "guis/assets/splash/%s", map );
		if ( declManager->FindMaterial( shot, false ) != NULL )
		{
			return shot;
		}
		shot = va( "levelshots/%s.tga", map );
		idFileLocal file( fileSystem->OpenFileRead( shot ) );
		if ( file != NULL )
		{
			return shot;
		}
		idStr base = map;
		base.StripPath();
		shot = va( "guis/assets/loadscreens/%s", base.c_str() );
		if ( declManager->FindMaterial( shot, false ) != NULL )
		{
			return shot;
		}
	}
	return "guis/assets/blankLevelShot";
}

void idSessionMenu::UpdateServerList()
{
	Message( "This BFG backend does not implement server browsing. Use the IP address field to connect directly to another Dominion host.", "Multiplayer" );
	UpdateServerListGUI();
	return;
#if 0 // Enable when the session backend implements ListServers and ServerInfo.
	if ( searchingServers )
	{
		session->CancelListServers();
	}
	searchingServers = true;
	session->ListServers( MakeCallback( this, &idSessionMenu::OnServerListReady ) );
#endif
}

void idSessionMenu::OnServerListReady()
{
	searchingServers = false;
	UpdateServerListGUI();
}

void idSessionMenu::SetServerSorting( int sort )
{
	serverSortDescending = serverSort == sort ? !serverSortDescending : false;
	serverSort = sort;
	UpdateServerListGUI();
}

void idSessionMenu::UpdateServerListGUI()
{
	const idList<mpMap_t> &maps = common->GetMapList();
	const idStrList &modes = common->GetModeDisplayList();
	idList<int> indices;
	for ( int i = 0; i < session->NumServers(); i++ )
	{
		const serverInfo_t *server = session->ServerInfo( i );
		if ( server == NULL || !server->joinable )
		{
			continue;
		}
		const int filter = cvarSystem->GetCVarInteger( "gui_filter_players" );
		if ( ( filter >= 1 && server->numPlayers >= server->maxPlayers ) || ( filter == 2 && server->numPlayers == 0 ) )
		{
			continue;
		}
		const int modeFilter = cvarSystem->GetCVarInteger( "gui_filter_gameType" );
		if ( modeFilter > 0 && server->gameMode != modeFilter - 1 )
		{
			continue;
		}
		indices.Append( i );
	}
	// Keep the original list IDs while changing display order.
	for ( int i = 1; i < indices.Num(); i++ )
	{
		for ( int j = i; j > 0; j-- )
		{
			const serverInfo_t &a = *session->ServerInfo( indices[j - 1] );
			const serverInfo_t &b = *session->ServerInfo( indices[j] );
			int order = a.serverName.Icmp( b.serverName );
			if ( serverSort == 2 )
			{
				order = a.numPlayers - b.numPlayers;
			}
			else if ( serverSort == 4 )
			{
				order = a.gameMode - b.gameMode;
			}
			else if ( serverSort == 5 )
			{
				const char *am = a.gameMap >= 0 && a.gameMap < maps.Num() ? maps[a.gameMap].mapName.c_str() : "";
				const char *bm = b.gameMap >= 0 && b.gameMap < maps.Num() ? maps[b.gameMap].mapName.c_str() : "";
				order = idStr::Icmp( am, bm );
			}
			if ( serverSortDescending ? order >= 0 : order <= 0 )
			{
				break;
			}
			int swap = indices[j];
			indices[j] = indices[j - 1];
			indices[j - 1] = swap;
		}
	}
	guiMainMenu_ServerList->SetStateChanges( false );
	guiMainMenu_ServerList->Clear();
	for ( int i = 0; i < indices.Num(); i++ )
	{
		const serverInfo_t &server = *session->ServerInfo( indices[i] );
		const char *map = server.gameMap >= 0 && server.gameMap < maps.Num() ? idLocalization::GetString( maps[server.gameMap].mapName ) : "";
		const char *mode = server.gameMode >= 0 && server.gameMode < modes.Num() ? idLocalization::GetString( modes[server.gameMode] ) : "";
		// BFG exposes no ping, password or mod fields; leave them unspecified.
		guiMainMenu_ServerList->Add( indices[i], va( "%s\t\t\t\t%d/%d\t--\t%s\t%s", server.serverName.c_str(), server.numPlayers, server.maxPlayers, mode, map ) );
	}
	guiMainMenu_ServerList->SetStateChanges( true );
	guiMainMenu_ServerList->SetSelection( indices.Num() > 0 ? 0 : -1 );
	UpdateServerInfo();
}

void idSessionMenu::UpdateServerInfo()
{
	const int index = guiMainMenu_ServerList->GetSelection( NULL, 0 );
	const serverInfo_t *server = index >= 0 && index < session->NumServers() ? session->ServerInfo( index ) : NULL;
	const idList<mpMap_t> &maps = common->GetMapList();
	const idStrList &modes = common->GetModeDisplayList();
	guiMainMenu->SetStateString( "server_name", server != NULL ? server->serverName.c_str() : "" );
	guiMainMenu->SetStateString( "server_map", server != NULL && server->gameMap >= 0 && server->gameMap < maps.Num() ? idLocalization::GetString( maps[server->gameMap].mapName ) : "" );
	guiMainMenu->SetStateString( "server_gameType", server != NULL && server->gameMode >= 0 && server->gameMode < modes.Num() ? idLocalization::GetString( modes[server->gameMode] ) : "" );
	guiMainMenu->SetStateString( "browser_levelshot", server != NULL && server->gameMap >= 0 && server->gameMap < maps.Num() ? MapScreenshot( maps[server->gameMap].mapFile ) : idStr( "guis/assets/blankLevelShot" ) );
	guiMainMenu->SetStateString( "server_IP", "" );
	guiMainMenu->SetStateString( "server_passworded", "" );
	const idList<idStr> *players = server != NULL ? session->ServerPlayerList( index ) : NULL;
	for ( int i = 0; i < 8; i++ )
	{
		guiMainMenu->SetStateString( va( "player%d", i + 1 ), players != NULL && i < players->Num() ? ( *players )[i].c_str() : "" );
	}
	guiMainMenu->StateChanged( com_frameTime, false );
}

CONSOLE_COMMAND_SHIP( menuRestart, "restart from the autosave selected by the original menu", NULL )
{
	const idStr map = gameLocal.GetMapName();
	if ( !commonLocal.LoadGame( "autosave" ) && !map.IsEmpty() )
	{
		commonLocal.StartNewGame( map, false, GAME_MODE_SINGLEPLAYER );
	}
}

CONSOLE_COMMAND_SHIP( menuStartMultiplayer, "start a BFG multiplayer match from the original menu", NULL )
{
	if ( args.Argc() == 3 )
	{
		commonLocal.StartNewGame( args.Argv( 1 ), false, atoi( args.Argv( 2 ) ) );
	}
}


/*
========================
idSessionMenu::SetMPMenuGuiVars
OLD GUIs - Multiplayer pause menu: native GUI keys, current BFG match capabilities.
========================
*/
void idSessionMenu::SetMPMenuGuiVars()
{
	const idPlayer *player = gameLocal.GetLocalPlayer();
	for( int i = 0; i < gameLocal.serverInfo.GetNumKeyVals(); ++i )
	{
		const idKeyValue *key = gameLocal.serverInfo.GetKeyVal( i );
		guiMPMenu->SetStateString( key->GetKey(), key->GetValue() );
	}
	const bool teamGame = gameLocal.mpGame.IsGametypeTeamBased();
	guiMPMenu->SetStateInt( "teamon", teamGame );
	guiMPMenu->SetStateInt( "teamoff", !teamGame );
	guiMPMenu->SetStateInt( "team", player != NULL ? player->team : 0 );
	guiMPMenu->SetStateString( "spectext", idLocalization::GetString( player != NULL && player->wantSpectate ? "#str_04249" : "#str_04250" ) );
	guiMPMenu->SetStateInt( "isLastMan", gameLocal.gameType == GAME_LASTMAN );
	const char **modeNames;
	const char **modeDisplayNames;
	const int modeCount = gameLocal.mpGame.GetGameModes( &modeNames, &modeDisplayNames );
	if( gameLocal.gameType >= 0 && gameLocal.gameType < modeCount )
	{
		guiMPMenu->SetStateString( "si_gameType", modeNames[gameLocal.gameType] );
	}
	// BFG automatically starts when enough players join; original ready/vote
	// rules do not exist in this backend. Keep their native disabled states.
	guiMPMenu->SetStateInt( "readyon", 0 );
	guiMPMenu->SetStateInt( "readyoff", 1 );
	guiMPMenu->SetStateString( "ui_ready", idLocalization::GetString( "#str_04247" ) );
	guiMPMenu->SetStateInt( "voteon", 0 );
	guiMPMenu->SetStateInt( "voteoff", 1 );
	guiMPMenu->SetStateInt( "callvote", 0 );
	guiMPMenu->SetStateString( "chattext", "" );
	guiMPMenu->SetStateInt( "messagemode", 0 );
	guiMPMenu->SetStateInt( "com_machineSpec", cvarSystem->GetCVarInteger( "com_machineSpec" ) );
	guiMPMenu->SetStateInt( "driver_prompt", 0 );
	// OLD GUIs - Skin buttons: highlight by the GUI list, not BFG's numeric skin index.
	SetMainMenuSkin( guiMPMenu );
	guiMPMenu->SetKeyBindingNames();
	guiMPMenu->StateChanged( com_frameTime, false );
}

/*
========================
idSessionMenu::HandleMPMenuCommand
OLD GUIs - Multiplayer pause menu: original commands dispatched through BFG services.
========================
*/
bool idSessionMenu::HandleMPMenuCommand( const char *cmd, const idCmdArgs &args, int &index )
{
	if( idStr::Icmp( cmd, "disconnect" ) == 0 )
	{
		// Leave the match after GUI dispatch returns; the shell then recreates the
		// ordinary main menu. Immediate teardown would destroy this active desktop.
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );
		ExitMenu();
		return true;
	}
	if( idStr::Icmp( cmd, "close" ) == 0 )
	{
		ExitMenu();
		return true;
	}
	if( idStr::Icmp( cmd, "spectate" ) == 0 )
	{
		gameLocal.mpGame.ToggleSpectate();
		ExitMenu();
		return true;
	}
	if( idStr::Icmp( cmd, "teamtoggle" ) == 0 )
	{
		const idPlayer *player = gameLocal.GetLocalPlayer();
		if( player != NULL && gameLocal.mpGame.IsGametypeTeamBased() )
		{
			session->GetActingGameStateLobbyBase().SetLobbyUserTeam( gameLocal.lobbyUserIDs[player->entityNumber], player->team ^ 1 );
		}
		ExitMenu();
		return true;
	}
	if( idStr::Icmp( cmd, "mpSkin" ) == 0 )
	{
		if( index < args.Argc() )
		{
			cvarSystem->SetCVarString( "ui_skin", args.Argv( index ) );
			SetPlayerSkin( args.Argv( index++ ) );
			SetMPMenuGuiVars();
		}
		return true;
	}
	if( idStr::Icmp( cmd, "chatmessage" ) == 0 )
	{
		idCmdArgs message;
		message.AppendArg( guiMPMenu->State().GetBool( "messagemode" ) ? "sayTeam" : "say" );
		message.AppendArg( guiMPMenu->State().GetString( "chattext" ) );
		cmdSystem->BufferCommandArgs( CMD_EXEC_APPEND, message );
		guiMPMenu->SetStateString( "chattext", "" );
		return true;
	}
	if( idStr::Icmp( cmd, "readytoggle" ) == 0 || idStr::Icmp( cmd, "callVote" ) == 0 ||
		idStr::Icmp( cmd, "voteyes" ) == 0 || idStr::Icmp( cmd, "voteno" ) == 0 )
	{
		Message( "Original Doom 3 ready/voting rules are not implemented by this BFG multiplayer backend.", "Multiplayer" );
		return true;
	}
	return false;
}

#endif // DOMINION: OLD GUIs - Menu
