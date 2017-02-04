/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Sprite.h"

SoESprite::SoESprite(void)
{
	Init();
}

void SoESprite::Init()
{
	m_PosX			  = 0;
	m_PosY			  = 0;
	m_PosZ			  = 0;
	m_VelX			  = 0;
	m_VelY			  = 0;
	m_Frame			  = 0;
	m_Delay		  	  = 0;
	m_State			  = 0;
	m_Type			  = 0;
	m_Angle			  = 0;
    m_ScaleFactor     = 1;
	m_Flipped		  = FALSE;
	m_ShadowOffsetX   = 0;
	m_ShadowOffsetY   = 0;
	m_ShadowValue	  = 0;
	m_AlphaValue	  = 0;
	m_TileCreated	  = FALSE;
    m_StretchedHeight = 0;
    m_StretchedWidth  = 0;
	m_Tile			  = NULL;
	m_Next			  = NULL;
	m_Prev			  = NULL;
}

SoESprite::~SoESprite(void)
{
	if (m_TileCreated == TRUE)
    {
		delete m_Tile;
        m_Tile = NULL;
    }
	
	m_Tile = NULL;
	m_Next = NULL;
	m_Prev = NULL;
}

HRESULT SoESprite::Create(SoETile* pTile)
{
    // If NULL is passed in return error
	if (pTile == NULL) 
		return -1;
	
    // If the m_Tile is already set return error
    if (GetTile() != NULL)
        return -2;
	
    // Specify that the class did NOT create the object pointed to by m_Tile
    m_Tile = pTile;
	m_TileCreated = FALSE;
	
	return 0;
}

HRESULT SoESprite::Create(SoEScreen *pScreen, const char* Filename, int w, int h, int num, BYTE memoryType)
{
    HRESULT rval;
	
    // If the filename is NULL return an error
	if (Filename == NULL) 
        return -1;
	
    // If the m_Tile is NOT NULL return an error
    if (GetTile() != NULL)
        return -2;
	
    // Create the m_Tile object
	m_Tile = new SoETile();
    rval = m_Tile->Create(pScreen, Filename, w, h, num, memoryType);
    if (rval == FALSE)
        return -3;
	
    m_TileCreated = TRUE;
	
	return 1;
}

HRESULT SoESprite::Draw(SoESurface* lpSoES, DWORD ScrnWorldX, DWORD ScrnWorldY, WORD BltType)
{
	RECT SpriteDestRect;	// Sprite RECT is defined in world space
	RECT SpriteSrcRect;		// RECT defining the source area for the BLT
	DWORD TilesInWidth;		// Number of tiles in width of sprite tile surface
	DWORD SpriteSrcX;
	DWORD SpriteSrcY;
    int blockWidth, blockHeight;
	HRESULT rval;
	
    // Get block width and height
    blockWidth  = GetTile()->m_BlockWidth;
    blockHeight = GetTile()->m_BlockHeight;
	
	// Validate the screen world space coordinates.
	if ((ScrnWorldX == NULL) ||(ScrnWorldX < 0)) 
		ScrnWorldX = 0;
	if ((ScrnWorldY == NULL) ||(ScrnWorldY < 0)) 
		ScrnWorldY = 0;
	
	// RECT defining the sprite in world space.
	SpriteDestRect.top    = m_PosY;
	SpriteDestRect.left   = m_PosX;
	SpriteDestRect.bottom = m_PosY + blockHeight;
	SpriteDestRect.right  = m_PosX + blockWidth;
	
	// Adjust the sprite RECT to screen space. Destination RECT for the BLT.
	SpriteDestRect.top    -= ScrnWorldY;
	SpriteDestRect.bottom -= ScrnWorldY;
	SpriteDestRect.left   -= ScrnWorldX;
	SpriteDestRect.right  -= ScrnWorldX;
	
	// Get the number of tiles in the sprite tile surface width
	TilesInWidth = GetTile()->GetWidth() / blockWidth;
	
	// Calc the upper left corner of the current frame of animation
	SpriteSrcX = (m_Frame % TilesInWidth) * blockWidth;
	SpriteSrcY = (m_Frame / TilesInWidth) * blockHeight;
	
	// Define the source RECT for the BLT.
	SpriteSrcRect.top    = SpriteSrcY;
	SpriteSrcRect.left   = SpriteSrcX;
	SpriteSrcRect.bottom = SpriteSrcY + blockHeight;
	SpriteSrcRect.right  = SpriteSrcX + blockWidth;
		
	switch (BltType)
	{
		case SOEBLT_BLK:
			rval = BlkBlt(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;

		case SOEBLT_BLKALPHA:
			rval = BlkAlpha(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
	
		case SOEBLT_BLKALPHAFAST:
			rval = BlkAlphaFast(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKSCALED:
			rval = BlkScaled(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKSHADOW:
			rval = BlkShadow(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKSHADOWFAST:
			rval = BlkShadowFast(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKROTATED:
			// rval = BlkRotated(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKROTOZOOM:
			rval = BlkRotoZoom(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKHFLIP:
			rval = BlkHFlip(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKVFLIP:
			rval = BlkVFlip(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_BLKSTRETCHED:
			rval = BlkStretched(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANS:
			rval = TransBlt(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSALPHA:  
			rval = TransAlpha(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSALPHAFAST:
			rval = TransAlphaFast(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSSCALED:
			rval = TransScaled(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSSHADOW:
			rval = TransShadow(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSSHADOWFAST:
			rval = TransShadowFast(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSROTATED:
			// rval = TransRotated(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSROTOZOOM:
			rval = TransRotoZoom(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSHFLIP:
			rval = TransHFlip(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSVFLIP:
			rval = TransVFlip(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		case SOEBLT_TRANSSTRETCHED:
			rval = TransStretched(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
						
		case SOEBLT_TRANSALPHAMASK:
			rval = TransAlphaMask(lpSoES, SpriteSrcRect, SpriteDestRect);
			break;
		
		default:
			break;
	}
	
    return rval;
}

HRESULT SoESprite::BlkRotoZoom(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
    DWORD w, h;
	
    w = SrcRect.right - SrcRect.left;
    h = SrcRect.bottom - SrcRect.top;
	
	rval = m_Tile->SoESurface::DrawBlkRotoZoom(lpSoESDest, DestRect.left + w/2, DestRect.top + h/2, &SrcRect, m_Angle, m_ScaleFactor);
	
	return rval;
}

HRESULT SoESprite::TransRotoZoom(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
    DWORD w, h;
	
    w = SrcRect.right - SrcRect.left;
    h = SrcRect.bottom - SrcRect.top;
	
	rval = m_Tile->SoESurface::DrawTransRotoZoom(lpSoESDest, DestRect.left + w/2, DestRect.top + h/2, &SrcRect, m_Angle, m_ScaleFactor);
	
	return rval;
}

void SoESprite::SetTile(SoETile* lpTile)
{
    // If this class created the SoETile then delete it.
    if (m_TileCreated)
    {
		delete m_Tile;
        m_TileCreated = FALSE;
    }
	
    // Set the m_Tile pointer
    m_Tile = lpTile;
}

void SoESprite::SetShadowOffset(LONG dx, LONG dy)
{
	m_ShadowOffsetX = dx;
	m_ShadowOffsetY = dy;
}

HRESULT SoESprite::TransStretched(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransStretched(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_StretchedWidth , m_StretchedHeight);
	
	return rval;
}

HRESULT SoESprite::BlkStretched(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkStretched(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_StretchedWidth , m_StretchedHeight);
	
	return rval;
}

HRESULT SoESprite::TransScaled(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransScaled(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_ScaleFactor);
	
	return rval;
}

HRESULT SoESprite::BlkScaled(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkScaled(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_ScaleFactor);
	
	return rval;
}

HRESULT SoESprite::TransBlt(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTrans(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::BlkBlt(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlk(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::TransHFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransHFlip(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::BlkHFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkHFlip(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::TransVFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransVFlip(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::BlkVFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkVFlip(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::BlkAlphaFast(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkAlphaFast(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}


HRESULT SoESprite::TransAlphaFast(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransAlphaFast(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}


HRESULT SoESprite::TransAlpha(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransAlpha(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_AlphaValue);
	
	return rval;
}

HRESULT SoESprite::BlkAlpha(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkAlpha(lpSoESDest, DestRect.left, DestRect.top, &SrcRect, m_AlphaValue);
	
	return rval;
}

HRESULT SoESprite::BlkShadow(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkShadow(lpSoESDest, DestRect.left - m_ShadowOffsetX, DestRect.top - m_ShadowOffsetY, &SrcRect, m_ShadowValue);
	
	return rval;
}

HRESULT SoESprite::TransShadow(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransShadow(lpSoESDest, DestRect.left - m_ShadowOffsetX, DestRect.top - m_ShadowOffsetY, &SrcRect, m_ShadowValue);
	
	return rval;
}

HRESULT SoESprite::BlkShadowFast(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawBlkShadowFast(lpSoESDest, DestRect.left - m_ShadowOffsetX, DestRect.top - m_ShadowOffsetY, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::TransShadowFast(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransShadowFast(lpSoESDest, DestRect.left - m_ShadowOffsetX, DestRect.top - m_ShadowOffsetY, &SrcRect);
	
	return rval;
}

HRESULT SoESprite::TransAlphaMask(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect)
{
	HRESULT rval;
	
	rval = m_Tile->SoESurface::DrawTransAlphaMask(lpSoESDest, DestRect.left, DestRect.top, &SrcRect);
	
	return rval;
}

BOOL SoESprite::SpriteHit(SoESprite* pSprite)
{
	if (pSprite == this) 
        return FALSE;
	
	RECT Rect1 = 
	{
		m_PosX, m_PosY,
			m_PosX + GetTile()->m_BlockWidth,
			m_PosY + GetTile()->m_BlockHeight 
	};
	
	RECT Rect2 = 
	{
		pSprite->GetPosX(), pSprite->GetPosY(),
			pSprite->GetPosX() + pSprite->GetTile()->m_BlockWidth,
			pSprite->GetPosY() + pSprite->GetTile()->m_BlockHeight 
	};
	
	if (Rect1.top > Rect2.bottom)
		return FALSE;
	else if (Rect1.bottom < Rect2.top)
		return FALSE;
	else if (Rect1.right < Rect2.left)
		return FALSE;
	else if (Rect1.left > Rect2.right)
		return FALSE;
	
	return TRUE;
}

BOOL SoESprite::TileHit(SoEMap* pMap, int Tile)
{
	int MapX, MapY, TileType;
	
    // Check top left
    MapX = (pMap->GetPosX() + GetPosX()) / pMap->GetTileWidth();
    MapY = (pMap->GetPosY() + GetPosY()) / pMap->GetTileHeight();
    TileType = pMap->GetTile(min(MapX, pMap->GetMapWidth() - 1), min(MapY, pMap->GetMapHeight() - 1));
    if (TileType == Tile)
		return TRUE;
	
    // Check top right
    MapX = (pMap->GetPosX() + GetPosX() + GetTile()->m_BlockWidth) / pMap->GetTileWidth();
    MapY = (pMap->GetPosY() + GetPosY()) / pMap->GetTileHeight();
    TileType = pMap->GetTile(min(MapX, pMap->GetMapWidth() - 1), min(MapY, pMap->GetMapHeight() - 1));
    if (TileType == Tile)
		return TRUE;
	
    // Check bottom left
    MapX = (pMap->GetPosX() + GetPosX()) / pMap->GetTileWidth();
    MapY = (pMap->GetPosY() + GetPosY() + GetTile()->m_BlockHeight) / pMap->GetTileHeight();
    TileType = pMap->GetTile(min(MapX, pMap->GetMapWidth() - 1), min(MapY, pMap->GetMapHeight() - 1));
    if (TileType == Tile)
		return TRUE;
	
    // Check bottom right
    MapX = (pMap->GetPosX() + GetPosX() + GetTile()->m_BlockWidth) / pMap->GetTileWidth();
    MapY = (pMap->GetPosY() + GetPosY() + GetTile()->m_BlockHeight) / pMap->GetTileHeight();
    TileType = pMap->GetTile(min(MapX, pMap->GetMapWidth() - 1), min(MapY, pMap->GetMapHeight() - 1));
    if (TileType == Tile)
		return TRUE;
	
	return FALSE;
}

BOOL SoESprite::SpriteHitPixel(SoESprite* pSprite)
{	
	SoESprite *rightSprite;
	SoESprite *leftSprite;
	UINT  intersectWidth;
	UINT  intersectHeight;
	POINT LSPoint;
	POINT RSPoint;
	UINT  LSFrame;
	UINT  RSFrame;
	LONG  LSPitch;
	LONG  RSPitch;
	UINT  LSWidth;
	UINT  LSHeight;
	UINT  RSWidth;
	UINT  RSHeight;
	UINT  LSFramesInWidth;
	UINT  RSFramesInWidth;
	UINT  LSFrameX;
	UINT  LSFrameY;
	UINT  RSFrameX;
	UINT  RSFrameY;
	DWORD LSColorKey;
	DWORD RSColorKey;
	BYTE* LSptr;
	BYTE* RSptr;
	UINT  x, y;
    RGBFORMAT pixelFormat;
	
	// If we are checking against ourseves, no collision.
	if (pSprite == this) 
		return FALSE;
	
	// Rectangle describing sprites current world location.
	RECT Rect1 = 
	{
		GetPosX(), 
			GetPosY(),
			GetPosX() + GetTile()->m_BlockWidth,
			GetPosY() + GetTile()->m_BlockHeight 
	};
	
	// Rectangle describing the current sprite we are checking colisions against.
	RECT Rect2 = 
	{
		pSprite->GetPosX(), 
			pSprite->GetPosY(),
			pSprite->GetPosX() + pSprite->GetTile()->m_BlockWidth,
			pSprite->GetPosY() + pSprite->GetTile()->m_BlockHeight 
	};
	
	// We must perform a bounding box collision detection first
	// otherwise we might get unexpected results.
	if (Rect1.top > Rect2.bottom) 
		return FALSE;
	else if (Rect1.bottom < Rect2.top) 
		return FALSE;
	else if (Rect1.right < Rect2.left) 
		return FALSE;
	else if (Rect1.left > Rect2.right) 
		return FALSE;
	
	// Determine which sprite is to the left and which is to the right.
	if (pSprite->GetPosX() > GetPosX())
	{
		rightSprite = pSprite;
		leftSprite  = this;
	}
	else
	{
		rightSprite = this;
		leftSprite  = pSprite;
	}
	
	// Get the width and height of each sprite
	LSWidth  = leftSprite->GetTile()->m_BlockWidth;
	LSHeight = leftSprite->GetTile()->m_BlockHeight;
	RSWidth  = rightSprite->GetTile()->m_BlockWidth;
	RSHeight = rightSprite->GetTile()->m_BlockHeight;
	
	// Get the current frame for each sprite
	LSFrame = leftSprite->GetFrame();
	RSFrame = rightSprite->GetFrame();
	
	// Get the amount the two sprites overlap in the X direction
	intersectWidth = LSWidth -(rightSprite->GetPosX() - leftSprite->GetPosX());
	
	// The intersecting width cannot be greater then the sprite width.
	if (intersectWidth > RSWidth)
		intersectWidth = RSWidth;
	
	// Determine which sprite is lower on the screen.
	if (leftSprite->GetPosY() > rightSprite->GetPosY()) 
	{
		LSPoint.y = 0;
		RSPoint.y = leftSprite->GetPosY() - rightSprite->GetPosY();
		intersectHeight = RSHeight -(leftSprite->GetPosY() - rightSprite->GetPosY());
		if (intersectHeight > LSWidth)
			intersectHeight = LSWidth;
	}
	else // left sprite is higher on the screen.
	{
		RSPoint.y = 0;
		LSPoint.y = rightSprite->GetPosY() - leftSprite->GetPosY();
		intersectHeight = LSHeight -(rightSprite->GetPosY() - leftSprite->GetPosY());
		if (intersectHeight > LSWidth)
			intersectHeight = LSWidth;
	}
	
	// Calculate the starting X position of collision.
	RSPoint.x = 0;
	LSPoint.x = rightSprite->GetPosX() - leftSprite->GetPosX();
	
	// Determine the number of frames that fit across the surface.
	LSFramesInWidth = leftSprite->GetTile()->GetWidth() / LSWidth;
	RSFramesInWidth = rightSprite->GetTile()->GetWidth() / RSWidth;
	
	// Find x & y location on the surface(s) of first pixel in question
	LSFrameX = ((LSFrame % LSFramesInWidth) * LSWidth) +(UINT)LSPoint.x;
	RSFrameX = ((RSFrame % RSFramesInWidth) * RSWidth) +(UINT)RSPoint.x;
	LSFrameY = ((LSFrame / LSFramesInWidth) * LSHeight) +(UINT)LSPoint.y;
	RSFrameY = ((RSFrame / RSFramesInWidth) * RSHeight) +(UINT)RSPoint.y;
	
	// Get the color key for each surface.
	LSColorKey = leftSprite->GetTile()->GetColorKey();
	RSColorKey = rightSprite->GetTile()->GetColorKey();
	
	
	// We are assumming that both surfaces have the same pixel bit depth.
    leftSprite->GetTile()->GetPixelFormat(&pixelFormat);
	switch (pixelFormat.bpp)
	{
		case 8:
			// The potential exists that the two sprites we are collition detecting
			// exist on the same surface.  In that case we only need to lock one surface.
			if (leftSprite->GetTile() == rightSprite->GetTile())
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = LSPitch;
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr += LSFrameX +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr += RSFrameX +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if ((*LSptr !=(BYTE)LSColorKey) &&(*RSptr !=(BYTE)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr ++;
						RSptr ++;
					}
					LSptr += LSPitch - intersectWidth;
					RSptr += RSPitch - intersectWidth;
				}
				leftSprite->GetTile()->UnLock();
			}
			else // Sprites are on different surfaces.
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				if (FAILED(rightSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = rightSprite->GetTile()->GetPitch();
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr += LSFrameX +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr += RSFrameX +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if ((*LSptr !=(BYTE)LSColorKey) &&(*RSptr !=(BYTE)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							rightSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr ++;
						RSptr ++;
					}
					LSptr += LSPitch - intersectWidth;
					RSptr += RSPitch - intersectWidth;
				}
				leftSprite->GetTile()->UnLock();
				rightSprite->GetTile()->UnLock();
			}
			break;
			
		case 15:
		case 16:
			// The potential exists that the two sprites we are collition detecting
			// exist on the same surface.  In that case we only need to lock one surface.
			if (leftSprite->GetTile() == rightSprite->GetTile())
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = LSPitch;
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 2) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 2) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if ((*((WORD*)LSptr) !=(WORD)LSColorKey) &&(*((WORD*)RSptr) !=(WORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 2;
						RSptr += 2;
					}
					LSptr += LSPitch -(intersectWidth * 2);
					RSptr += RSPitch -(intersectWidth * 2);
				}
				leftSprite->GetTile()->UnLock();
			}
			else // Sprites are on different surfaces.
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				if (FAILED(rightSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = rightSprite->GetTile()->GetPitch();
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 2) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 2) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if ((*((WORD*)LSptr) !=(WORD)LSColorKey) &&(*((WORD*)RSptr) !=(WORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							rightSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 2;
						RSptr += 2;
					}
					LSptr += LSPitch -(intersectWidth * 2);
					RSptr += RSPitch -(intersectWidth * 2);
				}
				leftSprite->GetTile()->UnLock();
				rightSprite->GetTile()->UnLock();
			}
			break;
			
		case 24:
			// The potential exists that the two sprites we are collition detecting
			// exist on the same surface.  In that case we only need to lock one surface.
			if (leftSprite->GetTile() == rightSprite->GetTile())
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = LSPitch;
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 3) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 3) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if (((*((DWORD*)LSptr) & 0xFFFFFF) !=(DWORD)LSColorKey) &&((*((DWORD*)RSptr) & 0xFFFFFF) !=(DWORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 3;
						RSptr += 3;
					}
					LSptr += LSPitch -(intersectWidth * 3);
					RSptr += RSPitch -(intersectWidth * 3);
				}
				leftSprite->GetTile()->UnLock();
			}
			else // Sprites are on different surfaces.
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				if (FAILED(rightSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = rightSprite->GetTile()->GetPitch();
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 3) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 3) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if (((*((DWORD*)LSptr) & 0xFFFFFF) !=(DWORD)LSColorKey) &&((*((DWORD*)RSptr) & 0xFFFFFF) !=(DWORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							rightSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 3;
						RSptr += 3;
					}
					LSptr += LSPitch -(intersectWidth * 3);
					RSptr += RSPitch -(intersectWidth * 3);
				}
				leftSprite->GetTile()->UnLock();
				rightSprite->GetTile()->UnLock();
			}
			break;
			
		case 32:
			// The potential exists that the two sprites we are collition detecting
			// exist on the same surface.  In that case we only need to lock one surface.
			if (leftSprite->GetTile() == rightSprite->GetTile())
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = LSPitch;
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 4) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 4) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if ((*((DWORD*)LSptr) !=(DWORD)LSColorKey) &&(*((DWORD*)RSptr) !=(DWORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 4;
						RSptr += 4;
					}
					LSptr += LSPitch -(intersectWidth * 4);
					RSptr += RSPitch -(intersectWidth * 4);
				}
				leftSprite->GetTile()->UnLock();
			}
			else // Sprites are on different surfaces.
			{
				if (FAILED(leftSprite->GetTile()->Lock()))
					return FALSE;
				if (FAILED(rightSprite->GetTile()->Lock()))
					return FALSE;
				
				// Get the pitch of the surface. Pitch is width of surface in bytes.
				LSPitch = leftSprite->GetTile()->GetPitch();
				RSPitch = rightSprite->GetTile()->GetPitch();
				
				// Initialize the pointers to the first pixel in the collition rectangle.
				LSptr  = (BYTE*)leftSprite->GetTile()->GetSurfacePointer();
				LSptr +=(LSFrameX * 4) +(LSFrameY *(UINT)LSPitch);
				
				RSptr  = (BYTE*)rightSprite->GetTile()->GetSurfacePointer();
				RSptr +=(RSFrameX * 4) +(RSFrameY *(UINT)RSPitch);
				
				
				for (y = 0; y < intersectHeight; y++)
				{
					for (x = 0; x < intersectWidth; x++)
					{
						if (((*((DWORD*)LSptr) & 0xFFFFFF) !=(DWORD)LSColorKey) &&((*((DWORD*)RSptr) & 0xFFFFFF) !=(DWORD)RSColorKey))
						{
							leftSprite->GetTile()->UnLock();
							rightSprite->GetTile()->UnLock();
							return TRUE;
						}
						LSptr += 4;
						RSptr += 4;
					}
					LSptr += LSPitch -(intersectWidth * 4);
					RSptr += RSPitch -(intersectWidth * 4);
				}
				leftSprite->GetTile()->UnLock();
				rightSprite->GetTile()->UnLock();
			}
			break;
			
		default:
			break;
	}
	return FALSE;
}
