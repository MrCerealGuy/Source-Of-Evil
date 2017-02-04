/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_GUI.h"

SoEGUIImage::SoEGUIImage()
{
	m_pScreen    = NULL;
	m_pImageSurf = NULL;
	m_bActive    = false;
	proc         = NULL;
}

SoEGUIImage::~SoEGUIImage()
{
	m_bActive = false;
	SAFEDELETE(m_pImageSurf);
}

void SoEGUIImage::DoClick(void)
{
	if (proc != NULL)
		proc();
}

bool SoEGUIImage::Create(SoEScreen* pScreen, const char* szFileName, show_type type, int nX, int nY)
{
	if (pScreen == NULL)
		return false;

	m_pScreen = pScreen;
	m_type    = type;
    m_nX      = nX;
	m_nY      = nY;

    m_pImageSurf = new SoESurface();

	if (m_pImageSurf == NULL)
		return false;

	if (FAILED(m_pImageSurf->Create(m_pScreen, szFileName)))
		return false;

	m_pScreen->AddSurfaceToList(m_pImageSurf);

	return (m_bActive = true);
}

Point SoEGUIImage::GetPosition(void)
{
	Point p;
	p.x = m_nX; p.y = m_nY;
	return p;
}

void SoEGUIImage::Update(void)
{
	int nWidth  = m_pImageSurf->GetWidth();
	int nHeight = m_pImageSurf->GetHeight();

	RECT rc;
	rc.left   = 0; rc.top = 0;
	rc.right  = nWidth;
	rc.bottom = nHeight;

	static int line;

	switch (m_type)
	{
	case SHOW_NORMAL:  /*OK*/
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_BYLINE_HOR_UPDOWN:  /*OK*/
		line++;

		if (line == nHeight)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.bottom = line;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_MOVE_DOWN:   /*OK*/
		if (line == 0)
			line = nHeight;

		line--;

		if (line == 1)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.top = line;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_MOVE_RIGHT:   /*OK*/
		if (line == 0)
			line = nWidth;
				
		line--;

		if (line == 1)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.left = line;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_MOVE_LEFT:   /*SHIT*/
		if (line == 0)
		{
			rc.left -= nWidth;
			rc.right = 0;
			//line = nWidth;
		}
				
		line++;

		if (line == nWidth)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.right++; rc.left++;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_BYLINE_VER_LEFTRIGHT:   /*OK*/
		line++;

		if (line == nWidth)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.right = line;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	case SHOW_BYLINE_VER_RIGHTLEFT:   /*SHIT*/
		if (line == 0)
			line = nWidth;
				
		line--;

		if (line == 1)
		{
			m_type = SHOW_NORMAL; line = 0;
			m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
			break;
		}

		rc.left = line;
		m_pImageSurf->DrawBlkSW(m_pScreen->GetBack(), m_nX, m_nY, &rc);
	break;

	default:
	break;
	}
}