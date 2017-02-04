/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGEJPG_H
#define SOE_IMAGEJPG_H

#include "SoE_ImageBase.h"
#include <mmreg.h>


//	for DWORD aligning a buffer
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

class SoEImageJPG : public SoEImageBase  
{
public:
	SoEImageJPG();
	virtual ~SoEImageJPG();

	SOE_LPDIRECTDRAWSURFACE GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType);
//#if DIRECTDRAW_VERSION >= SOE_DDVER
//    LPDIRECTDRAWSURFACE4 GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType);
//#else
//    LPDIRECTDRAWSURFACE GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType);
//#endif

	BOOL		Validate(const char *head, const int headsize, const char *foot, const int footsize);

private:
	// read a JPEG file to an RGB buffer - 3 bytes per pixel
	// returns a ptr to a buffer .
	// caller is responsible for cleanup!!!
	// BYTE *buf = JpegFile::JpegFileToRGB(....);
	// delete [] buf;
	BYTE * JpegFileToRGB( char * fileName, UINT *width, UINT *height );

	////////////////////////////////////////////////////////////////
	// write a JPEG file from a 3-component, 1-byte per component buffer
	BOOL RGBToJpegFile( char * fileName, BYTE *dataBuf, UINT width, UINT height,
						BOOL IsRGBcolor,// TRUE = RGB, FALSE = Grayscale
						int quality);	// 0 - 100



	// fetch width / height of an image
	BOOL GetJPGDimensions( char * fileName, UINT *width, UINT *height );


	//	utility functions
	//	to do things like DWORD-align, flip, convert to grayscale, etc.
	//
	// allocates a DWORD-aligned buffer, copies data buffer
	// caller is responsible for delete []'ing the buffer
	BYTE * MakeDwordAlignedBuf( BYTE *dataBuf, UINT widthPix, UINT height, UINT *uiOutWidthBytes);


	// if you have a DWORD aligned buffer, this will copy the
	// RGBs out of it into a new buffer. new width is widthPix * 3 bytes
	// caller is responsible for delete []'ing the buffer
	BYTE *RGBFromDWORDAligned( BYTE *inBuf, UINT widthPix, UINT widthBytes, UINT height );


	// vertically flip a buffer - for BMPs in-place
	// note, this routine works on a buffer of width widthBytes: not a buffer of widthPixels.
	BOOL VertFlipBuf( BYTE * inbuf, UINT widthBytes, UINT height );


	// NOTE :
	// the following routines do their magic on buffers with a whole number
	// of pixels per data row! these are assumed to be non DWORD-aligned buffers.

	// convert RGB to grayscale	using luminance calculation in-place
	BOOL MakeGrayScale( BYTE *buf, UINT widthPix, UINT height );
	
	// swap Red and Blue bytes in-place
	BOOL BGRFromRGB( BYTE *buf,	UINT widthPix, UINT height );

};


#endif SOE_IMAGEJPG_H
