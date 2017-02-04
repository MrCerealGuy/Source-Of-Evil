/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Finds the shortest free path through any pixel environment */

#include <Standard.h>

#include <PathFinder.h>

const int	PF_Direction_Left	 = -1,
					PF_Direction_Right = +1,

					PF_Ray_Launch			 = 0,
					PF_Ray_Crawl			 = 1,
					PF_Ray_Still			 = 2,
					PF_Ray_Failure		 = 3,
					PF_Ray_Deleted		 = 4,

					PF_Crawl_NoAttach	 = 0,
					PF_Crawl_Top			 = 1,
					PF_Crawl_Right		 = 2,
					PF_Crawl_Bottom		 = 3,
					PF_Crawl_Left			 = 4,
					
					PF_Draw_Rate			 = 10;

//------------------------------- CPathFinderRay ---------------------------------------------

CPathFinderRay::CPathFinderRay()
	{
	Default();
	}

CPathFinderRay::~CPathFinderRay()
	{
	Clear();
	}

void CPathFinderRay::Default()
	{
	Status=PF_Ray_Launch;
	X=Y=X2=Y2=TargetX=TargetY=0;
	Direction=0;
	Depth=0;
	From=NULL;
	Next=NULL;
	pPathFinder=NULL;
	CrawlStartX=CrawlStartY=CrawlAttach=0;
	}

void CPathFinderRay::Clear()
	{

	}

BOOL CPathFinderRay::Execute()
	{
	CPathFinderRay *pRay;
	int iX,iY,iLastX,iLastY;
	switch (Status)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case PF_Ray_Launch:
			// Path free
			if (PathFree(X2,Y2,TargetX,TargetY))
				{
				// Back shorten
				for (pRay=this; pRay->From; pRay=pRay->From)
					while (pRay->CheckBackRayShorten());
				// Set all waypoints
				for (pRay=this; pRay->From; pRay=pRay->From)
					pPathFinder->SetWaypoint(pRay->From->X2,pRay->From->Y2,pPathFinder->WaypointParameter);
				// Success
				pPathFinder->Success=TRUE;
				Status=PF_Ray_Still;
				return TRUE;
				}
			// Path intersected
			else
				{
				// Start crawling
				Status=PF_Ray_Crawl;
				CrawlStartX=X2; CrawlStartY=Y2; 
				CrawlAttach=FindCrawlAttach(X2,Y2);
				CrawlLength=0;
				if (!CrawlAttach) CrawlAttach=FindCrawlAttachDiagonal(X2,Y2,Direction);
				CrawlStartAttach=CrawlAttach;
				// Intersected but no attach found: unexpected failure
				if (!CrawlAttach)
					Status=PF_Ray_Failure; 
				return TRUE;
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case PF_Ray_Crawl:
			// Crawl
			iLastX=X2; iLastY=Y2;
			if (!Crawl())	
				{ Status=PF_Ray_Failure; break; } 
			// Back at crawl starting position: done and still
			if ((X2==CrawlStartX) && (Y2==CrawlStartY) && (CrawlAttach==CrawlStartAttach))
				{ Status=PF_Ray_Still; break; }
			// Crawl length
			CrawlLength++; 
			if (CrawlLength>=pPathFinder->MaxCrawl)
				{ Status=PF_Ray_Still; break; }
			// Check back path intersection
			iX=X; iY=Y;
			if (!PathFree(iX,iY,X2,Y2))
				// Insert split ray
				if (!pPathFinder->SplitRay(this,iLastX,iLastY))
					{ Status=PF_Ray_Failure; break; }
			// Try new ray at target
			iX=X2; iY=Y2;
			// If has been crawling for a while
			if (CrawlLength>pPathFinder->Threshold)
				// If all free...
				if ( PathFree(iX,iY,TargetX,TargetY) 
				 // ...or at least beyond threshold and not backwards toward crawl start
				 || ((Distance(iX,iY,X2,Y2)>pPathFinder->Threshold) && (Distance(iX,iY,CrawlStartX,CrawlStartY)>Distance(X2,Y2,CrawlStartX,CrawlStartY))) )
					{					
					// Still
					Status=PF_Ray_Still;
					// Launch new rays 
					if (!pPathFinder->AddRay(X2,Y2,TargetX,TargetY,Depth+1,PF_Direction_Left,this)
					 || !pPathFinder->AddRay(X2,Y2,TargetX,TargetY,Depth+1,PF_Direction_Right,this))
						Status=PF_Ray_Failure;
					}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case PF_Ray_Still: case PF_Ray_Failure: case PF_Ray_Deleted:
			return FALSE;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	return TRUE;
	}

/*void CPathFinderRay::Draw(C4FacetEx &cgo)
	{
	BYTE byColor=CRed;
	switch (Status)
		{
		case PF_Ray_Crawl: byColor=CRed; break;
		case PF_Ray_Still: byColor=CDRed; break;
		case PF_Ray_Failure: byColor=CYellow; break;
		case PF_Ray_Deleted: byColor=CGray2; break;
		}
	
	// Crawl attachment
	if (Status==PF_Ray_Crawl)
		{
		int iX=0,iY=0; CrawlToAttach(iX,iY,CrawlAttach);
		lpDDraw->DrawLine(cgo.Surface,
											cgo.X+X2-cgo.TargetX,cgo.Y+Y2-cgo.TargetY,
											cgo.X+X2-cgo.TargetX+7*iX,cgo.Y+Y2-cgo.TargetY+7*iY,
											CRed);
		//sprintf(OSTR,"%d",Depth); lpDDraw->TextOut(OSTR,cgo.Surface,cgo.X+X2-cgo.TargetX,cgo.Y+Y2-cgo.TargetY+20,CGray4);
		}

	// Ray line
	lpDDraw->DrawLine(cgo.Surface,
										cgo.X+X-cgo.TargetX,cgo.Y+Y-cgo.TargetY,
										cgo.X+X2-cgo.TargetX,cgo.Y+Y2-cgo.TargetY,
										byColor);

	// Crawler point
	lpDDraw->DrawFrame(cgo.Surface,
										 cgo.X+X2-cgo.TargetX-1,cgo.Y+Y2-cgo.TargetY-1,
										 cgo.X+X2-cgo.TargetX+1,cgo.Y+Y2-cgo.TargetY+1,
										 (Status==PF_Ray_Crawl) ? ((Direction==PF_Direction_Left) ? CGreen : CBlue) : byColor);

	// Search target point
	lpDDraw->DrawFrame(cgo.Surface,
										 cgo.X+TargetX-cgo.TargetX-2,cgo.Y+TargetY-cgo.TargetY-2,
										 cgo.X+TargetX-cgo.TargetX+2,cgo.Y+TargetY-cgo.TargetY+2,
										 CYellow);

	}*/

BOOL CPathFinderRay::PathFree(int &rX, int &rY, int iToX, int iToY)
	{
  int d,dx,dy,aincr,bincr,xincr,yincr,x,y;
	// Y based
  if (Abs(iToX-rX)<Abs(iToY-rY))
    {
    xincr=(iToX>rX) ? +1 : -1;			
		yincr=(iToY>rY) ? +1 : -1;
    dy=Abs(iToY-rY); dx=Abs(iToX-rX);   
		d=2*dx-dy; aincr=2*(dx-dy); bincr=2*dx; x=rX; y=rY;
    for (y=rY; y!=iToY; y+=yincr)
      {
			if (PointFree(x,y)) { rY=y; rX=x; }
			else return FALSE;
      if (d>=0) { x+=xincr; d+=aincr; } else d+=bincr;
      }
    }
	// X based
  else
    {
    yincr=(iToY>rY) ? +1 : -1;
    xincr=(iToX>rX) ? +1 : -1;
		dx=Abs(iToX-rX); dy=Abs(iToY-rY);
		d=2*dy-dx; aincr=2*(dy-dx); bincr=2*dy; x=rX; y=rY;
		for (x=rX; x!=iToX; x+=xincr)
      {
			if (PointFree(x,y)) { rY=y; rX=x; }
			else return FALSE;
      if (d>=0)	{ y+=yincr; d+=aincr; }
      else d+=bincr;
      }
    }

  return TRUE;
	}

BOOL CPathFinderRay::Crawl()
	{

	// No attach: crawl failure (shouldn't ever get here)
	if (!CrawlAttach) 
		return FALSE;

	// Last attach lost (don't check on first crawl for that might be a diagonal attach)
	if (CrawlLength)
		if (!IsCrawlAttach(X2,Y2,CrawlAttach))
			{
			// Crawl corner by last attach
			CrawlToAttach(X2,Y2,CrawlAttach);
			TurnAttach(CrawlAttach,-Direction);
			// Safety: new attach not found - unexpected failure
			if (!IsCrawlAttach(X2,Y2,CrawlAttach)) 
				return FALSE;
			// Corner okay
			return TRUE;		
			}

	// Check crawl target by attach
	int iTurned=0;
	while (!CrawlTargetFree(X2,Y2,CrawlAttach,Direction))
		{
		// Crawl target not free: turn attach
		TurnAttach(CrawlAttach,Direction); iTurned++;
		// Turned four times: all enclosed, crawl failure
		if (iTurned==4) 
			return FALSE;
		}

	// Crawl by attach
	CrawlByAttach(X2,Y2,CrawlAttach,Direction);

	// Success
	return TRUE;

	}

BOOL CPathFinderRay::PointFree(int iX, int iY)
	{
	return pPathFinder->PointFree(iX,iY,pPathFinder->PointFreeParameter);
	}

BOOL CPathFinderRay::CrawlTargetFree(int iX, int iY, int iAttach, int iDirection)
	{
	CrawlByAttach(iX,iY,iAttach,iDirection);
	return PointFree(iX,iY);
	}

void CPathFinderRay::CrawlByAttach(int &rX, int &rY, int iAttach, int iDirection)
	{
	switch (iAttach)
		{
		case PF_Crawl_Top: rX+=iDirection; break;
		case PF_Crawl_Bottom: rX-=iDirection; break;
		case PF_Crawl_Left: rY-=iDirection; break;
		case PF_Crawl_Right: rY+=iDirection; break;
		}
	}

void CPathFinderRay::TurnAttach(int &rAttach, int iDirection)
	{
	rAttach+=iDirection;
	if (rAttach>PF_Crawl_Left) rAttach=PF_Crawl_Top;
	if (rAttach<PF_Crawl_Top) rAttach=PF_Crawl_Left;
	}

int CPathFinderRay::FindCrawlAttach(int iX, int iY)
	{
	if (!PointFree(iX,iY-1)) return PF_Crawl_Top;
	if (!PointFree(iX,iY+1)) return PF_Crawl_Bottom;
	if (!PointFree(iX-1,iY)) return PF_Crawl_Left;
	if (!PointFree(iX+1,iY)) return PF_Crawl_Right;
	return PF_Crawl_NoAttach;
	}

void CPathFinderRay::CrawlToAttach(int &rX, int &rY, int iAttach)
	{
	switch (iAttach)
		{	
		case PF_Crawl_Top: rY--; break;
		case PF_Crawl_Bottom: rY++; break;
		case PF_Crawl_Left: rX--; break;
		case PF_Crawl_Right: rX++; break;
		}
	}

BOOL CPathFinderRay::IsCrawlAttach(int iX, int iY, int iAttach)
	{
	CrawlToAttach(iX,iY,iAttach);
	return !PointFree(iX,iY);
	}

int CPathFinderRay::FindCrawlAttachDiagonal(int iX, int iY, int iDirection)
	{
	// Going left
	if (iDirection==PF_Direction_Left)
		{
		// Top Left
		if (!PointFree(iX-1,iY-1)) return PF_Crawl_Top;
		// Bottom left
		if (!PointFree(iX-1,iY+1)) return PF_Crawl_Left;
		// Top right
		if (!PointFree(iX+1,iY-1)) return PF_Crawl_Right;
		// Bottom right
		if (!PointFree(iX+1,iY+1)) return PF_Crawl_Bottom;
		}
	// Going right
	if (iDirection==PF_Direction_Right)
		{
		// Top Left
		if (!PointFree(iX-1,iY-1)) return PF_Crawl_Left;
		// Bottom left
		if (!PointFree(iX-1,iY+1)) return PF_Crawl_Bottom;
		// Top right
		if (!PointFree(iX+1,iY-1)) return PF_Crawl_Top;
		// Bottom right
		if (!PointFree(iX+1,iY+1)) return PF_Crawl_Right;
		}
	return PF_Crawl_NoAttach;
	}

BOOL CPathFinderRay::CheckBackRayShorten()
	{
	CPathFinderRay *pRay,*pRay2;
	int iX,iY;
	for (pRay=From; pRay; pRay=pRay->From)
		{
		if (pRay==From) continue;
		iX=X; iY=Y;
		if (PathFree(iX,iY,pRay->X,pRay->Y))
			{
			// Delete jumped rays
			for (pRay2=From; pRay2!=pRay; pRay2=pRay2->From)
				pRay2->Status=PF_Ray_Deleted;
			// Shorten pRay to this
			pRay->X2=X; pRay->Y2=Y;
			From=pRay;
			// Success
			return TRUE;
			}
		}
	return FALSE;
	}

//------------------------------- CPathFinder ---------------------------------------------

CPathFinder::CPathFinder()
	{
	Default();
	}

CPathFinder::~CPathFinder()
	{
	Clear();
	}

void CPathFinder::Default()
	{
	PointFree=NULL;
	SetWaypoint=NULL;
	FirstRay=NULL;
	WaypointParameter=0;
	Success=FALSE;
	MaxDepth=50;
	MaxCrawl=1000;
	MaxRay=500;
	Threshold=10;
	}

void CPathFinder::Clear()
	{
	CPathFinderRay *pRay,*pNext;
	for (pRay=FirstRay; pRay; pRay=pNext) { pNext=pRay->Next; delete pRay; }
	FirstRay=NULL;
	}

void CPathFinder::Init(BOOL (*fnPointFree)(int, int, int), int iPointFreeParameter, int iDepth, int iCrawl, int iRay, int iThreshold)
	{
	// Set data
	PointFree = fnPointFree;
	PointFreeParameter = iPointFreeParameter;
	MaxDepth=iDepth;
	MaxCrawl=iCrawl;
	MaxRay=iRay;
	Threshold=iThreshold;
	}

/*void CPathFinder::Draw(C4FacetEx &cgo)
	{
	for (CPathFinderRay *pRay=FirstRay; pRay; pRay=pRay->Next)
		pRay->Draw(cgo);
	}*/

void CPathFinder::Run()
	{
	Success=FALSE;
	while (!Success && Execute());
	}

BOOL CPathFinder::Execute()
	{

	// Execute & count rays
	BOOL fContinue=FALSE;
	int iRays=0;
	for (CPathFinderRay *pRay=FirstRay; pRay && !Success; pRay=pRay->Next,iRays++)
		if (pRay->Execute())
			fContinue=TRUE;

	// Max ray limit
	if (iRays>=MaxRay) return FALSE;

	return fContinue;
	}

BOOL CPathFinder::Find(int iFromX, int iFromY, int iToX, int iToY, BOOL (*fnSetWaypoint)(int, int, int), int iWaypointParameter)
	{

	// Prepare
	Clear();

	// Parameter safety
	if (!fnSetWaypoint) return FALSE;
	SetWaypoint=fnSetWaypoint;
	WaypointParameter=iWaypointParameter;

	// Start & target coordinates must be free
	if (!PointFree(iFromX,iFromY,PointFreeParameter) || !PointFree(iToX,iToY,PointFreeParameter)) return FALSE;

	// Add the first two rays
	if (!AddRay(iFromX,iFromY,iToX,iToY,0,PF_Direction_Left,NULL)) return FALSE;
	if (!AddRay(iFromX,iFromY,iToX,iToY,0,PF_Direction_Right,NULL)) return FALSE;

	// Run
	Run();

	// Success
	return Success;

	}

BOOL CPathFinder::AddRay(int iFromX, int iFromY, int iToX, int iToY, int iDepth, int iDirection, CPathFinderRay *pFrom)
	{
	// Max depth
	if (iDepth>=MaxDepth) return FALSE;
	// Allocate and set new ray
	CPathFinderRay *pRay;
	if (!(pRay = new CPathFinderRay)) return FALSE;
	pRay->X=iFromX; pRay->Y=iFromY;
	pRay->X2=iFromX; pRay->Y2=iFromY;
	pRay->TargetX=iToX; pRay->TargetY=iToY;
	pRay->Depth=iDepth;
	pRay->Direction=iDirection;
	pRay->From=pFrom;
	pRay->pPathFinder=this;
	pRay->Next=FirstRay;
	FirstRay=pRay;
	return TRUE;
	}

BOOL CPathFinder::SplitRay(CPathFinderRay *pRay, int iAtX, int iAtY)
	{
	// Max depth
	if (pRay->Depth>=MaxDepth) return FALSE;
	// Allocate and set new ray
	CPathFinderRay *pNewRay;
	if (!(pNewRay = new CPathFinderRay)) return FALSE;
	pNewRay->Status=PF_Ray_Still;
	pNewRay->X=pRay->X; pNewRay->Y=pRay->Y;
	pNewRay->X2=iAtX; pNewRay->Y2=iAtY;
	pNewRay->TargetX=pRay->TargetX; pNewRay->TargetY=pRay->TargetY;
	pNewRay->Depth=pRay->Depth;
	pNewRay->Direction=pRay->Direction;
	pNewRay->From=pRay->From;
	pNewRay->pPathFinder=this;
	pNewRay->Next=FirstRay;
	FirstRay=pNewRay;
	// Adjust split ray
	pRay->From=pNewRay;
	pRay->X=iAtX; pRay->Y=iAtY;
	return TRUE;
	}

