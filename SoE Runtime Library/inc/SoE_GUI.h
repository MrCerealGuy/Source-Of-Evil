/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_GUI_H
#define SOE_GUI_H

#include "SoE.h"
#include "SoE_Sound.h"
#include "SoE_File.h"

typedef struct _Point
{
	int x, y;

} Point;

typedef struct
{
	char text[256];
	char data[256];

} GUI_LIST_ENTRY;

/* for SoEGUIImage */
enum show_type { SHOW_NORMAL, SHOW_BYLINE_HOR_UPDOWN, SHOW_BYLINE_VER_LEFTRIGHT,
	    		 SHOW_BYLINE_HOR_DOWNUP, SHOW_BYLINE_VER_RIGHTLEFT,
				 SHOW_MOVE_DOWN, SHOW_MOVE_UP, SHOW_MOVE_RIGHT, SHOW_MOVE_LEFT};

class SoEGUISystem;
class SoEGUIWindow;
class SoEGUIImage;
class SoEGUIButton;
class SoEGUIList;
class SoEGUIListEntry;

typedef SoEGUISystem*    PSOEGUISYSTEM;
typedef SoEGUIWindow*    PSOEGUIWINDOW;
typedef SoEGUIImage*     PSOEGUIIMAGE;
typedef SoEGUIButton*    PSOEGUIBUTTON;
typedef SoEGUIList*      PSOEGUILIST;
typedef SoEGUIListEntry* PSOEGUILISTENTRY;

class SoEGUISystem
{
public:
	SoEGUISystem();
	~SoEGUISystem();

	void Shutdown(void);

	PSOEGUIWINDOW CreateSoEWindow(int nX, int nY, int nWidth, int nHeight);

};

class SoEGUIWindow
{
public:
	SoEGUIWindow();
	~SoEGUIWindow();
};

class SoEGUIImage
{
public:

	SoEGUIImage();
	~SoEGUIImage();

	bool Create(SoEScreen* pScreen, const char* szFileName,
			        show_type type, int nX, int nY);
	Point GetPosition(void);
	void Update(void);
	bool IsActive(void) { return m_bActive; };
	void SetActivity(bool bActive = true) { m_bActive = bActive; };
	void (*proc)(void);
	void DoClick(void);
	SoESurface* GetSurface(void) { return m_pImageSurf; };

private:

	SoEScreen* m_pScreen;
	SoESurface* m_pImageSurf;
	show_type m_type;
	int m_nX, m_nY;
	bool m_bActive;
};

class SoEGUIButton
{
public:

	SoEGUIButton();
	~SoEGUIButton();

	bool Create(SoEScreen* pScreen, const char* szBtn0, const char* szBtn1, 
			RECT button);
	void Update(int mouse_x, int mouse_y);
	bool PointInRect(RECT rc, int nX, int nY);
	void DoClick(int mouse_x, int mouse_y);
	RECT GetRect(void);
    void (*proc)(void);
	bool IsActive(void) { return m_bActive; };
	void SetActivity(bool bActive = true) { m_bActive = bActive; };
	bool SetClickSound(SoESoundBuffer* pBuffer);
	void SetPosition(int nX, int nY);
	Point GetPosition(void);
	inline SoESurface* GetSurface(int i);

private:

    SoEScreen*  m_pScreen;
	SoESurface* m_pSurfBtn0;
	SoESurface* m_pSurfBtn1;
	SoESoundBuffer* m_pSoundBuffer;
	RECT m_button;
	bool m_bActive;
};

inline SoESurface* SoEGUIButton::GetSurface(int i)
{
	if (i == 0)
		return m_pSurfBtn0;
	else if (i == 1)
		return m_pSurfBtn1;
	else
		return NULL;
}

class SoEGUIListEntry
{
friend class SoEGUIList;

public:
	SoEGUIListEntry();
	SoEGUIListEntry(SoEGUIList* pList);
	~SoEGUIListEntry();

	bool Create(const char* szText, const char* szData, int nX, int nY, int nWidth, DWORD dwFontColor, DWORD dwBackColor, DWORD dwSelBackColor, SoEScreen* pScreen);
	DWORD GetFontColor(void);
	DWORD GetBackColor(void);
	Point GetPosition(void);
	void SetActivity(bool bActive = true) { m_bActive = bActive; };
	void Update(int mouse_x, int mouse_y);
	bool Selected(void) { return m_bSelected; };
	bool PointInRect(RECT rc, int nX, int nY);
	bool IsActive(void) { return m_bActive; };
	void DoClick(void);
	RECT GetRect(void);
	char* GetText(void) { return m_szText; };
	char* GetData(void) { return m_szData; };
	void (*proc)(void);
	SoEGUIList* GetList(void) { return m_pList; };
	SoESurface* GetSurface(void) { return m_pListEntrySurface; };

private:
	void Default(void);

public:
    bool m_bSelected;

private:
	int m_nX, m_nY;
	//int m_nWidth; /* in char size */ 
	DWORD m_dwFontColor;
	DWORD m_dwBackColor;
	DWORD m_dwSelBackColor;
	bool m_bActive;
	char* m_szText;
	char* m_szData;
	RECT m_rcTextRect;
	SoEScreen* m_pScreen;
	SoEGUIList* m_pList;
	SoESurface* m_pListEntrySurface;
};

class SoEGUIList
{
public:
	SoEGUIList(SoEScreen* pScreen, SoESoundBuffer* pSelSound, int nX, int nY, int nWidth);
	~SoEGUIList();

	bool CreateFromListFile(const char* szFileName);
	int GetListCount(void); 
	bool AddListEntry(SoEGUIListEntry* pNew);
	SoEGUIListEntry* GetListEntry(int nIndex);
	void Update(int mouse_x, int mouse_y);
	bool IsActive(void) { return m_bActive; };
	void SetActivity(bool bActive = true) { m_bActive = bActive; };
	SoEGUIListEntry* GetSelected(void);
	void DoClick(int mouse_x, int mouse_y);
	Point GetPosition(void);
	int GetIndex(SoEGUIListEntry* pEntry);
	SoESoundBuffer* GetSound(void) { return m_pSelSound; };
	int GetListEntryWidth(void) { return m_nEntryWidth; };
	bool Restore(void);

private:
	void ReadListEntry(SoEFile* list_file, GUI_LIST_ENTRY* entry);
	void Default(void);

private:
	PSOEGUILISTENTRY *m_listEntries;
	SoEScreen* m_pScreen;
	SoESoundBuffer* m_pSelSound;
	int m_nListCount;
	int m_nX, m_nY;
	int m_nEntryWidth;
	bool m_bActive;
};

#endif SOE_GUI_H