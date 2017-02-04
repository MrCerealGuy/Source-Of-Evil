/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGEPSD_H
#define SOE_IMAGEPSD_H

#include "SoE_Imagebase.h"

#pragma pack(1)
struct _psdHeader 
{
	char	magic[4];	//magic identifier (8BPS)
	WORD	version;	//must be 1
	WORD	pad[3];		//unneeded data (always 0)
	WORD	channels;	//number of channels
	DWORD	height;
	DWORD	width;
	WORD	bpc;		//bits per channel. must be 8
	WORD	colormode;	//colormode.  must be 3
	DWORD   ModeDataCount;
};
#pragma pack()

#define MAX_PSD_CHANNELS	24

class SoEImagePSD : public  SoEImageBase
{
public:
	SoEImagePSD(void);
	virtual ~SoEImagePSD(void);

	SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType);
	BOOL Validate(const CHAR *head, const int headsize, const CHAR *foot, const int footsize);

private:
	void		UnPackPSD( CHAR* lpCache, BYTE* pixels, WORD channelCnt, WORD compression );
	_psdHeader	m_psdInfo;
};

#endif SOE_IMAGEPSD_H
