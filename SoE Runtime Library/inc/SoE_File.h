/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_FILE_H
#define SOE_FILE_H

#ifndef SOE_H
#include "SoE.h"
#endif

#ifndef QWORD
#define QWORD unsigned __int64
#endif

#ifndef INT64
#define INT64 __int64
#endif

class SOEAPI SoEFile
{
public:
	SoEFile();
	virtual ~SoEFile();

	BOOL	Open(const LPSTR FileName);
	BOOL 	OpenRead(const LPSTR FileName);
	BOOL	OpenWrite(const LPSTR FileName);

	void	Close();

	// reading functions
	DWORD	Read(void *whence, UINT howmuch);
	
	UCHAR	ReadUCHAR();
	BYTE	ReadBYTE();

	USHORT	ReadUSHORT();
	WORD	ReadWORD();

	UINT	ReadUINT();
	DWORD	ReadDWORD();

	INT64	ReadINT64();
	QWORD	ReadQWORD();

	// writing functions
	DWORD	Write(const void *whence, UINT howmuch);

	DWORD	WriteUCHAR(const UCHAR data);
	DWORD	WriteBYTE(const BYTE data);

	DWORD	WriteUSHORT(const USHORT data);
	DWORD	WriteWORD(const WORD data);

	DWORD	WriteUINT(const UINT data);
	DWORD	WriteDWORD(const DWORD data);

	DWORD	WriteINT64(const INT64 data);
	DWORD	WriteQWORD(const QWORD data);

	// File pointer functions
	DWORD	Position();
	DWORD	Position(LONG lDistanceToMove, DWORD dwMoveMethod);
	
	void	SeekToStart();
	void	SeekToEnd();

	// information functions
	DWORD	FileSize();

protected:
	BOOL	OpenCommon(const LPSTR FileName, DWORD Access, DWORD Sharing, DWORD CreateDisposition);

private:
	HANDLE	m_fh;
};

#endif SOE_FILE_H
