/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Map.h"
#include "SoE_Sprite.h"

SoEMap::SoEMap(SoEScreen *pScreen)
{
	// initialize the class so we have a fresh start
	Init();
	
	m_SCREEN_W = pScreen->GetWidth();
	m_SCREEN_H = pScreen->GetHeight();
}

SoEMap::SoEMap(SoETile *pTiles, SoEScreen *pScreen)
{
	// initialize the class so we have a fresh start
	Init();
	
	m_Tiles = pTiles;
	m_Screen = pScreen;
	
	m_TileWidth = m_Tiles->m_BlockWidth;
	m_TileHeight = m_Tiles->m_BlockHeight;
	
	m_SCREEN_W = pScreen->GetWidth();
	m_SCREEN_H = pScreen->GetHeight();
	
	m_SCREEN_TW = m_SCREEN_W / m_TileWidth;
	m_SCREEN_TH = m_SCREEN_H / m_TileHeight;
	
	LoadTiles(pTiles);
}

SoEMap::~SoEMap()
{
	// delete the map data
	DELETEARRAY(m_DATA);
	
	
	// get rid of our reference to the tiles
	m_Tiles = NULL;
	
	// do the same with the screen reference
	m_Screen = NULL;
}

void SoEMap::Init()
{
	m_bScrollWrapping = FALSE;
	
	m_DATA = NULL;
	m_Tiles = NULL;
	m_Screen = NULL;
	
	m_TileWidth = 0;
	m_TileHeight = 0;
	
	m_PosX = 0;
	m_PosY = 0;
	
	m_ViewportX = 0;
	m_ViewportY = 0;
	
	m_SCREEN_W = 0;
	m_SCREEN_H = 0;
	
	m_SCREEN_TW = 0;
	m_SCREEN_TH = 0;
}

BOOL SoEMap::Create(int Width, int Height, int Fill)
{
	m_Width = Width;
	m_Height = Height;
	
	m_PixelWidth = m_Width * m_TileWidth;
	m_PixelHeight = m_Height * m_TileHeight;
	
	//  CHANGE:  Per David Maziarka 05-22-2000
	// m_SCREEN_TW = min(m_SCREEN_W, m_Width);
	// m_SCREEN_TH = min(m_SCREEN_H, m_Height);
    if (m_TileWidth)
		m_SCREEN_TW = m_SCREEN_W / m_TileWidth;
    if (m_TileHeight)
		m_SCREEN_TH = m_SCREEN_H / m_TileHeight;
	//  CHANGE:  Per David Maziarka 05-22-2000
	
	m_SIZE = m_Width * m_Height;
	return InitData(m_SIZE, Fill);
}

BOOL SoEMap::InitData(int nSIZE, int iFill)
{
	m_DATA = new SoEMapCell[nSIZE];
	if (m_DATA == NULL)
		return FALSE;
	
	for (int i = 0; i < m_SIZE; i++)
		m_DATA[i].SetTile(iFill);
	
	return TRUE;
}

BOOL SoEMap::Load(FILE *fptr)
{
	if (fptr == NULL)
		return FALSE;
	
	fread(&m_Width, sizeof(int), 1, fptr);
	fread(&m_Height, sizeof(int), 1, fptr);
	
	if (Create(m_Width, m_Height, 0) == FALSE)
		return FALSE;
	
    for (int x = 0; x < m_SIZE; x++)
		m_DATA[x].Load(fptr);
	
	return TRUE;
}

BOOL SoEMap::Load(const char *szFilename)
{
	FILE *fp;
	BOOL rval;
	
	if (szFilename == NULL)
		return FALSE;
	
	fp = fopen(szFilename, "rb");
	if (fp == NULL)
		return FALSE;
	
	rval = Load(fp);
	if (rval == TRUE && m_Tiles)
		rval = m_Tiles->LoadAnim(fp);
	
	fclose(fp);
	
	return rval;
}

BOOL SoEMap::Save(FILE *fptr)
{
	if (fptr == NULL)
		return FALSE;
	
	fwrite(&m_Width, sizeof(int), 1, fptr);
	fwrite(&m_Height, sizeof(int), 1, fptr);
	
	m_SIZE = m_Width * m_Height;
	
	for (int x = 0; x < m_SIZE; x++)
		m_DATA[x].Save(fptr);
	
	return TRUE;
}

BOOL SoEMap::Save(const char *szFilename)
{
	FILE *fp;
	BOOL retval;
	
	fp = fopen(szFilename, "wb");
	retval = Save(fp);
	if (retval)
		retval = m_Tiles->SaveAnim(fp);
	
	fclose(fp);
	
	return retval;
}

void SoEMap::Clear()
{
	for (int i = 0; i < m_SIZE; i++)
		m_DATA[i].SetTile(0);
}

void SoEMap::Fill(int TileNum)
{
	for (int i = 0; i < m_SIZE; i++)
		m_DATA[i].SetTile(TileNum);
}

void SoEMap::MoveTo(int PosX, int PosY)
{
	m_PosX = PosX;
	m_PosY = PosY;
}

int SoEMap::GetPosX()
{
	return m_PosX;
}

int SoEMap::GetPosY()
{
	return m_PosY;
}

void SoEMap::ScrollUp(int Offset)
{
	if (m_bScrollWrapping)
	{
		if (m_PosY - Offset >= 0)
			m_PosY -= Offset;
		else 
			m_PosY = (m_Height * m_TileHeight) - Offset + m_PosY;
	}
	else
	{
		m_PosY -= Offset;
		if (m_PosY < 0)
			m_PosY = 0;
	}
}

void SoEMap::ScrollDown(int Offset)
{
	if (m_bScrollWrapping)
	{
		if (m_PosY + Offset <(m_Height * m_TileHeight))
			m_PosY += Offset;
		else 
			m_PosY = Offset -(m_Height * m_TileHeight) + m_PosY;
	}
	else
	{
		m_PosY += Offset;
		if (m_PosY >(m_Height - m_SCREEN_TH) * m_TileHeight)
			m_PosY = max(0, (m_Height - m_SCREEN_TH) * m_TileHeight);
	}
}

void SoEMap::ScrollLeft(int Offset)
{
	if (m_bScrollWrapping)
	{
		if (m_PosX - Offset >= 0)
			m_PosX -= Offset;
		else 
			m_PosX = (m_Width * m_TileWidth) - Offset + m_PosX;
	}
	else
	{
		m_PosX -= Offset;
		if (m_PosX < 0)
			m_PosX = 0;
	}
}

void SoEMap::ScrollRight(int Offset)
{
	if (m_bScrollWrapping)
	{
		if (m_PosX + Offset <(m_Width * m_TileWidth))
			m_PosX += Offset;
		else 
			m_PosX = Offset -(m_Width * m_TileWidth) + m_PosX;
	}
	else
	{
		m_PosX += Offset;
		if (m_PosX >(m_Width - m_SCREEN_TW) * m_TileWidth)
			m_PosX = max(0, (m_Width - m_SCREEN_TW) * m_TileWidth);
	}
}

void SoEMap::EnableScrollWrapping(BOOL bEnable)
{
	m_bScrollWrapping = bEnable;
}

void SoEMap::ScreenTileSize(int Width, int Height)
{
	m_SCREEN_TW = Width;
	m_SCREEN_TH = Height;
}

int SoEMap::GetMapWidth()
{
	return m_Width;
}

int SoEMap::GetMapHeight()
{
	return m_Height;
}

int SoEMap::GetScreenWidth()
{
	return m_SCREEN_W;
}

int SoEMap::GetScreenHeight()
{
	return m_SCREEN_H;
}

int SoEMap::GetScreenBlocksWidth()
{
	return m_SCREEN_TW;
}

int SoEMap::GetScreenBlocksHeight()
{
	return m_SCREEN_TH;
}

void SoEMap::SetViewPort(LPRECT View)
{
	m_ViewportX = View->left;
	m_ViewportY = View->top;
	m_SCREEN_TW = (View->right - View->left) / m_TileWidth;
	m_SCREEN_TH = (View->bottom - View->top) / m_TileHeight;
}

RECT SoEMap::GetTileCoord(int MapX, int MapY)
{
	RECT TileCoord;
	TileCoord.left = (m_PosX -(m_TileWidth * MapX)) * -1;
	TileCoord.top = (m_PosY -(m_TileHeight * MapY)) * -1;
	TileCoord.right = TileCoord.left + m_TileWidth;
	TileCoord.bottom = TileCoord.top + m_TileHeight;
	return TileCoord;
}

SoEMapCell *SoEMap::GetCell(int MapX, int MapY)
{
	if (MapX<0 || MapY < 0 || MapX >= m_Width || MapY >= m_Height)
		return 0;
	return &m_DATA[MapX +(MapY * m_Width)];
}

int SoEMap::GetTile(int MapX, int MapY)
{
	if (MapX<0 || MapY < 0 || MapX >= m_Width || MapY >= m_Height)
		return 0;
	return m_DATA[MapX +(MapY * m_Width)].GetTile();
}

void SoEMap::SetTile(int MapX, int MapY, int Tile)
{
	if (MapX >= 0 && MapY >= 0 && MapX<m_Width && MapY < m_Height)
		m_DATA[MapX +(MapY * m_Width)].SetTile(Tile);
}

void SoEMap::LoadTiles(SoETile *pTiles)
{
	m_Tiles = pTiles;
	
	m_TileWidth = m_Tiles->GetBlockWidth();
	m_TileHeight = m_Tiles->GetBlockHeight();
	
	m_SCREEN_TW = m_SCREEN_W / m_TileWidth;
	m_SCREEN_TH = m_SCREEN_H / m_TileHeight;
}

int SoEMap::GetTileWidth()
{
	return m_TileWidth;
}

int SoEMap::GetTileHeight()
{
	return m_TileHeight;
}

void SoEMap::Draw(SoESurface* lpDDS, int BlitStyle, BOOL Wrap)
{
	int i, j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;
	int yidx = m_PosY/m_TileHeight;
	int xidx;
	
	xoffset = m_PosX % m_TileWidth;
	yoffset = m_PosY % m_TileHeight;
	
	// NEXT X NUMBER OF ROWS
	for (j = 0, ycoord = m_ViewportY; j < m_SCREEN_TH + 1; j++)
	{
		xcoord = m_ViewportX;
		xidx = m_PosX/m_TileWidth;
		
		for (i = 0; i < m_SCREEN_TW + 1; i++)
		{
			int tile = m_DATA[yidx*m_Width + xidx].GetTile();
			m_Tiles->Draw(xcoord - xoffset, ycoord - yoffset, lpDDS, BlitStyle, tile);
			xcoord += m_TileWidth;
			xidx++;
			if (xidx >= m_Width)
			{	
				if (Wrap)
					xidx = 0;
				else 
					break;
			}
		}
		
		yidx++;
		if (yidx >= m_Height)
		{
			if (Wrap)
				yidx = 0;
			else 
				break;
		}
		ycoord += m_TileHeight;
	}
	
	m_Tiles->NextFrame();
}

void SoEMap::DrawTrans(SoESurface* lpDDS)
{
	int i, j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;
	int yidx = m_PosY/m_TileHeight;
	int xidx;
	
	xoffset = m_PosX % m_TileWidth;
	yoffset = m_PosY % m_TileHeight;
	
	// NEXT X NUMBER OF ROWS
	for (j = 0, ycoord = m_ViewportY; j < m_SCREEN_TH + 1; j++)
	{
		xcoord = m_ViewportX;
		xidx = m_PosX/m_TileWidth;
		
		for (i = 0; i < m_SCREEN_TW + 1; i++)
		{
			int tile = m_DATA[yidx*m_Width + xidx].GetTile();
			m_Tiles->Draw(xcoord - xoffset, ycoord - yoffset, lpDDS, SOEBLT_TRANS, tile);
			xcoord += m_TileWidth;
			xidx++;
			if (xidx >= m_Width)
				xidx = 0;
		}
		
		yidx++;
		if (yidx >= m_Height)
			yidx = 0;
		ycoord += m_TileHeight;
	}
	
	m_Tiles->NextFrame();
}

void SoEMap::DrawClipped(SoESurface* lpDDS, LPRECT ClipRect)
{
	// This thing doesn't clip yet, we need to fix that.
	
	int i, j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;
	int yidx = m_PosY/m_TileHeight;
	int xidx;
	
	xoffset = m_PosX % m_TileWidth;
	yoffset = m_PosY % m_TileHeight;
	
	// NEXT X NUMBER OF ROWS
	for (j = 0, ycoord = m_ViewportY; j < m_SCREEN_TH + 1; j++)
	{
		xcoord = m_ViewportX;
		xidx = m_PosX/m_TileWidth;
		
		for (i = 0; i < m_SCREEN_TW + 1; i++)
		{
			int tile = m_DATA[yidx*m_Width + xidx].GetTile();
			m_Tiles->Draw(xcoord - xoffset, ycoord - yoffset, lpDDS, 0, tile);
			xcoord += m_TileWidth;
			xidx++;
			if (xidx >= m_Width)
				xidx = 0;
		}
		
		yidx++;
		if (yidx >= m_Height)
			yidx = 0;
		ycoord += m_TileHeight;
	}
	
	m_Tiles->NextFrame();
}
