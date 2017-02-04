/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_GUI.h"

SoEGUIButton::SoEGUIButton()
{
	m_pScreen   = NULL;
	m_pSurfBtn0 = NULL;
	m_pSurfBtn1 = NULL;
	proc        = NULL;
	m_pSoundBuffer = NULL;
	m_bActive   = false;
}

SoEGUIButton::~SoEGUIButton()
{
	m_bActive = false;

	//SAFEDELETE(m_pSurfBtn0);
	SAFEDELETE(m_pSurfBtn1);
}

bool SoEGUIButton::SetClickSound(SoESoundBuffer* pBuffer)
{
	if (pBuffer == NULL)
		return false;

	m_pSoundBuffer = pBuffer;
	return true;
}

RECT SoEGUIButton::GetRect(void)
{
	return m_button;
}

void SoEGUIButton::DoClick(int mouse_x, int mouse_y)
{
	if (!IsActive())
		return;

	if (!PointInRect(GetRect(), mouse_x, mouse_y))
		return;

	if (proc != NULL)
	{
		if (m_pSoundBuffer != NULL)
			m_pSoundBuffer->Play();
		proc();
	}
}

bool SoEGUIButton::PointInRect(RECT rc, int nX, int nY)
{
	if( nX >= rc.left )
		if( nX <= rc.right )
			if( nY >= rc.top )
				if( nY <= rc.bottom ) 
					return true;

	return false;
}

bool SoEGUIButton::Create(SoEScreen* pScreen, const char* szBtn0, const char* szBtn1, RECT button)
{
	if (pScreen == NULL)
		return false;

	m_pScreen = pScreen;
	m_button  = button;

    m_pSurfBtn0 = new SoESurface();

	if (m_pSurfBtn0 == NULL)
	{
		ERROR("alloc mem for m_pSurfBtn0");
		return false;
	}

	if (FAILED(m_pSurfBtn0->Create(pScreen, szBtn0)))
	{
		ERROR("create m_pSurfBtn0"); 
		return false;
	}

	m_pSurfBtn1 = new SoESurface();

	if (m_pSurfBtn1 == NULL)
	{
		ERROR("alloc mem for m_pSurfBtn1");
		return false;
	}

	if (FAILED(m_pSurfBtn1->Create(pScreen, szBtn1)))
	{
		ERROR("create m_pSurfBtn1"); 
		return false;
	}

	m_pScreen->AddSurfaceToList(m_pSurfBtn0);
	m_pScreen->AddSurfaceToList(m_pSurfBtn1);

	return (m_bActive = true);
}

Point SoEGUIButton::GetPosition(void)
{
    Point temp = {m_button.left, m_button.top};
	return temp;
}

void SoEGUIButton::SetPosition(int nX, int nY)
{
	RECT temp = m_button;

	m_button.left = nX; m_button.top = nY;
	m_button.right = nX + (temp.right - temp.left);
	m_button.bottom = nY + (temp.bottom - temp.top);
}

void SoEGUIButton::Update(int mouse_x, int mouse_y)
{
	if (!IsActive())
		return;

	RECT rc; 
	rc.left  = 0; rc.top = 0;
	rc.right = m_button.right; rc.bottom = m_button.bottom;
		
	if (PointInRect(m_button, mouse_x, mouse_y))
	{
		if (FAILED(m_pSurfBtn1->DrawBlkSW(m_pScreen->GetBack(), 
			m_button.left, m_button.top, &rc)))
		{
			ERROR("draw button#1");
		}
	}
	else
	{
		if (FAILED(m_pSurfBtn0->DrawBlkSW(m_pScreen->GetBack(), 
			m_button.left, m_button.top, &rc)))
		{
			ERROR("draw button#0");
		}
	}
}