/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_STURCTURES_H
#define SOE_STURCTURES_H

// This file is used to hold structure definitions that might be used
// thrroughout the SoE library.

typedef struct
{
    RGBQUAD depth;		// How many bits of precision per channel
    RGBQUAD position;	// At what bit position does the color start
	UINT    bpp;        // Bits Per Pixel
} RGBFORMAT;


#endif SOE_STURCTURES_H
