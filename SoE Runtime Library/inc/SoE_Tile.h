/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_TILE_H
#define SOE_TILE_H

#include "SoE_Surface.h"

#define SOEBLT_BLKALPHA25			11
#define SOEBLT_TRANSALPHA25 		111

struct SOE_ANIMSTRUCT
{
	INT32 framerate, pause, rnd;
    SOEBOOL pingpong;
	
	INT32 *anim, num;
};

class SOEAPI SoETile : public SoESurface
{
public:
	SoETile();
	virtual ~SoETile();

	SOEBOOL Create(SoEScreen *pScreen, const char* szFilename, int w, int h, int num, BYTE memoryType = SOEMEM_VIDTHENSYS);
    SOEBOOL Create(SoEScreen *pScreen, LONG lSize, fstream* fs, int w, int h, int num, BYTE memoryType = SOEMEM_VIDTHENSYS);
    SOEBOOL Create(SoEScreen *pScreen, LONG lSize, CHAR* lpCache, int w, int h, int num, BYTE memoryType = SOEMEM_VIDTHENSYS);
	SOEBOOL CreateFromTLE(SoEScreen *pScreen, const char* szFilename , BYTE memoryType = SOEMEM_VIDTHENSYS );

	const RECT *GetBlockRect(int block);

 // Anim stuff

	// adds animation frames to a specific index
	virtual SOEBOOL AddAnim( int framerate, int pause, int rnd, BOOL pingpong, INT32 *anim, int num );
	virtual SOEBOOL AddAnimSection( int framerate, int pause, int rnd, BOOL pingpong, int blockstart, int blockend );
	virtual SOEBOOL InsertTile( int anim, int pos, int tile );
	virtual SOEBOOL DelAnim( int pos );
	virtual SOEBOOL RemoveTile( int anim, int pos );
	virtual INT32 GetAnimTile( int tile );
	virtual void Draw( int x, int y, SoESurface *s, int BlitType, int tile );
	virtual void DrawFirst( int x, int y, SoESurface *s, int BlitType, int tile );
	virtual void NextFrame( );
	virtual SOEBOOL LoadAnim( FILE *fptr );
	virtual SOEBOOL SaveAnim( FILE *fptr );
	virtual inline INT32 GetAnimCount( void )	{	return m_Count;	};
	virtual SOE_ANIMSTRUCT * GetAnimData( int nr );

private:
	SOE_ANIMSTRUCT *m_Data;
	int m_Count, m_FrameCtr;
 // Anim stuff end 

public:
	INT32 GetBlockWidth() { return m_BlockWidth; }
	INT32 GetBlockHeight() { return m_BlockHeight; }
	INT32 GetBlockCount() { return m_BlockNum; }

protected:
	void Init();
	void SetSrc(INT32 block) { SoESurface::SetClipRect(&m_BlockRects[block]); }

public:
	INT32 m_BlockWidth;		// The width of one tile, in pixels
	INT32 m_BlockHeight;	// The height of one tile, in pixels
	INT32 m_BlockNum;		// The number of tiles in the bitmap file
	INT32 m_BlockX;			// The number of tiles (x)
	INT32 m_BlockY;			// The number of tiles (y)

private:
	RECT *m_BlockRects;    // Pointer to thelist of block rects
};

#endif