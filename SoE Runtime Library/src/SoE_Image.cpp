/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_Image.h"
#include "SoE_ImageTGA.h"
#include "SoE_ImagePSD.h"
#include "SoE_ImageBMP.h"
#include "SoE_ImageJPG.h"
#include "SoE_ImagePCX.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SoEImage::SoEImage(void){}
SoEImage::~SoEImage(void){}

SOE_LPDIRECTDRAWSURFACE SoEImage::GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType, DWORD type)
{
	SOE_LPDIRECTDRAWSURFACE lpdds = NULL;
	
	// Call GetImage for the appropriate image type
    lpdds = CreateDDSurface(lpDD, lSize, lpCache, memType, type);
	
	return lpdds;
}

SOE_LPDIRECTDRAWSURFACE SoEImage::GetImage(SOE_LPDIRECTDRAW lpDD, const char* filename, BYTE memType, DWORD type)
{
	SOE_LPDIRECTDRAWSURFACE lpdds = NULL;
    LONG lSize;
    fstream fs;
    CHAR* lpCache;
	
	fs.open(filename, ios::in | ios::binary | ios::nocreate);
    if (!fs.is_open())
		return NULL;
	
    // Seek to the end of the file.
	fs.seekp(0, ios::end);
	if (fs.fail())
    {
        fs.close();
		return NULL;
    }
	
	// Get the file size.
	lSize = LONG(fs.tellp());
	if (fs.fail())
    {
        fs.close();
        return NULL;
    }
	
	// Seek back to the beggining of the file.
	fs.seekp(0, ios::beg);
    if (fs.fail())
    {
        fs.close();
        return NULL;
    }
	
    // Allocate memory to hold the data
    lpCache = new CHAR[lSize];
    if (lpCache == NULL)
    {
        fs.close();
        return NULL;
    }
	
    // Read in the data
    fs.read(lpCache, lSize);
    if (fs.fail())
    {
        delete[] lpCache;
        fs.close();
        return NULL;
    }
	
    // Close the file stream
    fs.close();
	
    // Call GetImage for the appropriate image type
    lpdds = CreateDDSurface(lpDD, lSize, lpCache, memType, type);
	
    // Delete the cache
    delete[] lpCache;
	
	return lpdds;
}

SOE_LPDIRECTDRAWSURFACE SoEImage::GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, fstream* fsptr, BYTE memType, DWORD type)
{
	SOE_LPDIRECTDRAWSURFACE lpdds = NULL;
    CHAR* lpCache;
    int length, save;
	
    length = lSize;
	
    // If lSize equals zero get the size of the file.
    if (length == 0)
    {
        // Save the pointer location
        save = fsptr->tellg();
        if (fsptr->fail())
            return NULL;
		
        // Seek to end of file
        fsptr->seekg(0, ios::end);
        if (fsptr->fail())
            return NULL;
		
        // Get the size of the file
        length = fsptr->tellg();
        if (fsptr->fail())
            return NULL;
		
        // Seek back to save position
        fsptr->seekg(save, ios::beg);
        if (fsptr->fail())
            return NULL;
    }
	
    // Cache the whole file in memory
    // Allocate memory to hold the data
    lpCache = new CHAR[length];
    if (lpCache == NULL)
        return NULL;
	
    // Read in the data
    fsptr->read(lpCache, length);
    if (fsptr->fail())
    {
        delete[] lpCache;
        return NULL;
    }
	
    // Call GetImage for the appropriate image type
    lpdds = CreateDDSurface(lpDD, length, lpCache, memType, type);
	
    // Delete the cache
    delete[] lpCache;
	
	return lpdds;
}

SOE_LPDIRECTDRAWSURFACE SoEImage::GetImage(SOE_LPDIRECTDRAW lpDD, LONG lSize, FILE* fh, BYTE memType, DWORD type)
{
	SOE_LPDIRECTDRAWSURFACE lpdds = NULL;
    CHAR* lpCache;
    int length, save;
	
    length = lSize;
	
    // If lSize equals zero get the size of the file.
    if (length == 0)
    {
        // Save the pointer location
        save = ftell(fh);
        if (ferror(fh))
            return NULL;
		
        // Seek to end of file
        fseek(fh, 0, SEEK_END);
        if (ferror(fh))
            return NULL;
		
        // Get the size of the file
        length = ftell(fh);
        if (ferror(fh))
            return NULL;
		
        // Seek back to save position
        fseek(fh, save, SEEK_SET);
        if (ferror(fh))
            return NULL;
    }
	
    // Cache the whole file in memory
    // Allocate memory to hold the data
    lpCache = new CHAR[length];
    if (lpCache == NULL)
        return NULL;
	
    // Read in the data
    fread(lpCache, 1, length, fh);
    if (ferror(fh))
    {
        delete[] lpCache;
        return NULL;
    }
	
    // Call GetImage for the appropriate image type
    lpdds = CreateDDSurface(lpDD, length, lpCache, memType, type);
	
    // Delete the cache
    delete[] lpCache;
	
	return lpdds;
}

DWORD SoEImage::GetFileFormat(LONG lSize, CHAR* lpCache)
{
	DWORD type = SOEIMAGE_UNKNOWN;
    CHAR* pHead = NULL;
    CHAR* pFoot = NULL;
	
    pHead = lpCache;
    pFoot = &lpCache[lSize - footsize];
	
	SoEImageTGA tga;
	if (tga.Validate(pHead, headsize, pFoot, footsize))
	{
		type = SOEIMAGE_TGA;
	}
	
	if (type == SOEIMAGE_UNKNOWN)
	{
		SoEImageBMP bmp;
		if (bmp.Validate(pHead, headsize, pFoot, footsize))
		{
			type = SOEIMAGE_BMP;
		}
	}
	
	if (type == SOEIMAGE_UNKNOWN)
	{
		SoEImagePSD psd;
		if (psd.Validate(pHead, headsize, pFoot, footsize))
		{
			type = SOEIMAGE_PSD;
		}
	}
	
	if (type == SOEIMAGE_UNKNOWN)
	{
		SoEImagePCX pcx;
		if (pcx.Validate(pHead, headsize, pFoot, footsize))
		{
			type = SOEIMAGE_PCX;
		}
	}
	
	return type;
}

SOE_LPDIRECTDRAWSURFACE SoEImage::CreateDDSurface(SOE_LPDIRECTDRAW lpDD, LONG lSize, CHAR* lpCache, BYTE memType, DWORD type)
{
	SOE_LPDIRECTDRAWSURFACE lpdds = NULL;
	
    // Determine the type if UNKNOWN
    if (type == SOEIMAGE_UNKNOWN)
		type = GetFileFormat(lSize, lpCache);
	
	switch (type)
	{
		case SOEIMAGE_UNKNOWN:
			OutputDebugString("Error: SoEImage - Unsupported Image type.");
			break;
			
		case SOEIMAGE_BMP:
			{
				SoEImageBMP gf;
				lpdds = gf.GetImage(lpDD, lSize, lpCache, memType);
			}
			break;
			
		case SOEIMAGE_TGA:
			{
				SoEImageTGA gf;
				lpdds = gf.GetImage(lpDD, lSize, lpCache, memType);
			}
			break;
			
		case SOEIMAGE_PSD:
			{
				SoEImagePSD gf;
				lpdds = gf.GetImage(lpDD, lSize, lpCache, memType);
			}
			break;
			
		case SOEIMAGE_PCX:
			{
				SoEImagePCX gf;
				lpdds = gf.GetImage(lpDD, lSize, lpCache, memType);
			}
			break;
			
			// case SOEIMAGE_JPG:
			//	{
			//		SoEImageJPG gf;
			//		lpdds = gf.GetImage(pSurface, lSize, lpCache, memType);
			//	}
			//	break;
			
			// case SOEIMAGE_PNG:
			//	{
			//		SoEImagePNG gf;
			//		lpdds = gf.GetImage(pSurface, lSize, lpCache, memType);
			//	}
			//	break;
	}
	
	return lpdds;
}
