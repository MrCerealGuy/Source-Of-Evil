/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A structure for handling 256-color bitmap files */

#include <Windows.h>

#include <Standard.h>
#include <Bitmap256.h>

CBitmap256Info::CBitmap256Info()
	{
	Default();
	}

BOOL CBitmap256Info::Valid()
	{
  if (Head.bfType != *((WORD*)"BM") ) return FALSE;
  if ((Info.biBitCount!=8) || (Info.biCompression!=0)) return FALSE;
	return TRUE;
	}

int CBitmap256Info::FileBitsOffset()
	{
	return Head.bfOffBits-sizeof(CBitmap256Info);
	}

void CBitmap256Info::Set(int iWdt, int iHgt, BYTE *bypPalette)
	{
	Default();
  // Set header
  Head.bfType=*((WORD*)"BM");
  Head.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)+DWordAligned(iWdt)*iHgt;
  Head.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD);
  // Set bitmap info
  Info.biSize=sizeof(BITMAPINFOHEADER);
  Info.biWidth=iWdt;
  Info.biHeight=iHgt;
  Info.biPlanes=1;
  Info.biBitCount=8;
  Info.biCompression=0;
  Info.biSizeImage=iWdt*iHgt;
  Info.biClrUsed=Info.biClrImportant=256;
	// Set palette
  for (int cnt=0; cnt<256; cnt++)
    {
    Colors[cnt].rgbRed	 = bypPalette[cnt*3+0];
    Colors[cnt].rgbGreen = bypPalette[cnt*3+1];
    Colors[cnt].rgbBlue  = bypPalette[cnt*3+2];
    }    
	}

void CBitmap256Info::Default()
	{
	ZeroMem(this,sizeof CBitmap256Info);
	}
