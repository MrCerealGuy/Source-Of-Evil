/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include <SoE_ImageJPG.h>

SoEImageJPG::SoEImageJPG()
{
}

SoEImageJPG::~SoEImageJPG()
{
}

BOOL SoEImageJPG::Validate(const char *head, const int headsize, const char *foot, const int footsize)
{

	return FALSE;
}

SOE_LPDIRECTDRAWSURFACE SoEImageJPG::GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType)
//#if DIRECTDRAW_VERSION >= SOE_DDVER
//LPDIRECTDRAWSURFACE4 SoEImageJPG::GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType)
//#else
//LPDIRECTDRAWSURFACE SoEImageJPG::GetImage(SoEScreen* pScreen, const LPSTR filename, BYTE memType)
//#endif
{
	UINT height;
	UINT width;
	BYTE *dataBuf;
	UINT widthDW;

	//read the file
	dataBuf = JpegFileToRGB((char*)filename, &width, &height);
	if (dataBuf==NULL) 
	{
		return NULL;
	}
	
	// RGB -> BGR
	BGRFromRGB(dataBuf, width, height);
	
	// create a DWORD aligned buffer from the JpegFile object
	m_pStorage = (char*)MakeDwordAlignedBuf(dataBuf, width, height, &widthDW);
	
	// flip that buffer
	//VertFlipBuf(GetStoragePtr(), widthDW, height);
	// you now have a buffer ready to be used as a DIB
	
	SetImageWidth(width);
	SetImageHeight(height);
	SetImageBPP(24);

	//create surface, this uses member variables set during AllocateStorage
	if(!CreateSurface(pScreen->GetDD(), memType))
	{
		DeAllocateStorage();
		return NULL;
	}

	SetupBitmapInfo();

	SetInverted(TRUE);	//this saves us from manually flipping the surface.
	//if the images is inverted, then use -height instead of height.
	if(IsInverted())
		m_pBitmapInfo->bmiHeader.biHeight = -m_pBitmapInfo->bmiHeader.biHeight;

	return( CopyBits() );
}

#ifdef __cplusplus
	extern "C" {
#endif // __cplusplus
#define HAVE_BOOLEAN
#include "jpeglib.h"
#undef HAVE_BOOLEAN
#ifdef __cplusplus
	}
#endif // __cplusplus

#include <setjmp.h>

// error handler, to avoid those pesky exit(0)'s
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo);

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	/* Always display the message. */
	//MessageBox(NULL,buffer,"JPEG Fatal Error",MB_ICONSTOP);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

// store a scanline to our data buffer
void j_putRGBScanline(BYTE *jpegline, int widthPix, BYTE *outBuf, int row);
void j_putGrayScanlineToRGB(BYTE *jpegline, int widthPix, BYTE *outBuf, int row);

BYTE * SoEImageJPG::JpegFileToRGB(char * fileName, UINT *width, UINT *height)
{
	// get our buffer set to hold data
	BYTE *dataBuf = NULL;

	// basic code from IJG Jpeg Code v6 example.c

	*width=0;
	*height=0;

	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile=NULL;		/* source file */

	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
	char buf[250];

	if ((infile = fopen(fileName, "rb")) == NULL) {
		sprintf(buf, "JPEG :\nCan't open %s\n", fileName);
		//AfxMessageBox(buf);
		//MessageBox(NULL, buf, "JpegLoader", MB_OK);
		return NULL;
	}

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;


	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		jpeg_destroy_decompress(&cinfo);

		if (infile!=NULL)
			fclose(infile);

      if (dataBuf!=NULL)
      {
         delete [] dataBuf;
      }

		return NULL;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	dataBuf=(BYTE *)new BYTE[cinfo.output_width * 3 * cinfo.output_height];
	if (dataBuf==NULL) {

		//AfxMessageBox("JpegFile :\nOut of memory",MB_ICONSTOP);

		jpeg_destroy_decompress(&cinfo);
		
		fclose(infile);

		return NULL;
	}

	// how big is this thing gonna be?
	*width = cinfo.output_width;
	*height = cinfo.output_height;
	
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) 
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */

		// asuumer all 3-components are RGBs
		if (cinfo.out_color_components==3) 
		{
			
			j_putRGBScanline(buffer[0], 
								*width,
								dataBuf,
								cinfo.output_scanline-1);

		} 
		else if (cinfo.out_color_components==1) 
		{

			// assume all single component images are grayscale
			j_putGrayScanlineToRGB(buffer[0], 
								*width,
								dataBuf,
								cinfo.output_scanline-1);

		}

	}

	(void) jpeg_finish_decompress(&cinfo);
	
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return dataBuf;
}

BOOL SoEImageJPG::GetJPGDimensions(char * fileName, UINT *width, UINT *height)
{
	// basic code from IJG Jpeg Code v6 example.c

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile=NULL;		/* source file */
	char buf[250];

	if ((infile = fopen(fileName, "rb")) == NULL) {
		sprintf(buf, "JPEG :\nCan't open %s\n", fileName);
		//AfxMessageBox(buf);
		//MessageBox(NULL, buf, "JpegLoader", MB_OK);
		return FALSE;
	}

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;


	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {

		jpeg_destroy_decompress(&cinfo);

		if (infile!=NULL)
			fclose(infile);
		return FALSE;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);

	(void) jpeg_read_header(&cinfo, TRUE);

	// how big is this thing ?
	*width = cinfo.image_width;
	*height = cinfo.image_height;

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return TRUE;
}

BYTE *SoEImageJPG::RGBFromDWORDAligned(BYTE *inBuf, UINT widthPix, UINT widthBytes, UINT height)
{
	if (inBuf==NULL)
		return NULL;

	BYTE *tmp;
	tmp=(BYTE *)new BYTE[height * widthPix * 3];
	if (tmp==NULL)
		return NULL;

	UINT row;

	for (row=0;row<height;row++) {
		memcpy((tmp+row * widthPix * 3), 
				(inBuf + row * widthBytes), 
				widthPix * 3);
	}

	return tmp;
}

BOOL SoEImageJPG::RGBToJpegFile(char * fileName, BYTE *dataBuf, UINT widthPix, UINT height, BOOL color, int quality)
{
	if (dataBuf==NULL)
		return FALSE;
	if (widthPix==0)
		return FALSE;
	if (height==0)
		return FALSE;

	LPBYTE tmp;
	if (!color) {
		tmp = (BYTE*)new BYTE[widthPix*height];
		if (tmp==NULL) {
			//AfxMessageBox("Memory error");
			//MessageBox(NULL, "Memory error", "JpegLoader", MB_OK);
			return FALSE;
		}

		UINT row,col;
		for (row=0;row<height;row++) {
			for (col=0;col<widthPix;col++) {
				LPBYTE pRed, pGrn, pBlu;
				pRed = dataBuf + row * widthPix * 3 + col * 3;
				pGrn = dataBuf + row * widthPix * 3 + col * 3 + 1;
				pBlu = dataBuf + row * widthPix * 3 + col * 3 + 2;

				// luminance
				int lum = (int)(.299 * (double)(*pRed) + .587 * (double)(*pGrn) + .114 * (double)(*pBlu));
				LPBYTE pGray;
				pGray = tmp + row * widthPix + col;
				*pGray = (BYTE)lum;
			}
		}
	}

	struct jpeg_compress_struct cinfo;
	/* More stuff */
	FILE * outfile=NULL;			/* target file */
	int row_stride;			/* physical row widthPix in image buffer */

	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {

		jpeg_destroy_compress(&cinfo);

		if (outfile!=NULL)
			fclose(outfile);

		if (!color) {
			delete [] tmp;
		}
		return FALSE;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(fileName, "wb")) == NULL) {
		char buf[250];
		sprintf(buf, "JpegFile :\nCan't open %s\n", fileName);
		//AfxMessageBox(buf);
		return FALSE;
	}

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = widthPix; 	/* image widthPix and height, in pixels */
	cinfo.image_height = height;
	if (color) {
		cinfo.input_components = 3;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	} else {
		cinfo.input_components = 1;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	}

   jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = widthPix * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
 	LPBYTE outRow;
	if (color) {
		outRow = dataBuf + (cinfo.next_scanline * widthPix * 3);
	} else {
		outRow = tmp + (cinfo.next_scanline * widthPix);
	}

    (void) jpeg_write_scanlines(&cinfo, &outRow, 1);
  }

  jpeg_finish_compress(&cinfo);

  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  if (!color)
	  delete [] tmp;
  /* And we're done! */

  return TRUE;
}

void j_putRGBScanline(BYTE *jpegline, int widthPix, BYTE *outBuf, int row)
{
	int offset = row * widthPix * 3;
	int count;
	for (count=0;count<widthPix;count++) 
	{
		*(outBuf + offset + count * 3 + 0) = *(jpegline + count * 3 + 0);
		*(outBuf + offset + count * 3 + 1) = *(jpegline + count * 3 + 1);
		*(outBuf + offset + count * 3 + 2) = *(jpegline + count * 3 + 2);
	}
}

void j_putGrayScanlineToRGB(BYTE *jpegline, int widthPix, BYTE *outBuf, int row)
{
	int offset = row * widthPix * 3;
	int count;
	for (count=0;count<widthPix;count++) {

		BYTE iGray;

		// get our grayscale value
		iGray = *(jpegline + count);

		*(outBuf + offset + count * 3 + 0) = iGray;
		*(outBuf + offset + count * 3 + 1) = iGray;
		*(outBuf + offset + count * 3 + 2) = iGray;
	}
}

BYTE * SoEImageJPG::MakeDwordAlignedBuf(BYTE *dataBuf, UINT widthPix, UINT height, UINT *uiOutWidthBytes)
{
	if (dataBuf==NULL)
		return NULL;

	UINT uiWidthBytes;
	uiWidthBytes = WIDTHBYTES(widthPix * 24);

	DWORD dwNewsize=(DWORD)((DWORD)uiWidthBytes * 
							(DWORD)height);
	BYTE *pNew;

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
	pNew=(BYTE *)new BYTE[dwNewsize];
	if (pNew==NULL) {
		return NULL;
	}
	
	////////////////////////////////////////////////////////////
	// copy row-by-row
	UINT uiInWidthBytes = widthPix * 3;
	UINT uiCount;
	for (uiCount=0;uiCount < height;uiCount++) {
		BYTE * bpInAdd;
		BYTE * bpOutAdd;
		ULONG lInOff;
		ULONG lOutOff;

		lInOff=uiInWidthBytes * uiCount;
		lOutOff=uiWidthBytes * uiCount;

		bpInAdd= dataBuf + lInOff;
		bpOutAdd= pNew + lOutOff;

		memcpy(bpOutAdd,bpInAdd,uiInWidthBytes);
	}

	*uiOutWidthBytes=uiWidthBytes;
	return pNew;
}

BOOL SoEImageJPG::VertFlipBuf(BYTE  * inbuf, UINT widthBytes, UINT height)
{   
	BYTE  *tb1;
	BYTE  *tb2;

	if (inbuf==NULL)
		return FALSE;

	UINT bufsize;

	bufsize=widthBytes;

	tb1= (BYTE *)new BYTE[bufsize];
	if (tb1==NULL) {
		return FALSE;
	}

	tb2= (BYTE *)new BYTE [bufsize];
	if (tb2==NULL) {
		delete [] tb1;
		return FALSE;
	}
	
	UINT row_cnt;     
	ULONG off1=0;
	ULONG off2=0;

	for (row_cnt=0;row_cnt<(height+1)/2;row_cnt++) {
		off1=row_cnt*bufsize;
		off2=((height-1)-row_cnt)*bufsize;   
		
		memcpy(tb1,inbuf+off1,bufsize);
		memcpy(tb2,inbuf+off2,bufsize);	
		memcpy(inbuf+off1,tb2,bufsize);
		memcpy(inbuf+off2,tb1,bufsize);
	}	

	delete [] tb1;
	delete [] tb2;

	return TRUE;
}        

BOOL SoEImageJPG::BGRFromRGB(BYTE *buf, UINT widthPix, UINT height)
{
	if (buf==NULL)
		return FALSE;

	UINT col, row;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 3 + col * 3;
			pGrn = buf + row * widthPix * 3 + col * 3 + 1;
			pBlu = buf + row * widthPix * 3 + col * 3 + 2;

			// swap red and blue
			BYTE tmp;
			tmp = *pRed;
			*pRed = *pBlu;
			*pBlu = tmp;
		}
	}
	return TRUE;
}

BOOL SoEImageJPG::MakeGrayScale(BYTE *buf, UINT widthPix, UINT height)
{
	if (buf==NULL)
		return FALSE;

	UINT row,col;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 3 + col * 3;
			pGrn = buf + row * widthPix * 3 + col * 3 + 1;
			pBlu = buf + row * widthPix * 3 + col * 3 + 2;

			// luminance
			int lum = (int)(.299 * (double)(*pRed) + .587 * (double)(*pGrn) + .114 * (double)(*pBlu));

			*pRed = (BYTE)lum;
			*pGrn = (BYTE)lum;
			*pBlu = (BYTE)lum;
		}
	}
	return TRUE;
}



