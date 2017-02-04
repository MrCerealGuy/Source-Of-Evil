/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_SURFACE_H
#define SOE_SURFACE_H

#include <fstream.h>

#include "SoE_Screen.h"
#include "SoE_Resource.h"

// SOE BLT types
#define SOEBLT_BLK				0
#define SOEBLT_BLKALPHA			1
#define SOEBLT_BLKALPHAFAST		2
#define SOEBLT_BLKSCALED		3
#define SOEBLT_BLKSHADOW		4
#define SOEBLT_BLKSHADOWFAST	5
#define SOEBLT_BLKROTATED		6
#define SOEBLT_BLKROTOZOOM		7
#define SOEBLT_BLKHFLIP			8
#define SOEBLT_BLKVFLIP			9
#define SOEBLT_BLKSTRETCHED     10

#define SOEBLT_TRANS			100
#define SOEBLT_TRANSALPHA		101
#define SOEBLT_TRANSALPHAFAST	102
#define SOEBLT_TRANSSCALED		103
#define SOEBLT_TRANSSHADOW		104
#define SOEBLT_TRANSSHADOWFAST	105
#define SOEBLT_TRANSROTATED		106
#define SOEBLT_TRANSROTOZOOM	107
#define SOEBLT_TRANSHFLIP		108
#define SOEBLT_TRANSVFLIP		109
#define SOEBLT_TRANSSTRETCHED   110

#define SOEBLT_TRANSALPHAMASK	120

// SOE memory types
#define SOEMEM_VIDEOONLY		0
#define SOEMEM_SYSTEMONLY		1
#define SOEMEM_VIDTHENSYS		2

// SOE SIMD instuction set types
#define SOESIMD_NONE			0
#define SOESIMD_MMX		        2
#define SOESIMD_3DNOW		    4

struct TexMapTable
{
	int x;
	int px;
	int py;
};

class SOEAPI SoESurface
{
public:
	
	SoESurface();
	SoESurface(SoESurface& aSurface);
	virtual ~SoESurface();

	HRESULT Create(SoEScreen* pScreen, int Width, int Height, BYTE memoryType = SOEMEM_VIDTHENSYS);
	HRESULT Create(SoEScreen* pScreen, const char *szFilename, BYTE memoryType = SOEMEM_VIDTHENSYS);
	HRESULT Create(SoEScreen* pScreen, LONG size, FILE* fh, BYTE memoryType = SOEMEM_VIDTHENSYS);
	//HRESULT Create(SoEScreen* pScreen, SoEResource* r, RESHANDLE i, BYTE memoryType = SOEMEM_VIDTHENSYS);
    HRESULT Create(SoEScreen* pScreen, LONG size, fstream* fs, BYTE memoryType = SOEMEM_VIDTHENSYS);
    HRESULT Create(SoEScreen* pScreen, LONG size, CHAR* lpCache, BYTE memoryType = SOEMEM_VIDTHENSYS);
	
    inline HRESULT PutPixel(int X, int Y, DWORD Col) { return (*this.*m_fpPutPixel)(X, Y, Col); }
    inline HRESULT PutAAPixel(int X, int Y, DWORD Col, WORD ALPHA) { return (*this.*m_fpPutAAPixel)(X, Y, Col, ALPHA); }
    inline DWORD   GetPixel(int X, int Y) { return (*this.*m_fpGetPixel)(X, Y); }
    inline HRESULT VLine(int Y1, int Y2, int X, DWORD Col) { return (*this.*m_fpVLine)(Y1, Y2, X, Col); }
    inline HRESULT HLine(int X1, int X2, int Y, DWORD Col) { return (*this.*m_fpHLine)(X1, X2, Y, Col); }
	HRESULT Rect(int X1,int Y1,int X2,int Y2,DWORD Col);
	HRESULT FillRect(int X1, int Y1, int X2, int Y2, DWORD Col);
	HRESULT Line(int X1, int Y1, int X2, int Y2, DWORD Col);
	HRESULT AALine(int X1, int Y1, int X2, int Y2, DWORD Col);
	HRESULT Circle(int X, int Y, int Radius, DWORD Col);
	HRESULT FillCircle(int X, int Y, int Radius, DWORD Col);
	HRESULT RoundedRect(int X1, int Y1, int X2, int Y2, int Radius, DWORD Col);
    HRESULT Fill(DWORD FillColor);
	HRESULT InvertColors( int x1, int y1, int x2, int y2 );
    HRESULT TextXY(int X, int Y, COLORREF Col, LPCTSTR pString);
	HRESULT DrawText(LPCSTR pString, COLORREF col, LPRECT pRect);
    HRESULT SaveAsBMP(const char * szFilename);
    BOOL ValidateBlt(SoESurface* lpSoES, LONG *lDestX, LONG *lDestY, RECT *srcRect);
	BOOL ClipRect(RECT *Rect);
    BOOL LoadAlphaMask(const char* szFilename);

	HRESULT DrawBlk(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawBlkSW(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTrans(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransSW(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawBlkHFlip(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransHFlip(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawBlkVFlip(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransVFlip(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawBlkAlphaFast(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransAlphaFast(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawBlkAlpha(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, WORD ALPHA = 0);
	HRESULT DrawTransAlpha(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, WORD ALPHA = 0);
	HRESULT DrawBlkShadow(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, WORD SHADOW = 0);
	HRESULT DrawTransShadow(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, WORD SHADOW = 0);
	HRESULT DrawBlkShadowFast(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransShadowFast(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransAlphaMask(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL);
	HRESULT DrawTransScaled(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, float scaleFactor = 1);
	HRESULT DrawBlkScaled(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, float scaleFactor = 1);
	HRESULT DrawBlkStretched(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, LONG width = 0, LONG height = 0);
	HRESULT DrawTransStretched(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, LONG width = 0, LONG height = 0);
	HRESULT Fade(SoESurface* dest, LONG lDestX, LONG lDestY, RECT* srcRect = NULL, WORD ALPHA = 0);
    HRESULT DrawBlkRotoZoom(SoESurface* dest, int midX, int midY, RECT* area, double angle, double scale);
    HRESULT DrawTransRotoZoom(SoESurface* dest, int midX, int midY, RECT* area, double angle, double scale);
    




    HRESULT     SetColorKey(DWORD col);
	HRESULT     SetColorKey(void);
    DWORD       GetColorKey(void) { return m_ColorKey; }
	HRESULT     Restore(void);
	HRESULT     ChangeFont(const char* FontName, int Width, int Height, int Attributes = FW_NORMAL);
	HRESULT     SetFont(void);
	HRESULT     Lock(void);
	HRESULT     UnLock(void);
	HDC         GetDC();
	HRESULT     ReleaseDC();
    void        SetTextureEnabled( BOOL onOff ) { m_TextureEnabled = onOff; }
    BOOL        GetTextureEnabled( void ) { return m_TextureEnabled; }
    int         GetWidth( void ) { return m_PixelWidth;  }
    int         GetHeight( void ) { return m_PixelHeight; }
    DWORD       GetPitch( void ) { return m_DDSD.lPitch; }
    void*       GetSurfacePointer( void ) { return m_DDSD.lpSurface; }
    RECT*       GetClipRect(void) { return &m_ClipRect; }
	void        SetClipRect(RECT *clipRect);
    char*       GetFileName(char* name);
    SoEScreen*  GetScreen(void) { return m_Screen; }
    DWORD       GetSIMDType(void) { return m_SIMDInstrctionSet; }
    char*       GetFontName(char* name);
    BYTE*       GetAlphaMaskPointer(void) { return m_AlphaMask; }
    HRESULT     SetAlphaMaskPointer(BYTE* lpAlpha);
    void        SetNext(SoESurface* lpSoES) { m_Next = lpSoES; }
    void        SetPrev(SoESurface* lpSoES) { m_Prev = lpSoES; }
    SoESurface* GetNext(void) { return m_Next; }
    SoESurface* GetPrev(void) { return m_Prev; }
    DWORD       GetSIMDInstrctionSet(void) { return m_SIMDInstrctionSet; }
	void        GetPixelFormat(RGBFORMAT* format);
    HRESULT     ReLoadBitmap(const char* filename = NULL);
    void        FunctionMapper(void);
    void        SetCaptureFileName(char* name) { strncpy(m_CaptureFileName, name, 256); }
    char*       GetCaptureFileName(void) { return m_CaptureFileName; }
    LONG        GetSurfaceId(void) { return m_SurfaceId; }
	void GetSurfaceDescriptor(SOE_LPDDSURFACEDESC lpddsd);
	SOE_LPDIRECTDRAWSURFACE GetDDS( void ) { return m_lpDDS; }


protected:  // protected member functions
    void InitSoESurface( void );
    friend void SoEScreen::SetSurfaceScreen(SoESurface* surface);
    friend void SoEScreen::SetSurfaceWidth(SoESurface* surface, int w);
    friend void SoEScreen::SetSurfaceHeight(SoESurface* surface, int h);
    friend void SoEScreen::SetSurfacePixelFormat(SoESurface* surface);
	friend void SoEScreen::SetSurfaceDDSPointer(SoESurface* pSoES, SOE_LPDIRECTDRAWSURFACE lpdds);
    void TextureMap(SoESurface *dest, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, RECT* area, BOOL bTrans);
    void Scanleftside(int x1,int x2,int ytop,int lineheight,char side,int TexWidth,int TexHeight,RECT* dClip);
    void Scanrightside(int x1,int x2,int ytop,int lineheight,char side,int TexWidth,int TexHeight,RECT* dClip);
	
public:  // Private member data (all data should be private)

	SOE_DDSURFACEDESC       m_DDSD;		// A DDSURFACEDESC object, stores surface information
	SOE_LPDIRECTDRAWSURFACE m_lpDDS;		// The LPDIRECTDRAWSURFACE object
	int         m_PixelWidth;			// The surface width in pixels
	int         m_PixelHeight;			// The surface height in pixels
	char        m_FileName[256];		// The name of the bitmap file loaded, used in Restore
    char        m_CaptureFileName[256];
	HDC         m_DC;					// A HDC object, used for drawing text to the surface
	DWORD       m_ColorKey;				// The specified color key for this surface
	HFONT       m_Font;					// A HFONT object describing the currently selected font
	SoEScreen*  m_Screen;				// A pointer to a SoEScreen object
	DWORD       m_SIMDInstrctionSet;    // Type of SIMD instuction set available
	RECT        m_ClipRect;             // The rectangle to which drawing operations are clipped
	RGBFORMAT   m_PixelFormat;          // Structure that holds the information about the color depths, etc.
	char        m_FontName[256];        // Current font associated to the surface
	BOOL        m_TextureEnabled;       // Tells if this surface was enabled to be a texure
	BYTE*       m_AlphaMask;            // Pointer to the surfaces alpha mask
    SoESurface* m_Next;                 // Used in linked list of surfaces
    SoESurface* m_Prev;                 // Used in linked list of surfaces
    DWORD       m_LockCount;            // Reference count to avoid mulitple lock/Unlocks
    DWORD       m_DeviceContextCount;   // Reference count to avoid mulitple GetDC/ReleaseDC
    DWORD       m_MemoryLocation;       // What type of memory the surface is stored in
    LONG        m_SurfaceId;

    TexMapTable* m_leftTable;
    TexMapTable* m_rightTable;

    //Function  Pointer declarations
    HRESULT (SoESurface::* m_fpPutPixel)(int X, int Y, DWORD Col);
	HRESULT (SoESurface::* m_fpPutAAPixel)(int X, int Y, DWORD Col, WORD ALPHA);
	DWORD   (SoESurface::* m_fpGetPixel)(int X, int Y);
	HRESULT (SoESurface::* m_fpVLine)(int Y1, int Y2, int X, DWORD Col);
	HRESULT (SoESurface::* m_fpHLine)(int X1, int X2, int Y, DWORD Col);
    HRESULT (SoESurface::* fpDrawBlk)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawDDBlk)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTrans)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawDDTrans)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawBlkHFlip)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransHFlip)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawBlkVFlip)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransVFlip)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawBlkAlphaFast)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransAlphaFast)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawBlkAlpha)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, WORD ALPHA);
	HRESULT (SoESurface::* fpDrawTransAlpha)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, WORD ALPHA);
	HRESULT (SoESurface::* fpDrawBlkShadow)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, WORD SHADOW);
	HRESULT (SoESurface::* fpDrawTransShadow)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, WORD SHADOW);
	HRESULT (SoESurface::* fpDrawBlkShadowFast)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransShadowFast)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransAlphaMask)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect);
	HRESULT (SoESurface::* fpDrawTransScaled)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, float scaleFactor);
	HRESULT (SoESurface::* fpDrawBlkScaled)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, float scaleFactor);
	HRESULT (SoESurface::* fpDrawBlkStretched)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect, LONG width, LONG height);
	HRESULT (SoESurface::* fpDrawTransStretched)(SoESurface* lpSoES, LONG lDestX, LONG lDestY, RECT srcRect, LONG width, LONG height);
	HRESULT (SoESurface::* fpFade)(SoESurface* dest, LONG lDestX, LONG lDestY, RECT srcRect, WORD ALPHA);

    // Specific video mode function declarations
    HRESULT PutPixel8(int X, int Y, DWORD Col);
    HRESULT PutPixel16(int X, int Y, DWORD Col);
    HRESULT PutPixel24(int X, int Y, DWORD Col);
    HRESULT PutPixel32(int X, int Y, DWORD Col);

    HRESULT PutAAPixel8(int X, int Y, DWORD Col, WORD ALPHA);
    HRESULT PutAAPixel15(int X, int Y, DWORD Col, WORD ALPHA);
    HRESULT PutAAPixel16(int X, int Y, DWORD Col, WORD ALPHA);
    HRESULT PutAAPixel24(int X, int Y, DWORD Col, WORD ALPHA);
    HRESULT PutAAPixel32(int X, int Y, DWORD Col, WORD ALPHA);

    DWORD   GetPixel8(int X, int Y);
    DWORD   GetPixel16(int X, int Y);
    DWORD   GetPixel24(int X, int Y);
    DWORD   GetPixel32(int X, int Y);

    HRESULT VLine8(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine16(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine24(int Y1, int Y2, int X, DWORD Col);
    HRESULT VLine32(int Y1, int Y2, int X, DWORD Col);

    HRESULT HLine8(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine16(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine24(int Y1, int Y2, int X, DWORD Col);
    HRESULT HLine32(int Y1, int Y2, int X, DWORD Col);

};

#endif
