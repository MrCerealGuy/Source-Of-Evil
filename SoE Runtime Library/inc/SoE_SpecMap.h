/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

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
LPDIRECTDRAWSURFACE4 bgoff, fgoff;			/* offsets from start of graphic blocks */
LPDIRECTDRAWSURFACE4 fgoff2, fgoff3; 		/* more overlay blocks */
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

extern int maperror;		/* Set to a MER_ error if something wrong happens */
extern short int mapwidth, mapheight, mapblockwidth, mapblockheight, mapdepth;
extern short int mapblockstrsize, mapnumblockstr, mapnumblockgfx;
extern short int * mappt;
extern short int ** maparraypt;
extern unsigned char * mapcmappt;
extern char * mapblockgfxpt;
extern char * mapblockstrpt;
extern char * mapanimstrpt;
extern char * mapanimstrendpt;
extern GENHEAD mapgenheader;
extern PALETTEENTRY mappept[256];
extern short int * mapmappt[8];
extern short int ** mapmaparraypt[8];
extern LPDIRECTDRAWSURFACE4 maplpDDSTiles[1024];	// Tiles Direct Draw Surfaces
extern LPDIRECTDRAWSURFACE4 lpDDSParallax;

void MapFreeMem (void);
int MapLoad (char *, LPDIRECTDRAW4);
int MapDecode (unsigned char *, LPDIRECTDRAW4);
void MapInitAnims (void);
void MapUpdateAnims (void);
int MapDrawBG (LPDIRECTDRAWSURFACE4, int , int);
int MapDrawBGT (LPDIRECTDRAWSURFACE4, int, int);
int MapDrawFG (LPDIRECTDRAWSURFACE4, int, int, int);
int MapRestore (void);
int MapGenerateYLookup (void);
int MapChangeLayer (int);
BLKSTR * MapGetBlock (int, int);
void MapSetBlock (int, int, int);
void MapReleaseParallaxSurface (void);
void MapRestoreParallaxSurface (void *, int);
int MapCreateParallaxSurface (LPDIRECTDRAW4);
int MapDrawParallax (LPDIRECTDRAWSURFACE4, int, int);

