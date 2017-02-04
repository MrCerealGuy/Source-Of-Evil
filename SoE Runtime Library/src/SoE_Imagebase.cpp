/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_ImageBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SoEImageBase::SoEImageBase(void)
{
    m_pBitmapInfo = NULL;
    ZeroMemory(&m_BitmapInfo, sizeof(BITMAPINFO));
    m_iWidth      = 0;
    m_iHeight     = 0;
    m_cBpp        = 0;
    m_pStorage    = NULL;
    m_IsInverted  = FALSE;
	m_IsMirrored  = FALSE;
	m_pPalette    = NULL;
	m_iUsage      = DIB_RGB_COLORS;
    m_lpdds       = NULL;
}

SoEImageBase::~SoEImageBase(void)
{
    CloseFile();
	
	DeAllocateStorage();
}

BOOL SoEImageBase::OpenFile(const char* filename)
{
	CloseFile();
	
	m_file.OpenRead((char *)filename);
	
	return TRUE;
}

BOOL SoEImageBase::CloseFile(void)
{
	m_file.Close();
	
	return TRUE;
}

void SoEImageBase::SetupBitmapInfo()
{
	if (m_pBitmapInfo)
	{
		free(m_pBitmapInfo);
		m_pBitmapInfo = NULL;
	}
	// allocate 256 color rgbquads for maximum usage
	m_pBitmapInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256));
	if (m_pBitmapInfo == NULL)
		return;
	
	ZeroMemory(m_pBitmapInfo, sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256));
	
	m_pBitmapInfo->bmiHeader.biSize     = sizeof(m_pBitmapInfo->bmiHeader);
	if (IsMirrored())
		m_pBitmapInfo->bmiHeader.biWidth    = -m_iWidth;
	else
		m_pBitmapInfo->bmiHeader.biWidth    = m_iWidth;
	
	if (IsInverted())
		m_pBitmapInfo->bmiHeader.biHeight   = -m_iHeight;
	else
		m_pBitmapInfo->bmiHeader.biHeight   = m_iHeight;
	
	m_pBitmapInfo->bmiHeader.biPlanes   = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = m_cBpp;
}

BOOL SoEImageBase::AllocateStorage(int w, int h, unsigned char bpp)
{
	m_iWidth  = w;
	m_iHeight = h;
	m_cBpp    = bpp;
	
	return AllocateStorage(w * h *(bpp >> 3));
}

BOOL SoEImageBase::AllocateStorage(unsigned long size)
{
	DeAllocateStorage();
	
    m_pStorage = new CHAR[size];
	
	if (m_pStorage == NULL)
		return FALSE;
	
    return TRUE;
}

BOOL SoEImageBase::DeAllocateStorage()
{
	DELETEARRAY(m_pStorage);
	
	if (m_pBitmapInfo)
	{
		free(m_pBitmapInfo);
		m_pBitmapInfo = NULL;
	}
	
	return TRUE;
}

BOOL SoEImageBase::CreateSurface(SOE_LPDIRECTDRAW lpDD, BYTE memType)
{
    HRESULT rval;
	SOE_DDSURFACEDESC ddsd;
	
    m_lpdds = NULL;
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize   = sizeof(ddsd);
	
    switch (memType)
	{
		case SOEMEM_SYSTEMONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			break;
			
		case SOEMEM_VIDEOONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
			break;
			
		case SOEMEM_VIDTHENSYS:
		default:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			break;
	}
	
	ddsd.dwFlags  = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
	ddsd.dwWidth  = m_iWidth;
	ddsd.dwHeight = m_iHeight;
	
	// If we can't create the surface, return NULL
    rval = lpDD->CreateSurface(&ddsd, &m_lpdds, NULL);
	
	if (FAILED(rval))
    {
		return FALSE;
	}
	
	return TRUE;
}

SOE_LPDIRECTDRAWSURFACE SoEImageBase::CopyBits()
{
	HDC hdc;
    m_lpdds->GetDC(&hdc);
	int rc = StretchDIBits(hdc,
		0,
		0,
		m_iWidth,
		m_iHeight,
		0,
		0,
		m_iWidth,
		m_iHeight,
		m_pStorage,
		m_pBitmapInfo,
		m_iUsage,
		SRCCOPY);
	
	m_lpdds->ReleaseDC(hdc);
	
	// free up the raw memory
	DeAllocateStorage();
	
	if (rc == GDI_ERROR)
		return NULL;
	
	return m_lpdds;
}

