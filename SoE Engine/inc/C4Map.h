/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Create map from dynamic landscape data in scenario */

class C4MapCreator
{
public:
    C4MapCreator();

protected:
    int MapIFT;
    BYTE *MapBuf;
    int MapWdt,MapHgt,MapBufWdt;
    int Exclusive;

public:
    void Create(BYTE *bypBuffer, int iBufWdt, C4SLandscape &rLScape, C4TextureMap &rTexMap, BOOL fLayers=FALSE, int iPlayerNum=1);
	BOOL Load(BYTE **pbypBuffer, int &rBufWdt, int &rMapWdt, int &rMapHgt, C4Group &hGroup, const char *szEntryName, C4TextureMap &rTexMap);
 
protected:
    void Reset();
    void SetPix(int x, int y, BYTE col);
    void SetSpot(int x, int y, int rad, BYTE col);
    void DrawLayer(int x, int y, int size, BYTE col);
	void ValidateTextureIndices(C4TextureMap &rTexMap);
    BYTE GetPix(int x, int y);
};
