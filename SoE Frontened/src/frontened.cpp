/*********************************************************************

  Project Name : Source Of Evil Frontened
  Programmer   : Andreas Zahnleiter

 ********************************************************************/

/* $Id: frontened.cpp,v 1.0 2001/12/10 15:57:00 mafuba Exp $ */

// --- Includes ------------------------------------------------------

#define WIN32_LEAN_AND_MEAN                     // Required for windows programs
#include <windows.h>                            // Required for windows programs
#define SOEINCLUDEALL                           // This define includes all headers required by SoE
#include "..\..\SoE Runtime Library\inc\SoE.h"      // Header for SoE RunLib
#include "..\..\SoE Standard\inc\StdRegistry.h"
#include "..\inc\respack.h"                     // For respack_interface

#undef  SOE_ERROR
#define SOE_ERROR(x) MessageBox(NULL, x, "ERROR", MB_OK)

#define USE_NO_VIDEO
//#define USE_LOG_SYSTEM

//int SessionCount = 0;

TCHAR  SOE_TEMP_DIR[MAX_PATH];
char  COMPUTERNAME[256]; 

// --- Prototypes ----------------------------------------------------

void SinglePlayerMode(void);
void MultiPlayerMode(void);
void CreateNetworkGame(void);
void StartSingleSession(void);
void StartMultiSession(void);
void BackToMainMenu(void);
void BackToMultiPlayerModeMenu(void);
void SelectPlayer(void);
void SelectNetPlayer(void);
void SelectLevel(void);
void SelectNetLevel(void);
void Exit(void);
void Minimize(void);
bool SaveRestoreData(void);
bool LoadRestoreData(void);
void InitSoEGameEngine(void);   
void DeInitSoEGameEngine(void);  
bool RunNetworkGameServer(void);
void RunNetworkGameClient(void);
void UpdateNetworkGameList(void);
bool CreateAndLoadSound(void);
bool CreateFullScreenWindow(void);
bool CreateGUIComponents(void);
bool InitPlayerAndLevelLists(void);
void LOG(char* szLog);
BOOL InstallSoE(void);
bool ParseCommandLine(void);
void JoinNetworkGame(void);
bool RemoveLocalHostEntry(void);
void RestoreScreen(void);

// ResPack Modul Schnittstelle
const char* respack_interface(const char* file_name);

// --- Global Variables ----------------------------------------------

char szAppName[]   = "SourceOfEvil";   
char szClassName[] = "SourceOfEvil";
HINSTANCE g_hInst;                     
HWND g_hWnd; BOOL bActive;  
BOOL bWndMinimized = false;
const int BOTTOM_Y = 300;

#ifdef USE_NO_VIDEO
bool bNoVideo   = true;
#else
bool bNoVideo   = false;
#endif

bool bRestore           = false;
bool bCorrupted         = false;
int InitEngineSplashCount = 0;
int ShutdownEngineSplashCount = 0;
Point MousePos; 
LPSTR CmdLine;

SoEScreen  *Screen;     
SoEMouse   *Mouse;
SoEMovie   *Intro;
SoESound   *Sound;
SoESoundBuffer *BackgroundMusic;
SoESoundBuffer *ButtonSelSound;

enum menu_type { MENU_MAIN, MENU_SINGLEPLAYER, MENU_INITGAMEENGINE,
                 MENU_MULTIPLAYER, MENU_SHUTDOWNGAMEENGINE,
				 MENU_CREATENETWORKGAME, MENU_JOINNETWORKGAME };

menu_type current_menu = MENU_MAIN;

// --- Restore Data Structure -----------------------------------------

typedef struct
{
	show_type st_title;    /* TITLE SHOW TYPE */
	menu_type mt_menu;     /* MENU ID */
	DWORD dwBackMusicPos;  /* CURRENT MUSIC POSITION */

	/* Single Player */
	int level_list_sel;    /* SELECTED ENTRY IN LEVEL LIST */
	int player_list_sel;   /* SELECTED ENTRY IN PLAYER LIST */

	/* Multi Player */
	int net_level_list_sel;    /* SELECTED ENTRY IN NET LEVEL LIST */
	int net_player_list_sel;   /* SELECTED ENTRY IN NET PLAYER LIST */

} RESTORE_DATA;

RESTORE_DATA restore_data;

// --- Graphics User Interface Elements -------------------------------

PSOEGUIBUTTON Buttons[13];
PSOEGUIBUTTON MinimizeExitBtns[2];
PSOEGUIIMAGE  Title;
PSOEGUIIMAGE  Copyright;
PSOEGUIIMAGE  Captions[4]; /* Single Player, Multi... usw. */
PSOEGUIIMAGE  InitEngineSplash[13];
PSOEGUIIMAGE  ShutdownEngineSplash[13];
PSOEGUIIMAGE  NetworkGameListFrame;

PSOEGUILIST  level_list       = NULL;
PSOEGUILIST  player_list      = NULL;    
PSOEGUILIST  net_level_list   = NULL;
PSOEGUILIST  net_player_list  = NULL;        
PSOEGUILIST  networkgame_list = NULL;
PSOEGUIIMAGE level_img        = NULL;
PSOEGUIIMAGE player_img       = NULL;
PSOEGUIIMAGE net_level_img    = NULL;
PSOEGUIIMAGE net_player_img   = NULL;

#define LOG_OK     "...OK\n"
#define LOG_FAILED "...FAILED\n"
#define LOG_YES    "...YES\n"
#define LOG_NO     "...NO\n"

//--- Init -----------------------------------------------------------
//
// Function: Handles initialization of the SoE objects
// 
//--------------------------------------------------------------------

BOOL Init() 
{	
	int i;

	if (!bNoVideo)
		ShowCursor(FALSE);
		
	LOG("INIT:CreateFullScreenWindow()\n");
	if (!CreateFullScreenWindow())
		return false;

	LOG("INIT: CreateAndLoadSound()\n");
	if (!CreateAndLoadSound())
		return false;
	
	LOG("INIT: CreateGUIComponents()\n");
	if (!CreateGUIComponents())
		return false;

	if (bRestore && !bCorrupted)
	{
		/* restore music */
		BackgroundMusic->SetCurrentPlayPosition(restore_data.dwBackMusicPos);

		if (FAILED(BackgroundMusic->Play(DSBPLAY_LOOPING)))
			return false;

		/* remove own entry from net_game_list.dat */
		if (current_menu == MENU_CREATENETWORKGAME)
		{
			LOG("RemoveLocalEntryHostEntry : net_game_list.dat");
			if (!RemoveLocalHostEntry())
			{
				LOG(LOG_FAILED);
				return false;
			}
			LOG(LOG_OK);
		}
	}

	if (bRestore)
	{
		/* restore all lists */
		SoEGUIListEntry* tmp = level_list->GetListEntry(restore_data.level_list_sel);
		if (tmp != NULL) tmp->m_bSelected = true;
		
		tmp = player_list->GetListEntry(restore_data.player_list_sel);
		if (tmp != NULL) tmp->m_bSelected = true;
		
		tmp = net_level_list->GetListEntry(restore_data.net_level_list_sel);
		if (tmp != NULL) tmp->m_bSelected = true;
		
		tmp = net_player_list->GetListEntry(restore_data.net_player_list_sel);
		if (tmp != NULL) tmp->m_bSelected = true;
		

		for (i = 0; i != 13; i++)
		{
			ShutdownEngineSplash[i] = new SoEGUIImage();
		
			if (InitEngineSplash[i] == NULL)
				return false;
		}

		char temp[256];

		for (i = 0; i < 13; i++)
		{	
			sprintf(temp, "data\\gfx\\dsoee%d.bmp", i);
			LOG("LOAD: "); LOG(temp); LOG("\n");
			if (!ShutdownEngineSplash[i]->Create(Screen, respack_interface(temp), SHOW_NORMAL, 179, 272))
				return false;
		}
	
        LOG("DeInitSoEGameEngine()\n");
		DeInitSoEGameEngine();
	}

	if (!InitPlayerAndLevelLists())
		return false;

	if (!bRestore && !bCorrupted && bNoVideo && BackgroundMusic != NULL)
		if (FAILED(BackgroundMusic->Play(DSBPLAY_LOOPING)))
			return false;

	return true; 
}

void LOG(char* szLog)
{	
#ifdef USE_LOG_SYSTEM 
	FILE *fp = NULL;	
	fp = fopen(".\\Frontened.log", "a+");

	if (fp == NULL) return;
	
	fprintf(fp, szLog);
	fclose(fp);
#endif
}

//--- DeInit -----------------------------------------------------
//
// Function: Handles cleanup of SoE objects
//
//-------------------------------------------------------------------- 

void DeInit()
{	
	bActive = false;
	int i;

	for (i = 0; i != 13; i++)
		SAFEDELETE(InitEngineSplash[i]);

	for (i = 0; i != 13; i++)
		SAFEDELETE(Buttons[i]);
	
	for (i = 0; i != 2; i++)
		SAFEDELETE(MinimizeExitBtns[i]);

	SAFEDELETE(Captions[0]);
	SAFEDELETE(player_list);
	SAFEDELETE(level_list);
	SAFEDELETE(net_player_list);
	SAFEDELETE(net_level_list);
	SAFEDELETE(NetworkGameListFrame);
	SAFEDELETE(networkgame_list);
	SAFEDELETE(Title);
	SAFEDELETE(Copyright);
	SAFEDELETE(level_img);
	SAFEDELETE(player_img);
	SAFEDELETE(net_level_img);
	SAFEDELETE(net_player_img);

	if (Intro != NULL)
		Intro->Stop();

	SAFEDELETE(Intro);
	
	if (BackgroundMusic != NULL)
		BackgroundMusic->Stop();
	
	SAFEDELETE(BackgroundMusic);

	if (ButtonSelSound != NULL)
		ButtonSelSound->Stop(); 
	
	SAFEDELETE(ButtonSelSound);
	SAFEDELETE(Sound);  
	
	//LOG("DEINIT: GraphicsSystem\n");
	//Screen->ReleaseAllSurfaces();
	//SAFEDELETE(Screen);
}

bool InitPlayerAndLevelLists(void)
{
	if (level_list->GetListEntry(0) != NULL && !bRestore)
	{
		level_list->GetListEntry(0)->m_bSelected = true;
		level_list->GetListEntry(0)->proc();
	}

	if (player_list->GetListEntry(0) != NULL && !bRestore)
	{
		player_list->GetListEntry(0)->m_bSelected = true;
		player_list->GetListEntry(0)->proc();
	}

	if (net_level_list->GetListEntry(0) != NULL && !bRestore)
	{
		net_level_list->GetListEntry(0)->m_bSelected = true;
		net_level_list->GetListEntry(0)->proc();
	}

	if (net_player_list->GetListEntry(0) != NULL && !bRestore)
	{
		net_player_list->GetListEntry(0)->m_bSelected = true;
		net_player_list->GetListEntry(0)->proc();
	}

	return true;
}

bool CreateGUIComponents(void)
{
	int i;

	// --- Create buttons - main menu -----------------------------------

	/* Single Player Mode */

	Buttons[0] = new SoEGUIButton();

	if (Buttons[0] == NULL) return false;
	
	RECT rect;
	rect.left = 278; rect.top = BOTTOM_Y;
	rect.right = 278+244; rect.bottom = BOTTOM_Y+26;

	if (!Buttons[0]->Create(Screen, respack_interface("data\\gfx\\btn1_0.bmp"), 
		respack_interface("data\\gfx\\btn1_1.bmp"), rect))
		return false;

	Buttons[0]->proc = &SinglePlayerMode;
	Buttons[0]->SetClickSound(ButtonSelSound);

	/* Multi Player Mode */

	Buttons[1] = new SoEGUIButton();

	if (Buttons[1] == NULL) return false;

	rect.left = 278; rect.top = BOTTOM_Y+35;
	rect.right = 278+244; rect.bottom = BOTTOM_Y+35+26;
	
	if (!Buttons[1]->Create(Screen, respack_interface("data\\gfx\\btn2_0.bmp"), 
		respack_interface("data\\gfx\\btn2_1.bmp"), rect))
		return false;

	//Buttons[1]->proc = &MultiPlayerMode;
	Buttons[1]->SetClickSound(ButtonSelSound);

	Buttons[2] = new SoEGUIButton();

	if (Buttons[2] == NULL) return false;

	rect.left = 278; rect.top = BOTTOM_Y+70;
	rect.right = 278+244; rect.bottom = BOTTOM_Y+70+26;

	if (!Buttons[2]->Create(Screen, respack_interface("data\\gfx\\btn3_0.bmp"), 
		respack_interface("data\\gfx\\btn3_1.bmp"), rect))
		return false;

	Buttons[2]->SetClickSound(ButtonSelSound);

	Buttons[3] = new SoEGUIButton();

	if (Buttons[3] == NULL) return false;

	rect.left = 278; rect.top = BOTTOM_Y+105;
	rect.right = 278+244; rect.bottom = BOTTOM_Y+105+26;

	if (!Buttons[3]->Create(Screen, respack_interface("data\\gfx\\btn4_0.bmp"), 
		respack_interface("data\\gfx\\btn4_1.bmp"), rect))
		return false;

	Buttons[3]->SetClickSound(ButtonSelSound);

	/* Exit */

	Buttons[4] = new SoEGUIButton();

	if (Buttons[4] == NULL) return false;

	rect.left = 278; rect.top = BOTTOM_Y+140;
	rect.right = 278+244; rect.bottom = BOTTOM_Y+140+26;
	
	if (!Buttons[4]->Create(Screen, respack_interface("data\\gfx\\btn5_0.bmp"), 
		respack_interface("data\\gfx\\btn5_1.bmp"), rect))
		return false;

	Buttons[4]->proc = &Exit;
	Buttons[4]->SetClickSound(ButtonSelSound);

	Buttons[5] = new SoEGUIButton();

	if (Buttons[5] == NULL) return false;
	
	rect.left = 0; rect.top = 569;
	rect.right = 152; rect.bottom = 600;

	if (!Buttons[5]->Create(Screen, respack_interface("data\\gfx\\btn6_0.bmp"), 
		respack_interface("data\\gfx\\btn6_1.bmp"), rect))
		return false;
	
	Buttons[5]->proc = &BackToMainMenu;
	Buttons[5]->SetClickSound(ButtonSelSound);

	/* Create - Singleplayer game */

	Buttons[6] = new SoEGUIButton();

	if (Buttons[6] == NULL) return false;

	rect.left = 648; rect.top = 569;
	rect.right = 800; rect.bottom = 600;

	if (!Buttons[6]->Create(Screen, respack_interface("data\\gfx\\btn7_0.bmp"), 
		respack_interface("data\\gfx\\btn7_1.bmp"), rect)) 
		return false;
	
	Buttons[6]->proc = &StartSingleSession;
	Buttons[6]->SetClickSound(ButtonSelSound);

	/* Create - Multiplayer game */

	Buttons[7] = new SoEGUIButton();

	if (Buttons[7] == NULL) return false;

	rect.left = 0; rect.top = 170;
	rect.right = 152; rect.bottom = 201;

	if (!Buttons[7]->Create(Screen, respack_interface("data\\gfx\\btn8_0.bmp"), 
		respack_interface("data\\gfx\\btn8_1.bmp"), rect)) 
		return false;

	Buttons[7]->proc = &CreateNetworkGame;
	Buttons[7]->SetClickSound(ButtonSelSound);

	/* Join - Multiplayer game */

	Buttons[8] = new SoEGUIButton();

	if (Buttons[8] == NULL) return false;

	rect.left = 0; rect.top = 220;
	rect.right = 152; rect.bottom = 251;

	if (!Buttons[8]->Create(Screen, respack_interface("data\\gfx\\btn9_0.bmp"), 
		respack_interface("data\\gfx\\btn9_1.bmp"), rect)) 
		return false;

	Buttons[8]->proc = &JoinNetworkGame;
	Buttons[8]->SetClickSound(ButtonSelSound);

	/* Back to multiplayer game menu */

	Buttons[9] = new SoEGUIButton();

	if (Buttons[9] == NULL) return false;

	rect.left = 0; rect.top = 569;
	rect.right = 152; rect.bottom = 600;

	if (!Buttons[9]->Create(Screen, respack_interface("data\\gfx\\btn6_0.bmp"), 
		respack_interface("data\\gfx\\btn6_1.bmp"), rect)) 
		return false;

	Buttons[9]->proc = &BackToMultiPlayerModeMenu;
	Buttons[9]->SetClickSound(ButtonSelSound);

	/* Start multiplayer game */

	Buttons[10] = new SoEGUIButton();

	if (Buttons[10] == NULL) return false;

	rect.left = 648; rect.top = 569;
	rect.right = 800; rect.bottom = 600;

	if (!Buttons[10]->Create(Screen, respack_interface("data\\gfx\\btn7_0.bmp"), 
		respack_interface("data\\gfx\\btn7_1.bmp"), rect)) 
		return false;

	Buttons[10]->proc = &StartMultiSession;
	Buttons[10]->SetClickSound(ButtonSelSound);

	/* Update network game list */

	Buttons[11] = new SoEGUIButton();

	if (Buttons[11] == NULL) return false;

	rect.left = 0; rect.top = 270;
	rect.right = 152; rect.bottom = 301;

	if (!Buttons[11]->Create(Screen, respack_interface("data\\gfx\\btn10_0.bmp"), 
		respack_interface("data\\gfx\\btn10_1.bmp"), rect)) 
		return false;

	Buttons[11]->proc = &UpdateNetworkGameList;
	Buttons[11]->SetClickSound(ButtonSelSound);

	/* Start Button - join network game */

	Buttons[12] = new SoEGUIButton();

	if (Buttons[12] == NULL) return false;

	rect.left = 648; rect.top = 569;
	rect.right = 800; rect.bottom = 600;

	if (!Buttons[12]->Create(Screen, respack_interface("data\\gfx\\btn7_0.bmp"), 
		respack_interface("data\\gfx\\btn7_1.bmp"), rect)) 
		return false;

	Buttons[12]->proc = &RunNetworkGameClient;
	Buttons[12]->SetClickSound(ButtonSelSound);

// --- Create Minimize/Exit buttons ------------------------------

	MinimizeExitBtns[0] = new SoEGUIButton();

	if (MinimizeExitBtns[0] == NULL)
		return false;

	rect.left  = 750; rect.top = 0;
	rect.right = 750+16; rect.bottom = 16;

	if (!MinimizeExitBtns[0]->Create(Screen, respack_interface("data\\gfx\\min0btn.bmp"), 
		respack_interface("data\\gfx\\min1btn.bmp"), rect))
		return false;

	MinimizeExitBtns[0]->proc = NULL;//&Minimize;
	MinimizeExitBtns[0]->SetClickSound(ButtonSelSound);

	MinimizeExitBtns[1] = new SoEGUIButton();

	if (MinimizeExitBtns[1] == NULL)
		return false;

	rect.left  = 770; rect.top = 0;
	rect.right = 770+16; rect.bottom = 16;

	if (!MinimizeExitBtns[1]->Create(Screen, respack_interface("data\\gfx\\exit0btn.bmp"), 
		respack_interface("data\\gfx\\exit1btn.bmp"), rect))
		return false;

	MinimizeExitBtns[1]->proc = &Exit;
	MinimizeExitBtns[1]->SetClickSound(ButtonSelSound);
	
// --- Create title ----------------------------------------------

	Title = new SoEGUIImage();

	if (Title == NULL)
		return false;

	show_type t = SHOW_BYLINE_HOR_UPDOWN; 

	if (bRestore) t = restore_data.st_title;

	if (!Title->Create(Screen, respack_interface("data\\gfx\\title.bmp"), t, 0, 50))
		return false;

// --- Create copyright-------------------------------------------

	Copyright = new SoEGUIImage();

	if (Copyright == NULL)
		return false;

	if (!Copyright->Create(Screen, respack_interface("data\\gfx\\copyright.bmp"), SHOW_NORMAL,
			550, 580)) return false;

// --- Create Network game list frame ----------------------------

	NetworkGameListFrame = new SoEGUIImage();

	if (NetworkGameListFrame == NULL)
		return false; 

	if (!NetworkGameListFrame->Create(Screen, respack_interface("data\\gfx\\ng_list.bmp"), SHOW_NORMAL,
			200, 170)) return false;

// --- Create level list -----------------------------------------

	level_list = new SoEGUIList(Screen, ButtonSelSound, 0, 350, 130);

	if (level_list == NULL)
		return false;
	
	if (!level_list->CreateFromListFile(respack_interface("data\\sys\\level.lst")))
		return false;

	for (i = 0; i != level_list->GetListCount(); i++)
		level_list->GetListEntry(i)->proc = &SelectLevel;

// --- Create player list ----------------------------------------

	if ((player_list = new SoEGUIList(Screen, ButtonSelSound, 0, 200, 130)) == NULL)
		return false;

	if (!player_list->CreateFromListFile(respack_interface("data\\sys\\player.lst")))
		return false;

	for (i = 0; i != player_list->GetListCount(); i++)
		player_list->GetListEntry(i)->proc = &SelectPlayer;

// --- Create net level list -----------------------------------------

	net_level_list = new SoEGUIList(Screen, ButtonSelSound, 0, 350, 130);

	if (net_level_list == NULL)
		return false;

	if (!net_level_list->CreateFromListFile(respack_interface("data\\sys\\level.lst")))
		return false;

	for (i = 0; i != net_level_list->GetListCount(); i++)
		net_level_list->GetListEntry(i)->proc = &SelectNetLevel;

// --- Create net player list ----------------------------------------

	net_player_list = new SoEGUIList(Screen, ButtonSelSound, 0, 200, 130);

	if (net_player_list == NULL)
		return false;

	if (!net_player_list->CreateFromListFile(respack_interface("data\\sys\\player.lst")))
		return false;

	for (i = 0; i != net_player_list->GetListCount(); i++)
		net_player_list->GetListEntry(i)->proc = &SelectNetPlayer;

// --- Create network game list ----------------------------------

	networkgame_list = new SoEGUIList(Screen, ButtonSelSound, 205, 175, 531);

	if (networkgame_list == NULL)
		return false;

// --- Captions --------------------------------------------------
	
	/* Single Player Mode */
	Captions[0] = new SoEGUIImage();

	if (Captions[0] == NULL)
		return false;

	if (!Captions[0]->Create(Screen, respack_interface("data\\gfx\\spm.bmp"), SHOW_NORMAL, 170,50))
		return false;

	/* Multi Player Mode */
	Captions[1] = new SoEGUIImage();

	if (Captions[1] == NULL)
		return false;

	if (!Captions[1]->Create(Screen, respack_interface("data\\gfx\\mpm.bmp"), SHOW_NORMAL, 170,50))
		return false;

	/* Create Network Game */
	Captions[2] = new SoEGUIImage();

	if (Captions[2] == NULL)
		return false;

	if (!Captions[2]->Create(Screen, respack_interface("data\\gfx\\cng.bmp"), SHOW_NORMAL, 149, 50))
		return false;

	/* Join Network Game */
	Captions[3] = new SoEGUIImage();

	if (Captions[3] == NULL)
		return false;

	if (!Captions[3]->Create(Screen, respack_interface("data\\gfx\\jng.bmp"), SHOW_NORMAL, 149, 50))
		return false;

	/* Init SoE Game Engine Splash */

	for (i = 0; i != 13; i++)
	{
		InitEngineSplash[i] = new SoEGUIImage();
		
		if (InitEngineSplash[i] == NULL)
			return false;
	}

	char temp[256];
	for (i = 0; i < 13; i++)
	{
			sprintf(temp, "data\\gfx\\isoee%d.bmp", i);
			LOG("LOAD: "); LOG(temp); LOG("\n");

			if (!InitEngineSplash[i]->Create(Screen, respack_interface(temp), SHOW_NORMAL, 179, 272))
				return false;
	};

	return true;
}

bool CreateFullScreenWindow(void)
{
	LOG("Screen = new SoEScreen()");
	Screen = new SoEScreen();
	
	if (Screen == NULL) 
	{	
		LOG(LOG_FAILED);
		return false;   
	}
		
	LOG(LOG_OK);     
	
	LOG("Screen->CreateFullScreen(...)");
	if (FAILED(Screen->CreateFullScreen(g_hWnd, 800, 600, 32))) 
	{
		LOG(LOG_FAILED);
		return false;
	}
	LOG(LOG_OK);

	return true;
}

bool CreateAndLoadSound(void)
{
	LOG("Sound = new Sound()");
	if ((Sound = new SoESound()) == NULL) 
	{
		LOG(LOG_FAILED);
		return false;
	}

	LOG(LOG_OK);

	LOG("Sound->Create(...)\n");
	if (FAILED(Sound->Create((void*)g_hWnd, FALSE))) 
	{
		LOG(LOG_FAILED);
		return false;
	}
	LOG(LOG_OK);

	LOG("Initialize Background Music\n");
	if ((BackgroundMusic = new SoESoundBuffer()) == NULL)
	{
		LOG(LOG_FAILED);
		return false;
	}

	if (FAILED(BackgroundMusic->Load(Sound, respack_interface("data\\snd\\music.wav"))))
	{
		LOG(LOG_FAILED);	
		return false;
	}
	LOG(LOG_OK);

	if ((ButtonSelSound = new SoESoundBuffer()) == NULL)
		return false;

	if (FAILED(ButtonSelSound->Load(Sound, respack_interface("data\\snd\\select.wav"))))
		return false;

	return true;
}

void RestoreScreen(void)
{
	int i;

	if (Screen == NULL) 
		return;
	
	//Screen->GetBack()->Restore();
	//Screen->GetFront()->Restore();
	//Screen->RestoreAllSurfaces();

	SAFEDELETE(Captions[0]);
	SAFEDELETE(player_list);
	SAFEDELETE(level_list);
	SAFEDELETE(net_player_list);
	SAFEDELETE(net_level_list);
	SAFEDELETE(Title);
	SAFEDELETE(Copyright);
	SAFEDELETE(NetworkGameListFrame);
	SAFEDELETE(networkgame_list);
	SAFEDELETE(level_img);
	SAFEDELETE(player_img);
	SAFEDELETE(net_level_img);
	SAFEDELETE(net_player_img);

	for (i = 0; i != 13; i++)
		SAFEDELETE(InitEngineSplash[i]);

	for (i = 0; i != 13; i++)
		SAFEDELETE(Buttons[i]);
	for (i = 0; i != 2; i++)
		SAFEDELETE(MinimizeExitBtns[i]);

	CreateGUIComponents();

//	SelectLevel();
//	SelectPlayer();
//	SelectNetLevel();
//	SelectNetPlayer();
}

const char* respack_interface(const char* file_name)
{
	const char* target_file;
	static char archive_tmp_file[MAX_PATH];

	target_file = strrchr(file_name, '\\');

	if (target_file == NULL)
		target_file = file_name;
	else
		target_file++;	

	sprintf(archive_tmp_file, "%s%s", SOE_TEMP_DIR, target_file);

	if (extract_file("data.pak", file_name, archive_tmp_file) == -1)
	{	
		char buf[256];
		sprintf(buf, "Respack: could not extract file %s to %s\n", file_name, archive_tmp_file);
		LOG(buf);
		return NULL;
	}
	
	return (const char*)archive_tmp_file;
}

bool RemoveLocalHostEntry(void)
{
	FILE* pFile = NULL;
	int i;

	const char* file_name = "net_game_list.dat";
	pFile = fopen(file_name, "r");

	/* file must exists */
	if (pFile == NULL) return false;

	char data[256][1024];
	char c; int Rows;

	for (i = 0; i != 256; i++)
	{
		for (int j = 0; j != 1024; j++)
		{
			c = fgetc(pFile);

			if (c == EOF) { data[i][j] = 0; break; }

			if (c == '\n') { data[i][j] = 0; break; }

			data[i][j] = c;
		}

		if (c == EOF) { Rows = i; break; }
	}

	fclose(pFile);

	/* only one row ?*/
	if ((Rows + 1) == 1)
	{
		DeleteFile("net_game_list.dat");
		return true;
	}


	/* more than one row, pick up the row and delete it */

	FILE* pNewFile = NULL;
	pNewFile = fopen("net_game_list.tmp", "w");

	if (pNewFile == NULL) return false;

	char host[256]; bool bhost = false;

	for (i = 0; i != (Rows + 1); i++)
	{
		bhost = false;

		/* copy host string */
		for (int k = 0; k != 256; k++)
		{
			/* dont skip entry number */
			if (data[i][k] != ';') fputc(data[i][k], pNewFile);

			if (data[i][k] == ';')
			{
				k++;

				for (int m = 0; ; k++, m++)
				{
					if (data[i][k] == ';') { host[m] = 0; bhost = true; break; }

					host[m] = data[i][k];
				}
			}

			if (bhost) break;
		}

		if (strcmp((const char*)COMPUTERNAME, (const char*)host) == 0) continue;

		for (int j = 0; j != 1024; j++)
		{
			if (data[i][j] == 0) 
			{ 
				if (i != Rows) 
					fputc('\n', pNewFile); 
				
				break;
			}

			fputc(data[i][j], pNewFile);	
		}
	}
	
	fclose(pNewFile);

	/* update entry number */

	pFile = NULL;
	pFile = fopen("net_game_list.tmp", "r");
	if (pFile == NULL) return false;

	int nEntries;
	fscanf(pFile, "%d", &nEntries);
	nEntries--;


	pNewFile = NULL;
	pNewFile = fopen("net_game_list.dat", "w");
	if (pNewFile == NULL) return false;

	char data2[1024];

	for (i = 0; ; i++)
	{
		c = fgetc(pFile);

		if (c == EOF) break;

		data2[i] = c;
	}

	data2[i] = 0;
	fprintf(pNewFile,"%d%s", nEntries, data2);

	fclose(pFile); fclose(pNewFile);
	DeleteFile("net_game_list.tmp");

	return true;	
}

//--- DoFrame ----------------------------------------------------
//
// Function: Performs drawing of the current frame 
//
//--------------------------------------------------------------------

void DoFrame() 
{
	int i;

    if (!bActive)
		return;

	if (Screen == NULL)
		return;

	Screen->GetBack()->Fill(0); 

	if (bNoVideo == false  && !bCorrupted)
	{
		if (Intro != NULL)
		{
			if (!Intro->Update())
			{
				bNoVideo = true;

				Sleep(2000); ShowCursor(TRUE);
				
				/* video finished, play sound */
				if (!bCorrupted)
				{
					if (FAILED(BackgroundMusic->Play(DSBPLAY_LOOPING)))
					{
						SOE_ERROR("play sound buffer");
						return;
					}
				}
			}
		}

		Screen->Flip();
		return;
	}

	/* MENU_INITGAMEENGINE */
	if (current_menu == MENU_INITGAMEENGINE)
	{
		static int nPeriod;

		InitEngineSplash[InitEngineSplashCount]->Update(); ++nPeriod; 
		if (nPeriod == 3) {++InitEngineSplashCount; nPeriod = 0;}

		if (InitEngineSplashCount == 13) InitEngineSplashCount = 0;
	}

	/* MENU_SHUTDOWNGAMEENGINE */
	if (current_menu == MENU_SHUTDOWNGAMEENGINE)
	{
		static int nPeriod;

		ShutdownEngineSplash[ShutdownEngineSplashCount]->Update(); ++nPeriod; 
		if (nPeriod == 3) {++ShutdownEngineSplashCount; nPeriod = 0;}

		if (ShutdownEngineSplashCount == 13) ShutdownEngineSplashCount = 0;
	}

	/* MENU_JOINNETWORKGAME */
	if (current_menu == MENU_JOINNETWORKGAME)
	{
		/* Join Network Game */
		if (Captions[3] != NULL)
			Captions[3]->Update();

		for (int i = 0; i != 2; i++)
		{
			if (MinimizeExitBtns[i] == NULL)
				continue;

			MinimizeExitBtns[i]->Update(MousePos.x, MousePos.y);
		}

		/* Start Button - join network game */
		if (Buttons[12] != NULL)
			Buttons[12]->Update(MousePos.x, MousePos.y);

		/* Net player list */
		if (net_player_list != NULL)
			net_player_list->Update(MousePos.x, MousePos.y);

		/* Net player list picture */
		if (net_player_list->GetSelected() != NULL)
			if (net_player_img != NULL)
				if (net_player_img->IsActive())
					net_player_img->Update();

		/* BackToMultiPlayerModeMenu Button */
		if (Buttons[9] != NULL)
			Buttons[9]->Update(MousePos.x, MousePos.y);
	}

	/* MENU_CREATENETWORKGAME */
	if (current_menu == MENU_CREATENETWORKGAME)
	{
		/* Create Network Game */
		if (Captions[2] != NULL)
			Captions[2]->Update();

		for (int i = 0; i != 2; i++)
		{
			if (MinimizeExitBtns[i] == NULL)
				continue;

			MinimizeExitBtns[i]->Update(MousePos.x, MousePos.y);
		}

		/* Net level list */
		if (net_level_list != NULL)
			net_level_list->Update(MousePos.x, MousePos.y);

	    /* Net level list picture */
		if (net_level_list->GetSelected() != NULL)
			if (net_level_img != NULL)
				if (net_level_img->IsActive())
					net_level_img->Update();

		/* Net player list */
		if (net_player_list != NULL)
			net_player_list->Update(MousePos.x, MousePos.y);

		/* Net player list picture */
		if (net_player_list->GetSelected() != NULL)
			if (net_player_img != NULL)
				if (net_player_img->IsActive())
					net_player_img->Update();

		/* StartMultiSession Button */
		if (Buttons[10] != NULL)
			Buttons[10]->Update(MousePos.x, MousePos.y);

		/* BackToMultiPlayerModeMenu Button */
		if (Buttons[9] != NULL)
			Buttons[9]->Update(MousePos.x, MousePos.y);
	}

	/* MENU_MULTIPLAYER */
	if (current_menu == MENU_MULTIPLAYER)
	{
		/* Single Player Mode */
		if (Captions[1] != NULL)
			Captions[1]->Update();
			
		for (int i = 0; i != 2; i++)
		{
			if (MinimizeExitBtns[i] == NULL)
				continue;

			MinimizeExitBtns[i]->Update(MousePos.x, MousePos.y);
		}

		/* Create */
		if (Buttons[7] != NULL)
			Buttons[7]->Update(MousePos.x, MousePos.y);

		/* Join */
		if (networkgame_list->GetSelected() != NULL)
			if (Buttons[8] != NULL)
				Buttons[8]->Update(MousePos.x, MousePos.y);


		/* Update */
		if (Buttons[11] != NULL)
			Buttons[11]->Update(MousePos.x, MousePos.y);

		/* BackToMainMenu Button */
		Buttons[5]->Update(MousePos.x, MousePos.y);

		if (NetworkGameListFrame != NULL)
			NetworkGameListFrame->Update();

		/* Network game list */
		if (networkgame_list != NULL)
			networkgame_list->Update(MousePos.x, MousePos.y);
	}

	/* MENU_MAIN */
    if (current_menu == MENU_MAIN)
	{   
		RECT rc;
		rc.left = 0; rc.top = 0; 
		rc.right = 800; rc.bottom = 138;
      
		if (Title != NULL)
			Title->Update();

		if (Copyright != NULL)
			Copyright->Update();

		for (i = 0; i != 5; i++)
		{
			if (Buttons[i] == NULL)
				continue;
			
			Buttons[i]->Update(MousePos.x, MousePos.y);
		}

		for (i = 0; i != 2; i++)
		{
			if (MinimizeExitBtns[i] == NULL)
				continue;

			MinimizeExitBtns[i]->Update(MousePos.x, MousePos.y);
		}	
	}

	/* MENU_SINGLEPLAYER */
	if (current_menu == MENU_SINGLEPLAYER)
	{  
		/* Single Player Mode */
		if (Captions[0] != NULL)
			Captions[0]->Update();
			
		for (int i = 0; i != 2; i++)
		{
			if (MinimizeExitBtns[i] == NULL)
				continue;

			MinimizeExitBtns[i]->Update(MousePos.x, MousePos.y);
		}	

		/* BackToMainMenu Button */
		Buttons[5]->Update(MousePos.x, MousePos.y);

		/* StartSingleSession Button */
		Buttons[6]->Update(MousePos.x, MousePos.y);

		if (level_list != NULL)
			level_list->Update(MousePos.x, MousePos.y);

		if (level_list->GetSelected() != NULL)
			if (level_img != NULL)
				if (level_img->IsActive())
					level_img->Update();

		if (player_list != NULL)
			player_list->Update(MousePos.x, MousePos.y);

		if (player_list->GetSelected() != NULL)
			if (player_img != NULL)
				if (player_img->IsActive())
					player_img->Update();
	}

	Screen->Flip(); 
}

void BackToMainMenu(void)
{
	current_menu = MENU_MAIN;
	LOG("Set current menu = MENU_MAIN\n");	
}

void BackToMultiPlayerModeMenu(void)
{
	current_menu = MENU_MULTIPLAYER;
	LOG("Set current menu = MENU_MULTIPLAYER\n");
}

void StartMultiSession(void)
{
	char buf[256], cmd[256]; 

	LOG("StartMultiSession()\n");

	if (net_level_list == NULL || net_level_list->GetSelected() == NULL) 
		return;
	
	respack_interface("data\\sys\\engine.exe");
	respack_interface("data\\sys\\level1.c4s");
	respack_interface("data\\sys\\objects.c4d");
	respack_interface("data\\sys\\graphics.c4g");
	respack_interface("data\\sys\\sound.c4g");
	respack_interface("data\\sys\\music.c4g");
	respack_interface("data\\sys\\material.c4g");
	
	/* copy all player files */
	for (int i = 0; i != net_player_list->GetListCount(); i++)
	{
		sprintf(buf,"data\\sys\\%s.c4p",player_list->GetListEntry(i)->GetData());
		respack_interface(buf);
	}
	
	if (!SaveRestoreData())
		return;
		
	sprintf(cmd,"%sengine.exe /Lobby %s.c4s objects.c4d %s.c4p /Fullscreen", SOE_TEMP_DIR,
		net_level_list->GetSelected()->GetData(), net_player_list->GetSelected()->GetData());

	InitSoEGameEngine();

	bActive = false;
	Sleep(2000);
	Exit();

	SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","Active",1);

	/* Start SoE Game Engine */
	WinExec((const char*)cmd, SW_NORMAL);
	RunNetworkGameServer();
}

void StartSingleSession(void)
{
	char buf[256],cmd[256];

	if (level_list == NULL || level_list->GetSelected() == NULL)
		return;
	
	respack_interface("data\\sys\\engine.exe");
	respack_interface("data\\sys\\level1.c4s");
	respack_interface("data\\sys\\objects.c4d");
	respack_interface("data\\sys\\graphics.c4g");
	respack_interface("data\\sys\\sound.c4g");
	respack_interface("data\\sys\\music.c4g");
	respack_interface("data\\sys\\material.c4g");

	sprintf(buf,"data\\sys\\%s.c4p",player_list->GetSelected()->GetData());

	respack_interface(buf);

	if (!SaveRestoreData())
		return;

	sprintf(cmd,"%sengine.exe %s.c4s objects.c4d %s.c4p /Fullscreen", SOE_TEMP_DIR,
		level_list->GetSelected()->GetData(), player_list->GetSelected()->GetData());

	LOG("Init SoEGameEngine\n");
	InitSoEGameEngine();

	bActive = false;
	Sleep(2000);
	Exit();

	SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","Active",0);
	
    /* Start SoE Game Engine */
	LOG("Start SoEGameEngine\n");
	WinExec((const char*)cmd, SW_NORMAL);	
}

void SelectPlayer(void)
{
	if (player_list == NULL || current_menu != MENU_SINGLEPLAYER)
		return;

	SAFEDELETE(player_img);
	player_img = new SoEGUIImage();

	if (player_img == NULL) return;

	char buf[256];
	sprintf(buf,"data\\gfx\\plr_pics\\%s.bmp",player_list->GetSelected()->GetData());
	
	if (!player_img->Create(Screen, respack_interface(buf), SHOW_NORMAL, 400, 200))
		return;
}

void SelectNetPlayer(void)
{
	if (net_player_list == NULL)
		return;

	if ((current_menu == MENU_CREATENETWORKGAME) || (current_menu == MENU_JOINNETWORKGAME))
		return;

	SAFEDELETE(net_player_img);
	net_player_img = new SoEGUIImage();

	if (net_player_img == NULL) return;

	char buf[256];
	sprintf(buf,"data\\gfx\\plr_pics\\%s.bmp",net_player_list->GetSelected()->GetData());
	
	if (!net_player_img->Create(Screen, respack_interface(buf), SHOW_NORMAL, 400, 200))
		return;
}

void SelectLevel(void)
{
	if (level_list == NULL)
		return;

	if (current_menu != MENU_SINGLEPLAYER)
		return;

	SAFEDELETE(level_img);
	level_img = new SoEGUIImage();

	if (level_img == NULL) return;

	char buf[256];
	sprintf(buf,"data\\gfx\\%s.bmp",level_list->GetSelected()->GetData());
	
	if (!level_img->Create(Screen, respack_interface(buf), SHOW_NORMAL, 400, 350))
		return;
}

void SelectNetLevel(void)
{
	if (net_level_list == NULL)
		return;

	if (current_menu != MENU_CREATENETWORKGAME)
		return;

	SAFEDELETE(net_level_img);
	net_level_img = new SoEGUIImage();

	if (net_level_img == NULL) return;

	char buf[256];
	sprintf(buf,"data\\gfx\\%s.bmp",net_level_list->GetSelected()->GetData());
	
	if (!net_level_img->Create(Screen, respack_interface(buf), SHOW_NORMAL, 400, 350))
		return;
}

void SinglePlayerMode(void)
{
	current_menu = MENU_SINGLEPLAYER;
	LOG("Set current menu = MENU_SINGLEPLAYER\n");

	SelectLevel();
	SelectPlayer();
}

void MultiPlayerMode(void)
{
	current_menu = MENU_MULTIPLAYER;
	LOG("Set current menu = MENU_MULTIPLAYER\n");
}

void CreateNetworkGame(void)
{
	current_menu = MENU_CREATENETWORKGAME;
	LOG("Set current menu = MENU_CREATENETWORKGAME\n");
	LOG("CreateNetworkGame::SelectNetLevel()\n");
	SelectNetLevel();
	LOG("CreateNetworkGame::SelectNetPlayer()\n");
	SelectNetPlayer();
}

void JoinNetworkGame(void)
{
	current_menu = MENU_JOINNETWORKGAME;
	LOG("Set current menu = MENU_JOINNETWORKGAME\n");
	LOG("JoinNetworkGame::SelectNetPlayer()\n");
	SelectNetPlayer();
}

/* Exit and deinstall */
void Exit(void)
{
	if (g_hWnd != NULL)
			DestroyWindow(g_hWnd);
	g_hWnd = NULL;

	
	LOG("PostQuitMessage(...)\n");
	PostQuitMessage(0);
}

void Minimize(void)
{
	ShowWindow(g_hWnd, SW_MINIMIZE);
	UpdateWindow(g_hWnd);
}

bool SaveRestoreData(void)
{
	SoEFile dat;
	char buf[MAX_PATH];

	sprintf(buf, "%srestore.dat", SOE_TEMP_DIR);
	
	/* Create restore data */ 
	restore_data.st_title = SHOW_NORMAL;
	restore_data.mt_menu  = current_menu;
	
	restore_data.level_list_sel  = level_list->GetIndex(level_list->GetSelected());
	restore_data.player_list_sel = player_list->GetIndex(player_list->GetSelected());
	
	restore_data.net_level_list_sel  = net_level_list->GetIndex(net_level_list->GetSelected());
	restore_data.net_player_list_sel = net_player_list->GetIndex(net_player_list->GetSelected());
	restore_data.dwBackMusicPos = BackgroundMusic->GetCurrentPlayPosition();

	if (!dat.OpenWrite(buf))
		return false;

	if (dat.Write((const void*)&restore_data, sizeof(restore_data)) == 0)
		return false;

	return true;
}

bool LoadRestoreData(void)
{
	SoEFile dat;
	char buf[MAX_PATH];

	sprintf(buf, "%srestore.dat", SOE_TEMP_DIR);

	if (!dat.OpenRead(buf))
	{
		sprintf(buf, "Could not load %srestore.dat\n", SOE_TEMP_DIR);
		LOG(buf);
		return false;
	}

	if (dat.Read((void*)&restore_data, sizeof(restore_data)) == 0)
		return false;

	current_menu = restore_data.mt_menu;

	return true;
}

void InitSoEGameEngine(void)
{
	ShowCursor(FALSE);
	current_menu = MENU_INITGAMEENGINE;
}

void DeInitSoEGameEngine(void)
{
	/* Restore menu system */
	switch(current_menu)
	{
	case MENU_SINGLEPLAYER:
		SinglePlayerMode();
	break;

	case MENU_MULTIPLAYER:
		MultiPlayerMode();
	break;

	case MENU_CREATENETWORKGAME:
		CreateNetworkGame();
	break;
	
	case MENU_MAIN:
		BackToMainMenu();
	break;

	default:
	break;
	}
}

//--- WinProc --------------------------------------------------------
//
// Function: Handles application messages
//
//--------------------------------------------------------------------

static long PASCAL WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message)
	{
		case WM_COMMAND:
			
		break;

		case WM_KEYDOWN:          
		{
			switch(wParam) 
			{          
				case VK_ESCAPE:   		
					if (current_menu == MENU_MAIN)
						SendMessage(hWnd, WM_CLOSE, 0, 0); 
					else
					{
						if (current_menu == MENU_CREATENETWORKGAME)
							BackToMultiPlayerModeMenu();
						else
							BackToMainMenu();
					}
				break; 

				case VK_F1:
						
				break;
			}
		}
		return 0;

		case WM_ACTIVATEAPP:
			bActive = wParam;

			if (bActive && bWndMinimized)
			{
				bWndMinimized = false;
			
				ShowWindow(hWnd, SW_RESTORE);
				UpdateWindow(hWnd);
				
				RestoreScreen();
			}
		break;

		case WM_CLOSE: 
		case WM_DESTROY: 
		{
		
		}

		case WM_MOUSEMOVE:
		{
			MousePos.x = LOWORD(lParam);
			MousePos.y = HIWORD(lParam);
	
			break;
		}

		case WM_LBUTTONDOWN:
		{
			MousePos.x = LOWORD(lParam);
			MousePos.y = HIWORD(lParam);

			if (current_menu == MENU_MAIN)
			{
				/* Single Player Mode */
				if (Buttons[0] != NULL && !bCorrupted)								
					Buttons[0]->DoClick(MousePos.x, MousePos.y);

				/* Multi Player Mode */
				if (Buttons[1] != NULL && !bCorrupted)
					Buttons[1]->DoClick(MousePos.x, MousePos.y);
						
				/* Exit */
				if (Buttons[4] != NULL)	
					Buttons[4]->DoClick(MousePos.x, MousePos.y);
					
			} /* MENU_MAIN */

			if (current_menu == MENU_SINGLEPLAYER)
			{

				/* BackToMainMenu */
				if (Buttons[5] != NULL)
					Buttons[5]->DoClick(MousePos.x, MousePos.y);
						
				/* StartSingleSession */
				if (Buttons[6] != NULL)	
					Buttons[6]->DoClick(MousePos.x, MousePos.y);
						
				/* Level list */
				if (level_list != NULL)
					level_list->DoClick(MousePos.x, MousePos.y);

				/* Player list */
				if (player_list != NULL)
					player_list->DoClick(MousePos.x, MousePos.y);

			} /* MENU_SINGLEPLAYER */

			if (current_menu == MENU_MULTIPLAYER)
			{
				/* BackToMainMenu */
				if (Buttons[5] != NULL)
					Buttons[5]->DoClick(MousePos.x, MousePos.y);

				/* Create network game */
				if (Buttons[7] != NULL)
					Buttons[7]->DoClick(MousePos.x, MousePos.y);

				/* Join network game */
				if (networkgame_list->GetSelected() != NULL)
					if (Buttons[8] != NULL)
						Buttons[8]->DoClick(MousePos.x, MousePos.y);

				/* Update network game list */
				if (Buttons[11] != NULL)
					Buttons[11]->DoClick(MousePos.x, MousePos.y);

				/* Network game list */
				if (networkgame_list != NULL)
					networkgame_list->DoClick(MousePos.x, MousePos.y);
			}

			if (current_menu == MENU_JOINNETWORKGAME)
			{
				/* BackToMultiPlayerModeMenu */
				if (Buttons[9] != NULL)
					Buttons[9]->DoClick(MousePos.x, MousePos.y);

				/* Net player list */
				if (net_player_list != NULL)
					net_player_list->DoClick(MousePos.x, MousePos.y);

				/* Start Button - join network game */
				if (Buttons[12] != NULL)
					Buttons[12]->DoClick(MousePos.x, MousePos.y);
			}

			if (current_menu == MENU_CREATENETWORKGAME)
			{
				/* BackToMultiPlayerModeMenu */
				if (Buttons[9] != NULL)
					Buttons[9]->DoClick(MousePos.x, MousePos.y);

				/* Start MultiSession Button */
				if (Buttons[10] != NULL)
					Buttons[10]->DoClick(MousePos.x, MousePos.y);

				/* Net level list */
				if (net_level_list != NULL)
					net_level_list->DoClick(MousePos.x, MousePos.y);

				/* Net player list */
				if (net_player_list != NULL)
					net_player_list->DoClick(MousePos.x, MousePos.y);

			}

			/* Minimize */
			if (MinimizeExitBtns[0] != NULL)
				MinimizeExitBtns[0]->DoClick(MousePos.x, MousePos.y);
				
			/* Exit */
			if (MinimizeExitBtns[1] != NULL)
				MinimizeExitBtns[1]->DoClick(MousePos.x, MousePos.y);	
		}
		break;

		case WM_RBUTTONDOWN:
		{
			SaveRestoreData();
		}
		break;

		case WM_SIZE:
		//	if (wParam == SIZE_MAXIMIZED)
		//	{
		//		LOG("SoE Window : Action = SIZE_MAXIMIZED\n");
		//		ShowWindow(hWnd, SW_RESTORE);
		//		UpdateWindow(hWnd);
		//		bActive = true;
		//	}

			if (wParam == SIZE_MINIMIZED)
			{
				LOG("SoE Window : Action = SIZE_MINIMIZED\n");
				bWndMinimized = true;
				bActive = false;
			}
		break;
				
		return 0;

		default: 
			return DefWindowProc(hWnd, message, wParam, lParam); 	
    }

	return 0;
}

//--- InitApp --------------------------------------------------------
//
// Function: Creates the window
// 
//--------------------------------------------------------------------

static BOOL InitApp(int nCmdShow) 
{ 
	WNDCLASS WndClass;                                 
	WndClass.style = CS_HREDRAW | CS_VREDRAW;          
		                                               
	WndClass.lpfnWndProc = WinProc;                    
	WndClass.cbClsExtra = 0; 
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = g_hInst;                      
	WndClass.hIcon = NULL;						       
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);       
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;                         
	WndClass.lpszClassName = szClassName;              
	RegisterClass(&WndClass);                          

	LOG("CreateWindowEx(...)");
	g_hWnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		szClassName, 
		szAppName, 
		WS_POPUP,  
		0, 
		0, 
		GetSystemMetrics(SM_CXFULLSCREEN),  
		GetSystemMetrics(SM_CYFULLSCREEN),  
		NULL, 
		NULL, 
		g_hInst,
		NULL); 


	if (!g_hWnd) { LOG(LOG_FAILED); return FALSE; }
	LOG(LOG_OK);
	
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);        

	return TRUE;
} 

bool RunNetworkGameServer(void)
{
	FILE* pFile = NULL;

	bool bFileExists = true;

	/* check if file exists */
	pFile = fopen("net_game_list.dat", "r");
	if (pFile == NULL)
		bFileExists = false;
	else
		fclose(pFile);

	LOG("Open net_game_list.dat to append data");

	pFile = fopen("net_game_list.dat", "a");

	if (pFile == NULL) { LOG(LOG_FAILED); return false; }

	LOG(LOG_OK);

	char data[256];

	if (!bFileExists)
		sprintf(data, "1;%s;%s on %s;", COMPUTERNAME, net_level_list->GetSelected()->GetText(), COMPUTERNAME); 
	else
		sprintf(data, "\n;%s;%s on %s;", COMPUTERNAME, net_level_list->GetSelected()->GetText(), COMPUTERNAME); 
	
	fprintf(pFile, (const char*)data);
	fclose(pFile);

	if (bFileExists)
	{
		LOG("Open net_game_list.dat to update data");

		pFile = fopen("net_game_list.dat", "r");

		if (pFile == NULL) { LOG(LOG_FAILED); return false; }

		LOG(LOG_OK);

		int nEntries;
		fscanf(pFile, "%d", &nEntries);
		nEntries++;
	
		FILE* pNewFile = fopen("net_game_list.dat", "w");

		if (pNewFile == NULL) return false;

		char data2[1024];

		char c;
		int i;

		for (i = 0; ; i++)
		{
			c = fgetc(pFile);

			if (c == EOF) break;

			data2[i] = c;
		}

		data2[i] = 0;
		fprintf(pNewFile,"%d%s", nEntries, data2);
	
		fclose(pFile);
		fclose(pNewFile);
	}

	return true;
}

void RunNetworkGameClient(void)
{
	SoEGUIListEntry* pEntry = NULL;
	char buf[256], cmd[256]; 

	if (networkgame_list != NULL)
		pEntry = networkgame_list->GetSelected();
	
	respack_interface("data\\sys\\engine.exe");
	respack_interface("data\\sys\\level1.c4s");
	respack_interface("data\\sys\\objects.c4d");
	respack_interface("data\\sys\\graphics.c4g");
	respack_interface("data\\sys\\sound.c4g");
	respack_interface("data\\sys\\music.c4g");
	respack_interface("data\\sys\\material.c4g");
	
	sprintf(buf,"data\\sys\\%s.c4p",net_player_list->GetSelected()->GetData());
	respack_interface(buf);
	
	if (!SaveRestoreData())
		return;
	
	sprintf(cmd,"%sengine.exe /Join:%s %s.c4p /Fullscreen", SOE_TEMP_DIR,
			pEntry->GetData(), net_player_list->GetSelected()->GetData());

//	sprintf(cmd,"%s%s\\soe.exe /Join:%s %s%s\\objects.c4d %s.c4p /Fullscreen",SOE_TEMP_DIR,COMPUTERNAME,
//			pEntry->GetData(),SOE_TEMP_DIR,COMPUTERNAME,net_player_list->GetSelected()->GetData());

	InitSoEGameEngine();

	bActive = false;
	Sleep(2000);
	Exit();

	SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","Active",1);

	/* Start SoE Game Engine */
	WinExec((const char*)cmd, SW_NORMAL);
}

void UpdateNetworkGameList(void)
{
	char file_name[256];

	strcpy(file_name, SOE_TEMP_DIR);
	strcpy(file_name, strcat(file_name, "net_game_list.dat"));

	SAFEDELETE(networkgame_list)

	networkgame_list = new SoEGUIList(Screen, ButtonSelSound, 205, 175, 531);

	if (networkgame_list == NULL) return;

	LOG("Update network game list");
	if (!networkgame_list->CreateFromListFile((const char*)file_name))
	{
		LOG(LOG_FAILED);
		return;
	}
	LOG(LOG_OK);
}

BOOL InstallSoE(void)
{
	BOOL ret;

	TCHAR pGamePath[MAX_PATH];
	if (GetModuleFileName(NULL, pGamePath, MAX_PATH) == 0)
		return FALSE;

	/* General */
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "Language", "DE");
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\General","DebugMode",1);
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "RXFontName", "Orbit-B");
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\General","RXFontSize",10);

	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "FEFontName", "Orbit-B");
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\General","FEFontSize",9);
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "LogPath", ".\\");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "PlayerPath", ".\\");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General\\", "GamePath", pGamePath);

	
	/* Developer */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Developer","Active",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Developer","SendDefReload",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Developer","AutoEditScan",1);

	/* Graphics */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","Resolution",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","SplitscreenDividers",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ShowPlayerInfoAlways",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ShowPortraits",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ShowCommands",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ShowCommandKeys",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ShowStartupMessages",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","ColorAnimation",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","SmokeLevel",200);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","DDrawAccel",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","VerboseObjectLoading",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Graphics","VideoModule",0);

	/* Sound */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Sound","RXSound",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Sound","RXSoundLoops",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Sound","RXMusic",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Sound","FEMusic",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Sound","FESamples",1);

	/* Gamepad */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","MaxX",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","MinX",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","MaxY",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","MinY",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button1",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button2",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button3",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button4",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button5",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button6",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button7",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button8",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button9",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button10",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button11",4294967295);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Gamepad","Button12",4294967295);

	/* Console */
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Console", "Viewport1", "Maximized");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Console", "Property", "440,362");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Console", "Viewport0", "-4,-4,808,580");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Console", "Component", "127,182");
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Console", "Main", "0,0");

	/* Controls */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key1",81);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key2",87);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key3",69);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key4",65);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key5",83);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key6",68);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key7",89);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key8",88);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key9",67);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key10",226);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key11",86);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd1Key12",70);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key1",103);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key2",104);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key3",151);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key4",100);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key5",101);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key6",102);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key7",97);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key8",98);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key9",99);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key10",96);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key11",110);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd2Key12",107);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key1",73);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key2",79);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key3",80);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key4",75);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key5",76);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key6",192);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key7",188);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key8",190);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key9",189);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key10",77);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key11",222);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd3Key12",186);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key1",49);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key2",50);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key3",51);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key4",52);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key5",53);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key6",54);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key7",55);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key8",56);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key9",57);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Controls","Kbd4Key10",48);

	/* Network */
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","Active",0);
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Network", "LocalName", COMPUTERNAME);
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Network", "LocalAddress", COMPUTERNAME);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","ControlRate",1);
	ret = SetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\Network", "WorkPath", "Network\\");
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","Lobby",1);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","NoRuntimeJoin",0);
    ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","NoReferenceRequest",0);
	ret = SetRegistryDWord(HKEY_CURRENT_USER,"Software\\GreenByte Studios\\Source Of Evil\\Network","AsynchronousControl",0);

	return ret;
}

bool ParseCommandLine(void)
{
	char buf[256]; strcpy(buf, CmdLine);

	for (int i = 0; buf[i] != 0; ++i)
	{
		if (buf[i] == '-')
		{
			++i;

			// restore
			if (buf[i] == 'r')
			{
				bNoVideo   = true;
				bRestore   = true;

				continue;
			}
		}
	}

	return true;

}

//--- WinMain --------------------------------------------------------
//
// Function: Inital function called by windows
//
//--------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) 
{	 
	CmdLine = lpCmdLine;
	char szLog[255];
	DWORD dwRetVal = 0;
	g_hInst = hInstance;

	sprintf(szLog, "\n\nSoE Log System: %s %s\n\n", __DATE__, __TIME__);
	LOG(szLog);

	//  Gets the temp path env string (no guarantee it's a valid path).
    dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
                           SOE_TEMP_DIR); // buffer for path 
    if (dwRetVal > MAX_PATH || (dwRetVal == 0))
    {
        LOG("GetTempPath failed\n");
        return false;
    }

	strcpy(SOE_TEMP_DIR, strcat(SOE_TEMP_DIR, "SOETEMP\\"));
	CreateDirectory(SOE_TEMP_DIR, NULL);
	LOG(SOE_TEMP_DIR);
	
	LOG("CMDLINE: "); LOG(lpCmdLine); LOG("\n");

	// parse command line eg. -s server
	LOG("ParseCommandLine()\n");
	if (!ParseCommandLine()) 
	{
		LOG("ParseCommandLine()...FAILED\n");
		return -1;
	}
	LOG("ParseCommandLine()...OK\n");

	// get name of local machine
	GetRegistryString("Software\\Microsoft\\Windows Media\\WMSDK\\General\\", "ComputerName", COMPUTERNAME, sizeof(COMPUTERNAME));
	LOG("Computer name: "); LOG(COMPUTERNAME); LOG("\n");

	// install registry information
	if (!bRestore)
	{
		LOG("InstallSoE()\n");
		if (!InstallSoE())
		{
			LOG("InstallSoE()...FAILED\n");
			return -1;
		}
		LOG("InstallSoE()...OK\n");
	}
	else
	{	
		LOG("LoadRestoreData()\n");
		if (!LoadRestoreData())
		{
			LOG("LoadRestoreData()...FAILED\n");
			return -1;
		}
		LOG("LoadRestoreData()...OK\n");
	}

	if (!InitApp(nCmdShow)) return -1; 

#ifdef USE_NO_VIDEO
		bNoVideo = true;
#endif

	if (!Init()) 
	{        
		LOG("Init()...FAILED\nExit Source Of Evil\n");
		PostQuitMessage(0);
		return -1;
	} 

	MSG msg;

	while (1) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
		{
	    if (!GetMessage(&msg, NULL, 0, 0 )) return msg.wParam;
			
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
		else 
		{
			DoFrame();     
		}
	}

	return 0;
}
