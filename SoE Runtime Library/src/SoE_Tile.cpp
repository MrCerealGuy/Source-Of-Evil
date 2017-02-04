/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Map.h"

typedef struct
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[256];
} BITMAPINFO_256;

SoETile::SoETile() : SoESurface()
{
	Init();
}

SoETile::~SoETile()
{
	if (m_BlockRects != NULL)
	{
		delete[] m_BlockRects;
		m_BlockRects = 0; 
	}
	for (int i = 0; i < m_Count; i++)
		DELETEARRAY(m_Data[i].anim);
	
	DELETEARRAY(m_Data);
}

void SoETile::Init()
{
	// reset out block vars
	m_BlockWidth = m_BlockHeight = m_BlockNum = 0;
	m_BlockX = m_BlockY = 0;
	
	// clear the pointer to our RECT structures
	m_BlockRects = NULL;
	
	// Reset our animation data
	m_Count = 0;
	m_FrameCtr = 0;
	m_Data = NULL;
}

SOEBOOL SoETile::Create(SoEScreen *pScreen, const char* szFilename, int w, int h, int num, BYTE memoryType)
{
	int i;
	DWORD x, y;
	
	if (szFilename == NULL) 
		return FALSE;
	
	// Moved it here so it will load the BMP first [MND]
	if (FAILED(SoESurface::Create(pScreen , szFilename , memoryType)))
		return FALSE;
	
	m_BlockWidth = w;
	m_BlockHeight = h;
	
	m_BlockX = (GetWidth()/w);
	m_BlockY = (GetHeight()/h);
	
	// if num==0 count the tiles [MND]
	if (num == 0)
		num = m_BlockX*m_BlockY;
	m_BlockNum = num;
	
	// allocate space for our rects
	m_BlockRects = new RECT[num];
	if (m_BlockRects == NULL)
		return FALSE;
	
	// clear out the block rect memory
	ZeroMemory(m_BlockRects, sizeof(RECT) * num);
	for (i = 0, x = 0, y = 0; i < num; i++, x += m_BlockWidth)
	{
		if (x>(DWORD)(GetWidth() - m_BlockWidth))
		{
			x = 0;
			y += m_BlockHeight;
		}
		
		m_BlockRects[i].left = x;
		m_BlockRects[i].top = y;
		m_BlockRects[i].right = x + m_BlockWidth;
		m_BlockRects[i].bottom = y + m_BlockHeight;
	}
	
	return TRUE;
}

SOEBOOL SoETile::Create(SoEScreen *pScreen, LONG lSize, fstream* fs, int w, int h, int num, BYTE memoryType)
{
	int i;
	DWORD x, y;
	
	if (fs == NULL) 
		return FALSE;
	
	// Moved it here so it will load the BMP first [MND]
	if (SoESurface::Create(pScreen, lSize, fs, memoryType) == FALSE)
		return FALSE;
	
	m_BlockWidth = w;
	m_BlockHeight = h;
	
	m_BlockX = (GetWidth()/w);
	m_BlockY = (GetHeight()/h);
	
	// if num==0 count the tiles [MND]
	if (num == 0)
		num = m_BlockX*m_BlockY;
	m_BlockNum = num;
	
	// allocate space for our rects
	m_BlockRects = new RECT[num];
	if (m_BlockRects == NULL)
		return FALSE;
	
	// clear out the block rect memory
	ZeroMemory(m_BlockRects, sizeof(RECT) * num);
	for (i = 0, x = 0, y = 0; i < num; i++, x += m_BlockWidth)
	{
		if (x >(DWORD)GetWidth() - m_BlockWidth)
		{
			x = 0;
			y += m_BlockHeight;
		}
		
		m_BlockRects[i].left = x;
		m_BlockRects[i].top = y;
		m_BlockRects[i].right = x + m_BlockWidth;
		m_BlockRects[i].bottom = y + m_BlockHeight;
	}
	
	return TRUE;
}

SOEBOOL SoETile::Create(SoEScreen *pScreen, LONG lSize, CHAR* lpCache, int w, int h, int num, BYTE memoryType)
{
	int i;
    CHAR* cache;
	DWORD x, y;
	
	if (lpCache == NULL) 
		return FALSE;
	
    cache = lpCache;
	
	// Moved it here so it will load the BMP first [MND]
	if (SoESurface::Create(pScreen, lSize, cache, memoryType) == FALSE)
		return FALSE;
	
	m_BlockWidth = w;
	m_BlockHeight = h;
	
	m_BlockX = (GetWidth()/w);
	m_BlockY = (GetHeight()/h);
	
	// if num==0 count the tiles [MND]
	if (num == 0)
		num = m_BlockX*m_BlockY;
	m_BlockNum = num;
	
	// allocate space for our rects
	m_BlockRects = new RECT[num];
	if (m_BlockRects == NULL)
		return FALSE;
	
	// clear out the block rect memory
	ZeroMemory(m_BlockRects, sizeof(RECT) * num);
	for (i = 0, x = 0, y = 0; i < num; i++, x += m_BlockWidth)
	{
		if (x >(DWORD)GetWidth() - m_BlockWidth)
		{
			x = 0;
			y += m_BlockHeight;
		}
		
		m_BlockRects[i].left = x;
		m_BlockRects[i].top = y;
		m_BlockRects[i].right = x + m_BlockWidth;
		m_BlockRects[i].bottom = y + m_BlockHeight;
	}
	
	return TRUE;
}

static void SetBitmapInfo(long width, long height, BITMAPINFO_256 &info, PBYTE pal)
{
	BITMAPINFOHEADER &h = info.bmiHeader;
	
	h.biSize			= sizeof(info.bmiHeader);
	h.biWidth			= width;
	h.biHeight			= height;
	h.biPlanes			= 1;
	h.biBitCount		= 8;
	h.biCompression		= BI_RGB;
	h.biSizeImage		=((width +3) & ~3) * height;
	h.biXPelsPerMeter	= 0;
	h.biYPelsPerMeter	= 0;
	h.biClrUsed			= 256; // Max. amount of colors
	h.biClrImportant	= 256; // All colors are important
	
	if (pal)
	{
		for (int col = 0; col < 256; col++)
		{
			info.bmiColors[col].rgbRed		= pal[0];
			info.bmiColors[col].rgbGreen	= pal[1];
			info.bmiColors[col].rgbBlue		= pal[2];
			info.bmiColors[col].rgbReserved	= 0;
			
			pal += 3;
		}
	}
	else
	{
		::ZeroMemory(info.bmiColors, sizeof(info.bmiColors));
	}
}

#if 0
SOEBOOL SoETile::CreateFromTLE(SoEScreen *pScreen, const char* szFilename , BYTE memoryType)
{
	if (szFilename == NULL) 
		return FALSE;
	
	// So, let's try to load the tle file.
	FILE *tleFile = fopen(szFilename, "rb"); // open for binary read
	WORD count;    // number of bytes stored in the tle file
	WORD width;    // width of each tile
	WORD height;   // height of each tile
	BYTE pal[768]; // the palette used by the tiles
	
	if (!tleFile)
	{
		return FALSE;
	}
	
	// read the number of tiles and their dimensions
	fread(&count , sizeof(WORD), 1, tleFile);
	fread(&width , sizeof(WORD), 1, tleFile);
	fread(&height, sizeof(WORD), 1, tleFile);
	
	// read the (dos) palette
	fread(pal, 768, 1, tleFile);
	
	// read all of the tile data at once
	PBYTE buffer;
	DWORD size = count * width * height;
	DWORD count_read;
	
	// create a buffer which can handle "size" bytes
	buffer = (PBYTE) malloc(size);
	if (!buffer)
	{
		// uups, no memory
		fclose(tleFile);
		return FALSE;
	}
	
	// Read "size" bytes
	count_read = fread(buffer, 1, size, tleFile);
	
	if (count_read != size)
	{
		// Something went wrong. Either a read error,
		// or a wrong file format
		free(buffer);
		fclose(tleFile);
		return FALSE;
	}
	
	fclose(tleFile); // close TLE file
    
	DWORD image_width    = count * width;
    DWORD image_height   = height;
    LONG  count_per_line = count;
    LONG  count_per_col  = 1;
	
    DWORD maxWidth = pScreen->GetWidth();
    if (image_width > maxWidth)
    {
        count_per_line = maxWidth / width;
        count_per_col  = (count / count_per_line) +1;
        
        image_width    = count_per_line * width;
        image_height   = count_per_col  * height;
    }
	
	// Since SoE awaits tile data in a DD surface, let's create one
	// with a height of "height" and a size of ("width" * "count")
	if (
		FAILED(SoESurface::Create(pScreen , image_width , image_height , memoryType))
		return FALSE;
    // m_lpDDS = DDCreateSurface(pScreen->m_lpDD, image_width, image_height, &ddsd);
	if (GetDDS() == NULL)
	{
		free(buffer);
		MessageBox(NULL, "CreateSurface failed", "Dbg", MB_OK);
		return FALSE;
	}
    
	// Now, we create a DIBSection to hold the tiles
	// First step is to set-up the information
	HDC				hDC = ::CreateDC("DISPLAY", NULL, NULL, NULL);
	BITMAPINFO_256	BitmapInfo;	
	UINT			iUsage= DIB_RGB_COLORS;
	
	SetBitmapInfo(image_width, image_height, BitmapInfo, pal);
	
	// Then to create the DIBSection
	BYTE *bmp_data; // Going to get the buffer
	HBITMAP hBitmap;
	hBitmap = CreateDIBSection(
		hDC,
		(BITMAPINFO*)&BitmapInfo, 
		iUsage, 
		(void **) &bmp_data, 
		NULL, 0);
	
	DeleteDC(hDC);
	
	// Fill with 0
	FillMemory(bmp_data, ((width +3) & ~3) * image_height, 0);
	
	// Since TLE files store 256 color tiles, we need to check
	// the format of the SoE- screen.
	// If we have less than 8 bpp, we return with an error.
	// If we have 8 bpp, we copy the data
	// If we have more than 8bpp, we use  the palette data to
	//    fill the surface with rgbx values.
	
	LONG dwLineWidth = (image_width +3) & ~3;
	LONG offset = 0;
    LONG btm = (image_height - 1) * dwLineWidth;
    LONG line_height = dwLineWidth *height;
    LONG x = 0, y = 0, foo = count_per_col - 1;
	PBYTE surface; 
	PBYTE source = buffer;
    LONG  transfered =count;
	
	int line;
	
    for (y = 0; y <(count_per_col - 1); y++)
    {
        for (x = 0, offset = 0; x < count_per_line; x++, offset += width)
        {
            surface = bmp_data + btm + offset;
			
            for (line = 0; line < height; line++)
			{
				memcpy(surface, source, width);
				source  += width;
                surface -= dwLineWidth;
			}
            transfered--;
        }
        btm -= line_height;
    }
	
    for (x = 0, offset = 0; transfered >0; x++, offset += width, transfered--)
    {
        surface = bmp_data + btm + offset;
		
        for (line = 0; line < height; line++)
		{
			memcpy(surface, source, width);
			source  += width;
            surface -= dwLineWidth;
		}
    }
	free(buffer);
	
	// At this point, we have a DIB holding the
	// tile data, and a surface holding nothing, so
	// let's copy the DIB to the surface
	DDCopyBitmap(GetDDS(), hBitmap, 0, 0, 0, 0);
	
	// Now, we can free the DIB
	::DeleteObject(hBitmap);
	
	m_pFilename = NULL;
	// szFilename;
	
	Screen = pScreen; 
	
	m_BlockWidth  = width;
	m_BlockHeight = height;
	m_BlockNum    = count;
	
	return TRUE;
}
#endif

const RECT *SoETile::GetBlockRect(int block)
{
	return &m_BlockRects[block];
}

SOEBOOL SoETile::LoadAnim(FILE *fptr)
{
	int i;
	for (i = 0; i < m_Count; i++)
		DELETEARRAY(m_Data[i].anim);
	
	DELETEARRAY(m_Data);
	m_Count = 0;
	
	if (!feof(fptr))
	{
		int i;
		
		fread(&m_Count, sizeof(int), 1, fptr);
		m_Data = new SOE_ANIMSTRUCT[m_Count];
		
		for (i = 0; i < m_Count; i++)
		{
			fread(&m_Data[i], sizeof(SOE_ANIMSTRUCT), 1, fptr);
			m_Data[i].anim = new INT32[m_Data[i].num];
			fread(m_Data[i].anim, sizeof(INT32), m_Data[i].num, fptr);
		}
	}
	
	return TRUE;
}

SOEBOOL SoETile::SaveAnim(FILE *fptr)
{
	fwrite(&m_Count, sizeof(int), 1, fptr);
	
	for (int i = 0; i < m_Count; i++)
	{
		fwrite(&m_Data[i], sizeof(SOE_ANIMSTRUCT), 1, fptr);
		fwrite(m_Data[i].anim, sizeof(int), m_Data[i].num, fptr);
	}
	return TRUE;
}

SOEBOOL SoETile::AddAnim(int framerate, int pause, int rnd, BOOL pingpong, INT32 *anim, int num)
{
	SOE_ANIMSTRUCT *ptr = new SOE_ANIMSTRUCT[m_Count + 1];
	CopyMemory(ptr, m_Data, sizeof(SOE_ANIMSTRUCT) * m_Count);
	
	ptr[m_Count].framerate = framerate;
	ptr[m_Count].pause = pause;
	ptr[m_Count].rnd = rnd;
	ptr[m_Count].pingpong = pingpong;
	ptr[m_Count].anim = new INT32[num];
	ptr[m_Count].num = num;
	CopyMemory(ptr[m_Count].anim, anim, num*sizeof(INT32));
	
	DELETEARRAY(m_Data);
	m_Data = ptr;
	m_Count++;
	
	return TRUE;
}

SOEBOOL SoETile::AddAnimSection(int framerate, int pause, int rnd, BOOL pingpong, int blockstart, int blockend)
{
	INT32 *anim = NULL;
	int bcount = 0;
	
	// build up the block list
	
	// determine the number of blocks
	bcount = blockend - blockstart;
	
	// allocate enough space for that
	anim = new INT32[bcount];
	if (anim == NULL)
		return FALSE;
	
	if (!AddAnim(framerate, pause, rnd, pingpong, anim, bcount))
	{
		delete[] anim;
		return FALSE;
	}
	
	return TRUE;
}

SOEBOOL SoETile::InsertTile(int anim, int pos, int tile)
{
	if (anim >= m_Count || anim < 0)
		return FALSE;
	
	if (pos>m_Data[anim].num || pos < 0)
		return FALSE;
	
	INT32 *ptr = new INT32[m_Data[anim].num + 1];
	
	if (pos>0)
		CopyMemory(ptr, m_Data[anim].anim, pos*sizeof(INT32));
	if (pos < m_Data[anim].num)
		CopyMemory(&ptr[pos + 1], &m_Data[anim].anim[pos], (m_Data[anim].num - pos)*sizeof(INT32));
	ptr[pos] = tile;
	m_Data[anim].num++;
	m_Data[anim].anim = ptr;
	
	return TRUE;
}

SOEBOOL SoETile::DelAnim(int pos)
{
	if (pos >= m_Count || pos < 0)
		return FALSE;
	
	SOE_ANIMSTRUCT *ptr = new SOE_ANIMSTRUCT[m_Count - 1];
	
	if (pos>0)
		CopyMemory(ptr, m_Data, pos*sizeof(SOE_ANIMSTRUCT));
	if (pos < m_Count - 1)
		CopyMemory(&ptr[pos], &m_Data[pos + 1], (m_Count - pos)*sizeof(SOE_ANIMSTRUCT));
	DELETEARRAY(m_Data[pos].anim);
	DELETEARRAY(m_Data);
	
	m_Count--;
	m_Data = ptr;
	return TRUE;
}

SOEBOOL SoETile::RemoveTile(int anim, int pos)
{
	if (anim >= m_Count || anim < 0)
		return FALSE;
	
	if (pos >= m_Data[anim].num || pos < 0 || m_Data[anim].num == 1)
		return FALSE;
	
	INT32 *ptr = new INT32[m_Data[anim].num - 1];
	
	if (pos>0)
		CopyMemory(ptr, m_Data[anim].anim, (pos)*sizeof(INT32));
	if (pos < m_Data[anim].num)
		CopyMemory(&ptr[pos], &m_Data[anim].anim[pos + 1], (m_Data[anim].num - pos)*sizeof(INT32));
	
	m_Data[anim].num--;
	m_Data[anim].anim = ptr;
	
	return TRUE;
}

INT32 SoETile::GetAnimTile(int tile)
{
	if (tile >= 0)
		return tile;
	else
	{
		// Get anim tile
		tile = (-tile) - 1;
		if (tile >= m_Count)
			return 0;
		
		SOE_ANIMSTRUCT *ptr = &m_Data[tile];
		
		int pos;
		if (ptr->pingpong)
		{
			int i = (m_FrameCtr/ptr->framerate)%(ptr->num*2 - 2 + ptr->pause);
			if (i < (ptr->num))
				pos = i;
			else
			{
				pos = ptr->num - (i - (ptr->num)) - 1 - 1;
				if (pos < 0)
					pos = 0;
			}
		}
		else
		{
			pos = (m_FrameCtr/ptr->framerate)%(ptr->num + ptr->pause);
			if (pos >= ptr->num)
				pos = 0;
		}
		
		tile = ptr->anim[pos];
	}
	
	return tile;
}

void SoETile::Draw(int x, int y, SoESurface *s, int BlitType, int tile)
{
	// If no tile then just return
	if (tile == 0)
		return;
	
	tile = GetAnimTile(tile);
	
	switch (BlitType)
	{
		case SOEBLT_BLK:	
			SoESurface::DrawBlk(s, x, y, &m_BlockRects[tile]);
			break;
		case SOEBLT_BLKALPHAFAST:
			SoESurface::DrawBlkAlphaFast(s, x, y, &m_BlockRects[tile]);
			break;
		case SOEBLT_BLKALPHA25:
			SoESurface::DrawBlkAlpha(s, x, y, &m_BlockRects[tile], 25);
			break;
		case SOEBLT_TRANS:	
			SoESurface::DrawTrans(s, x, y, &m_BlockRects[tile]);
			break;
		case SOEBLT_TRANSALPHAFAST:
			SoESurface::DrawTransAlphaFast(s, x, y, &m_BlockRects[tile]);
			break;
		case SOEBLT_TRANSALPHA25:
			SoESurface::DrawTransAlpha(s, x, y, &m_BlockRects[tile], 25);
			break;
	}
}

void SoETile::DrawFirst(int x, int y, SoESurface *s, int BlitType, int tile)
{
	// If no tile then just return
	if (tile == 0)
		return;
	
	if (tile < 0)
	{
		// Get anim tile
		tile = (-tile) - 1;
		if (tile >= m_Count)
			return;
		
		tile = m_Data[tile].anim[0];
	}
	
	Draw(x, y, s, BlitType, tile);
}

void SoETile::NextFrame(void)
{
	m_FrameCtr++;
}

SOE_ANIMSTRUCT *SoETile::GetAnimData(int nr)
{
	if (nr >= 0 && nr < m_Count)
		return &m_Data[nr];
	return NULL;
}
