/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifdef SOE_LOG

#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "SoE.h"
#include "SoE_Screen.h"
#include "SoE_Surface.h"
#include "SoE_Log.h"

// our instance of the log
SoELog g_SoElog;


SoELog::SoELog()
{
	m_LogType = SOELOG_TYPE_FILE,
		m_Screen = NULL;
	m_Level = SOELOG_LEVEL_1;
	m_dwStartTime = timeGetTime();
	SYSTEMTIME st;
    GetLocalTime(&st);
	// YYYYMMDDHHMMSS
    sprintf(m_FileName, "SoE%.4d%.2d%.2d%.2d%.2d%.2d.log", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

SoELog::~SoELog()
{
}

void SoELog::Level(const int NewLevel)
{
	// save the verfied type
	m_Level = NewLevel;
}

int SoELog::Level()
{
	return m_Level;
}

void SoELog::LogType(const int NewType)
{
	// save the verfied type
	m_LogType = NewType;
}

int SoELog::LogType()
{
	return m_LogType;
}

void SoELog::Screen(SoEScreen *s)
{
	// save the screen
	m_Screen = s;
}

SoEScreen *SoELog::Screen()
{
	return m_Screen;
}

void SoELog::ClearFile()
{
	DeleteFile(m_FileName);
}

void SoELog::Clear()
{
	switch (m_LogType)
	{
		case SOELOG_TYPE_FILE:
			ClearFile();
			break;
		case SOELOG_TYPE_STDOUT:
		case SOELOG_TYPE_STDERR:
			// don't think these can be cleared
			break;
	};
}

void SoELog::FileWrite(const char *buff)
{
	FILE *fp = NULL;
	
	// open our logging file
    fp = fopen(m_FileName, "a");
    if (fp == NULL)
		return;
	
	fprintf(fp, buff);
	
	fclose(fp);
}

void SoELog::COMWrite(const int portnum, const char *buff)
{
	FILE *fp = NULL;
	char port[10];
	
	// open our logging file
	switch (portnum)
	{
		case SOELOG_TYPE_COM1:
			lstrcpy(port, "COM1:");
			break;
		case SOELOG_TYPE_COM2:
			lstrcpy(port, "COM2:");
			break;
	};
	
	fp = fopen(port, "w");
	if (fp == NULL)
		return;
	
	fprintf(fp, buff);
	
	fclose(fp);
}

void SoELog::LPTWrite(const int portnum, const char *buff)
{
	FILE *fp = NULL;
	char port[10];
	
	// open our logging file
	switch (portnum)
	{
		case SOELOG_TYPE_LPT1:
			lstrcpy(port, "LPT1:");
			break;
		case SOELOG_TYPE_LPT2:
			lstrcpy(port, "LPT2:");
			break;
	};
	
	fp = fopen(port, "w");
	if (fp == NULL)
		return;
	
	fprintf(fp, buff);
	
	fclose(fp);
}

void SoELog::GUIWrite(const char *buff)
{
	// allocate a static array of 5 lines of text
	// holding 255 chars each
	static char lines[5][256];
	static curline = 0;
	SIZE sz;
	int i;
	SoESurface *s;
	int x, y, h, w;
	
	lstrcpy(lines[curline], buff);
	curline++;
	
	// if we've ran over our EOL then start at the beginning
	if (curline == 5)
		curline = 0;
	
	// sanity check
	if (m_Screen == NULL)
	{
		FileWrite("No SoEScreen object supplied for GUIWIN output\n");
		return;
	}
	
	s = m_Screen->GetFront();
	
	s->GetDC();
    s->SetFont();
	
    // get the height and width current font (use M since its widest)
    ::GetTextExtentPoint32(s->GetDC(), "M", 1, &sz);
	
	s->ReleaseDC();
	
	w = sz.cx * 26;
	h = sz.cy * 6;
	
	x = s->GetWidth() - w;
	y = s->GetHeight() - h;
	
	// fill the rect with black
	s->FillRect(x, y, x + w, y + h, RGB(255, 255, 255));
	s->FillRect(x + 1, y + 1, x + w - 1, y + h - 1, 0);
	
	// start the text slightly indented
	x += (sz.cx/2);
	y += (sz.cy/2);
	
	s->GetDC();
    s->SetFont();
	
	// render the window
	for (i = 0; i < 5; i++)
	{
		s->TextXY(x, y, RGB(255, 255, 255), lines[i]);
		y += sz.cy;
	}
	
	s->ReleaseDC();
	
	s = NULL;
}

void SoELog::WriteInternal(const char *buff)
{
	if ((m_LogType & SOELOG_TYPE_FILE) == SOELOG_TYPE_FILE)
		FileWrite(buff);
	if ((m_LogType & SOELOG_TYPE_STDOUT) == SOELOG_TYPE_STDOUT)
		fprintf(stdout, buff);
	if ((m_LogType & SOELOG_TYPE_STDERR) == SOELOG_TYPE_STDERR)
		fprintf(stderr, buff);
	if ((m_LogType & SOELOG_TYPE_COM1) == SOELOG_TYPE_COM1)
		COMWrite(SOELOG_TYPE_COM1, buff);
	if ((m_LogType & SOELOG_TYPE_COM2) == SOELOG_TYPE_COM2)
		COMWrite(SOELOG_TYPE_COM2, buff);
	if ((m_LogType & SOELOG_TYPE_LPT1) == SOELOG_TYPE_LPT1)
		LPTWrite(SOELOG_TYPE_LPT1, buff);
	if ((m_LogType & SOELOG_TYPE_LPT2) == SOELOG_TYPE_LPT2)
		LPTWrite(SOELOG_TYPE_LPT2, buff);
	if ((m_LogType & SOELOG_TYPE_GUIWIN) == SOELOG_TYPE_GUIWIN)
		GUIWrite(buff);
	
#if _MSC_VER
	// output to the debug window also
	if ((m_LogType & SOELOG_TYPE_DEBUGWIN) == SOELOG_TYPE_DEBUGWIN)
		ODS(buff);
#endif
}

void SoELog::Write(const char *fmt, ...)
{
    char    buff[256];
    va_list	va;
	
    va_start(va, fmt);
	
    // put the time in milliseconds at beginning of line.
    wsprintf(buff, "%u: ", timeGetTime() - m_dwStartTime);
	wvsprintf(&buff[lstrlen(buff)], fmt, va);
	lstrcat(buff, "\n");
	
	WriteInternal(buff);
}

void SoELog::Write2(const char *fmt, ...)
{
    char    buff[256];
    va_list	va;
	
	if (!(m_Level == SOELOG_LEVEL_2))
		return;
	
    va_start(va, fmt);
	
    // put the time in milliseconds at beginning of line.
    wsprintf(buff, "%d: ", timeGetTime() - m_dwStartTime);
	wvsprintf(&buff[lstrlen(buff)], fmt, va);
	lstrcat(buff, "\n");
	
	WriteInternal(buff);
}

#endif			
