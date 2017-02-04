/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_IMAGEBMP_H
#define SOE_IMAGEBMP_H

#include "SoE_ImageBase.h"

class SoEImageBMP : public SoEImageBase  
{
public:
	SoEImageBMP(void);
	virtual ~SoEImageBMP(void);

	SOE_LPDIRECTDRAWSURFACE GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType);
	BOOL Validate(const CHAR *head, const int headsize, const CHAR *foot, const int footsize);

private:
	BOOL ReadRLE8Format(CHAR* lpCache, DWORD offset);

	LPBITMAPINFO		m_pbi;
	BITMAPINFO			m_bi;
	BITMAPFILEHEADER	m_bif;
	int					m_iColoursUsed;
};

#endif SOE_IMAGEBMP_H
