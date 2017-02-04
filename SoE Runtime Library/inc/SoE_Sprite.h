/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_SPRITE_H
#define SOE_SPRITE_H

#include "SoE_Tile.h"
#include "SoE_Map.h"

class SOEAPI SoESprite
{
public:
	SoESprite(void);
	virtual ~SoESprite(void);

	HRESULT Create(SoETile* pTile);
	HRESULT Create(SoEScreen *pScreen, const char* Filename, int w, int h, int num, BYTE memoryType);

	void SetPos(int pX, int pY) { m_PosX = pX; m_PosY = pY; }
	void SetPosX(int pX) { m_PosX = pX; }
	void SetPosY(int pY) { m_PosY = pY; }
	void GetPos(int &pX, int &pY) const { pX = m_PosX; pY = m_PosY; }
    int  GetPosX(void) const { return m_PosX; }
    int  GetPosY(void) const { return m_PosY; }
	void SetVel(int vX, int vY) { m_VelX = vX; m_VelY = vY; }
	void SetVelX(int vX) { m_VelX = vX; }
	void SetVelY(int vY) { m_VelY = vY; }
	void GetVel(int &vX, int &vY) const { vX = m_VelX; vY = m_VelY; }
    int  GetVelX(void) const { return m_VelX; }
    int  GetVelY(void) const { return m_VelY; }
	void SetFrame(int frame) { m_Frame = frame; }
    int  GetFrame(void) { return m_Frame; }
	void SetDelay(int delay) { m_Delay = delay; }
    int  GetDelay(void) {return m_Delay;}
	void SetState(int state) { m_State = state; }
    int  GetState(void) {return m_State;}
	void SetType(int type) { m_Type = type; }
    int  GetType(void) {return m_Type;}
    void SetAngle(int angle) { m_Angle = angle; }
    int  GetAngle(void) {return m_Angle;}
    void SetScale(float scale) { m_ScaleFactor = scale; }
    float GetScale(void) {return m_ScaleFactor;}
    void SetShadowOffset(LONG dx, LONG dy);
	void SetShadowValue(WORD Shade) { m_ShadowValue = Shade; }
    WORD GetShadowValue(void) { return m_ShadowValue; }
	void SetAlphaValue(WORD Shade) { m_AlphaValue = Shade; }
    WORD GetAlphaValue(void) { return m_AlphaValue; }
	void SetColorKey(DWORD Col) { GetTile()->SetColorKey(Col); }
	void SetColorKey(void) { m_Tile->SetColorKey(); }
    DWORD GetColorKey(void) { return m_Tile->GetColorKey(); }
    void SetStretchWidth(WORD width) { m_StretchedWidth = width; }
    WORD GetStretchWidth(void) { return m_StretchedWidth; }
    void SetStretchHeight(WORD height) { m_StretchedHeight = height; }
    WORD GetStretchHeight(void) { return m_StretchedHeight; }
    void SetNext(SoESprite* lpSoES) { m_Next = lpSoES; }
    void SetPrev(SoESprite* lpSoES) { m_Prev = lpSoES; }
    SoESprite* GetNext(void) { return m_Next; }
    SoESprite* GetPrev(void) { return m_Prev; }
    void       SetTile(SoETile* lpTile);
    SoETile*   GetTile(void) { return m_Tile; }
	HRESULT Lock(void) { return m_Tile->Lock(); }
	HRESULT UnLock(void) { return m_Tile->UnLock(); }
	BOOL    SpriteHit(SoESprite* pSprite);
	BOOL    TileHit(SoEMap* pMap, int Tile);
	BOOL    SpriteHitPixel(SoESprite* pSprite);
    BOOL    IsFlipped(void) { return m_Flipped; }
	HRESULT Draw(SoESurface* lpSoES, DWORD ScrnWorldX, DWORD ScrnWorldY, WORD BltType);


protected:
	void Init(void);
    inline HRESULT BlkBlt(SoESurface* lpDDSDest, RECT SrcRect, RECT DestRect);
	inline HRESULT TransBlt(SoESurface* lpDDS, RECT SrcRect, RECT DestRect);
	inline HRESULT TransHFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT BlkHFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT TransVFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT BlkVFlip(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT BlkAlpha(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT BlkAlphaFast(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT BlkShadow(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT BlkShadowFast(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransAlpha(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransAlphaFast(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransShadow(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransShadowFast(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransAlphaMask(SoESurface* dest, RECT srcRect, RECT destRect);
	inline HRESULT TransScaled(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT BlkScaled(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT TransStretched(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
	inline HRESULT BlkStretched(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
    inline HRESULT BlkRotoZoom(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);
    inline HRESULT TransRotoZoom(SoESurface* lpSoESDest, RECT SrcRect, RECT DestRect);


private:
	int m_PosX;				// The sprite's X position
	int m_PosY;				// The sprite's Y position
	int m_PosZ;             // The sprite's Z position
	int m_VelX;				// The sprite's X velocity
	int m_VelY;				// The sprite's Y velocity
	int m_Frame;			// The current frame
	int m_Delay;			// Used for game timing, the time till the next frame
	int m_State;			// User defined state.  Walking, jumping, etc
	int m_Type;				// User defined type.  Health, weapon, etc
	int m_Angle;			// The sprite's angle of rotation
	float m_ScaleFactor;	// The amount by which to scale the sprite if Drawing with SOEBLT_XXXSCALED
	BOOL m_Flipped;			// Is the sprite flipped?
	LONG m_ShadowOffsetX;   // Amount to offset the shadow in the X direction
	LONG m_ShadowOffsetY;   // Amount to offset the shadow in the Y direction
	WORD m_ShadowValue;     // Alpha value used in drawing the shadow 0-256
	WORD m_AlphaValue;      // Alpha value used when alpha blending 0-256
	BOOL m_TileCreated;     // Used internal to determine if the class created the SoETile object or not
	WORD m_StretchedHeight; // Used to define the height of the stretched sprite 
	WORD m_StretchedWidth;  // Used to define the width of the stretched sprite
	SoETile*   m_Tile;		// A SoETile pointer to the sprite's bitmap data
	SoESprite* m_Next;		// A SoESprite pointer to the next sprite in a SoESpriteList
	SoESprite* m_Prev;		// A SoESprite pointer to the previous sprite in a SoESpriteList
};

class SOEAPI SoESpriteList
{
public:
	SoESpriteList(void);
	virtual ~SoESpriteList(void);

    void AddSprite(SoESprite* pSprite) { m_spriteList.Add(pSprite); }
    void DelSprite(SoESprite* pSprite) { m_spriteList.Remove(pSprite, TRUE); }
    LONG GetCount(void) { return m_spriteList.GetCount(); }
    SoESprite* GetFirst(void) { return m_spriteList.GetFirst(); }
    SoESprite* GetNext(SoESprite* pSprite) { return m_spriteList.Next(pSprite); }
	virtual HRESULT Draw(SoESurface* lpSoES, DWORD ScrnWorldX, DWORD ScrnWorldY, WORD BltType);

private:
	SoECList<SoESprite> m_spriteList;  // Linked List template
};

#endif
