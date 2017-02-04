/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Sings up or retrieves game information from a master server */

#include <C4Include.h>

#include <Winsock.h>
#include <StdHttp.h>

C4MasterServerClient::C4MasterServerClient()
{
	Default();
}

C4MasterServerClient::~C4MasterServerClient()
{
	Clear();
}

void C4MasterServerClient::Default()
{
	Terminate=false;
	Reference=false;
	AdvanceReference=false;
	hThread=NULL; idThread=0;
	hWnd=NULL;
	Address[0]=0;
	LocalAddress[0]=0;
	ReferenceFilename[0]=0;
	Directory[0]=0;
	KeepPeriod=20;
	ReferencePeriod=40;
}

void C4MasterServerClient::Clear()
{
	if (hThread)
	{
		// Ask thread to terminate nicely
		Terminate=true;
		DWORD dwExit=STILL_ACTIVE;

		// Wait 3 seconds for thread to close
		for (int cnt=0; cnt<6; cnt++)
		{
			if (!GetExitCodeThread(hThread,&dwExit) || (dwExit!=STILL_ACTIVE)) break;
			Sleep(500);
		}

		// Kill thread
		if (dwExit==STILL_ACTIVE)
			TerminateThread(hThread,999);
	}

	hThread=NULL;
}

BOOL C4MasterServerClient::Init(HWND hwnd, const char *szAddress, const char *szDirectory, int iKeepPeriod, int iReferencePeriod)
{	
	// Store data
	hWnd = hwnd;
	SCopy(szAddress,Address,_MAX_PATH);
	SCopy(szDirectory,Directory,_MAX_PATH);
	KeepPeriod = Max( iKeepPeriod, 60 );
	ReferencePeriod = Max( iReferencePeriod, 120 );

	// Scan master server address/path
	if (SCharCount('/',Address))
	{
		// Split and truncate script host directory
		SCopy(Address+SCharPos('/',Address),Directory);
		Address[SCharPos('/',Address)]=0;
	}

	// Launch thread
	if (!( hThread = CreateThread( NULL, 0, &ThreadFunction, this, 0, &idThread ) ))
	{ 
		Message(IDS_NET_NOTHREAD); return FALSE; 
	}

	// Success
	return TRUE;
}

DWORD WINAPI C4MasterServerClient::ThreadFunction(void *lpPar)
{
	C4MasterServerClient *pClient = (C4MasterServerClient*) lpPar;

	return pClient->Run();
}

void C4MasterServerClient::Message(WORD idMsg, const char *szMsgPar)
{
	PostMessage( hWnd, WM_USER_LOG, idMsg, (LPARAM) szMsgPar );
}

BOOL C4MasterServerClient::Send(const char *szAction, const char *szFilename, BYTE *bpData, int iSize)
{
	// Safety
	if (!Address || !Address[0]) return FALSE;

	// Get address
	char szAddress[C4MaxTitle+1]; 
	if (!SCopyEnclosed(Address,'(',')',szAddress,C4MaxTitle)) SCopy(Address,szAddress);	

	// Init http	
	CStdHttp StdHttp;
	char szAgent[_MAX_PATH+1],szVersion[10];
	sprintf(szAgent,"Source Of Evil/%d.%d%d (Engine)",C4XVer1,C4XVer2,C4XVer3);
	sprintf(szVersion,"%d.%d%d.0",C4XVer1,C4XVer2,C4XVer3);
	StdHttp.Init( szAgent, Directory, "application/x-cpmaster-request", szVersion);

	// Connect
	if (!StdHttp.Connect(szAddress)) return FALSE;

	// Post message 
	char szMessage[1024+1];
	sprintf(szMessage,"version=%d.%d%d.0&action=%s&filename=%s\r\n",C4XVer1,C4XVer2,C4XVer3,szAction,szFilename);
	if (!StdHttp.Post(szMessage,bpData,iSize)) return FALSE;

	// Receive answer
	CStdHttpMessage Msg;
	if (!StdHttp.Receive(Msg)) return FALSE;

	// No success: log server error string
	static char szServerError[1024+1];
	if (!Msg.Success) 
	{ 
		if (Msg.Text) { SCopy(Msg.Data,szServerError,1024); SReplaceChar(szServerError,0x0D,0x00); Message(IDS_NET_SERVERERROR,szServerError); } 
		else Message(IDS_NET_SERVERERROR);

		return FALSE; 
	}

	// Disconnect
	StdHttp.Disconnect();
	return TRUE;
}

BOOL ValidFilenameCharacter(char cChar)
{
	if ((cChar == '_') || (cChar == '-')) return TRUE;
	if (Inside(cChar,'a','z')) return TRUE;
	if (Inside(cChar,'A','Z')) return TRUE;
	if (Inside(cChar,'0','9')) return TRUE;
	if ((cChar == '�') || (cChar == '�') || (cChar == '�')) return TRUE;
	if ((cChar == '�') || (cChar == '�') || (cChar == '�')) return TRUE;
	if (cChar == '�') return TRUE;
	if (cChar == '.') return TRUE;

	return FALSE;
}

int C4MasterServerClient::Run()
{
	int KeepTime=0;
	int ReferenceTime=ReferencePeriod;

	// Log sign up
	Message(IDS_NET_SIGNUP,Address);

	// Determine local address (watch Address Hostname/IP format, getbyhostname only)
	CStdHttp StdHttp;
	if (!StdHttp.GetLocalAddress(Address,LocalAddress))
		SCopy("IP not available",LocalAddress);

	while (!Terminate)
	{
		// Reference
		ReferenceTime++;
		if (ReferenceTime>=ReferencePeriod)
		{
			ReferenceTime=0;
	
			// Request reference file creation by main thread
			SendMessage(hWnd,WM_USER_CREATEREFERENCE,0,(LPARAM)LocalAddress);
	
			// Wait for reference
			while (!Reference)
				if (Terminate) return FALSE;
	
			// Check valid reference filename
			char szValidName[_MAX_PATH+1]; SCopy(ReferenceFilename,szValidName);
			for (char *cpChar = GetFilename(szValidName); *cpChar; cpChar++)
				if (!ValidFilenameCharacter(*cpChar)) *cpChar='_';
	
			if (!SEqual(ReferenceFilename,szValidName))
			{	
				RenameItem(ReferenceFilename, szValidName); SCopy(szValidName,ReferenceFilename);	
			}
			
			// Upload reference file
			CStdFile hFile; BYTE *bpData; int iSize;
			if (!hFile.Load(ReferenceFilename,&bpData,&iSize)) 
			{ 
				Message(IDS_NET_REFFILEERROR); return FALSE; 
			}
			
			if (!Send("upload",GetFilename(ReferenceFilename),bpData,iSize)) 
			{ 
				Message(IDS_NET_NOSIGNUP); delete [] bpData; return FALSE; 
			}
			delete [] bpData;
			EraseItem(ReferenceFilename);
			Reference=false;
		}

		// Reference advance
		if (AdvanceReference) { ReferenceTime=ReferencePeriod; AdvanceReference=false; }

		// Keep
		KeepTime++;
		if (KeepTime>=KeepPeriod)
		{
			KeepTime=0;
		
			// Send keep
			if (!Send("keep",GetFilename(ReferenceFilename))) 
			{ 
				Message(IDS_NET_NOSIGNUP); return FALSE; 
			}
		}

		// Delay
		Sleep(1000);
	}

	// Send delete
	Message(IDS_NET_SIGNOFF);
	Send("delete",GetFilename(ReferenceFilename));

	// Terminated
	return TRUE;
}

void C4MasterServerClient::ForceReference()
{
	AdvanceReference = true;
}
