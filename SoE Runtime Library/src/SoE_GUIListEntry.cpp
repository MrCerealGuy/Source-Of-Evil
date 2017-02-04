/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_GUI.h"

SoEGUIListEntry::SoEGUIListEntry()
{
	Default();	
}

SoEGUIListEntry::SoEGUIListEntry(SoEGUIList* pList)
{
	if (pList == NULL)
		delete this;

	m_pList = pList;

	Default();
}

void SoEGUIListEntry::Default(void)
{
	m_nX = m_nY = 0;
	m_bActive = m_bSelected = false;
	m_dwFontColor = m_dwBackColor = 0;
	m_pListEntrySurface = NULL;
	char* m_szText = NULL;
	proc = NULL;
}

SoEGUIListEntry::~SoEGUIListEntry()
{
	DELETEARRAY(m_szText);
	DELETEARRAY(m_szData);
}

bool SoEGUIListEntry::Create(const char* szText, const char* szData, int nX, int nY, int nWidth, DWORD dwFontColor, DWORD dwBackColor, DWORD dwSelBackColor, SoEScreen* pScreen)
{
	if (pScreen == NULL)
		return false;

	//if (GetList() == NULL)
	//	return false;

	m_nX = nX; m_nY = nY;
	m_dwFontColor = dwFontColor;
	m_dwBackColor = dwBackColor;
	m_dwSelBackColor = dwSelBackColor;
	m_pScreen = pScreen;

	m_szText = new char[nWidth];
		
	for (int i = 0; i != nWidth; i++)
		m_szText[i] = ' ';

    strcpy(m_szText, szText);
	m_szText[lstrlen(szText)] = ' ';

	if (szData != NULL)
	{
		m_szData = new char[lstrlen(szData)];

		for (i = 0; i != lstrlen(szData); i++)
			m_szData[i] = ' ';

		strcpy(m_szData, szData);
	}
	else
		m_szData = NULL;

	m_rcTextRect.left = nX;
	m_rcTextRect.top  = nY;
	//m_nWidth = nWidth;

	return (m_bActive = true);
}

DWORD SoEGUIListEntry::GetFontColor(void)
{
	return m_dwFontColor;
}

DWORD SoEGUIListEntry::GetBackColor(void)
{
	if (Selected())
		return m_dwSelBackColor;
	return m_dwBackColor;
}

Point SoEGUIListEntry::GetPosition(void)
{
	Point p = {m_nX, m_nY};
	return p;
}

bool SoEGUIListEntry::PointInRect(RECT rc, int nX, int nY)
{
	if( nX >= rc.left )
		if( nX <= rc.right )
			if( nY >= rc.top )
				if( nY <= rc.bottom ) 
					return true;

	return false;
}

void SoEGUIListEntry::Update(int mouse_x, int mouse_y)
{
	if (!IsActive())
		return;

	/* create only once time */
	if (m_pListEntrySurface == NULL)
	{
		m_pListEntrySurface = new SoESurface();

		if (m_pListEntrySurface == NULL)
			return;

		if (FAILED(m_pListEntrySurface->Create(m_pScreen, GetRect().right - GetRect().left, GetRect().bottom - GetRect().top, SOEMEM_VIDTHENSYS)))
		{
			delete m_pListEntrySurface;
			return;
		}
	}

	/* fill surface with black color */
	m_pListEntrySurface->Fill(0x000000);

	HDC hDC = m_pListEntrySurface->GetDC();

	if (hDC == NULL)
	{
		/* Surface lost? Restore all */
		(void)m_pList->Restore();

		return;
	}

	char szText[1024];
	strcpy(szText, (const char*)GetText());
	for (int i = strlen(GetText())-1; i != sizeof(szText); ++i) szText[i] = ' ';
	
	SetBkColor(hDC, GetBackColor());
	SetTextColor(hDC, GetFontColor());
	SetTextAlign(hDC, TA_LEFT);
	TextOut(hDC, 0, 0, (const char*)szText, sizeof(szText));

	(void)m_pListEntrySurface->ReleaseDC();

	/* copy text to back buffer */
	(void)m_pListEntrySurface->DrawBlkSW(m_pScreen->GetBack(), GetPosition().x, GetPosition().y, NULL);
}

RECT SoEGUIListEntry::GetRect(void)
{
	HDC hDC = m_pScreen->GetBack()->GetDC();

	TEXTMETRIC tm; 
	GetTextMetrics(hDC, &tm);
		
	m_rcTextRect.left = GetPosition().x;
	m_rcTextRect.top  = GetPosition().y;

	m_rcTextRect.bottom = m_rcTextRect.top + tm.tmHeight;
	m_rcTextRect.right  = m_rcTextRect.left + GetList()->GetListEntryWidth();

	m_pScreen->GetBack()->ReleaseDC();

	return m_rcTextRect;
}

void SoEGUIListEntry::DoClick(void)
{
	if (!IsActive() && !GetList())
		return;

	m_bSelected = true;

	if (GetList()->GetSound() != NULL)
		GetList()->GetSound()->Play();

	if (proc != NULL)	
		proc();	
}