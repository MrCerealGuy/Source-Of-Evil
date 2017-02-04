/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_TYPES_H
#define SOE_TYPES_H

/////////////////////////////////////////////////////////////////////
// signed types
/////////////////////////////////////////////////////////////////////
#undef INT8
#define INT8 signed char

#undef INT16
#define INT16 signed short

#undef INT32
#define INT32 signed long

#undef INT64
#define INT64 __int64


/////////////////////////////////////////////////////////////////////
// unsigned types
/////////////////////////////////////////////////////////////////////
#undef UINT8
#define UINT8 unsigned char

#undef UINT16
#define UINT16 unsigned short

#undef UINT32
#define UINT32 unsigned long

#undef UINT64
#define UINT64 unsigned __int64


#undef SOEBOOL
#define SOEBOOL int

#endif SOE_TYPES_H