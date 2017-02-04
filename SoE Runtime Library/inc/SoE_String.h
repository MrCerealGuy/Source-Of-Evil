/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_STRING_H
#define SOE_STRING_H

#include <windows.h>

char* BSTRtoSTRING(BSTR p);
BSTR STRINGtoBSTR(const char* p);

#endif