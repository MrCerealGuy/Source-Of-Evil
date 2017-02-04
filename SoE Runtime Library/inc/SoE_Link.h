/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_LINK_H
#define SOE_LINK_H

//////////////////////////////////////////////////////////////////////////////////
// SoELinkSession Structure
//////////////////////////////////////////////////////////////////////////////////
struct SoELinkSession
{
	char *m_Name;
	GUID  m_Guid;
	GUID  m_AppGuid;
	DWORD m_Flags;
	DWORD m_MaxPlayers;
	DWORD m_CurrentPlayers;
};

//////////////////////////////////////////////////////////////////////////////////
// SoELinkPlayer Structure
//////////////////////////////////////////////////////////////////////////////////
struct SoELinkPlayer
{
	DPID m_ID;
	char *m_Name;
	DWORD m_Flags;
	HANDLE m_Event;
};

//////////////////////////////////////////////////////////////////////////////////
// SoELink Class
//////////////////////////////////////////////////////////////////////////////////
class SOEAPI SoELink
{
public:
	SoELink(void);
	virtual ~SoELink(void);

	BOOL CreateIPX(void* hWnd);
	BOOL CreateTCPIP(void* hWnd, char* IPAddress);
	BOOL CreateModem(void* hWnd, char* PhoneNo);
	BOOL CreateSerial(void* hWnd);

	BOOL EnumSessions(SoELinkSession* Sessions, GUID APP_GUID);
	BOOL HostSession(SoELinkSession* Session);
	BOOL JoinSession(SoELinkSession* Session);
	BOOL CloseSession(void);

	BOOL EnumPlayers(SoELinkPlayer* Players, DWORD Flags = 0);
	BOOL CreatePlayer(SoELinkPlayer* Player);
	BOOL DestroyPlayer(SoELinkPlayer* Player);

	HRESULT Send(DPID idFrom, DPID idTo, DWORD dwFlags,
							 LPVOID lpData, DWORD dwDataSize);
	HRESULT Receive(LPDPID lpidFrom, LPDPID lpidTo,	DWORD dwFlags,
									LPVOID lpData, LPDWORD lpdwDataSize);

public:
	HRESULT rval;
	LPDIRECTPLAY3A m_DP;
	LPDIRECTPLAYLOBBY2A m_DPLobby;
};

#endif
