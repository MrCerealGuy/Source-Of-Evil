/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_LAYER_H
#define SOE_LAYER_H

#include "SoE_Surface.h"

class SOEAPI SoELayer
{
public:
    SoELayer(void);
    virtual ~SoELayer(void);

	HRESULT Create(SoEScreen *pScreen, char *szFilename, BOOL memoryType = SOEMEM_VIDTHENSYS );
	HRESULT Create(SoESurface* pSurface);

	void ScrollUp(int Offset);
	void ScrollDown(int Offset);
	void ScrollLeft(int Offset);
	void ScrollRight(int Offset);
	void MoveTo(int XOffset, int YOffset);

	virtual HRESULT Draw(SoESurface* lpDDS);
	virtual HRESULT Draw(int X, int Y, SoESurface* lpDDS);
	virtual HRESULT DrawTrans(int X, int Y, SoESurface* lpDDS);

public:
	int         m_XOffset;   // The current X position in pixels
	int         m_YOffset;   // The current Y position in pixels
	SoESurface* m_pSurface;
	BOOL        m_bSurfaceCreated;
};

#endif
