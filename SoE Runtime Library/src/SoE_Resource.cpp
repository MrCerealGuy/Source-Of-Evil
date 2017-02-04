/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Resource.h"
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif  // _MSC_VER

#define BLOCK_SIZE 16384

SoEResource::SoEResource(void)
{
	m_pFE = NULL;
	m_Infile = 0;
	m_Outfile = 0;
	m_nFiles = 0;
	version = 0;
	Directory = NULL;
	fName = NULL;
	fHandle = NULL;
}

SoEResource::~SoEResource(void)
{
	if (version == 1)
	{
		DeleteFile("SOE.TMP");
		if (m_Infile)
			close(m_Infile);
		if (m_Outfile)
			close(m_Outfile);
	}
	else
	{
		SAFEDELETE(Directory);
		SAFEDELETE(fName);
		if (fHandle != NULL)
		{
			fclose(fHandle);
			fHandle = NULL;
		}
	}
}

BOOL SoEResource::Open(const char *Filename)
{
	SOERESOURCEFILEHEADER	rh;
	
	if (NULL == (fHandle = fopen(Filename, "rb")))
		return false;
	if (1 !=(fread(&rh, sizeof(rh), 1, fHandle)))
		return false;
	if (rh.Signature != 'SOER')
	{
		// compatibility mode. old soeresource haven't used signature or versioning at all...
		version = 1;
		fclose(fHandle);
		return (OpenV1(Filename));
	}
	
	// resource file > version 1. (1=old style)
	
	version = rh.Version;
	fName = strdup(Filename);
	NumOfEntries = rh.NumOfEntries;
	
	Directory = new SoEResDirEntry[NumOfEntries];
	for (UINT i = 0; i < NumOfEntries; i++)
	{
		fread(&Directory[i].pos, sizeof(Directory[i].pos), 1, fHandle);
		fread(&Directory[i].size, sizeof(Directory[i].size), 1, fHandle);
		fread(&Directory[i].type, sizeof(Directory[i].type), 1, fHandle);
		fread(&Directory[i].id, sizeof(Directory[i].id), 1, fHandle);
	}
	// fread(Directory, NumOfEntries * sizeof(SoEResDirEntry), 1, fHandle);
	
	fseek(fHandle, 0, SEEK_END);
	fSize = ftell(fHandle);
	rewind(fHandle);
	
	return true;
}

BOOL SoEResource::OpenV1(const char *Filename)
{
	int bytes;
	
	// Open the resource file
	m_Infile = open(Filename, O_RDONLY | O_BINARY);
	if (m_Infile < 1)
		return FALSE;
	
	// Read the number of files in the resource
	bytes = read(m_Infile, &m_nFiles, sizeof(int));
	if (bytes != sizeof(int))
	{
		Close();
		return FALSE;
	}
	
	// Make space for the file header
	m_pFE = new FILEENTRY[m_nFiles];
	if (m_pFE == NULL)
	{
		Close();
		return FALSE;
	}
	
	// Read the header file
	bytes = read(m_Infile, m_pFE, m_nFiles * sizeof(FILEENTRY));
	if (bytes !=(int)(m_nFiles * sizeof(FILEENTRY)))
	{
		Close();
		return FALSE;
	}
	
	return TRUE;
}

void SoEResource::Close(void)
{
	if (version == 1)
	{
		DeleteFile("SOE.TMP");
		if (m_Infile)
			close(m_Infile);
		if (m_Outfile)
			close(m_Outfile);
	}
	else
	{
		SAFEDELETE(Directory);
		SAFEDELETE(fName);
		if (fHandle != NULL)
		{
			fclose(fHandle);
			fHandle = NULL;
		}
	}
}

char* SoEResource::GetFile(const char *Filename)
{
	char *buffer;
	int i;
	
	DeleteFile("SOE.TMP");
	
	for (i = 0; i < m_nFiles; i++)
	{
		if (strnicmp(Filename, m_pFE[i].name, 13) == 0)
			break;
	}
	
	long offset = m_pFE[i].offset;
	lseek(m_Infile, offset, SEEK_SET);
	
	m_Outfile = open("SOE.TMP", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
	if (m_Outfile < 1)
		return NULL;
	
	int size = m_pFE[i + 1].offset - offset;
	
	// Get space for the buffer
	buffer = new char[size];
	if (buffer == NULL)
		return NULL;
	
	read(m_Infile, buffer, size);
	write(m_Outfile, buffer, size);
	
	close(m_Outfile);
	
	return (char*)"SOE.TMP";
}

FILE* SoEResource::GetfHandle()
{
	return fHandle;
}

RESHANDLE SoEResource::GetHandle(UINT id)
{
	for (UINT i = 0; i < NumOfEntries; i++)
	{
		if (Directory[i].id == id)
			return i;
	}
#ifdef _MSC_VER
	_ASSERT(FALSE);
#endif
	return -1;		// no such id
}

FILE* SoEResource::Seek(RESHANDLE i)
{
	fseek(fHandle, Directory[i].pos, SEEK_SET);
	return fHandle;
}

UINT SoEResource::GetPos(RESHANDLE i)
{
	return Directory[i].pos;
}

UINT SoEResource::GetSize(RESHANDLE i)
{
	return Directory[i].size;
}

UCHAR SoEResource::GetType(RESHANDLE i)
{
	return Directory[i].type;
}

