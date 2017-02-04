/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_LOG_H
#define SOE_LOG_H

#include "SoE_Screen.h"

// SoE Logging Types.  May be combined together to log to different
// areas simultaneously
#define SOELOG_TYPE_FILE			0x00000001L
#define SOELOG_TYPE_STDOUT			0x00000002L
#define SOELOG_TYPE_STDERR			0x00000004L
#define SOELOG_TYPE_COM1			0x00000008L
#define SOELOG_TYPE_COM2			0x00000010L
#define SOELOG_TYPE_LPT1			0x00000020L
#define SOELOG_TYPE_LPT2			0x00000040L
#define SOELOG_TYPE_GUIWIN			0x00000080L
#define SOELOG_TYPE_WINSOCK			0x00000100L
#define SOELOG_TYPE_DEBUGWIN		0x00000200L

// Logging levels.  More detail as you increase
#define SOELOG_LEVEL_1				0x00000001L
#define SOELOG_LEVEL_2				0x00000002L

class SOEAPI SoELog
{
public:
	SoELog(void);
	~SoELog(void);

public:
	// write output to the current log for level 0
	void Write(const char *, ...);
	// write output to the current log for level <= 1
	void Write2(const char *, ...);
	// clears the current log
	void Clear();

	void Screen(SoEScreen *);
	SoEScreen *Screen();

	void LogType(const int);
	int LogType();

	void Level(const int);
	int Level();

protected:
	void WriteInternal(const char *);
	void ClearFile();
	void FileWrite(const char *buff);
	void COMWrite(const int, const char *);
	void LPTWrite(const int, const char *);
	void GUIWrite(const char *);

private:
	SoEScreen *m_Screen;
	int m_LogType;
	int m_Level;
	DWORD m_dwStartTime;
	char m_FileName[MAX_PATH];
};

#ifdef SOE_LOG

extern SoELog g_SoElog;

// debug logging functions
#define SOELOG g_SoElog.Write
#define SOELOG2 g_SoElog.Write2
#define SOELOGSCREEN g_SoElog.Screen
#define SOELOGLEVEL g_SoElog.Level
#define SOELOGTYPE g_SoElog.LogType
#define SOELOGCLEAR g_SoElog.Clear

#else

// logging is NOT enabled

// just define it and let it go for a non-logging build
#ifdef _MSC_VER
#define SOELOG ((void)0)
#define SOELOG2 ((void)0)
#define SOELOGSCREEN ((void)0)
#define SOELOGLEVEL ((void)0)
#define SOELOGTYPE ((void)0)
#define SOELOGCLEAR ((void)0)
#else
#define SOELOG
#define SOELOG2
#define SOELOGSCREEN
#define SOELOGLEVEL
#define SOELOGTYPE
#define SOELOGCLEAR
#endif         

#endif		


#endif		
