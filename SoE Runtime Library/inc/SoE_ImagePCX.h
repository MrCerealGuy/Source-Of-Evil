/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGEPCX_H
#define SOE_IMAGEPCX_H

#include "SoE_ImageBase.h"

// Structures relating to the PCX format
typedef struct _PCX_RGB
{
	unsigned char	Red;
	unsigned char	Green;
	unsigned char	Blue;
}  PCX_RGB, *LPPCX_RGB;


typedef struct _PCX_HEADER
{
	char	Manufacturer;
	char	Version;
	char	Encoding;
	char	BitsPerPixel;
	short	Xmin, Ymin, Xmax, Ymax;
	short	HDpi, VDpi;
	char	Colormap[48];
	char	Reserved;
	char	NPlanes;
	short	BytesPerLine;
	short	PaletteInfo;
	short	HscreenSize;
	short	VscreenSize;
	char	Filler[54];
} PCX_HEADER, *LPPCX_HEADER;

class SoEImagePCX : public SoEImageBase  
{
public:
	SoEImagePCX(void);
	virtual ~SoEImagePCX(void);

    SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType);
	BOOL Validate(const CHAR *head, const int headsize, const CHAR *foot, const int footsize);

private:
    int DecodePcxLine(char *dest, char* &src, int bytes);

	PCX_HEADER m_hdr;
	int        m_iColoursUsed;
};

#endif SOE_IMAGEPCX_H
