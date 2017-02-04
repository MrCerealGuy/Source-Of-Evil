/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_String.h"

char* BSTRtoSTRING(BSTR p)
{
	char* bf, *ret;
	int size;
	size = WideCharToMultiByte(CP_ACP, NULL, p, -1, NULL, 0, NULL,
		NULL);
	bf = new char[size];
	WideCharToMultiByte(CP_ACP, NULL, p, -1, bf, size, NULL, NULL);

	if (p != NULL)
	{
		ret = new char[strlen(bf)+1];
		strcpy(ret, bf);
	}
	else ret = NULL;

	return (ret);
}

BSTR STRINGtoBSTR(const char* p)
{
  static WCHAR* wbf;
	BSTR bbf;
	int size;
	size = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
	wbf = new WCHAR[size];
	MultiByteToWideChar(CP_ACP, 0, p, -1, wbf, size);
	bbf = SysAllocString(wbf);
	delete[] wbf;
	return bbf;
}