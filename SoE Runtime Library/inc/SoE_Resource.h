/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_RESOURCE_H
#define SOE_RESOURCE_H

//////////////////////////////////////////////////////////////////////////////////
// Structure used for each file entry in a resource file.
//////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	long offset;
	char name[13];
} FILEENTRY, *LPFILEENTRY;

//////////////////////////////////////////////////////////////////////////////////
//version 2 resource files
//////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	UINT	Signature;			//signature of the resource file, 'SOER'
	UINT	Version;			//currently version 2
	UINT	NumOfEntries;		//number of resource file entries
} SOERESOURCEFILEHEADER;

typedef UINT RESHANDLE;

class SOEAPI SoEResDirEntry
{
public:
	SoEResDirEntry()	{pos=0; size=0; type=0; id=0;}		//constructor - so new will be cleared

	UINT		pos;					//position in RF of particular file
	UINT		size;					//size of particular file
	UCHAR		type;					//are 256 different types enough?
	UINT		id;						//id of the file, see IDHFILE keyword of ResourceCompiler
};

class SOEAPI SoEResource
{
public:
	SoEResource(void);
	virtual ~SoEResource(void);

	BOOL Open(const char *Filename);
	BOOL OpenV1(const char *Filename);
	void Close(void);
	char* GetFile(const char *Filename);

public:
	LPFILEENTRY m_pFE;			// file entry structure for each file in a resource file
	int m_Infile;				// handle to the resource file
	int m_Outfile;				// handle to the output file
	int m_nFiles;				// number of files in a resource file

//version 2 resource files
	char	*fName;							//name of Resource File
	FILE	*fHandle;						//handle of Resource File
	UINT	version;						//version of resource file
	UINT	NumOfEntries;
	UINT	fSize;							//Resource File Size
	SoEResDirEntry	*Directory;

	FILE*	GetfHandle();
	RESHANDLE GetHandle(UINT id);
	FILE*	Seek(RESHANDLE i);
	UINT GetPos(RESHANDLE i);
	UINT GetSize(RESHANDLE i);
	UCHAR GetType(RESHANDLE i);
};

#endif
