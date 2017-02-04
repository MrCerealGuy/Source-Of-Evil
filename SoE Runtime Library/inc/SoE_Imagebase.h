/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGEBASE_H
#define SOE_IMAGEBASE_H

#include "SoE.h"
#include "SoE_File.h"

#define SOEIMAGE_UNKNOWN  0
#define SOEIMAGE_BMP      1
#define SOEIMAGE_PSD      2
#define SOEIMAGE_TGA      3
#define SOEIMAGE_PCX      4
#define SOEIMAGE_JPG      5
#define SOEIMAGE_LAST     6

#define BIG2LIT(a)			(a>>8 & 0xff) | ((a & 0x00ff)<<8)
#define DWORDBIG2LIT(b)     MAKELONG( BIG2LIT(HIWORD(b)), BIG2LIT(LOWORD(b)) );


class SoEImageBase  
{
public:
	SoEImageBase();
	virtual ~SoEImageBase();

	//must implement in derived classes
	virtual BOOL Validate(const CHAR *head, const int headsize, const CHAR *foot, const int footsize) = 0;
	virtual SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType ) = 0;

protected:
	void			SetupBitmapInfo();
    BOOL			AllocateStorage( int w, int h, unsigned char bpp );
	BOOL			AllocateStorage( unsigned long size);
	BOOL			DeAllocateStorage(void);
    BOOL			OpenFile( const char *filename );
    BOOL			CloseFile(void);
    BOOL			CreateSurface(SoESurface* pSurface, BYTE memType);
    CHAR*           GetStoragePtr(void)				{ return m_pStorage; }
	BOOL			IsInverted(void)				{ return m_IsInverted; }
	BOOL			IsMirrored(void)				{ return m_IsMirrored; }
    void			SetInverted(BOOL b)				{ m_IsInverted = b; }
	void			SetMirrored(BOOL b)				{ m_IsMirrored = b; }
    void			SetImageWidth(int w)			{ m_iWidth = w; }
    void			SetImageHeight(int h)			{ m_iHeight = h; }
    void			SetImageBPP(unsigned char bpp)	{ m_cBpp = bpp; }
    int				GetImageWidth(void)				{ return m_iWidth; }
    int				GetImageHeight(void)			{ return m_iHeight; }
    unsigned char	GetImageBPP(void)               { return m_cBpp; }
    unsigned long	GetSizeInBytes(void)			{ return m_iWidth * m_iHeight * (m_cBpp>>3); }
	SOE_LPDIRECTDRAWSURFACE CopyBits(void);
    BOOL			CreateSurface(SOE_LPDIRECTDRAW lpDD, BYTE memType);	

protected:
	BITMAPINFO*     m_pBitmapInfo;  // 
	BITMAPINFO		m_BitmapInfo;   // 
	SoEFile			m_file;         // file handler
    int				m_iWidth;		// signed because if -, that means image is backwards.
    int				m_iHeight;		// signed because if -, that means image is upside down.
    unsigned char   m_cBpp;			// char, valids are only 1,2,4,8,16,24,32
	CHAR*           m_pStorage;     // char, 8bit or less is accessed by byte
    BOOL            m_IsInverted;   // 
	BOOL			m_IsMirrored;   // 
	PALETTEENTRY*   m_pPalette;     // 
	unsigned int	m_iUsage;		// DIB_RGB_COLORS or DIB_PAL_COLORS
	SOE_LPDIRECTDRAWSURFACE m_lpdds;	
};

#endif SOE_IMAGEBASE_H
