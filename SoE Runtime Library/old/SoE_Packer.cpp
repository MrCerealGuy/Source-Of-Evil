/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Surface.h"
#include "SoE_Packer.h"

void DibBlit( SoESurface *lpDDS, int dw, int dh, char *data, int sw, int sh, int bpp  )
{	
	BITMAPINFO bi;
	ZeroMemory( &bi, sizeof( bi ) );
	bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bi.bmiHeader.biWidth = sw;
	bi.bmiHeader.biHeight = sh;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = bpp;
	bi.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(  lpDDS->GetDC(), 0,0, dw, dh,
					0,0, sw, sh, data, &bi,
					DIB_RGB_COLORS,SRCCOPY);
	lpDDS->ReleaseDC( );
}

int SoEPacker::FileInfo( char *in, int &w, int &h, BYTE &b )
{	
	fin = fopen( in, "rb" );
	FilePtrInfo( fin, w, h, b );
	fclose( fin );
	return textsize;
}

int SoEPacker::FilePtrInfo( FILE *in, int &w, int &h, BYTE &b )
{	fin = in;

	DWORD pos = ftell( fin );
	ReadHeader( );
	fseek( fin, pos, SEEK_SET);

	w=width;
	h=height;
	b=bpp;
	return textsize;
}

BOOL SoEPacker::PackFile( char *in, char *out )
{	fin = fopen( in, "rb" );
	fout = fopen( out, "wb" );
	if( !fin || !fout ) return FALSE;
	fseek(fin, 0L, SEEK_END);
	width=textsize = ftell(fin);
	height=1;
	if( !WriteHeader( ) ) return FALSE;
	rewind(fin);

	mode=0;
	Encode( );

	fclose( fin );
	fclose( fout );
	return TRUE;
}

BOOL SoEPacker::PackRam( BYTE *in, DWORD w, DWORD h, BYTE d, char *out )
{	
	fout = fopen( out, "wb" );
	if( !in || !fout ) return FALSE;
	width=w;
	height=h;
	bpp=d;
	textsize=w*h*d;
	if( !WriteHeader( ) ) return FALSE;

	ramptr=in;
	mode=1;
	Encode( );

	fclose( fout );
	return TRUE;
}

BOOL SoEPacker::PackTo( DWORD w, DWORD h, BYTE d, PACKPROC lpProc, void *data, char *out )
{	
	fout = fopen( out, "wb" );
	if( !fout ) return FALSE;

	width=w;
	height=h;
	bpp=d;
	textsize = width*height*d;
	if( !WriteHeader( ) ) return FALSE;

	m_data=data;
	m_lpPProc=lpProc;
	mode=2;
	Encode( );

	fclose( fout );
	return TRUE;
}

BOOL SoEPacker::ExpandToFile( char *in, char *out )
{	
	fin = fopen( in, "rb" );
	fout = fopen( out, "wb" );
	if( !fin || !fout ) return FALSE;
	if( !ReadHeader( ) ) return FALSE;

	mode=0;
	Decode( );

	fclose( fin );
	fclose( fout );
	return TRUE;
}

BYTE *SoEPacker::ExpandToRam( char *in )
{
	fin = fopen( in, "rb" );
	ExpandToRam( fin );
	fclose( fin );

	return ramptr;
}

BYTE *SoEPacker::ExpandToRam( FILE *in )
{
	fin = in;
	if( !fin ) return 0;
	if( !ReadHeader( ) ) return FALSE;

	ramptr=new BYTE[textsize];
	mode=1;
	Decode( );

	return ramptr;
}

BOOL SoEPacker::ExpandTo( char *in, EXPANDPROC lpProc, void *data )
{	
	fin = fopen( in, "rb" );
	ExpandTo( fin, lpProc, data );
	fclose( fin );
	return TRUE;
}

BOOL SoEPacker::ExpandTo( FILE *in, EXPANDPROC lpProc, void *data )
{
	fin=in;
	if( !fin ) return FALSE;
	if( !ReadHeader( ) ) return FALSE;

	mode=2;
	m_lpEProc = lpProc;
	m_data = data;
	Decode( );

	return TRUE;
}

BOOL SoEPacker::ExpandSurface( char *in, SoESurface *lpDDS )
{
	fin = fopen( in, "rb" );
	ExpandSurface( fin, lpDDS );
	fclose( fin );
	return TRUE;
}

BOOL SoEPacker::ExpandSurface( FILE *in, SoESurface *lpDDS )
{	
	int sw, sh;
	BYTE bpp;
	
	FilePtrInfo( in, sw, sh, bpp );
	bpp*=8;

	char *tmp = (char*)ExpandToRam( in );
	DibBlit( lpDDS, lpDDS->GetWidth(), lpDDS->GetHeight(), tmp, sw, sh, bpp );
	delete [] tmp;

	return TRUE;
}

void SoEPacker::Init( int comp )
{
	low=0;
	high=Q4;
	value=0;
	shifts=0;

	codesize=sizeof( textsize );

	pos=0;
	buffer=0;
	mask = (comp) ? 128 : 0;
}

int SoEPacker::SpecWrite( BYTE ch )
{
	int ret=-1;
	switch( mode )
	{	case 0:	ret=putc( ch, fout );
				break;

		case 1:	ramptr[pos]=ch;
				ret=0;
				break;

		case 2: m_lpEProc( ch, pos, width, height, m_data );
				ret=0;
				break;
	}

	pos++;
	return ret;
}

int SoEPacker::SpecRead( void )
{
	int ret=-1;
	
	if( pos<textsize )
	{	switch( mode )
		{	case 0:	ret=getc(fin);
					break;
	
			case 1: ret=ramptr[pos];
					break;

			case 2:	ret=(DWORD)m_lpPProc( pos, width, height, m_data );
					break;
		}
	}
	pos++;
	return ret;
}

BOOL SoEPacker::ReadHeader( void )
{
	char tmp[10];
	if( fread(&tmp, 3, 1, fin ) < 1 )
		return FALSE;
	
	if (fread(&textsize, sizeof( textsize ), 1, fin) < 1)
		return FALSE;

	if( fread(&width, sizeof( width ), 1, fin ) < 1 )
		return FALSE;

	if( fread(&height, sizeof( height ), 1, fin ) < 1 )
		return FALSE;

	if( fread(&bpp, sizeof( bpp ), 1, fin ) < 1 )
		return FALSE;

	if( !textsize ) return FALSE;

	return TRUE;
}

BOOL SoEPacker::WriteHeader( void )
{
	if( fwrite("MND", 3, 1, fout ) < 1 )
		return FALSE;

	if (fwrite(&textsize, sizeof( textsize ), 1, fout) < 1)
		return FALSE;

	if( fwrite(&width, sizeof( width ), 1, fout ) < 1 )
		return FALSE;

	if( fwrite(&height, sizeof( height ), 1, fout ) < 1 )
		return FALSE;

	if( fwrite(&bpp, sizeof( bpp ), 1, fout ) < 1 )
		return FALSE;

	if (textsize == 0) return FALSE;

	return TRUE;
}

void SoEPacker::StartModel(void)
{	
	sym_cum[N_CHAR] = 0;
	for( int sym = N_CHAR; sym >= 1; sym-- )
	{	int ch = sym - 1;
		char_to_sym[ch] = sym;
		sym_to_char[sym] = ch;
		sym_freq[sym] = 1;
		sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
	}
	sym_freq[0] = 0;
	position_cum[N] = 0;

	for( int i = N; i >= 1; i-- )
		position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
}

void SoEPacker::UpdateModel(int sym)
{	
	int i;
	
	if (sym_cum[0] >= MAX_CUM)
	{	int c = 0;
		for( i=N_CHAR; i>0; i-- )
		{	sym_cum[i] = c;
			c+=(sym_freq[i] = (sym_freq[i] + 1) >> 1);
		}
		sym_cum[0] = c;
	}

	for (i = sym; sym_freq[i] == sym_freq[i - 1]; i--) ;
	if (i < sym) 
	{	int ch_i = sym_to_char[i];    /* swap */
		int ch_sym = sym_to_char[sym];
		sym_to_char[i] = ch_sym;
		sym_to_char[sym] = ch_i;
		char_to_sym[ch_i] = sym;
		char_to_sym[ch_sym] = i;
	}
	sym_freq[i]++;
	while (--i >= 0) sym_cum[i]++;
}

int SoEPacker::GetBit(void)
{
	if( (mask >>= 1) == 0 )
	{	buffer = getc(fin);
		mask = 128;
	}
	return( (buffer & mask) != 0 );
}

int SoEPacker::BinarySearchSym(WORD x)
{	
	int i=1, j=N_CHAR;
	
	while( i<j )
	{	int k = (i+j)/2;
		if(sym_cum[k] > x) i = k + 1;
		else j = k;
	}
	return i;
}

int SoEPacker::BinarySearchPos(WORD x)
{	
	int i=1, j=N;
	
	while (i < j)
	{	int k = (i + j) / 2;
		if (position_cum[k] > x) i = k + 1;
		else j = k;
	}
	return i - 1;
}

void SoEPacker::StartDecode(void)
{	
	for (int i=0; i<M+2; i++)
		value = 2 * value + GetBit();
}

int SoEPacker::DecodeChar(void)
{	
	int	 sym, ch;
	DWORD range;
	
	range = high - low;
	sym = BinarySearchSym((WORD)(((value - low + 1) * sym_cum[0] - 1) / range));
	high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
	low +=       (range * sym_cum[sym    ]) / sym_cum[0];
	for ( ; ; )
	{	if (low >= Q2)
		{	value -= Q2;
			low -= Q2;
			high -= Q2;
		}
		else if (low >= Q1 && high <= Q3)
		{	value -= Q1;
			low -= Q1;
			high -= Q1;
		}
		else if (high > Q2) break;
		low += low;
		high += high;
		value = 2 * value + GetBit();
	}
	ch = sym_to_char[sym];
	UpdateModel(sym);
	return ch;
}

int SoEPacker::DecodePosition(void)
{	
	int position;
	DWORD range;
	
	range = high - low;
	position = BinarySearchPos((WORD)(((value - low + 1) * position_cum[0] - 1) / range));
	high = low + (range * position_cum[position    ]) / position_cum[0];
	low +=       (range * position_cum[position + 1]) / position_cum[0];
	for ( ; ; )
	{	if (low >= Q2)
		{	value -= Q2;
			low -= Q2;
			high -= Q2;
		}
		else if (low >= Q1 && high <= Q3)
		{	value -= Q1;
			low -= Q1;
			high -= Q1;
		} else if (high > Q2) break;
		low += low;
		high += high;
		value = 2 * value + GetBit();
	}
	return position;
}

BOOL SoEPacker::Decode(void)
{	
	int  i, j, k, r, c;
	DWORD count;

	Init( 0 );
	
	StartDecode();
	StartModel();
	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;

	for (count = 0; count < textsize; )
	{	c = DecodeChar();
		if (c < 256)
		{	SpecWrite( c );
			text_buf[r++] = c;
			r &= (N - 1);
			count++;
		}
		else
		{	i = (r - DecodePosition() - 1) & (N - 1);
			j = c - 255 + THRESHOLD;

			for (k = 0; k < j; k++)
			{	c = text_buf[(i + k) & (N - 1)];
				SpecWrite( c );
				text_buf[r++] = c;
				r &= (N - 1);
				count++;
			}
		}
	}

	return TRUE;
}

BOOL SoEPacker::PutBit(int bit)
{
	if( bit ) buffer |= mask;
	if( (mask >>= 1) == 0 )
	{	if (putc(buffer, fout) == EOF) return FALSE;
		buffer = 0;
		mask = 128;
		codesize++;
	}

	return TRUE;
}

void SoEPacker::FlushBitBuffer(void)
{	
	for(int i = 0; i < 7; i++)
		PutBit(0);
}

void SoEPacker::InitTree(void)
{	
	int  i;

	for (i=N+1; i<=N+256; i++) rson[i]=NIL;
	for (i=0; i<N; i++) dad[i]=NIL;
}

void SoEPacker::InsertNode(int r)
{	
	int  i, p, cmp, temp;
	BYTE *key;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	for ( ; ; )
	{	if (cmp >= 0)
		{	if (rson[p] != NIL) p = rson[p];
			else
			{	rson[p] = r;
				dad[r] = p;
				return;
			}
		}
		else
		{	if (lson[p] != NIL) p = lson[p];
			else 
			{	lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > THRESHOLD)
		{	if (i > match_length)
			{	match_position = (r - p) & (N - 1);
				if ((match_length = i) >= F) break;
			}
			else if (i == match_length)
			{	if ((temp = (r - p) & (N - 1)) < match_position)
					match_position = temp;
			}
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;
}

void SoEPacker::DeleteNode(int p)
{	
	int  q;
	
	if (dad[p] == NIL) return;
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else
	{	q = lson[p];
		if (rson[q] != NIL)
		{	do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;
	else                   lson[dad[p]] = q;
	dad[p] = NIL;
}

void SoEPacker::Output(int bit)
{	
	PutBit(bit);
	for ( ; shifts > 0; shifts--) PutBit(! bit);
}

void SoEPacker::EncodeChar(int ch)
{	
	int  sym;
	DWORD range;

	sym = char_to_sym[ch];
	range = high - low;
	high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
	low +=       (range * sym_cum[sym    ]) / sym_cum[0];
	for ( ; ; )
	{	if (high <= Q2) Output(0);
		else if (low >= Q2)
		{	Output(1);
			low -= Q2;
			high -= Q2;
		} else if (low >= Q1 && high <= Q3)
		{	shifts++;
			low -= Q1;
			high -= Q1;
		} else break;
		low += low;
		high += high;
	}
	UpdateModel(sym);
}

void SoEPacker::EncodePosition(int position)
{	
	DWORD range;

	range = high - low;
	high = low + (range * position_cum[position    ]) / position_cum[0];
	low +=       (range * position_cum[position + 1]) / position_cum[0];
	for ( ; ; )
	{	if (high <= Q2) Output(0);
		else if (low >= Q2)
		{	Output(1);
			low-=Q2;
			high-=Q2;
		} else if (low >= Q1 && high <= Q3)
		{	shifts++;
			low-=Q1;
			high-=Q1;
		} else break;
		low+=low;
		high+=high;
	}
}

void SoEPacker::EncodeEnd(void)
{	
	shifts++;
	if (low < Q1) Output(0);  else Output(1);
	FlushBitBuffer();
}

BOOL SoEPacker::Encode(void)
{	
	int  i, c, len, r, s, last_match_length;

	Init( 1 );

	StartModel();
	InitTree();
	s = 0;  r = N - F;
	for (i = s; i < r; i++) text_buf[i] = ' ';
	for (len = 0; len < F && (c = SpecRead()) != EOF; len++)
		text_buf[r + len] = c;

	for (i = 1; i <= F; i++) InsertNode(r - i);
	InsertNode(r);
	do 
	{	if (match_length > len) match_length = len;
		if (match_length <= THRESHOLD)
		{	match_length = 1;
			EncodeChar(text_buf[r]);
		} 
		else
		{	EncodeChar(255 - THRESHOLD + match_length);
			EncodePosition(match_position - 1);
		}
		last_match_length = match_length;
		for (i = 0; i < last_match_length && (c = SpecRead()) != EOF; i++)
		{	DeleteNode(s);
			text_buf[s] = c;
			if (s < F - 1) text_buf[s + N] = c;
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}

		while (i++ < last_match_length)
		{	DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);
		}
	} while (len > 0);
	EncodeEnd();
	return TRUE;
}