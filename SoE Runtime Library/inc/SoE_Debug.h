/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_DEBUG_H
#define SOE_DEBUG_H

#include <assert.h>

#ifdef  NDEBUG

#define SOEASSERT(exp)     ((void)0)

#else

#define SOEASSERT(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

#endif

#endif SOE_DEBUG_H
