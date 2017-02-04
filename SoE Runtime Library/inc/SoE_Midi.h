/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_MIDI_H
#define SOE_MIDI_H

#ifdef NTDX3

typedef struct SOE_MIDI_TYP
{
	char*	fileName;
	int		id;	              // ID of where the file is in the array
	int		state;            // current state of the midi file
} SOE_MIDI, *SOE_MIDI_PTR;

#else

#include <wchar.h>
#include <direct.h>

#include "..\..\SoE DirectX\Inc\Dsound.h"
#include "..\..\SoE DirectX\Inc\Dmksctrl.h"
#include "..\..\SoE DirectX\Inc\Dmusici.h"
#include "..\..\SoE DirectX\Inc\Dmusicc.h"
#include "..\..\SoE DirectX\Inc\Dmusicf.h"

#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP,MB_PRECOMPOSED, y,-1,x,_MAX_PATH);

typedef struct SOE_MIDI_TYP
{
	IDirectMusicSegment			*m_segment;
	IDirectMusicSegmentState	*m_segstate;
	int							id;				// ID of where the file is in the array
	int							state;			// current state of the midi file
} SOE_MIDI, *SOE_MIDI_PTR;

#endif

#include "SoE.h"
#include "SoE_Sound.h"

#define SOE_NUM_SEGMENTS 64	  // maximum number of midi files that can be loaded
#define MIDI_NULL        0	  // midi has not been loaded
#define MIDI_LOADED      1	  // midi has been loaded, but is not active
#define MIDI_PLAYING     2	  // midi is currently playing
#define MIDI_STOPPED     3	  // midi is current midi, but not playing

class SoEMidi
{
public:
    SoEMidi();
	virtual ~SoEMidi();

	HRESULT Init(HWND hWnd, SoESound* pSoEDS = NULL);
	HRESULT Shutdown();                                // called in ~SoEMidi

	int     LoadMidi(const char * name);
	HRESULT Play(int MidiID);
	HRESULT Stop(int MidiID);
	HRESULT Remove(int MidiID);
	HRESULT RemoveAll();
	BOOL    IsPlaying(int MidiID);
	HRESULT SetVolume(long Vol);
	long    GetVolume();

	SOE_MIDI                 m_Midi[SOE_NUM_SEGMENTS]; // array of midi's and data
	int                      m_ActiveMidi;             // current id of active midi
	HWND                     m_hWnd;
#ifndef NTDX3
    IDirectMusicPerformance* m_Performance;
	IDirectMusicLoader*      m_Loader;
#endif
};

#endif SOE_MIDI_H