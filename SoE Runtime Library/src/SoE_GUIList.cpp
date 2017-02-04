/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_GUI.h"

SoEGUIList::SoEGUIList(SoEScreen* pScreen, SoESoundBuffer* pSelSound, int nX, int nY, int nWidth)
{	
	if (pScreen == NULL)
		return;

	Default();

	m_pScreen   = pScreen;
	m_pSelSound = pSelSound; /* can be NULL */
	m_nX = nX;
	m_nY = nY;
	m_nEntryWidth  = nWidth;
}

SoEGUIList::~SoEGUIList()
{
	for (int i = 0; i != GetListCount(); i++)
		if (GetListEntry(i) != NULL)
			delete (GetListEntry(i));
}

void SoEGUIList::Default(void)
{
	m_nListCount = 0;
	m_bActive = false;
}

Point SoEGUIList::GetPosition(void)
{
	Point p = {m_nX, m_nY};
	return p;
}

SoEGUIListEntry* SoEGUIList::GetSelected(void)
{
	SoEGUIListEntry* temp = NULL;

	for (int i = 0; i != GetListCount(); i++)
	{
			temp = GetListEntry(i);
			if (temp == NULL) continue;

			if (temp->Selected())
				return temp;
	}

	return NULL;
}

void SoEGUIList::DoClick(int mouse_x, int mouse_y)
{
	if (!IsActive())
			return;

	SoEGUIListEntry* entry;

	for (int i = 0; i != GetListCount(); i++)
	{
		entry = GetListEntry(i);

		if (entry == NULL) continue;
		if (entry->PointInRect(entry->GetRect(),mouse_x, mouse_y))
		{
		    for (int i = 0; i != GetListCount(); i++)
				GetListEntry(i)->m_bSelected = false;

			entry->DoClick();
			return;
		}
	}
}

bool SoEGUIList::CreateFromListFile(const char* szFileName)
{
	SoEFile list_file;

	if (!list_file.OpenRead((const LPSTR)szFileName))
		return false;

	/* get number of entries */
	int nEntries; FILE* temp;
	temp = fopen(szFileName, "r");
	fscanf(temp, "%d", &nEntries);
	fclose(temp);

	GUI_LIST_ENTRY EntryData;
	PSOEGUILISTENTRY list_entry;

	for (int i = 0; i != nEntries; i++)
	{			
		ReadListEntry(&list_file, &EntryData);

		list_entry = new SoEGUIListEntry(this);

		if (list_entry == NULL) return false;

	    if (!list_entry->Create(EntryData.text, EntryData.data, m_nX, m_nY+(i*20), 35, RGB(128,0,0), RGB(0,0,0), RGB(0,128,0), m_pScreen))
			continue;

	    //list_entry->SetActivity();

		/* add to level list */
		if (list_entry != NULL)
	    	AddListEntry(list_entry);
	}

	list_file.Close();

	return (m_bActive = true);
}

bool SoEGUIList::Restore(void)
{
	for (int i = 0; i != GetListCount(); ++i)
	{
		SoEGUIListEntry* pEntry = GetListEntry(i);

		if (!pEntry) return false;

		delete pEntry->m_pListEntrySurface;

		pEntry->m_pListEntrySurface = new SoESurface();

		if (pEntry->m_pListEntrySurface == NULL)
			return false;

		if (FAILED(pEntry->m_pListEntrySurface->Create(m_pScreen, pEntry->GetRect().right - pEntry->GetRect().left, pEntry->GetRect().bottom - pEntry->GetRect().top, SOEMEM_VIDTHENSYS)))
		{
			delete pEntry->m_pListEntrySurface;
			return false;
		}
	}

	return true;
}

void SoEGUIList::ReadListEntry(SoEFile* list_file, GUI_LIST_ENTRY* entry)
{
	if (list_file == NULL)
		return;

	if (entry == NULL)
		return;

	char buf[256]; int i = 0;

	/* read until ';' */
	buf[i] = list_file->ReadUCHAR();
	for (;;)
	{
		if (buf[i] == ';') break;
		i++; buf[i] = list_file->ReadUCHAR();
	}
		
	/* read data until ';' */
	i = 0; buf[i] = list_file->ReadUCHAR();
	for (;;)
	{
		if (buf[i] == ';') break;
		i++; buf[i] = list_file->ReadUCHAR();
	}
	buf[i] = 0;
	strcpy(entry->data, (const char*)buf); 

	/* read text until ';' */
	i = 0; buf[i] = list_file->ReadUCHAR();
	for (;;)
	{
		if (buf[i] == ';') break;
		i++; buf[i] = list_file->ReadUCHAR();
	}
	buf[i] = 0;
	strcpy(entry->text, (const char*)buf);
}

bool SoEGUIList::AddListEntry(SoEGUIListEntry* pNew)
{
	if (pNew == NULL)
		return false;

	if (GetListCount() == 0)
	{		
		m_nListCount = 1;
		m_listEntries = (PSOEGUILISTENTRY*)malloc(GetListCount()*sizeof(PSOEGUILISTENTRY));
	}
	else
	{
		++m_nListCount; 
		PSOEGUILISTENTRY* temp = (PSOEGUILISTENTRY*)malloc(GetListCount()*sizeof(PSOEGUILISTENTRY));
			
		for (int i = 0; i != GetListCount()-1; i++)
			temp[i] = m_listEntries[i];

		free(m_listEntries);

		m_listEntries = (PSOEGUILISTENTRY*)malloc(GetListCount()*sizeof(PSOEGUILISTENTRY));

		for (i = 0; i != GetListCount()-1; i++)
			m_listEntries[i] = temp[i];

		free(temp);
		//realloc(m_listEntries, GetListCount()*sizeof(PSOEGUILISTENTRY));
	}

	m_listEntries[GetListCount()-1] = pNew;
		
	return true;
}

int SoEGUIList::GetListCount(void)
{
		return m_nListCount;
}

int SoEGUIList::GetIndex(SoEGUIListEntry* pEntry)
{
	for (int i = 0; i != GetListCount(); i++)
	{
		if (GetListEntry(i) == pEntry)
			return i;
	}

	return -1;
}

SoEGUIListEntry* SoEGUIList::GetListEntry(int nIndex)
{
		if (nIndex > (GetListCount()-1))
				return NULL;

		SoEGUIListEntry* temp = NULL;
		temp = m_listEntries[nIndex];
		return temp;
}

void SoEGUIList::Update(int mouse_x, int mouse_y)
{
		if (!IsActive()) return;

		SoEGUIListEntry* temp = NULL;

		for (int i = 0; i != GetListCount(); i++)
		{
			temp = GetListEntry(i);
			if (temp == NULL) continue;

			temp->Update(mouse_x, mouse_y);
		}
}