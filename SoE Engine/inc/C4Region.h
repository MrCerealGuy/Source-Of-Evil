/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*															     */
/*****************************************************************/


/* Screen area marked for mouse interaction */

const int C4RGN_MaxCaption=256;

class C4Region  
{
friend C4RegionList;
public:
	C4Region();
	~C4Region();

public:
	int X,Y,Wdt,Hgt;
	char Caption[C4RGN_MaxCaption+1];
	int Com,RightCom,MoveOverCom,HoldCom;
	int Data;
	C4ID id;
	C4Object *Target;
	C4Menu *Menu;

protected:
	C4Region *Next;

public:
	void Set(C4Facet &fctArea, const char *szCaption=NULL, C4Object *pTarget=NULL, C4Menu *pMenu=NULL);
	void Clear();
	void Default();
	void Set(int iX, int iY, int iWdt, int iHgt, const char *szCaption, int iCom, int iMoveOverCom, int iHoldCom, int iData, C4Object *pTarget, C4Menu *pMenu);

protected:
	void ClearPointers(C4Object *pObj);
};

class C4RegionList  
{
public:
	C4RegionList();
	~C4RegionList();
	
protected:
	int AdjustX,AdjustY;
	C4Region *First;

public:
	void ClearPointers(C4Object *pObj);
	void SetAdjust(int iX, int iY);
	void Clear();
	void Default();
	C4Region* Find(int iX, int iY);
	BOOL Add(int iX, int iY, int iWdt, int iHgt, const char *szCaption=NULL, int iCom=COM_None, C4Object *pTarget=NULL, int iMoveOverCom=COM_None, int iHoldCom=COM_None, int iData=0, C4Menu *pMenu=NULL);
	BOOL Add(C4Facet &fctArea, const char *szCaption=NULL, int iCom=COM_None, C4Object *pTarget=NULL, int iMoveOverCom=COM_None, int iHoldCom=COM_None, int iData=0, C4Menu *pMenu=NULL);
	BOOL Add(C4Region &rRegion);
	BOOL Add(C4RegionList &rRegionList, BOOL fAdjust=TRUE);
};

