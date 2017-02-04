/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32
#define _WIN32
#endif
#define  STRICT
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include "..\..\SoE DirectX\Inc\Ddraw.h"

#define RBDIRECTDRAW LPDIRECTDRAW4
#define RBDIRECTDRAWSURFACE LPDIRECTDRAWSURFACE4
#define RBSURFACEDESC DDSURFACEDESC2

/* USER SETTINGS, YOU NEED TO SET THESE FOR YOUR PROJECT! */
/* uncomment the next line to use Blt instead of BltFast (see docs) */
//#define RBUSEBLTONLY

/* uncomment the SYSTEMMEMORY line to put tiles in system mem (requires Blt, see docs) */
/* only one of the next two lines should be uncommented */
#define RBSCAPS_TYPE DDSCAPS_OFFSCREENPLAIN
//#define RBSCAPS_TYPE DDSCAPS_SYSTEMMEMORY

/* These define the width and height of the drawn area... */
#define MSCRW 640
#define MSCRH 480

/* These define how far from the left and top of the surface the drawn area is */
#define MMOX 0
#define MMOY 0

#define MBSW 16			/* Width of block, in pixels */
#define MBSH 16			/* Height of block, in pixels */

/* Size of Parallax tile, ignore if not using Parallax */
#define PARAW 128			/* Width of parallax block, in pixels, MUST be multiple of MBSW */
#define PARAH 128			/* Height of parallax block, in pixels, MUST be multiple of MBSH */

#define MTRANSP8 0		/* Transparency index for paletted screen (0 to 255)*/
#define MTRANSPR 0xFF	/* 8bit RED transparency value for high/true colour */
#define MTRANSPG 0x00	/* 8bit GREEN transparency value for high/true colour */
#define MTRANSPB 0xFF	/* 8bit BLUE transparency value for high/true colour */
/* END OF USER SETTINGS */

#define MER_NONE 0		/* All the horrible things that can go wrong */
#define MER_OUTOFMEM 1
#define MER_MAPLOADERROR 2
#define MER_NOOPEN 3

#define AN_END -1			/* Animation types, AN_END = end of anims */
#define AN_NONE 0			/* No anim defined */
#define AN_LOOPF 1		/* Loops from start to end, then jumps to start etc */
#define AN_LOOPR 2		/* As above, but from end to start */
#define AN_ONCE 3			/* Only plays once */
#define AN_ONCEH 4		/* Only plays once, but holds end frame */
#define AN_PPFF 5			/* Ping Pong start-end-start-end-start etc */
#define AN_PPRR 6			/* Ping Pong end-start-end-start-end etc */
#define AN_PPRF 7			/* Used internally by playback */
#define AN_PPFR 8			/* Used internally by playback */
#define AN_ONCES 9		/* Used internally by playback */

typedef struct {				/* Structure for data blocks */
RBDIRECTDRAWSURFACE bgoff, fgoff;			/* offsets from start of graphic blocks */
RBDIRECTDRAWSURFACE fgoff2, fgoff3; 		/* more overlay blocks */
unsigned long int user1, user2;	/* user long data */
unsigned short int user3, user4;	/* user short data */
unsigned char user5, user6, user7;	/* user byte data */
unsigned char tl : 1;				/* bits for collision detection */
unsigned char tr : 1;
unsigned char bl : 1;
unsigned char br : 1;
unsigned char trigger : 1;			/* bit to trigger an event */
unsigned char unused1 : 1;
unsigned char unused2 : 1;
unsigned char unused3 : 1;
} BLKSTR;

typedef struct {		/* Animation control structure */
signed char antype;	/* Type of anim, AN_? */
signed char andelay;	/* Frames to go before next frame */
signed char ancount;	/* Counter, decs each frame, till 0, then resets to andelay */
signed char anuser;	/* User info */
long int ancuroff;	/* Points to current offset in list */
long int anstartoff;	/* Points to start of blkstr offsets list, AFTER ref. blkstr offset */
long int anendoff;	/* Points to end of blkstr offsets list */
} ANISTR;

typedef struct {			/* Generic structure for chunk headers */
char id1, id2, id3, id4;	/* 4 byte header id. */
long int headsize;		/* size of header chunk. */
} GENHEAD;

typedef struct {		/* Map header structure */
char mapverhigh;		/* map version number to left of . (ie X.0). */
char mapverlow;		/* map version number to right of . (ie 0.X). */
char lsb;			/* if 1, data stored LSB first, otherwise MSB first. */
char reserved;
short int mapwidth;	/* width in blocks. */
short int mapheight;	/* height in blocks. */
short int reserved1;
short int reserved2;
short int blockwidth;	/* width of a block (tile) in pixels. */
short int blockheight;	/* height of a block (tile) in pixels. */
short int blockdepth;	/* depth of a block (tile) in planes (ie. 256 colours is 8) */
short int blockstrsize;	/* size of a block data structure */
short int numblockstr;	/* Number of block structures in BKDT */
short int numblockgfx;	/* Number of 'blocks' in graphics (BGFX) */
} MPHD;

/* All global variables used by Mappy playback are here */
int maperror;		/* Set to a MER_ error if something wrong happens */
short int mapwidth, mapheight, mapblockwidth, mapblockheight, mapdepth;
short int mapblockstrsize, mapnumblockstr, mapnumblockgfx;
FILE * mapfilept;
short int * mappt = NULL;
short int ** maparraypt = NULL;
unsigned char * mapcmappt = NULL;
char * mapblockgfxpt = NULL;
char * mapblockstrpt = NULL;
char * mapanimstrpt = NULL;
char * mapanimstrendpt;
GENHEAD mapgenheader;
PALETTEENTRY mappept[256];
RBDIRECTDRAWSURFACE maplpDDSTiles[1024];	// Tiles Direct Draw Surfaces
RBDIRECTDRAWSURFACE lpDDSParallax = NULL;
short int * mapmappt[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
short int ** mapmaparraypt[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
/* End of Mappy globals */


void MapReleaseParallaxSurface (void)
{
	if (lpDDSParallax!=NULL) { lpDDSParallax->Release (); lpDDSParallax = NULL; }
}

void MapRestoreParallaxSurface (void* srcpt, int pitch)
{
int i;
unsigned char * destpt;
RBSURFACEDESC mddsd;
RECT cliprect;

	if (lpDDSParallax==NULL) return;
	if (lpDDSParallax->IsLost ()==DDERR_SURFACELOST) {
		if (lpDDSParallax->Restore ()!=DD_OK) return;
	}

/* Must restore surface contents here! */
	memset (&mddsd, 0, sizeof(RBSURFACEDESC));
	mddsd.dwSize = sizeof(RBSURFACEDESC);
	if (lpDDSParallax->Lock (NULL, &mddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL)!=DD_OK) return;
	destpt = (unsigned char *) mddsd.lpSurface;
	for (i=0;i<PARAH;i++) {
		memcpy (destpt+(i*mddsd.lPitch), ((char *) srcpt)+(i*pitch), PARAW*((mapdepth+1)/8));
	}
	lpDDSParallax->Unlock (NULL);

/* OK, now the important part, must duplicate the top row and left column
 * in the extra space allocated
 */

	cliprect.left = 0; cliprect.right = PARAW;
	cliprect.top = 0; cliprect.bottom = MBSH;
	lpDDSParallax->BltFast (0, PARAH, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
	cliprect.left = 0; cliprect.right = MBSW;
	cliprect.top = 0; cliprect.bottom = PARAH+MBSH;
	lpDDSParallax->BltFast (PARAW, 0, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );

	return;
}

int MapCreateParallaxSurface (RBDIRECTDRAW lpDD)
/* BITMAP width must be a multiple of mapblockwidth
 * BITMAP height must be a multiple of mapblockheight
 */
{
RBSURFACEDESC mddsd;
DDCOLORKEY ddck;

	if (lpDDSParallax!=NULL) return -1;

	memset (&mddsd, 0, sizeof (mddsd));
	memset (&ddck, 0, sizeof (ddck));
	mddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	mddsd.ddsCaps.dwCaps = RBSCAPS_TYPE;
	mddsd.dwHeight = PARAW+MBSW;	/* Width is desired width + blockwidth */
	mddsd.dwWidth = PARAH+MBSH;	/* Height is desired height + block height */
	mddsd.dwSize = sizeof (mddsd);
	lpDD->CreateSurface (&mddsd, &lpDDSParallax, NULL);
	if (lpDDSParallax==NULL) return -1;
	lpDDSParallax->SetColorKey( DDCKEY_SRCBLT, &ddck );	/* Just setting to 0 for this test */

	return 0;
}

int MapDrawParallax (RBDIRECTDRAWSURFACE lpDDSDest, int mapxo, int mapyo)
/* NOTE: I've made it so you pass the x and y of the non-parallax layer,
 * ie the same params as to MapDrawBGT
 *
 * lpDDSDest = lpdirectdrawsurface.
 * mapxo = offset, in pixels, from the left edge of the map.
 * mapyo = offset, in pixels, from the top edge of the map.
 */
{
RECT cliprect;
#ifdef RBUSEBLTONLY
RECT dstrect;
#endif
int i, j, k, mapvclip, maphclip, indentx, indenty, temptop, templeft;
short int *mymappt;
short int *mymap2pt;
BLKSTR *blkdatapt;
ANISTR *myanpt;

	if (lpDDSParallax==NULL) return -1;

	maphclip = mapxo%PARAW;	/* Number of pixels to clip from left */
	mapvclip = mapyo%PARAH;	/* Number of pixels to clip from top */
	indentx = ((mapxo/2)%PARAW)-maphclip;
	indenty = ((mapyo/2)%PARAH)-mapvclip;

/* Draw top left clipped block */
	cliprect.left = maphclip+indentx;
	cliprect.top = mapvclip+indenty;
	cliprect.right = MBSW+indentx;
	cliprect.bottom = MBSH+indenty;
	while (cliprect.right<=cliprect.left) cliprect.right += MBSW;
	while (cliprect.bottom<=cliprect.top) cliprect.bottom += MBSH;
	temptop = cliprect.top;

#ifdef RBUSEBLTONLY
	dstrect.left = MMOX;
	dstrect.top = MMOY;
	dstrect.right = MMOX+(MBSW-(mapxo%MBSW));
	dstrect.bottom = MMOY+(MBSH-(mapyo%MBSH));
#endif

	mymappt = (short int *) mappt;
	mymappt += (mapxo/MBSW)+((mapyo/MBSH)*mapwidth);
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( MMOX, MMOY, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
	for (k=-1;k<(mapvclip/MBSH);k++) {
		indenty += MBSH; if (indenty>=PARAH) indenty -= PARAH;
	}

/* Draw left clipped blocks */
	cliprect.top = indenty;
	cliprect.bottom = MBSH+indenty;

	i = MMOY+MBSH-(mapvclip%MBSH); while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( MMOX, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
		i += MBSH;
		cliprect.bottom += MBSH;
		cliprect.top += MBSH; if (cliprect.top>=PARAH) { cliprect.top -= PARAH; cliprect.bottom -= PARAH; }
		indenty += MBSH; if (indenty>=PARAH) indenty -= PARAH;
	}

/* Draw bottom left clipped block */
	mymappt += mapwidth;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	cliprect.bottom = indenty+(MBSH-(i-(MSCRH+MMOY-MBSH)));
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
	{
		dstrect.top = i;
		dstrect.bottom = MMOY+MSCRH;
		lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
	}
#else
		lpDDSDest->BltFast( MMOX, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
	cliprect.right += MBSW;
	cliprect.left += MBSW; if (cliprect.left>=PARAW) { cliprect.left -= PARAW; cliprect.right -= PARAW; }
	for (k=-1;k<(maphclip/MBSW);k++) {
		indentx += MBSW; if (indentx>=PARAW) indentx -= PARAW;
	}

/* Draw bottom clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.top = i;
	dstrect.bottom = MMOY+MSCRH;
#endif
	cliprect.left = indentx;
	cliprect.right = MBSW+indentx;

	j = MMOX+MBSW-(maphclip%MBSW); while (j<(MSCRW+MMOX-MBSW))
	{
		mymappt ++;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.left= j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
		j += MBSW;
		cliprect.right += MBSW;
		cliprect.left += MBSW; if (cliprect.left>=PARAW) { cliprect.left -= PARAW; cliprect.right -= PARAW; }
		indentx += MBSW; if (indentx>=PARAW) indentx -= PARAW;
	}

/* Draw bottom right clipped block */
	cliprect.right = indentx+(MBSW-(j-(MSCRW+MMOX-MBSW)));
	mymappt ++;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
	{
		dstrect.left= j;
		dstrect.right = MMOX+MSCRW;
		lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
	}
#else
		lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif

/* Draw right clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.left= j;
	dstrect.right = MMOX+MSCRW;
#endif
	cliprect.bottom = MBSH+indenty;
	while (i>(MMOY+MBSH))
	{
		cliprect.bottom -= MBSH;
		cliprect.top -= MBSH; if (cliprect.top<0) { cliprect.top += PARAH; cliprect.bottom += PARAH; }
		indenty -= MBSH; if (indenty<0) indenty += PARAH;
		mymappt -= mapwidth; i -= MBSH;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
	}

/* Draw top right clipped block */
#ifdef RBUSEBLTONLY
	dstrect.top = MMOY;
	dstrect.bottom = i;
#endif
	cliprect.bottom -= MBSH;
	cliprect.top -= MBSH; if (cliprect.top<0) { cliprect.top += PARAH; cliprect.bottom += PARAH; }
	indenty -= MBSH; if (indenty<0) indenty += PARAH;
	mymappt -= mapwidth; i = MMOY;
	cliprect.top = temptop; cliprect.bottom = MBSH+temptop-mapvclip;
	while (cliprect.bottom<=cliprect.top) cliprect.bottom += MBSH;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif

/* Draw top clipped blocks */
	cliprect.right = MBSW+cliprect.left;

	while (j>(MMOX+MBSW))
	{
		cliprect.right -= MBSW;
		cliprect.left -= MBSW; if (cliprect.left<0) { cliprect.left += PARAW; cliprect.right += PARAW; }
		indentx -= MBSW; if (indentx<0) indentx += PARAW;
		mymappt --; j -= MBSW;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.left = j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT );
#endif
	}

/* Draw unclipped (full) blocks */
	cliprect.top = indenty; cliprect.bottom = indenty+MBSH;
	cliprect.right -= MBSW;
	cliprect.left -= MBSW; if (cliprect.left<0) { cliprect.left += PARAW; cliprect.right += PARAW; }
	templeft = cliprect.left;

	i = MMOY+MBSH-(mapvclip%MBSH);
	while (i<(MSCRH+MMOY-MBSH))
	{
		cliprect.bottom += MBSH;
		cliprect.top += MBSH; if (cliprect.top>=PARAH) { cliprect.top -= PARAH; cliprect.bottom -= PARAH; }
		cliprect.left = templeft; cliprect.right = templeft+MBSW;
		mymappt += mapwidth;
		mymap2pt = mymappt;
		j = MMOX+MBSW-(maphclip%MBSW); while (j<(MSCRW+MMOX-MBSW))
		{
			cliprect.right += MBSW;
			cliprect.left += MBSW; if (cliprect.left>=PARAW) { cliprect.left -= PARAW; cliprect.right -= PARAW; }
			if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
			else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
				blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
			if (blkdatapt->trigger) {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, lpDDSParallax, &cliprect, DDBLT_WAIT, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast( j, i, lpDDSParallax, &cliprect, DDBLTFAST_WAIT )!=DD_OK)
#endif
					return -1; }
			mymappt++; j += MBSW;
		}
		mymappt = mymap2pt;
		i += MBSH;
	}
	return 0;
}

/* End of Parallax functions */

unsigned long int Mapbyteswapl (unsigned long int i)
{
unsigned long int j;
	j = i&0xFF; j <<= 8; i >>= 8; j |= i&0xFF; j <<= 8; i >>= 8;
	j |= i&0xFF; j <<= 8; i >>= 8; j |= i&0xFF; return j;
}

void MapFreeMem (void)
{
int i;

	MapReleaseParallaxSurface ();
	for (i=0;i<8;i++) { if (mapmappt[i]!=NULL) { free (mapmappt[i]); mapmappt[i] = NULL; } }
	mappt = NULL;
	for (i=0;i<8;i++) { if (mapmaparraypt[i]!=NULL) { free (mapmaparraypt[i]); mapmaparraypt[i] = NULL; } }
	maparraypt = NULL;
	if (mapcmappt!=NULL) { free (mapcmappt); mapcmappt = NULL; }
	if (mapblockgfxpt!=NULL) { free (mapblockgfxpt); mapblockgfxpt = NULL; }
	if (mapblockstrpt!=NULL) { free (mapblockstrpt); mapblockstrpt = NULL; }
	if (mapanimstrpt!=NULL) { free (mapanimstrpt); mapanimstrpt = NULL; }

	for (i=0;(maplpDDSTiles[i]!=NULL);i++)
	{ maplpDDSTiles[i]->Release(); maplpDDSTiles[i] = NULL; }
}

int MapGenerateYLookup (void)
{
int i, j;

	for (i=0;i<8;i++) {
		if (mapmaparraypt[i]!=NULL) { free (mapmaparraypt[i]); mapmaparraypt[i] = NULL; }
		if (mapmappt[i]!=NULL) {
			mapmaparraypt[i] = (short int **) malloc (mapheight*sizeof(short int *));
			if (mapmaparraypt[i] == NULL) return -1;
			for (j=0;j<mapheight;j++) mapmaparraypt[i][j] = (mapmappt[i]+(j*mapwidth));
			if (mapmappt[i] == mappt) maparraypt = mapmaparraypt[i];
		}
	}
	return 0;
}

BLKSTR * MapGetBlock (int x, int y)
{
short int * mymappt;
ANISTR * myanpt;

	if (maparraypt!= NULL) {
		mymappt = maparraypt[y]+x;
	} else {
		mymappt = mappt;
		mymappt += x;
		mymappt += y*mapwidth;
	}
	if (*mymappt>=0) return (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		return (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
}

void MapSetBlock (int x, int y, int strvalue)
{
short int * mymappt;

	if (maparraypt!= NULL) {
		mymappt = maparraypt[y]+x;
	} else {
		mymappt = mappt;
		mymappt += x;
		mymappt += y*mapwidth;
	}
	if (strvalue>=0) *mymappt = strvalue*sizeof(BLKSTR);
	else *mymappt = strvalue*sizeof(ANISTR);
}

int MapChangeLayer (int newlyr)
{
	if (newlyr<0 || newlyr>7 || mapmappt[newlyr] == NULL) return -1;
	mappt = mapmappt[newlyr]; maparraypt = mapmaparraypt[newlyr];
	return newlyr;
}

void MapCMAPtoPE (unsigned char * mycmappt, PALETTEENTRY * mypept)
{
int i;
	for (i=0;i<256;i++)
	{
		mypept[i].peRed = (BYTE) *mycmappt; mycmappt++;
		mypept[i].peGreen = (BYTE) *mycmappt; mycmappt++;
		mypept[i].peBlue = (BYTE) *mycmappt; mycmappt++;
		mypept[i].peFlags = 0;
	}
}

int MapHighTo8 (void)
{
int i, j;
char * tempblockgfxpt;
unsigned char * temp24bitbuffer, * blkgfxpt;
HDC temphdc, myhdc;
HBITMAP htempbm, holdbm;
BITMAPINFOHEADER bih;
RGBQUAD rgbq;

/* This isn't needed, but left for reference
	if (mapdepth == 8)
	{
		memset (&tempbm, 0, sizeof(BITMAP));
		tempbm.bmWidth = mapblockwidth;
		tempbm.bmHeight = mapblockheight;
		tempbm.bmWidthBytes = mapblockwidth;
		tempbm.bmPlanes = 1;
		tempbm.bmBitsPixel = mapdepth;
	} else {
*/
	memset (&bih, 0, sizeof(BITMAPINFOHEADER));
	memset (&rgbq, 0, sizeof(RGBQUAD));
	bih.biSize = sizeof (BITMAPINFOHEADER);
	bih.biWidth = mapblockwidth;
	bih.biHeight = -mapblockheight;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;

	temp24bitbuffer = (unsigned char *) malloc (mapblockwidth*mapblockheight*3);
	if (temp24bitbuffer == NULL) return -1;
/* This isn't needed, but left for reference
	}
*/
	tempblockgfxpt = mapblockgfxpt;
	temphdc = CreateCompatibleDC (NULL);
	for (i=0;i<mapnumblockgfx;i++)
	{
		if ((maplpDDSTiles[i]->GetDC (&myhdc)) != DD_OK) return -1;
		switch (mapdepth)
		{
/* This isn't needed, but left for reference
		case 8:
			tempbm.bmBits = tempblockgfxpt;
			htempbm = CreateBitmapIndirect (&tempbm);
			if (htempbm == NULL) return -1;
			break;
*/
		case 15:
			blkgfxpt = (unsigned char *) tempblockgfxpt;
			for (j=0;j<(mapblockwidth*mapblockheight*3);j+=3)
			{
				temp24bitbuffer[0+j] = (unsigned char) ((*(blkgfxpt+1)&0x1F)<<3);
				temp24bitbuffer[1+j] = (unsigned char) (((((*blkgfxpt)&0x3)<<3)|((*(blkgfxpt+1))>>5))<<3);
				temp24bitbuffer[2+j] = (unsigned char) (((*blkgfxpt)&0x7C)<<1);
				blkgfxpt += 2;
			}
			htempbm = CreateDIBitmap (myhdc, &bih, CBM_INIT,
				 temp24bitbuffer, (BITMAPINFO *) &bih, DIB_RGB_COLORS);
			if (htempbm == NULL) return -1;
			break;
		case 16:
			blkgfxpt = (unsigned char *) tempblockgfxpt;
			for (j=0;j<(mapblockwidth*mapblockheight*3);j+=3)
			{
				temp24bitbuffer[0+j] = (unsigned char) ((*(blkgfxpt+1)&0x1F)<<3);
				temp24bitbuffer[1+j] = (unsigned char) (((((*blkgfxpt)&0x7)<<3)|((*(blkgfxpt+1))>>5))<<2);
				temp24bitbuffer[2+j] = (unsigned char) ((*blkgfxpt)&0xF8);
				blkgfxpt += 2;
			}
			htempbm = CreateDIBitmap (myhdc, &bih, CBM_INIT,
				 temp24bitbuffer, (BITMAPINFO *) &bih, DIB_RGB_COLORS);
			if (htempbm == NULL) return -1;
			break;
		case 24:
			blkgfxpt = (unsigned char *) tempblockgfxpt;
			for (j=0;j<(mapblockwidth*mapblockheight*3);j+=3)
			{
				temp24bitbuffer[2+j] = *blkgfxpt; blkgfxpt++;
				temp24bitbuffer[1+j] = *blkgfxpt; blkgfxpt++;
				temp24bitbuffer[0+j] = *blkgfxpt; blkgfxpt++;
			}
			htempbm = CreateDIBitmap (myhdc, &bih, CBM_INIT,
				 temp24bitbuffer, (BITMAPINFO *) &bih, DIB_RGB_COLORS);
			if (htempbm == NULL) return -1;
			break;
		case 32:
			blkgfxpt = (unsigned char *) tempblockgfxpt;
			for (j=0;j<(mapblockwidth*mapblockheight*3);j+=3)
			{
				blkgfxpt ++;
				temp24bitbuffer[2+j] = *blkgfxpt; blkgfxpt++;
				temp24bitbuffer[1+j] = *blkgfxpt; blkgfxpt++;
				temp24bitbuffer[0+j] = *blkgfxpt; blkgfxpt++;
			}
			htempbm = CreateDIBitmap (myhdc, &bih, CBM_INIT,
				 temp24bitbuffer, (BITMAPINFO *) &bih, DIB_RGB_COLORS);
			if (htempbm == NULL) return -1;
			break;
		}
		holdbm = (HBITMAP) SelectObject (temphdc, htempbm);
		BitBlt (myhdc, 0, 0, mapblockwidth, mapblockheight, temphdc, 0, 0, SRCCOPY);
		SelectObject (temphdc, holdbm);
		DeleteObject (htempbm);
		maplpDDSTiles[i]->ReleaseDC(myhdc);
		tempblockgfxpt += mapblockwidth*mapblockheight*((mapdepth+1)/8);
	}
	DeleteDC (temphdc);
/* This line was replaced by the one below
	if (mapdepth != 8) free (temp24bitbuffer);
*/
	free (temp24bitbuffer);
	return 0;
}

int MapCopyBlocksToSurfaces (void)
{
unsigned int i, j, k, rshift, gshift, bshift, rbits, gbits, bbits, rval, gval, bval;
unsigned char * tempblockgfxpt, * tempsurfacept;
RBSURFACEDESC mddsd;
DDPIXELFORMAT mddpf;

	tempblockgfxpt = (unsigned char *) mapblockgfxpt;
	memset (&mddpf, 0, sizeof(DDPIXELFORMAT));
	mddpf.dwSize = sizeof(DDPIXELFORMAT);
	if (maplpDDSTiles[0]->GetPixelFormat(&mddpf)!=DD_OK) return -1;
	if (mddpf.dwRGBBitCount>8) {
		j = (int) mddpf.dwRBitMask; rshift = 0;
		i = 1; while (!(i&j)) { rshift++; i<<=1; }
		rbits = 0; while (i&j) { rbits++; i<<=1; }
		j = (int) mddpf.dwGBitMask; gshift = 0;
		i = 1; while (!(i&j)) { gshift++; i<<=1; }
		gbits = 0; while (i&j) { gbits++; i<<=1; }
		j = (int) mddpf.dwBBitMask; bshift = 0;
		i = 1; while (!(i&j)) { bshift++; i<<=1; }
		bbits = 0; while (i&j) { bbits++; i<<=1; }
	}

	if (mapdepth!=8 && mddpf.dwRGBBitCount==8) MapHighTo8 ();
	if (mapdepth==8 && mddpf.dwRGBBitCount==8) {
		for (i=0;i<(unsigned)mapnumblockgfx;i++)
		{
			memset (&mddsd, 0, sizeof(RBSURFACEDESC));
			mddsd.dwSize = sizeof(RBSURFACEDESC);
			if (maplpDDSTiles[i]->Lock (NULL, &mddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL)!=DD_OK) return -1;
			tempsurfacept = (unsigned char *) mddsd.lpSurface;
			for (j=mapblockheight;j>0;j--)
			{
				memcpy (tempsurfacept, tempblockgfxpt, mapblockwidth);
				tempsurfacept += mddsd.lPitch;
				tempblockgfxpt += mapblockwidth;
			}
			maplpDDSTiles[i]->Unlock (NULL);
		}
	} else {

	for (i=0;i<(unsigned)mapnumblockgfx;i++)
	{
		memset (&mddsd, 0, sizeof(RBSURFACEDESC));
		mddsd.dwSize = sizeof(RBSURFACEDESC);
		if (maplpDDSTiles[i]->Lock (NULL, &mddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL)!=DD_OK) return -1;
		for (j=0;j<(unsigned)mapblockheight;j++) {
		tempsurfacept = (unsigned char *) mddsd.lpSurface;
		tempsurfacept += j*mddsd.lPitch;
		for (k=mapblockwidth;k>0;k--) {
		switch (mapdepth) {
			case 8:
				rval = mapcmappt[tempblockgfxpt[0]*3];
				rval >>= (8-rbits);
				gval = mapcmappt[tempblockgfxpt[0]*3+1];
				gval >>= (8-gbits);
				bval = mapcmappt[tempblockgfxpt[0]*3+2];
				bval >>= (8-bbits);
				tempblockgfxpt++;
				break;
			case 15:
				rval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0x7E00;
				rval >>= (7+(8-rbits));
				gval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0x03E0;
				gval >>= (2+(8-gbits));
				bval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0x001F;
				bval <<= 3;
				bval >>= (8-bbits);
				tempblockgfxpt += 2;
				break;
			case 16:
				rval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0xF800;
				rval >>= (8+(8-rbits));
				gval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0x07E0;
				gval >>= (3+(8-gbits));
				bval = ((((unsigned long int)tempblockgfxpt[0])<<8)|((unsigned long int)tempblockgfxpt[1]))&0x001F;
				bval <<= 3;
				bval >>= (8-bbits);
				tempblockgfxpt += 2;
				break;
			case 24:
				rval = (unsigned long int) tempblockgfxpt[0];
				rval >>= (8-rbits);
				gval = (unsigned long int) tempblockgfxpt[1];
				gval >>= (8-gbits);
				bval = (unsigned long int) tempblockgfxpt[2];
				bval >>= (8-bbits);
				tempblockgfxpt += 3;
				break;
			case 32:
				rval = (unsigned long int) tempblockgfxpt[1];
				rval >>= (8-rbits);
				gval = (unsigned long int) tempblockgfxpt[2];
				gval >>= (8-gbits);
				bval = (unsigned long int) tempblockgfxpt[3];
				bval >>= (8-bbits);
				tempblockgfxpt += 4;
				break;
			default:
				return -1;
		}
		rval <<= rshift; rval &= mddpf.dwRBitMask;
		gval <<= gshift; gval &= mddpf.dwGBitMask;
		bval <<= bshift; bval &= mddpf.dwBBitMask;
		switch (mddpf.dwRGBBitCount) {
/* Apparently 16bits are reported even in 555 (15bit) mode */
			case 16:
				*((unsigned short int *)tempsurfacept) = (unsigned short int) (rval|gval|bval);
				tempsurfacept += 2;
				break;
			case 24:
				tempsurfacept[2] = (unsigned char) (((rval|gval|bval)>>16)&0xFF);
				tempsurfacept[1] = (unsigned char) (((rval|gval|bval)>>8)&0xFF);
				tempsurfacept[0] = (unsigned char) ((rval|gval|bval)&0xFF);
				tempsurfacept += 3;
				break;
			case 32:
				tempsurfacept[3] = (unsigned char) (((rval|gval|bval)>>24)&0xFF);
				tempsurfacept[2] = (unsigned char) (((rval|gval|bval)>>16)&0xFF);
				tempsurfacept[1] = (unsigned char) (((rval|gval|bval)>>8)&0xFF);
				tempsurfacept[0] = (unsigned char) ((rval|gval|bval)&0xFF);
				tempsurfacept += 4;
				break;
		}
	} }
		maplpDDSTiles[i]->Unlock (NULL);
	}
	}
	return 0;
}

int MapRestore (void)
{
int i;

	i = 0;
	while (maplpDDSTiles[i] != NULL)
	{
		if (maplpDDSTiles[i]->Restore() != DD_OK) return -1;
		i++;
	}
	return MapCopyBlocksToSurfaces ();
}

DWORD MapTranspToPixel (LPDDPIXELFORMAT lpDDpf)
{
DWORD transpresult;
unsigned int i, j, rshift, gshift, bshift, rbits, gbits, bbits;

		transpresult = 0;
		if (lpDDpf->dwRGBBitCount>8) {
			j = (int) lpDDpf->dwRBitMask; rshift = 0;
			i = 1; while (!(i&j)) { rshift++; i<<=1; }
			rbits = 0; while (i&j) { rbits++; i<<=1; }
			j = (int) lpDDpf->dwGBitMask; gshift = 0;
			i = 1; while (!(i&j)) { gshift++; i<<=1; }
			gbits = 0; while (i&j) { gbits++; i<<=1; }
			j = (int) lpDDpf->dwBBitMask; bshift = 0;
			i = 1; while (!(i&j)) { bshift++; i<<=1; }
			bbits = 0; while (i&j) { bbits++; i<<=1; }

			transpresult = (((MTRANSPR<<rshift)>>(8-rbits))&lpDDpf->dwRBitMask) |
								(((MTRANSPG<<gshift)>>(8-gbits))&lpDDpf->dwGBitMask) |
								(((MTRANSPB<<bshift)>>(8-bbits))&lpDDpf->dwBBitMask);
		}
		return transpresult;
}

int MapRelocate (RBDIRECTDRAW lpDD)
{
int i, j;
BLKSTR * myblkstrpt;
ANISTR * myanpt;
long int * myanblkpt;
RBSURFACEDESC mddsd;
DDPIXELFORMAT mddpf;
DDCOLORKEY ddck;
DWORD transpcol;

	memset (&mddsd, 0, sizeof (mddsd));
	memset (&mddpf, 0, sizeof (mddpf));
	memset (&ddck, 0, sizeof (ddck));

	mddpf.dwSize = sizeof (mddpf);

	for (i=0;i<mapnumblockgfx;i++)
	{
		mddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		mddsd.ddsCaps.dwCaps = RBSCAPS_TYPE;
		mddsd.dwHeight = mapblockheight;
		mddsd.dwWidth = mapblockwidth;
		mddsd.dwSize = sizeof (mddsd);
		if (lpDD->CreateSurface (&mddsd, &maplpDDSTiles[i], NULL)!= DD_OK)
			{ MapFreeMem (); return -1; }

		if (!i) {
			if (maplpDDSTiles[i]->GetPixelFormat(&mddpf)!=DD_OK) { MapFreeMem (); return -1; }
			if (mddpf.dwRGBBitCount!=8) { transpcol = MapTranspToPixel (&mddpf); }
			else transpcol = MTRANSP8;
		}
		ddck.dwColorSpaceLowValue = transpcol;
		ddck.dwColorSpaceHighValue = transpcol;
		maplpDDSTiles[i]->SetColorKey( DDCKEY_SRCBLT, &ddck );
	}
	maplpDDSTiles[i] = NULL;
	if (MapCopyBlocksToSurfaces ()) { MapFreeMem (); return -1; }

	i = mapnumblockstr; j = mapblockwidth*mapblockheight*((mapdepth+1)/8);
	myblkstrpt = (BLKSTR *) mapblockstrpt;
	while (i)
	{
		myblkstrpt->bgoff = maplpDDSTiles[(((long int) myblkstrpt->bgoff)/j)];
		if (myblkstrpt->fgoff!=0) myblkstrpt->fgoff = maplpDDSTiles[(((long int) myblkstrpt->fgoff)/j)];
		if (myblkstrpt->fgoff2!=0) myblkstrpt->fgoff2 = maplpDDSTiles[(((long int) myblkstrpt->fgoff2)/j)];
		if (myblkstrpt->fgoff3!=0) myblkstrpt->fgoff3 = maplpDDSTiles[(((long int) myblkstrpt->fgoff3)/j)];
		myblkstrpt++; i--;
	}

	if (mapanimstrpt!=NULL) {
	myanpt = (ANISTR *) mapanimstrendpt; myanpt--;
	while (myanpt->antype!=AN_END)
	{
		myanpt->anstartoff += (long int) mapanimstrendpt;
		myanpt->anendoff += (long int) mapanimstrendpt;
		myanpt->ancuroff += (long int) mapanimstrendpt;
		myanpt--;
	}
	myanblkpt = (long int *) mapanimstrpt; while (myanblkpt != (long int *) myanpt)
	{
		*myanblkpt += (long int) mapblockstrpt;
		myanblkpt++;
	} }
	
	return 0;
}

int MapDecodeMPHD (void)
{
MPHD * hdrmempt;

	hdrmempt = (MPHD *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (hdrmempt==NULL) return 0;
	fread (hdrmempt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	mapwidth=hdrmempt->mapwidth;
	mapheight=hdrmempt->mapheight;
	mapblockwidth=hdrmempt->blockwidth;
	mapblockheight=hdrmempt->blockheight;
	mapdepth=hdrmempt->blockdepth;
	mapblockstrsize=hdrmempt->blockstrsize;
	mapnumblockstr=hdrmempt->numblockstr;
	mapnumblockgfx=hdrmempt->numblockgfx;
	free (hdrmempt);
	return 0;
}

int MapDecodeCMAP (void)
{
	mapcmappt = (unsigned char *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapcmappt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapcmappt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	MapCMAPtoPE (mapcmappt, mappept);
	return 0;
}

int MapDecodeBKDT (void)
{
	mapblockstrpt = (char *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapblockstrpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapblockstrpt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

int MapDecodeANDT (void)
{
	mapanimstrpt = (char *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapanimstrpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	mapanimstrendpt = (char *) (mapanimstrpt+(Mapbyteswapl(mapgenheader.headsize)));
	fread (mapanimstrpt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

int MapDecodeBGFX (void)
{
	mapblockgfxpt = (char *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapblockgfxpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapblockgfxpt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);

	return 0;
}

int MapDecodeBODY (void)
{
	mappt = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mappt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mappt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	mapmappt[0] = mappt;
	return 0;
}
int MapDecodeLYR1 (void)
{
	mapmappt[1] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[1]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[1], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR2 (void)
{
	mapmappt[2] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[2]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[2], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR3 (void)
{
	mapmappt[3] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[3]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[3], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR4 (void)
{
	mapmappt[4] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[4]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[4], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR5 (void)
{
	mapmappt[5] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[5]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[5], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR6 (void)
{
	mapmappt[6] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[6]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[6], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}
int MapDecodeLYR7 (void)
{
	mapmappt[7] = (short int *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mapmappt[7]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mapmappt[7], Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	return 0;
}

int MapDecodeNULL (void)
{
char * mynllpt;
	mynllpt = (char *) malloc (Mapbyteswapl(mapgenheader.headsize));
	if (mynllpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	fread (mynllpt, Mapbyteswapl(mapgenheader.headsize), 1, mapfilept);
	free (mynllpt);
	return 0;
}

int MapLoad (char * mapname, RBDIRECTDRAW lpDD)
{
int i;
long int mapfilesize;

	MapFreeMem ();
	maperror = 0;

	mapfilept = fopen (mapname, "rb");
	if (mapfilept==NULL) { maperror = MER_NOOPEN; return -1; }
	fseek (mapfilept, 0, SEEK_END);
	mapfilesize = ftell (mapfilept);
	fseek (mapfilept, 0, SEEK_SET);
	if (fread (&mapgenheader, 1, sizeof(GENHEAD), mapfilept)!=sizeof(GENHEAD))
	{ maperror = MER_MAPLOADERROR; fclose (mapfilept); return -1; }

	if (mapgenheader.id1!='F') maperror = MER_MAPLOADERROR;
	if (mapgenheader.id2!='O') maperror = MER_MAPLOADERROR;
	if (mapgenheader.id3!='R') maperror = MER_MAPLOADERROR;
	if (mapgenheader.id4!='M') maperror = MER_MAPLOADERROR;

	if (mapfilesize==-1) maperror = MER_MAPLOADERROR;
	if (mapfilesize!=(signed)((Mapbyteswapl(mapgenheader.headsize))+8)) maperror = MER_MAPLOADERROR;

	if (maperror) { fclose (mapfilept); return -1; }

	if (fread (&mapgenheader, 1, sizeof(GENHEAD)-4,mapfilept)!=sizeof(GENHEAD)-4)
	{ maperror = MER_MAPLOADERROR; fclose (mapfilept); return -1; }
	maperror=MER_MAPLOADERROR;
	if (mapgenheader.id1=='F') { if (mapgenheader.id2=='M') { if (mapgenheader.id3=='A')
	{ if (mapgenheader.id4=='P') maperror=MER_NONE; } } }
	if (maperror) { fclose (mapfilept); return -1; }

	while (mapfilesize!=(ftell(mapfilept)))
	{
		if (fread (&mapgenheader, 1, sizeof(GENHEAD),mapfilept)!=sizeof(GENHEAD))
		{ maperror = MER_MAPLOADERROR; fclose (mapfilept); return -1; }

		i = 0;
		if (mapgenheader.id1=='M') { if (mapgenheader.id2=='P') { if (mapgenheader.id3=='H')
		{ if (mapgenheader.id4=='D') { MapDecodeMPHD (); i = 1; } } } }
		if (mapgenheader.id1=='C') { if (mapgenheader.id2=='M') { if (mapgenheader.id3=='A')
		{ if (mapgenheader.id4=='P') { MapDecodeCMAP (); i = 1; } } } }
		if (mapgenheader.id1=='B') { if (mapgenheader.id2=='K') { if (mapgenheader.id3=='D')
		{ if (mapgenheader.id4=='T') { MapDecodeBKDT (); i = 1; } } } }
		if (mapgenheader.id1=='A') { if (mapgenheader.id2=='N') { if (mapgenheader.id3=='D')
		{ if (mapgenheader.id4=='T') { MapDecodeANDT (); i = 1; } } } }
		if (mapgenheader.id1=='B') { if (mapgenheader.id2=='G') { if (mapgenheader.id3=='F')
		{ if (mapgenheader.id4=='X') { MapDecodeBGFX (); i = 1; } } } }
		if (mapgenheader.id1=='B') { if (mapgenheader.id2=='O') { if (mapgenheader.id3=='D')
		{ if (mapgenheader.id4=='Y') { MapDecodeBODY (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='1') { MapDecodeLYR1 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='2') { MapDecodeLYR2 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='3') { MapDecodeLYR3 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='4') { MapDecodeLYR4 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='5') { MapDecodeLYR5 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='6') { MapDecodeLYR6 (); i = 1; } } } }
		if (mapgenheader.id1=='L') { if (mapgenheader.id2=='Y') { if (mapgenheader.id3=='R')
		{ if (mapgenheader.id4=='7') { MapDecodeLYR7 (); i = 1; } } } }
		if (!i) MapDecodeNULL ();
		if (maperror) { fclose (mapfilept); return -1; }
	}
	fclose (mapfilept);
	return (MapRelocate (lpDD));
}

int MapMemDecodeMPHD (MPHD * mapmempt)
{
	mapwidth=mapmempt->mapwidth;
	mapheight=mapmempt->mapheight;
	mapblockwidth=mapmempt->blockwidth;
	mapblockheight=mapmempt->blockheight;
	mapdepth=mapmempt->blockdepth;
	mapblockstrsize=mapmempt->blockstrsize;
	mapnumblockstr=mapmempt->numblockstr;
	mapnumblockgfx=mapmempt->numblockgfx;
	return 0;
}

int MapMemDecodeCMAP (unsigned char * cmppt)
{
int i;
unsigned char * tempcmappt;
	mapcmappt = (unsigned char *) malloc (Mapbyteswapl(*(long int *)cmppt));
	cmppt += 4;
	if (mapcmappt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	tempcmappt = mapcmappt;
	for (i=0;i<(256*3);i++) { *tempcmappt = *cmppt; tempcmappt++; cmppt++; }
	MapCMAPtoPE (mapcmappt, mappept);
	return 0;
}

int MapMemDecodeBKDT (unsigned char * mapmempt)
{
int i, j;
unsigned char * temppt;
	i = Mapbyteswapl(*(long int *)mapmempt);
	mapmempt += 4;
	mapblockstrpt = (char *) malloc (i);
	if (mapblockstrpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	temppt = (unsigned char *) mapblockstrpt;
	for (j=0;j<i;j++) { *temppt = *mapmempt; temppt++; mapmempt++; }

	return 0;
}

int MapMemDecodeANDT (unsigned char * mapmempt)
{
int i, j;
unsigned char * temppt;
	i = Mapbyteswapl(*(long int *)mapmempt);
	mapmempt += 4;
	mapanimstrpt = (char *) malloc (i);
	if (mapanimstrpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	mapanimstrendpt = (char *) (mapanimstrpt+i);
	temppt = (unsigned char *) mapanimstrpt;
	for (j=0;j<i;j++) { *temppt = *mapmempt; temppt++; mapmempt++; }

	return 0;
}

int MapMemDecodeBGFX (unsigned char * mapmempt)
{
int i, j;
unsigned char * temppt;
	i = Mapbyteswapl(*(long int *)mapmempt);
	mapmempt += 4;
	mapblockgfxpt = (char *) malloc (i);
	if (mapblockgfxpt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	temppt = (unsigned char *) mapblockgfxpt;
	for (j=0;j<i;j++) { *temppt = *mapmempt; temppt++; mapmempt++; }
	return 0;
}

int MapMemDecodeBODY (unsigned char * mapmempt)
{
int i, j;
unsigned char * temppt;
	i = Mapbyteswapl(*(long int *)mapmempt);
	mapmempt += 4;
	mappt = (short int *) malloc (i);
	if (mappt==NULL) { maperror = MER_OUTOFMEM; return -1; }
	mapmappt[0] = mappt;
	temppt = (unsigned char *) mappt;
	for (j=0;j<i;j++) { *temppt = *mapmempt; temppt++; mapmempt++; }
	return 0;
}
int MapMemDecodeLYR (unsigned char * mapmempt, int maplayernum)
{
int i, j;
unsigned char * temppt;
	i = Mapbyteswapl(*(long int *)mapmempt);
	mapmempt += 4;
	mapmappt[maplayernum] = (short int *) malloc (i);
	if (mapmappt[maplayernum]==NULL) { maperror = MER_OUTOFMEM; return -1; }
	temppt = (unsigned char *) mappt;
	for (j=0;j<i;j++) { *temppt = *mapmempt; temppt++; mapmempt++; }
	return 0;
}

int MapDecode (unsigned char * mapmempt, RBDIRECTDRAW lpDD)
{
int i;
long int maplength;

	MapFreeMem ();
	maperror = 0;

	if (*mapmempt!='F') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+1)!='O') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+2)!='R') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+3)!='M') maperror = MER_MAPLOADERROR;
	mapmempt += 4;
	maplength = (Mapbyteswapl(*(long int *)mapmempt)+8);

/*	Just keeping this bit here in case I need it.
	if (maplength!=((Mapbyteswapl(*(long int *)mapmempt))+8)) maperror = MER_MAPLOADERROR;
*/
	if (maperror) return -1;
	mapmempt += 4;

	if (*mapmempt!='F') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+1)!='M') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+2)!='A') maperror = MER_MAPLOADERROR;
	if (*(mapmempt+3)!='P') maperror = MER_MAPLOADERROR;
	mapmempt+=4; i = 12;

	if (maperror) return -1;

	while (i!=maplength)
	{
		if ((*mapmempt=='M') && (*(mapmempt+1)=='P') && (*(mapmempt+2)=='H') &&
		(*(mapmempt+3)=='D')) MapMemDecodeMPHD ((MPHD *) (mapmempt+8));
		if ((*mapmempt=='C') && (*(mapmempt+1)=='M') && (*(mapmempt+2)=='A') &&
		(*(mapmempt+3)=='P')) MapMemDecodeCMAP (mapmempt+4);
		if ((*mapmempt=='B') && (*(mapmempt+1)=='K') && (*(mapmempt+2)=='D') &&
		(*(mapmempt+3)=='T')) MapMemDecodeBKDT (mapmempt+4);
		if ((*mapmempt=='A') && (*(mapmempt+1)=='N') && (*(mapmempt+2)=='D') &&
		(*(mapmempt+3)=='T')) MapMemDecodeANDT (mapmempt+4);
		if ((*mapmempt=='B') && (*(mapmempt+1)=='G') && (*(mapmempt+2)=='F') &&
		(*(mapmempt+3)=='X')) MapMemDecodeBGFX (mapmempt+4);
		if ((*mapmempt=='B') && (*(mapmempt+1)=='O') && (*(mapmempt+2)=='D') &&
		(*(mapmempt+3)=='Y')) MapMemDecodeBODY (mapmempt+4);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='1')) MapMemDecodeLYR (mapmempt+4, 1);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='2')) MapMemDecodeLYR (mapmempt+4, 2);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='3')) MapMemDecodeLYR (mapmempt+4, 3);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='4')) MapMemDecodeLYR (mapmempt+4, 4);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='5')) MapMemDecodeLYR (mapmempt+4, 5);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='6')) MapMemDecodeLYR (mapmempt+4, 6);
		if ((*mapmempt=='L') && (*(mapmempt+1)=='Y') && (*(mapmempt+2)=='R') &&
		(*(mapmempt+3)=='7')) MapMemDecodeLYR (mapmempt+4, 7);
		if (maperror) return -1;
		mapmempt += 4; i += Mapbyteswapl(*(long int *)mapmempt)+8;
		mapmempt += Mapbyteswapl(*(long int *)mapmempt); mapmempt += 4;
	}
	return (MapRelocate (lpDD));
}

void MapInitAnims (void)
{
ANISTR * myanpt;
	if (mapanimstrpt==NULL) return;
	myanpt = (ANISTR *) mapanimstrendpt; myanpt--;
	while (myanpt->antype!=-1)
	{
		if (myanpt->antype==AN_PPFR) myanpt->antype = AN_PPFF;
		if (myanpt->antype==AN_PPRF) myanpt->antype = AN_PPRR;
		if (myanpt->antype==AN_ONCES) myanpt->antype = AN_ONCE;
		if ((myanpt->antype==AN_LOOPR) || (myanpt->antype==AN_PPRR))
		{
		myanpt->ancuroff = myanpt->anstartoff;
		if ((myanpt->anstartoff)!=(myanpt->anendoff)) myanpt->ancuroff=(myanpt->anendoff)-4;
		} else {
		myanpt->ancuroff = myanpt->anstartoff;
		}
		myanpt->ancount = myanpt->andelay;
		myanpt--;
	}
}

void MapUpdateAnims (void)
{
ANISTR * myanpt;

	if (mapanimstrpt==NULL) return;
	myanpt = (ANISTR *) mapanimstrendpt; myanpt--;
	while (myanpt->antype!=-1)
	{
		if (myanpt->antype!=AN_NONE) { myanpt->ancount--; if (myanpt->ancount<0) {
		myanpt->ancount = myanpt->andelay;
		if (myanpt->antype==AN_LOOPF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) myanpt->ancuroff = myanpt->anstartoff;
		} }
		if (myanpt->antype==AN_LOOPR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4))
				myanpt->ancuroff = (myanpt->anendoff)-4;
		} }
		if (myanpt->antype==AN_ONCE)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->antype = AN_ONCES;
				myanpt->ancuroff = myanpt->anstartoff; }
		} }
		if (myanpt->antype==AN_ONCEH)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) {
			if (myanpt->ancuroff!=((myanpt->anendoff)-4)) myanpt->ancuroff+=4;
		} }
		if (myanpt->antype==AN_PPFF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->ancuroff -= 8;
			myanpt->antype = AN_PPFR;
			if (myanpt->ancuroff<myanpt->anstartoff) myanpt->ancuroff +=4; }
		} } else {
		if (myanpt->antype==AN_PPFR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4)) { myanpt->ancuroff += 8;
			myanpt->antype = AN_PPFF;
			if (myanpt->ancuroff>myanpt->anendoff) myanpt->ancuroff -=4; }
		} } }
		if (myanpt->antype==AN_PPRR)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff-=4;
			if (myanpt->ancuroff==((myanpt->anstartoff)-4)) { myanpt->ancuroff += 8;
			myanpt->antype = AN_PPRF;
			if (myanpt->ancuroff>myanpt->anendoff) myanpt->ancuroff -=4; }
		} } else {
		if (myanpt->antype==AN_PPRF)
		{
			if (myanpt->anstartoff!=myanpt->anendoff) { myanpt->ancuroff+=4;
			if (myanpt->ancuroff==myanpt->anendoff) { myanpt->ancuroff -= 8;
			myanpt->antype = AN_PPRR;
			if (myanpt->ancuroff<myanpt->anstartoff) myanpt->ancuroff +=4; }
		} } }
	} } myanpt--; }
}

int MapDrawBG (RBDIRECTDRAWSURFACE lpDDSDest, int mapxo, int mapyo)
/* lpDDSDest = lpdirectdrawsurface.
 * mapxo = offset, in pixels, from the left edge of the map.
 * mapyo = offset, in pixels, from the top edge of the map.
 */
{
RECT cliprect;
#ifdef RBUSEBLTONLY
RECT dstrect;
#endif
int i, j, mapvclip, maphclip;
short int *mymappt;
short int *mymap2pt;
BLKSTR *blkdatapt;
ANISTR *myanpt;
/* first, draw blocks that are clipped horizontally (left) */
	maphclip = mapxo%MBSW;	/* Number of pixels to clip from left */
	mapvclip = mapyo%MBSH;	/* Number of pixels to clip from top */

/* Draw top left clipped block */
	cliprect.left = maphclip;
	cliprect.top = mapvclip;
	cliprect.right = MBSW;
	cliprect.bottom = MBSH;

#ifdef RBUSEBLTONLY
	dstrect.left = MMOX;
	dstrect.top = MMOY;
	dstrect.right = MMOX+(MBSW-maphclip);
	dstrect.bottom = MMOY+(MBSH-mapvclip);
#endif

	mymappt = (short int *) mappt;
	mymappt += (mapxo/MBSW)+((mapyo/MBSH)*mapwidth);
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
	lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
	lpDDSDest->BltFast( MMOX, MMOY, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
/* Draw left clipped blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip; while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
		dstrect.top = i;
		dstrect.bottom = i+MBSH;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		i += MBSH;
	}
/* Draw bottom left clipped block */
	mymappt += mapwidth;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	cliprect.bottom = MBSH-(i-(MSCRH+MMOY-MBSH));
#ifdef RBUSEBLTONLY
	dstrect.top = i;
	dstrect.bottom = MMOY+MSCRH;
	lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
	lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
/* Draw bottom clipped blocks */
	cliprect.left = 0;
	j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
	{
		mymappt ++;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
		dstrect.left= j;
		dstrect.right = j+MBSW;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		j += MBSW;
	}
/* Draw bottom right clipped block */
	cliprect.right = MBSW-(j-(MSCRW+MMOX-MBSW));
	mymappt ++;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
	dstrect.left= j;
	dstrect.right = MMOX+MSCRW;
	lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
	lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
/* Draw right clipped blocks */
	cliprect.bottom = MBSH;
	while (i>(MMOY+MBSH))
	{
		mymappt -= mapwidth; i -= MBSH;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
		dstrect.top = i;
		dstrect.bottom = i+MBSH;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw top right clipped block */
#ifdef RBUSEBLTONLY
	dstrect.top = MMOY;
	dstrect.bottom = i;
#endif
	mymappt -= mapwidth; i = MMOY;
	cliprect.top = mapvclip;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
	lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
	lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
/* Draw top clipped blocks */
	cliprect.right = MBSW;
	while (j>(MMOX+MBSW))
	{
		mymappt --; j -= MBSW;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
		dstrect.left = j;
		dstrect.right = j+MBSW;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw unclipped (full) blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip;
	while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		mymap2pt = mymappt;
		j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
		{
			if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
			else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
				blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
#ifdef RBUSEBLTONLY
			dstrect.left = j;
			dstrect.right = j+MBSW;
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			if (lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, NULL, DDBLT_WAIT, NULL)!=DD_OK)
#else
			if (lpDDSDest->BltFast(j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT)!=DD_OK)
#endif
				return -1;
			mymappt++; j += MBSW;
		}
		mymappt = mymap2pt;
		i += MBSH;
	}
	return 0;
}

int MapDrawBGT (RBDIRECTDRAWSURFACE lpDDSDest, int mapxo, int mapyo)
/* lpDDSDest = lpdirectdrawsurface.
 * mapxo = offset, in pixels, from the left edge of the map.
 * mapyo = offset, in pixels, from the top edge of the map.
 */
{
RECT cliprect;
#ifdef RBUSEBLTONLY
RECT dstrect;
#endif
int i, j, mapvclip, maphclip;
short int *mymappt;
short int *mymap2pt;
BLKSTR *blkdatapt;
ANISTR *myanpt;
/* first, draw blocks that are clipped horizontally (left) */
	maphclip = mapxo%MBSW;	/* Number of pixels to clip from left */
	mapvclip = mapyo%MBSH;	/* Number of pixels to clip from top */

/* Draw top left clipped block */
	cliprect.left = maphclip;
	cliprect.top = mapvclip;
	cliprect.right = MBSW;
	cliprect.bottom = MBSH;

#ifdef RBUSEBLTONLY
	dstrect.left = MMOX;
	dstrect.top = MMOY;
	dstrect.right = MMOX+(MBSW-maphclip);
	dstrect.bottom = MMOY+(MBSH-mapvclip);
#endif

	mymappt = (short int *) mappt;
	mymappt += (mapxo/MBSW)+((mapyo/MBSH)*mapwidth);
	if (*mymappt) {
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
#else
		lpDDSDest->BltFast( MMOX, MMOY, blkdatapt->bgoff, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	else
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( MMOX, MMOY, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw left clipped blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip; while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		if (*mymappt) {
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		else
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		}
		i += MBSH;
	}
/* Draw bottom left clipped block */
	mymappt += mapwidth;
	cliprect.bottom = MBSH-(i-(MSCRH+MMOY-MBSH));
	if (*mymappt) {
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
	{
		dstrect.top = i;
		dstrect.bottom = MMOY+MSCRH;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}
#else
		lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	else
#ifdef RBUSEBLTONLY
	{
		dstrect.top = i;
		dstrect.bottom = MMOY+MSCRH;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
	}
#else
		lpDDSDest->BltFast( MMOX, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw bottom clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.top = i;
	dstrect.bottom = MMOY+MSCRH;
#endif
	cliprect.left = 0;
	j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
	{
		mymappt ++;
		if (*mymappt) {
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.left= j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		else
#ifdef RBUSEBLTONLY
		{
			dstrect.left= j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		}
		j += MBSW;
	}
/* Draw bottom right clipped block */
	cliprect.right = MBSW-(j-(MSCRW+MMOX-MBSW));
	mymappt ++;
	if (*mymappt) {
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
	{
		dstrect.left= j;
		dstrect.right = MMOX+MSCRW;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	else
#ifdef RBUSEBLTONLY
	{
		dstrect.left= j;
		dstrect.right = MMOX+MSCRW;
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
	}
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw right clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.left= j;
	dstrect.right = MMOX+MSCRW;
#endif
	cliprect.bottom = MBSH;
	while (i>(MMOY+MBSH))
	{
		mymappt -= mapwidth; i -= MBSH;
		if (*mymappt) {
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		else
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		}
	}
/* Draw top right clipped block */
#ifdef RBUSEBLTONLY
	dstrect.top = MMOY;
	dstrect.bottom = i;
#endif
	mymappt -= mapwidth; i = MMOY;
	cliprect.top = mapvclip;
	if (*mymappt) {
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	else
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
#else
		lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
	}
/* Draw top clipped blocks */
	cliprect.right = MBSW;
	while (j>(MMOX+MBSW))
	{
		mymappt --; j -= MBSW;
		if (*mymappt) {
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
		if (blkdatapt->trigger)
#ifdef RBUSEBLTONLY
		{
			dstrect.left = j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		else
#ifdef RBUSEBLTONLY
		{
			dstrect.left = j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, &cliprect, DDBLT_WAIT, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect, DDBLTFAST_WAIT );
#endif
		}
	}
/* Draw unclipped (full) blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip;
	while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		mymap2pt = mymappt;
		j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
		{
			if (*mymappt) {
			if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
			else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
				blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
			if (blkdatapt->trigger) {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, NULL,
					DDBLT_WAIT | DDBLT_KEYSRC, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast( j, i, blkdatapt->bgoff, &cliprect,
					DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY )!=DD_OK)
#endif
					return -1;
			} else {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, blkdatapt->bgoff, NULL,
					DDBLT_WAIT, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast(j, i, blkdatapt->bgoff, &cliprect,
					DDBLTFAST_WAIT)!=DD_OK)
#endif
					return -1;
			} }
			mymappt++; j += MBSW;
		}
		mymappt = mymap2pt;
		i += MBSH;
	}
	return 0;
}

int MapDrawFG (RBDIRECTDRAWSURFACE lpDDSDest, int mapxo, int mapyo, int mapfg)
/* lpDDSDest = lpdirectdrawsurface.
 * mapxo = offset, in pixels, from the left edge of the map.
 * mapyo = offset, in pixels, from the top edge of the map.
 * mapfg = foreground layer (0,1 or 2).
 */
{
RECT cliprect;
#ifdef RBUSEBLTONLY
RECT dstrect;
#endif
int i, j, mapvclip, maphclip;
short int *mymappt;
short int *mymap2pt;
BLKSTR *blkdatapt;
ANISTR *myanpt;
RBDIRECTDRAWSURFACE mapgfxpt;

/* first, draw blocks that are clipped horizontally (left) */
	maphclip = mapxo%MBSW;	/* Number of pixels to clip from left */
	mapvclip = mapyo%MBSH;	/* Number of pixels to clip from top */

/* Draw top left clipped block */
	cliprect.left = maphclip;
	cliprect.top = mapvclip;
	cliprect.right = MBSW;
	cliprect.bottom = MBSH;

#ifdef RBUSEBLTONLY
	dstrect.left = MMOX;
	dstrect.top = MMOY;
	dstrect.right = MMOX+(MBSW-maphclip);
	dstrect.bottom = MMOY+(MBSH-mapvclip);
#endif

	mymappt = (short int *) mappt;
	mymappt += (mapxo/MBSW)+((mapyo/MBSH)*mapwidth);
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
#else
		lpDDSDest->BltFast( MMOX, MMOY, mapgfxpt, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
/* Draw left clipped blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip; while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
	{
		dstrect.top = i;
		dstrect.bottom = i+MBSH;
		lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}
#else
		lpDDSDest->BltFast( MMOX, i, mapgfxpt, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		i += MBSH;
	}
/* Draw bottom left clipped block */
	mymappt += mapwidth;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	cliprect.bottom = MBSH-(i-(MSCRH+MMOY-MBSH));
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
	{
		dstrect.top = i;
		dstrect.bottom = MMOY+MSCRH;
		lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}
#else
		lpDDSDest->BltFast( MMOX, i, mapgfxpt, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
/* Draw bottom clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.top = i;
	dstrect.bottom = MMOY+MSCRH;
#endif
	cliprect.left = 0;
	j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
	{
		mymappt ++;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
		{
			dstrect.left= j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, mapgfxpt, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
		j += MBSW;
	}
/* Draw bottom right clipped block */
	cliprect.right = MBSW-(j-(MSCRW+MMOX-MBSW));
	mymappt ++;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
	{
		dstrect.left= j;
		dstrect.right = MMOX+MSCRW;
		lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	}
#else
		lpDDSDest->BltFast( j, i, mapgfxpt, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
/* Draw right clipped blocks */
#ifdef RBUSEBLTONLY
	dstrect.left= j;
	dstrect.right = MMOX+MSCRW;
#endif
	cliprect.bottom = MBSH;
	while (i>(MMOY+MBSH))
	{
		mymappt -= mapwidth; i -= MBSH;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
		{
			dstrect.top = i;
			dstrect.bottom = i+MBSH;
			lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, mapgfxpt, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	}
/* Draw top right clipped block */
#ifdef RBUSEBLTONLY
	dstrect.top = MMOY;
	dstrect.bottom = i;
#endif
	mymappt -= mapwidth; i = MMOY;
	cliprect.top = mapvclip;
	if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
	else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
		blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
		lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
			DDBLT_WAIT | DDBLT_KEYSRC, NULL);
#else
		lpDDSDest->BltFast( j, i, mapgfxpt, &cliprect,
			DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
/* Draw top clipped blocks */
	cliprect.right = MBSW;
	while (j>(MMOX+MBSW))
	{
		mymappt --; j -= MBSW;
		if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
		else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
			blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
	if (!mapfg) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff;
	else if (mapfg == 1) mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff2;
	else mapgfxpt = (RBDIRECTDRAWSURFACE) blkdatapt->fgoff3;
	if (mapgfxpt != NULL)
#ifdef RBUSEBLTONLY
		{
			dstrect.left = j;
			dstrect.right = j+MBSW;
			lpDDSDest->Blt (&dstrect, mapgfxpt, &cliprect,
				DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		}
#else
			lpDDSDest->BltFast( j, i, mapgfxpt, &cliprect,
				DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
#endif
	}
/* Draw unclipped (full) blocks */
	cliprect.top = 0;
	i = MMOY+MBSH-mapvclip;
	while (i<(MSCRH+MMOY-MBSH))
	{
		mymappt += mapwidth;
		mymap2pt = mymappt;
		j = MMOX+MBSW-maphclip; while (j<(MSCRW+MMOX-MBSW))
		{
			if (*mymappt) {
			if (*mymappt>=0) blkdatapt = (BLKSTR*) (((char *)mapblockstrpt) + *mymappt);
			else { myanpt = (ANISTR *) (mapanimstrendpt + *mymappt);
				blkdatapt = (BLKSTR *) *((long int *)(myanpt->ancuroff)); }
			switch (mapfg) {
				case 0:
					if (blkdatapt->fgoff != NULL) {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, blkdatapt->fgoff, NULL,
					DDBLT_WAIT | DDBLT_KEYSRC, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast( j, i, blkdatapt->fgoff, &cliprect,
					DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY )!=DD_OK)
#endif
					return -1; }
					break;
				case 1:
					if (blkdatapt->fgoff2 != NULL) {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, blkdatapt->fgoff2, NULL,
					DDBLT_WAIT | DDBLT_KEYSRC, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast( j, i, blkdatapt->fgoff2, &cliprect,
					DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY )!=DD_OK)
#endif
					return -1; }
					break;
				case 2:
					if (blkdatapt->fgoff3 != NULL) {
#ifdef RBUSEBLTONLY
				dstrect.left = j;
				dstrect.right = j+MBSW;
				dstrect.top = i;
				dstrect.bottom = i+MBSH;
				if (lpDDSDest->Blt (&dstrect, blkdatapt->fgoff3, NULL,
					DDBLT_WAIT | DDBLT_KEYSRC, NULL)!=DD_OK)
#else
				if (lpDDSDest->BltFast( j, i, blkdatapt->fgoff3, &cliprect,
					DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY )!=DD_OK)
#endif
					return -1; }
					break;
			} }
			mymappt++; j += MBSW;
		}
		mymappt = mymap2pt;
		i += MBSH;
	}
	return 0;
}


