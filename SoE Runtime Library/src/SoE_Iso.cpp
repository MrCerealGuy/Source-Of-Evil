/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Map.h"

void SoEIsoMap::ScrollUp(int Offset)
{
	m_PosY -= Offset;
	if (m_PosY < -1)
		m_PosY = -1;
}

void SoEIsoMap::ScrollDown(int Offset)
{
	m_PosY += Offset;
	if (m_PosY > m_Height -(m_SCREEN_TH*2))
		m_PosY = m_Height -(m_SCREEN_TH*2);
}

void SoEIsoMap::ScrollLeft(int Offset)
{
	m_PosX -= Offset;
	if (m_PosX < 0)
		m_PosX = 0;
}

void SoEIsoMap::ScrollRight(int Offset)
{
	m_PosX += Offset;
	if (m_PosX >(m_Width - m_SCREEN_TW))
		m_PosX = (m_Width - m_SCREEN_TW);
}

void SoEIsoMap::BltTileTrans(SoESurface* lpDDS, int xdest, int ydest, int w, int h, int xoff, int yoff, int x, int y)
{
	HRESULT rval;
	RECT src;
	int tile_num;
	int x1, y1;
	int mapx, mapy;
    int index;
	
	// mapx = ((m_PosX + xdest) % m_PixelWidth) / m_TileWidth;
	// mapy = ((m_PosY + ydest) % m_PixelHeight) / m_TileHeight;
	
	mapx = x;
	mapy = y;
	
    index = (mapy * m_Width) + mapx;
    if (index >=(m_Width * m_Height))
        return;
	
	// tile_num = m_DATA[(mapy * m_Width) + mapx].GetTile();
    tile_num = m_DATA[index].GetTile();
	
	if (tile_num == 0)
		return;
	tile_num = tile_num & 0x0000000f;
	
	int TILE_SW = m_Tiles->GetWidth() / m_TileWidth;
	
	x1 = tile_num % TILE_SW;
	x1 = (x1 * m_TileWidth) + xoff;
	
	y1 = tile_num / TILE_SW;
	y1 = (y1 * m_TileHeight) + yoff;
	
	src.top = y1;
	src.left = x1;
	src.bottom = y1 + h;
	src.right = x1 + w;
	
	rval = lpDDS->GetDDS()->BltFast(xdest, ydest, m_Tiles->GetDDS(), &src, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	if (rval == DDERR_SURFACELOST)
		m_Tiles->Restore();
}

void SoEIsoMap::DrawTrans(SoESurface* lpDDS)
{
	int i, j;
	int xoffset, yoffset;
	int xcoord = 0, ycoord = 0;
	
	xoffset = m_TileWidth/2;
	yoffset = m_TileHeight/2;
	
	int y = m_PosY;
	
	int* odd;
	int* even;
	odd = new int[m_SCREEN_TW + 1];
	even = new int[m_SCREEN_TW];
	odd[0] = m_PosX;
	if ((m_PosY % 2) == 0)
	{
		for (i = 0; i < m_SCREEN_TW; i++)
		{
			even[i] = odd[i];
			odd[i + 1] = even[i] + 1;
			if (odd[i + 1] == m_Width)
				odd[i + 1] = 0;
		}
	}
	
	else 
	{
		for (i = 0; i < m_SCREEN_TW; i++)
		{
			even[i] = odd[i] + 1;
			if (even[i] == m_Width)
				even[i] = 0;
			odd[i + 1] = even[i];
		}
	}
	
	if (m_PosY != -1)
	{
		// FIRST ROW
		BltTileTrans(lpDDS, 0, 0, xoffset, yoffset, xoffset, yoffset,
			odd[0], y);
		
		for (i = 0; i < m_SCREEN_TW - 1; i++)
		{
			xcoord += m_TileWidth;
			BltTileTrans(lpDDS, xcoord - xoffset, 0, m_TileWidth, yoffset, 0, yoffset, odd[i + 1], y);
		}
		
		xcoord += m_TileWidth;
        BltTileTrans(lpDDS, xcoord - xoffset, 0, xoffset, yoffset, 0, yoffset, odd[m_SCREEN_TW] , y);
	}
	
	// NEXT X NUMBER OF ROWS
	for (j = 0; j < m_SCREEN_TH - 1; j++)
	{
		// even
		xcoord = 0;
		y += 1;
		BltTileTrans(lpDDS, 0, ycoord, m_TileWidth, m_TileHeight, 0, 0, even[0], y);
		for (i = 0; i < m_SCREEN_TW - 1; i++)
		{
			xcoord += m_TileWidth;
			BltTileTrans(lpDDS, xcoord, ycoord, m_TileWidth, m_TileHeight, 0, 0, even[i + 1], y);
		}
		
		ycoord +=(m_TileHeight/2);
		
		// Odd
		y += 1;
		xcoord = 0;
		BltTileTrans(lpDDS, 0, ycoord, xoffset, m_TileHeight, xoffset, 0, odd[0], y);
		for (i = 0; i < m_SCREEN_TW - 1; i++)
		{
			xcoord += m_TileWidth;
			BltTileTrans(lpDDS, xcoord - xoffset, ycoord, m_TileWidth, m_TileHeight, 0, 0, odd[i + 1], y);
		}
		xcoord += m_TileWidth;
		BltTileTrans(lpDDS, xcoord - xoffset, ycoord, xoffset, m_TileHeight, 0, 0, odd[m_SCREEN_TW], y);
		ycoord +=(m_TileHeight/2);
	}
	// SECOND TO LAST ROW
	xcoord = 0;
	y += 1;
	BltTileTrans(lpDDS, 0, ycoord, m_TileWidth, m_TileHeight, 0, 0, even[0], y);
	for (i = 0; i < m_SCREEN_TW - 1; i++)
	{
		xcoord += m_TileWidth;
		BltTileTrans(lpDDS, xcoord, ycoord, m_TileWidth, m_TileHeight, 0, 0, even[i + 1], y);
	}
	ycoord +=(m_TileHeight/2);
	
	if (y !=(m_Height - 1))
	{
		// LAST ROW
		y += 1;
		xcoord = 0;
		BltTileTrans(lpDDS, 0, ycoord, xoffset, yoffset, xoffset, 0, odd[0], y);
		
		for (i = 0; i < m_SCREEN_TW - 1; i++)
		{
			xcoord += m_TileWidth;
			BltTileTrans(lpDDS, xcoord - xoffset, ycoord, m_TileWidth, yoffset, 0, 0, odd[i + 1], y);
		}
		
		xcoord += m_TileWidth;
        BltTileTrans(lpDDS, xcoord - xoffset, ycoord, xoffset, yoffset, 0, 0, odd[m_SCREEN_TW], y);
	}
}

POINT SoEIsoMap::ProcessClicks(int x, int y)
{
	// translate to screen coordinates
	int mousex = x / m_TileWidth;
	int mousey = y / m_TileHeight;
	int quad, offsetx, offsety;
	
	
	if ((mousex < m_SCREEN_TW) &&(mousey < m_SCREEN_TH))
	{ // part of screen w/ map
		offsetx = x % m_TileWidth;
		offsety = y % m_TileHeight;
		
		// 0 = middle, 1 = 1st quad. , 2 = 2nd etc.
		if ((offsetx >(m_TileWidth / 2)) &&(offsety <(m_TileHeight / 2)))
		{ // 1st quadrant
			// figure out which triangle
			if (offsety < 4)
			{
				if (offsetx <=(32 +(offsety * 2)))
					quad = 0;
				else
					quad = 1;
			}
			
			else if (offsety < 11)
			{
				if (offsetx <=(33 +(offsety * 2)))
					quad = 0;
				else
					quad = 1;
			}
			
			else
			{
				if (offsetx <=(34 +(offsety * 2)))
					quad = 0;
				else
					quad = 1;
			}
		}
		
		if ((offsetx <(m_TileWidth / 2)) &&(offsety <(m_TileHeight / 2)))
		{ // 2nd quadrant
			// figure out which triangle
			if (offsety < 4)
			{
				if (offsetx  >=(31 -(offsety * 2)))
					quad = 0;
				else
					quad = 2;
			}
			
			else if (offsety < 11)
			{
				if (offsetx >=(30 -(offsety * 2)))
					quad = 0;
				else
					quad = 2;
			}
			
			else
			{
				if (offsetx >=(29 -(offsety * 2)))
					quad = 0;
				else
					quad = 2;
			}
		}
		
		if ((offsetx >(m_TileWidth / 2)) &&(offsety >(m_TileHeight / 2)))
		{ // 4th quadrant
			// figure out which triangle
			if (offsety < 20)
			{
				if (offsetx <=(32 +(offsety - 16) * 2))
					quad = 0;
				else
					quad = 4;
			}
			
			else if (offsety < 27)
			{
				if (offsetx <=(33 +(offsety - 16) * 2))
					quad = 0;
				else
					quad = 4;
			}
			
			else
			{
				if (offsetx <=(34 +(offsety - 16) * 2))
					quad = 0;
				else
					quad = 4;
			}
		}
		
		if ((offsetx <(m_TileWidth / 2)) &&(offsety >(m_TileHeight / 2)))
		{ // 3rd quadrant
			// figure out which triangle
			if (offsety < 20)
			{
				if (offsetx  >=(31 -(offsety - 16) * 2))
					quad = 0;
				else
					quad = 3;
			}
			
			else if (offsety < 27)
			{
				if (offsetx >=(30 -(offsety - 16) * 2))
					quad = 0;
				else
					quad = 3;
			}
			
			else
			{
				if (offsetx >=(29 -(offsety - 16) * 2))
					quad = 0;
				else
					quad = 3;
			}
		}
	}
	
	POINT p;
	switch (quad)
	{
		case 0:
			if ((m_PosY % 2) == 0)  // even
				p.x = mousex;
			else   // odd
				p.x = mousex + 1;
			p.y = mousey * 2 + 1;
			break;
		case 1:
			p.x = mousex + 1;
			p.y = mousey * 2;
			break;
		case 2:
			p.x = mousex;
			p.y = mousey * 2;
			break;
		case 3:
			p.x = mousex;
			p.y = mousey * 2 + 2;
			break;
		case 4:
			p.x = mousex + 1;
			p.y = mousey * 2 + 2;
			break;
	}
	return p;
}
