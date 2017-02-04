/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*															     */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*															     */
/*****************************************************************/

#include <C4Include.h>

C4Application Application;
C4Engine	  Engine; 
C4Console	  Console;
C4FullScreen  FullScreen;
C4Game		  Game;

//#ifdef C4SHAREWARE
//C4ConfigShareware Config;
//#else
C4Config	      Config;
//#endif

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	/* ONLY FOR DEDICATED SERVER */

	// TODO command line option -dserver
	
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, "SOE_DEDICATED_SERVER");
	DWORD dwWaitResult = WaitForSingleObject(hMutex, 5000L);
	
	if (dwWaitResult == WAIT_TIMEOUT)
		return C4XRV_Failure;

	/* ONLY FOR DEDICATED SERVER */  

	char path[MAX_PATH];
	GetRegistryString("Software\\GreenByte Studios\\Source Of Evil\\General", "GamePath", path, sizeof(path));
	path[strlen(path)-strlen("SoE.exe")] = '\0';

	// Init application
	if (!Application.Init(hInst,nCmdShow,lpszCmdParam))
	{	
		ReleaseMutex(hMutex);
		
		SetCurrentDirectory(path);
		WinExec("SoE.exe -r", SW_NORMAL);

		return C4XRV_Failure;
	}

	// Execute application
	Application.Run();

	SetCurrentDirectory(path);
	WinExec("SoE.exe -r", SW_NORMAL);
	
	// Return exit code
	if (!Game.GameOver) 
	{
		ReleaseMutex(hMutex);
		return C4XRV_Aborted;	
	}

	ReleaseMutex(hMutex);
	return C4XRV_Completed;
}


