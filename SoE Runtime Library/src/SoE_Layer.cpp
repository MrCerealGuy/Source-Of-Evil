/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Layer.h"

SoELayer::SoELayer(void)
{	
	m_XOffset         = 0;
	m_YOffset         = 0;
	m_pSurface        = NULL;
	m_bSurfaceCreated = FALSE;
}

SoELayer::~SoELayer(void) 
{	
	// Delete the surface if we created it and it is not null;
	if ((m_bSurfaceCreated == TRUE) &&(m_pSurface != NULL))
	{
		delete m_pSurface;
		m_pSurface = NULL;
	}
}

HRESULT SoELayer::Create(SoEScreen *pScreen, char *szFilename, BOOL memoryType) 
{	
    HRESULT rval;
	
	// Validate pScreen
    if (pScreen == NULL)
        return -1;
	
    // Validate szFilename
    if (szFilename == NULL)
        return -2;
	
	// Create the SoESurface.
	m_pSurface = new SoESurface();
	if (m_pSurface == NULL)
		return -3;
    rval = m_pSurface->Create(pScreen , szFilename , memoryType);
	
    return rval;
}

HRESULT SoELayer::Create(SoESurface* pSurface) 
{	
    m_pSurface = pSurface;
	
    return 1;
}

void SoELayer::ScrollUp(int Offset)
{
    m_YOffset += Offset;
	
    if (m_YOffset >= m_pSurface->GetHeight()) 
        m_YOffset -= m_pSurface->GetHeight();
}

void SoELayer::ScrollDown(int Offset)
{
    m_YOffset -= Offset;
	
    if (m_YOffset < 0) 
        m_YOffset = m_pSurface->GetHeight() + m_YOffset;
}

void SoELayer::ScrollLeft(int Offset)
{
    m_XOffset += Offset;
	
    if (m_XOffset >= m_pSurface->GetWidth()) 
        m_XOffset -= m_pSurface->GetWidth();
}

void SoELayer::ScrollRight(int Offset)
{
    m_XOffset -= Offset;
	
    if (m_XOffset < 0) 
        m_XOffset = m_pSurface->GetWidth() + m_XOffset;
}

void SoELayer::MoveTo(int XOffset, int YOffset)
{
    m_XOffset = XOffset;
    m_YOffset = YOffset;
}

HRESULT SoELayer::Draw(SoESurface* s)
{
	HRESULT rval;
    RECT rect;
    int h, w;
	
    h = m_pSurface->GetHeight();
    w = m_pSurface->GetWidth();
	
    rect.top    = h - m_YOffset;
    rect.left   = w - m_XOffset;
    rect.bottom = h;
    rect.right  = w;
    m_pSurface->DrawBlk(s, 0, 0, &rect);
	
    rect.top    = h - m_YOffset;
    rect.left   = 0;
    rect.bottom = h;
    rect.right  = w - m_XOffset;
	m_pSurface->DrawBlk(s, m_XOffset, 0, &rect);
	
    rect.top    = 0;
    rect.left   = w - m_XOffset;
    rect.bottom = h - m_YOffset;
    rect.right  = w;
	m_pSurface->DrawBlk(s, 0, m_YOffset, &rect);
	
    rect.top    = 0;
    rect.left   = 0;
    rect.bottom = h - m_YOffset;
    rect.right  = w - m_XOffset;
	rval = m_pSurface->DrawBlk(s, m_XOffset, m_YOffset, &rect);
	
	return rval;
}

HRESULT SoELayer::Draw(int X, int Y, SoESurface* s)
{
	HRESULT rval;
    RECT rect;
    int h, w;
	
    h = m_pSurface->GetHeight();
    w = m_pSurface->GetWidth();
	
    rect.top    = h - m_YOffset;
    rect.left   = w - m_XOffset;
    rect.bottom = h;
    rect.right  = w;
    m_pSurface->DrawBlk(s, 0, 0, &rect);
	
    rect.top    = h - m_YOffset;
    rect.left   = 0;
    rect.bottom = h;
    rect.right  = w - m_XOffset;
	m_pSurface->DrawBlk(s, m_XOffset + X, 0, &rect);
	
    rect.top    = 0;
    rect.left   = w - m_XOffset;
    rect.bottom = h - m_YOffset;
    rect.right  = w;
	m_pSurface->DrawBlk(s, 0, m_YOffset + Y, &rect);
	
    rect.top    = 0;
    rect.left   = 0;
    rect.bottom = h - m_YOffset;
    rect.right  = w - m_XOffset;
	rval = m_pSurface->DrawBlk(s, m_XOffset, m_YOffset, &rect);
	
	return rval;
}

HRESULT SoELayer::DrawTrans(int X, int Y, SoESurface* s)
{
	HRESULT rval;
    RECT rect;
    int h, w;
	
    h = m_pSurface->GetHeight();
    w = m_pSurface->GetWidth();
	
    rect.top    = h - m_YOffset;
    rect.left   = w - m_XOffset;
    rect.bottom = h;
    rect.right  = w;
    m_pSurface->DrawTrans(s, 0, 0, &rect);
	
    rect.top    = h - m_YOffset;
    rect.left   = 0;
    rect.bottom = h;
    rect.right  = w - m_XOffset;
    m_pSurface->DrawTrans(s, m_XOffset + X, 0, &rect);
	
    rect.top    = 0;
    rect.left   = w - m_XOffset;
    rect.bottom = h - m_YOffset;
    rect.right  = w;
    m_pSurface->DrawTrans(s, 0, m_YOffset + Y, &rect);
	
    rect.top    = 0;
    rect.left   = 0;
    rect.bottom = h - m_YOffset;
    rect.right  = w - m_XOffset;
    rval = m_pSurface->SoESurface::DrawTrans(s, m_XOffset, m_YOffset, &rect);
	
	return rval;
}