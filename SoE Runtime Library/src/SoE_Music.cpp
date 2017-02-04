/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Music.h"

SoEMusic::SoEMusic(void *hWnd)
{
	m_hWnd = ((HWND)hWnd);
}

SoEMusic::~SoEMusic()
{
	Stop();
}

BOOL SoEMusic::Play(const char *Filename)
{
	if (Filename == NULL)
		return FALSE;
	char buffer[256];
	
	sprintf(buffer, "open %s type sequencer alias MUSIC", Filename);
	
	if (mciSendString("close all", NULL, 0, NULL) != 0)
	{
		return (FALSE);
	}
	
	if (mciSendString(buffer, NULL, 0, NULL) != 0)
	{
		return (FALSE);
	}
	
	if (mciSendString("play MUSIC from 0 notify", NULL, 0, m_hWnd) != 0)
	{
		return (FALSE);
	}
	
	return TRUE;
}

SoEMusic::Stop()
{
	if (mciSendString("close all", NULL, 0, NULL) != 0)
	{
		return (FALSE);
	}   
	
	return TRUE;
}

BOOL SoEMusic::Pause(void)
{
	// Pause if we're not already paused
	if (mciSendString("stop MUSIC", NULL, 0, NULL) != 0)
	{
		return (FALSE);
	}
	
	return TRUE;
}

BOOL SoEMusic::Resume(void)
{
	if (mciSendString("play MUSIC notify", NULL, 0, m_hWnd) != 0)
	{
		return (FALSE);
	}
	
	return TRUE;
}

BOOL SoEMusic::Restart(void)
{
	if (mciSendString("play MUSIC from 0 notify", NULL, 0, m_hWnd) != 0)
	{
		return (FALSE);
	}
	
	return TRUE;
}
