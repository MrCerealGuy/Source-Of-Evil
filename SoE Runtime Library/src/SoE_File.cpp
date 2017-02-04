/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_File.h"

SoEFile::SoEFile()
{
	m_fh = INVALID_HANDLE_VALUE;
}

SoEFile::~SoEFile()
{
	Close();
}

DWORD SoEFile::Position()
{
	if (m_fh == INVALID_HANDLE_VALUE)
		return 0;
	return SetFilePointer(m_fh, 0, NULL, FILE_CURRENT);
}

DWORD SoEFile::Position(LONG lDistanceToMove, DWORD dwMoveMethod)
{
	if (m_fh == INVALID_HANDLE_VALUE)
		return 0;
	return SetFilePointer(m_fh, lDistanceToMove, NULL, dwMoveMethod);
}

DWORD SoEFile::FileSize()
{
	if (m_fh == INVALID_HANDLE_VALUE)
		return 0;
	return GetFileSize(m_fh, NULL);
}

void SoEFile::Close()
{
	if (m_fh != INVALID_HANDLE_VALUE)
		CloseHandle(m_fh);
	m_fh = INVALID_HANDLE_VALUE;
}

void SoEFile::SeekToStart()
{
	if (m_fh == INVALID_HANDLE_VALUE)
		return;
	Position(0, FILE_BEGIN);
}

void SoEFile::SeekToEnd()
{
	if (m_fh == INVALID_HANDLE_VALUE)
		return;
	Position(0, FILE_END);
}

BOOL SoEFile::OpenCommon(const LPSTR FileName, DWORD Access, DWORD Sharing, DWORD CreateDisposition)
{
	Close();
	
	if ((m_fh = CreateFile(FileName, Access, Sharing, NULL, CreateDisposition,
		FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
		return FALSE;
	
	return TRUE;
}

BOOL SoEFile::Open(const LPSTR FileName)
{
	return OpenCommon(FileName, GENERIC_READ | GENERIC_WRITE, 0, OPEN_ALWAYS);
}

BOOL SoEFile::OpenRead(const LPSTR FileName)
{
	return OpenCommon(FileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
}

BOOL SoEFile::OpenWrite(const LPSTR FileName)
{
	return OpenCommon(FileName, GENERIC_WRITE, 0, OPEN_ALWAYS);
}

DWORD SoEFile::Read(void *whence, UINT howmuch)
{
	DWORD dwBytesRead;
	
	// read the file into memory
	if (ReadFile(m_fh, whence, howmuch, &dwBytesRead, NULL))
		return dwBytesRead;
	else
		return 0;
}

UCHAR SoEFile::ReadUCHAR()
{
	UCHAR whence;
	Read(&whence, sizeof(UCHAR));
	return whence;
}

BYTE SoEFile::ReadBYTE()
{
	BYTE whence;
	Read(&whence, sizeof(BYTE));
	return whence;
}

USHORT SoEFile::ReadUSHORT()
{
	USHORT whence;
	Read(&whence, sizeof(USHORT));
	return whence;
}

WORD SoEFile::ReadWORD()
{
	WORD whence;
	Read(&whence, sizeof(WORD));
	return whence;
}

UINT SoEFile::ReadUINT()
{
	UINT whence;
	Read(&whence, sizeof(UINT));
	return whence;
}

DWORD SoEFile::ReadDWORD()
{
	DWORD whence;
	Read(&whence, sizeof(DWORD));
	return whence;
}

INT64 SoEFile::ReadINT64()
{
	INT64 whence;
	Read(&whence, sizeof(INT64));
	return whence;
}

QWORD SoEFile::ReadQWORD()
{
	QWORD whence;
	Read(&whence, sizeof(QWORD));
	return whence;
}

DWORD SoEFile::Write(const void *whence, UINT howmuch)
{
	DWORD dwBytesWrote;
	
	// read the file into memory
	if (WriteFile(m_fh, whence, howmuch, &dwBytesWrote, NULL))
		return dwBytesWrote;
	else
		return 0;
}

DWORD SoEFile::WriteUCHAR(const UCHAR data)
{
	return Write(&data, sizeof(UCHAR));
}

DWORD SoEFile::WriteBYTE(const BYTE data)
{
	return Write(&data, sizeof(BYTE));
}

DWORD SoEFile::WriteUSHORT(const USHORT data)
{
	return Write(&data, sizeof(USHORT));
}

DWORD SoEFile::WriteWORD(const WORD data)
{
	return Write(&data, sizeof(WORD));
}

DWORD SoEFile::WriteUINT(const UINT data)
{
	return Write(&data, sizeof(UINT));
}

DWORD SoEFile::WriteDWORD(const DWORD data)
{
	return Write(&data, sizeof(DWORD));
}

DWORD SoEFile::WriteINT64(const INT64 data)
{
	return Write(&data, sizeof(INT64));
}

DWORD SoEFile::WriteQWORD(const QWORD data)
{
	return Write(&data, sizeof(QWORD));
}
