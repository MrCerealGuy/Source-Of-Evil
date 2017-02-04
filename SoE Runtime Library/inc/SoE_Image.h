/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGE_H
#define SOE_IMAGE_H

#include <fstream.h>
#include "SoE_Imagebase.h"

const int headsize = 256;
const int footsize = 64;

class SoEImage  
{
public:
	SoEImage(void);
	virtual ~SoEImage(void);

	SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, const char* filename, BYTE memType = SOEMEM_VIDTHENSYS, DWORD type = SOEIMAGE_UNKNOWN );
    SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType = SOEMEM_VIDTHENSYS, DWORD type = SOEIMAGE_UNKNOWN );
    SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, fstream* fsptr, BYTE memType = SOEMEM_VIDTHENSYS, DWORD type = SOEIMAGE_UNKNOWN);
    SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, FILE* fh, BYTE memType = SOEMEM_VIDTHENSYS, DWORD type = SOEIMAGE_UNKNOWN);
	SOE_LPDIRECTDRAWSURFACE CreateDDSurface(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType, DWORD type );

private:
	DWORD GetFileFormat(LONG lSize, CHAR* lpCache);

};

#endif SOE_IMAGE_H
