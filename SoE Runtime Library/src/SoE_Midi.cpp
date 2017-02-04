/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_Midi.h"

SoEMidi::SoEMidi(void)
{
    m_hWnd       = NULL;
    m_ActiveMidi = -1;
	
	for (int i = 0; i < SOE_NUM_SEGMENTS; i++)
	{
#ifdef NTDX3
        m_Midi[i].fileName = NULL;
#else
        m_Midi[i].m_segment   = NULL;  
		m_Midi[i].m_segstate  = NULL;  
#endif
		m_Midi[i].id       = i;
		m_Midi[i].state    = MIDI_NULL;
	}	
}

SoEMidi::~SoEMidi(void)
{
	Shutdown();
}

HRESULT SoEMidi::Init(HWND hWnd, SoESound *pSoEDS)
{
	m_hWnd       = hWnd;
	
#ifndef NTDX3
	
	BOOL UseSoESound;
	
	// check for SoESound...
	if (pSoEDS != NULL)
	{
		if (pSoEDS->GetDS() != NULL)
		{
			UseSoESound = TRUE;
		}
		else
			return -1;
	}
	else
		UseSoESound = FALSE;
	
	// set up directmusic
	// initialize COM
	if (FAILED(CoInitialize(NULL)))
	{
		return -2;
	}
	
	// create the performance
	if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
		NULL,
		CLSCTX_INPROC,
		IID_IDirectMusicPerformance,
		(void**)&m_Performance)))    
	{       
		return -3;
	}
	
	if (!UseSoESound)
	{
		if (FAILED(m_Performance->Init(NULL, NULL, NULL)))
		{
			Shutdown();
			return -4;
		}
	}
	else
	{
		if (FAILED(m_Performance->Init(NULL, pSoEDS->GetDS(), NULL)))
		{
			Shutdown();
			return -5;
		}
	}
	
	
	// add the port to the performance
	if (FAILED(m_Performance->AddPort(NULL)))
	{    
		return -6;
	}
	
	// create the loader to load midi files
	if (FAILED(CoCreateInstance(
		CLSID_DirectMusicLoader,
		NULL,
		CLSCTX_INPROC, 
		IID_IDirectMusicLoader,
		(void**)&m_Loader)))
	{
		return -7;
	}
#endif
	
	return 1;
}

HRESULT SoEMidi::Shutdown()
{
    HRESULT rval;
	
#ifdef NTDX3
	
	if ((rval = mciSendString("close all", NULL, 0, NULL)) != 0)
		return -1;
	
	rval = RemoveAll();
	
#else
	
    // If there is any music playing, stop it.
	if (m_Performance)
		m_Performance->Stop(NULL, NULL, 0, 0); 
	
	// delete all the midis if they already haven't been
	rval = RemoveAll();
	
	// CloseDown and Release the performance object.    
	if (m_Performance)
	{
		m_Performance->CloseDown();
		m_Performance->Release();  
		m_Performance = NULL;
	}
	
	// Release the loader object.
	if (m_Loader)
	{
		m_Loader->Release();
		m_Loader = NULL;
	}
	
	// Release COM
	CoUninitialize(); 
	
#endif
	
    return rval;
}

int SoEMidi::LoadMidi(const char *name)
{
    int id = -1;
	int index;
	
#ifdef NTDX3
	
	for (index = 0; index < SOE_NUM_SEGMENTS; index++)
	{
		if (m_Midi[index].state == MIDI_NULL)
		{
			id = index;
			break;
		}
	}
	
	if (id == -1)
		return -1;
	
	
    m_Midi[id].fileName = new char[256];
	strncpy(m_Midi[id].fileName, name, 256);
	m_Midi[id].state = MIDI_LOADED;
	
#else
	
    DMUS_OBJECTDESC ObjDesc; 
	HRESULT hr;
	IDirectMusicSegment* pSegment = NULL;
	
	for (index = 0; index < SOE_NUM_SEGMENTS; index++)
	{
		// is this one open
		if (m_Midi[index].state == MIDI_NULL)
        {
			// validate id, but don't validate object until loaded
			id = index;
			break;
		}
	}
	
	// found good id?
	if (id == -1)
		return -1;
	
	// get current working directory
	char szDir[_MAX_PATH];
	WCHAR wszDir[_MAX_PATH]; 
	
	if (_getcwd(szDir, _MAX_PATH) == NULL)
	{
		return -1;
	}
	
	MULTI_TO_WIDE(wszDir, szDir);
	
	// tell the loader were to look for files
	hr = m_Loader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE);
	
	if (FAILED(hr)) 
	{
		return -1;
	}
	
	// convert filename to wide string
	WCHAR wfilename[_MAX_PATH]; 
	MULTI_TO_WIDE(wfilename, name);
	
	// setup object description
	ObjDesc.dwSize = sizeof(ObjDesc);
	ObjDesc.guidClass = CLSID_DirectMusicSegment;
	wcscpy(ObjDesc.wszFileName, wfilename);
	ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
	
	// load the object and query it for the IDirectMusicSegment interface
	hr = m_Loader->GetObject(&ObjDesc, IID_IDirectMusicSegment, (void**) &pSegment);
	
	if (FAILED(hr))
		return -1;
	
	// ensure that the segment plays as a standard MIDI file
	hr = pSegment->SetParam(GUID_StandardMIDIFile, - 1, 0, 0, (void*)m_Performance);
	
	if (FAILED(hr))
		return -1;
	
	// The next step is to download the instruments. 
	hr = pSegment->SetParam(GUID_Download, -1, 0, 0, (void*)m_Performance);
	
	if (FAILED(hr))
		return -1;
	
	// at this point we have MIDI loaded and a valid object
	
	m_Midi[id].m_segment  = pSegment;
	m_Midi[id].m_segstate = NULL;
	m_Midi[id].state      = MIDI_LOADED;
	
#endif
	
	return id;
}

HRESULT SoEMidi::Play(int MidiID)
{
    HRESULT rval;
	
#ifdef NTDX3
	
    char buffer[256];
	
	if ((m_Midi[MidiID].fileName == NULL) &&(m_Midi[MidiID].state != MIDI_LOADED))
		return -1;
	
	sprintf(buffer, "open %s type sequencer alias MUSIC", m_Midi[MidiID].fileName);
	
	if ((rval = mciSendString("close all", NULL, 0, NULL)) != 0)
		return -2;
	
	
	if ((rval = mciSendString(buffer, NULL, 0, NULL)) != 0)
		return -3;
	
	
	if ((rval = mciSendString("play MUSIC from 0 notify", NULL, 0, m_hWnd)) != 0)
		return -4;
	
	
	m_Midi[MidiID].state = MIDI_PLAYING;
	m_ActiveMidi         = MidiID;
	
#else
	
    if (m_Midi[MidiID].m_segment && m_Midi[MidiID].state != MIDI_NULL)
	{
		// if there is an active midi then stop it
		if (m_ActiveMidi != -1)
			Stop(m_ActiveMidi);
		
		// play segment and force tracking of state variable
		rval = m_Performance->PlaySegment(m_Midi[MidiID].m_segment, 0, 0, &m_Midi[MidiID].m_segstate);
        if (FAILED(rval))
            return rval;
		
		m_Midi[MidiID].state = MIDI_PLAYING;
		
		// set the active midi segment
		m_ActiveMidi = MidiID;
		
        // small bug, without this sometimes it won't run...
		Sleep(100);  
	} 
	else
		rval = -1;
#endif
	
	return rval;
}

HRESULT SoEMidi::Stop(int MidiID)
{
    HRESULT rval;
	
#ifdef NTDX3
	
	if ((rval = mciSendString("close all", NULL, 0, NULL)) != 0)
		return -1;
	
	m_Midi[MidiID].state = MIDI_STOPPED;
	
#else
	
    if (m_Midi[MidiID].m_segment && m_Midi[MidiID].state != MIDI_NULL)
	{
		// play segment and force tracking of state variable
		rval = m_Performance->Stop(m_Midi[MidiID].m_segment, NULL, 0, 0);
        if (FAILED(rval))
            return rval;
		
		m_Midi[MidiID].state = MIDI_STOPPED;
	}
	else
		rval = -1;
#endif
	
    // reset active id
    m_ActiveMidi = -1;
	
	return rval;
}

HRESULT SoEMidi::Remove(int MidiID)
{
    HRESULT rval;
	
#ifdef NTDX3
	
	if ((rval = Stop(MidiID)) != 0)
		return rval;
	
	if (m_Midi[MidiID].fileName != NULL)
    {
        delete[] m_Midi[MidiID].fileName;
        m_Midi[MidiID].fileName = NULL;
    }
	
	m_Midi[MidiID].state = MIDI_NULL;
	
#else
	
    if (m_Midi[MidiID].m_segment)
	{
		m_Midi[MidiID].m_segment->SetParam(GUID_Unload, -1, 0, 0, (void*)m_Performance); 
		
		// Release the segment and set to null
		m_Midi[MidiID].m_segment->Release(); 
		m_Midi[MidiID].m_segment  = NULL;
		m_Midi[MidiID].m_segstate = NULL;
		m_Midi[MidiID].state      = MIDI_NULL;
		
		rval = 1;
	}
	else
		rval = -1;
	
#endif
	
	return rval;
}

HRESULT SoEMidi::RemoveAll()
{
    HRESULT rval;
	
	for (int i = 0; i < SOE_NUM_SEGMENTS; i++)
	{
		if ((rval = Remove(i)) != 0)
			return rval;
	}
	
	return rval;
}

BOOL SoEMidi::IsPlaying(int MidiID)
{
    BOOL rval;
	
#ifndef NTDX3
	
    if (m_Midi[MidiID].m_segment && m_Midi[MidiID].state !=MIDI_NULL)
	{
		// get the status and translate to our defines
		if (m_Performance->IsPlaying(m_Midi[MidiID].m_segment, NULL) == S_OK) 
			m_Midi[MidiID].state = MIDI_PLAYING;
		else
			m_Midi[MidiID].state = MIDI_STOPPED;
		
		if (m_Midi[MidiID].state == MIDI_PLAYING)
			rval = TRUE;
		else
			rval = FALSE;
	} // end if
	else
		rval = FALSE;
	
#else
	
	if (m_Midi[MidiID].state == MIDI_PLAYING)
		rval = TRUE;
	else
		rval = FALSE;
	
#endif
	
    return rval;
}

HRESULT SoEMidi::SetVolume(long Vol)
{
#ifndef NTDX3
	
    HRESULT rval;
	
    if (m_Performance)
	{
		rval = m_Performance->SetGlobalParam(GUID_PerfMasterVolume, &Vol, sizeof(Vol));
		if (FAILED(rval))
			return rval;
	}
	
	return rval;
	
#endif
	
    return 1;
}

LONG SoEMidi::GetVolume()
{
#ifndef NTDX3
	
    long ret;
    HRESULT rval;
	
	if (m_Performance)
	{
		rval = m_Performance->GetGlobalParam(GUID_PerfMasterVolume, &ret, sizeof(ret));
		if (FAILED(rval))
			return 0;
	}
	
	return ret;
	
#endif
	
	return 0;
}
