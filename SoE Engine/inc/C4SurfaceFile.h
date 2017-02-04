/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Another C4Group bitmap-to-surface loader and saver */

class C4Group;

SURFACE GroupReadSurface(C4Group &hGroup, BYTE *bpPalette=NULL);

BOOL SaveSurface(const char *szFilename, SURFACE sfcSurface, BYTE *bpPalette);
