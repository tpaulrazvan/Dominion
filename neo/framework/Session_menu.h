/*
 * Original Doom 3 fullscreen menu integration.
 * Command names and GUI state follow Doom 3's GPL Session_menu.cpp.
 * This file is part of DOMINION and is licensed under GPL-3.0-or-later.
 */
#ifndef __SESSION_MENU_H__
#define __SESSION_MENU_H__

#if defined( DOMINION ) // OLD GUIs - Menu
class idUserInterfaceLocal;

class idSessionMenu
{
public:
	idSessionMenu( bool mapSpawned, idSoundWorld *menuSoundWorld );
	~idSessionMenu();
	bool Initialize( const char *gameDirectory );
	void Activate( bool show );
	bool IsActive() const
	{
		return active;
	}
	bool HandleEvent( const sysEvent_t *event );
	void Update();
	void SetSaveGameGuiVars();
	void PlayIntro( bool play );

private:
	idUserInterfaceLocal *LoadGui( const char *path, const char *directory = NULL );
	void SetGUI( idUserInterfaceLocal *gui );
	void SetMainMenuGuiVars();
	// OLD GUIs - Multiplayer pause menu: native state/commands, existing BFG services.
	void SetMPMenuGuiVars();
	bool HandleMPMenuCommand( const char *cmd, const idCmdArgs &args, int &index );
	// OLD GUIs - Skin buttons: use the original GUI's skin order for either menu.
	void SetMainMenuSkin( idUserInterfaceLocal *gui = NULL );
	void SetModsMenuGuiVars();
	void DispatchCommand( const char *command );
	bool HandleSaveGameMenuCommand( const idCmdArgs &args, int &index );
	void UpdateSaveGameInfo();
	idUserInterface *GetActiveMenu();
	void StartMenu( bool playIntro );
	void ExitMenu();
	void HandleMainMenuCommands( const char *command );
	void HandleRestartMenuCommands( const char *command );
	void HandleIntroMenuCommands( const char *command );
	void SetPbMenuGuiVars();
	void InitializeCompatibilityCVars();
	void ReadVideoSettings();
	void ApplyVideoSettings();
	void SetMachineSpec();
	void ExecMachineSpec();
	void SetPlayerSkin( const char *skin );
	void UpdateServerList();
	void UpdateServerListGUI();
	void SetServerSorting( int sort );
	idStr MapScreenshot( const char *map ) const;
	void UpdateMPLevelShot();
	void OnServerListReady();
	void UpdateServerInfo();

	void Message( const char *text, const char *title );

	int com_frameTime; // Original menu time, independent of the paused game clock.
	bool active;
	bool mapSpawned;
	bool activateIntro;
	bool searchingServers;
	int serverSort;
	bool serverSortDescending = false;
	int readVideoMode = -1;
	int readMultiSamples = 0;
	int readShadows = 1;
	int readFullscreen = 0;
	int readVsync = 1;
	idStr currentMPMap;
	idSoundWorld *menuSoundWorld;
	idUserInterfaceLocal *guiMainMenu;
	idUserInterfaceLocal *guiMPMenu = NULL; // OLD GUIs - Multiplayer pause menu
	idUserInterfaceLocal *guiRestartMenu;
	idUserInterfaceLocal *guiIntro;
	idUserInterfaceLocal *guiMsg;
	idUserInterfaceLocal *guiActive;
	idUserInterfaceLocal *guiMsgRestore;
	idListGUI *guiMainMenu_MapList;
	idListGUI *guiMainMenu_ServerList;
	idStrList modsList;
	idList<idSaveGameDetails> saveGameDetails;
};

// Check both formats in each directory before moving to the next directory.
// This lets a mod GUI override a base SWF, while retaining SWF priority in a mod.
bool Session_LoadMenu( const char *swfName, bool mapSpawned, idSoundWorld *menuSoundWorld,
                       idSWF *&swf, idSessionMenu *&legacy );
#endif
#endif
