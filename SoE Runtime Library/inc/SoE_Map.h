/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_MAP_H
#define SOE_MAP_H

#include "SoE_Tile.h"

class SOEAPI SoEMapCell
{
public:
	SoEMapCell();
	virtual ~SoEMapCell();

	virtual void Save(FILE *fp);
	virtual void Load(FILE *fp);
	virtual int GetTile();
	virtual void SetTile(int Tile);
private:
	int TileNum;
};

class SOEAPI SoEMap
{
public:
	SoEMap(SoEScreen *pScreen);
	SoEMap(SoETile *pTiles, SoEScreen *pScreen);
	virtual ~SoEMap();

	// creation functions
	virtual BOOL Create(int Width, int Height, int Fill);

	virtual BOOL Load( FILE *fptr );
	virtual BOOL Load(const char *szFilename);
	virtual BOOL Save( FILE *fptr );
	virtual BOOL Save(const char *szFilename);
	virtual void Clear(void);
	virtual void Fill(int TileNum);

	// drawing functions
	virtual void Draw(SoESurface* lpDDS, int BlitStyle = 0, BOOL Wrap=TRUE );
	virtual void DrawTrans(SoESurface* lpDDS);
	virtual void DrawClipped(SoESurface* lpDDS, LPRECT ClipRect);

	// location functions
	virtual void MoveTo(int PosX, int PosY);
	virtual int GetPosX();
	virtual int GetPosY();
	virtual void ScrollUp(int Offset);
	virtual void ScrollDown(int Offset);
	virtual void ScrollLeft(int Offset);
	virtual void ScrollRight(int Offset);
	void EnableScrollWrapping(BOOL bEnable);
	virtual void ScreenTileSize(int Width, int Height);
	virtual void SetViewPort(LPRECT View);
	virtual RECT GetTileCoord(int MapX, int MapY);
	virtual int  GetTile(int MapX, int MapY);
	virtual SoEMapCell *GetCell(int MapX, int MapY);
	virtual void SetTile(int MapX, int MapY, int Tile);
	virtual void LoadTiles(SoETile *pTiles);
	virtual BOOL InitData(int iSIZE, int iFill);
	virtual int GetMapWidth();
	virtual int GetMapHeight();
	virtual int GetTileWidth();
	virtual int GetTileHeight();
	virtual int GetScreenWidth();
	virtual int GetScreenHeight();
	virtual int GetScreenBlocksWidth();
	virtual int GetScreenBlocksHeight();

protected:
	void Init();

protected:
	int m_PosX;			// The current map X position in pixels
	int m_PosY;			// The current map Y position in pixels
	int m_Width;		// The map width in map units
	int m_Height;		// The map height in map units
	int m_TileWidth;	// Tile width in pixels, from the SoETile pointer
	int m_TileHeight;	// Tile height in pixels, from the SoETile pointer
	int m_PixelWidth;	// The width of the map in pixels
	int m_PixelHeight;	// The height of the map in pixels
	int m_ViewportX;	// The starting X position of the Viewport
	int m_ViewportY;	// The starting Y position of the Viewport
	int m_SCREEN_TW;		// Number of tiles wide on screen
	int m_SCREEN_TH;		// Number of tiles high on screen
	int m_SCREEN_W;		// Screen pixel width, from SoEScreen pointer
	int m_SCREEN_H;		// Screen pixel height, from SoEScreen pointer
	int m_SIZE;			// Map size, width * height

protected:
	SoEMapCell *m_DATA;		// Pointer to the SoEMapCell object for tiles
	SoEScreen *m_Screen;	// Pointer to the SoEScreen object
	SoETile *m_Tiles;		// Pointer to the SoETile object

	BOOL m_bScrollWrapping;	// turn this on to wrap while you scroll
};

class SOEAPI SoEIsoMap : public SoEMap
{
public:
	SoEIsoMap(SoETile *pTiles, SoEScreen *pScreen):SoEMap(pTiles, pScreen) {};
    virtual void ScrollUp(int);
    virtual void ScrollDown(int);
    virtual void ScrollLeft(int);
    virtual void ScrollRight(int);
    virtual void BltTileTrans(SoESurface*, int, int, int, int, int, int, int, int);
    virtual void DrawTrans(SoESurface*);
	virtual POINT ProcessClicks(int, int);
};

#endif
