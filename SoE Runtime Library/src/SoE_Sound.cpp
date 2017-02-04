/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Sound.h"

// Makes the mixed LONG value out of the Left and Right channels...
#define MAKEVOL(right,left) MAKELONG(((right*65535L)/100), ((left*65535L)/100))

SoESound::SoESound()
{
	m_SoundOff = TRUE;
	m_3dSound = FALSE;
	m_lpDS = NULL;
#if DIRECTDRAW_VERSION >= SOE_DDVER
	m_lp3DListener = NULL;
#endif
}

SoESound::~SoESound()
{
	RELEASE(m_lpDS);
}

HRESULT SoESound::Create(void *hWnd, BOOL Exclusive)
{
	HRESULT rval;
	
	rval = DirectSoundCreate(NULL, &m_lpDS, NULL);
	if (rval != DS_OK)
		return rval;
	
	rval = m_lpDS->SetCooperativeLevel((HWND)hWnd, (Exclusive) ? DSSCL_EXCLUSIVE : DSSCL_NORMAL);
	if (rval != DS_OK)
		return rval;
	
	m_SoundOff = FALSE;
	if (Exclusive)
		return SetFormat(22050, TRUE, TRUE);
	
	return DS_OK;
}

HRESULT SoESound::GetCaps(void)
{
	if (m_lpDS == NULL)
		return DSERR_UNINITIALIZED;
	return m_lpDS->GetCaps(&m_DSCaps);
}

HRESULT SoESound::SetFormat(long Frequency, BOOL Bit16, BOOL stereo)
{
	if (m_lpDS == NULL)
		return DSERR_UNINITIALIZED;
	
	LPDIRECTSOUNDBUFFER lpDSB;
    DSBUFFERDESC dsbdesc;
	WAVEFORMATEX wfx;
	HRESULT rval;
	
	// Get primary SoundBuffer
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
	rval = m_lpDS->CreateSoundBuffer(&dsbdesc, &lpDSB, NULL); 
	if (FAILED(rval))
		return rval;
	
	// Set the desired Format
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = Frequency;
	wfx.wBitsPerSample = (Bit16) ? 16 : 8;
	wfx.nChannels = (stereo) ? 2 : 1;
	wfx.nBlockAlign = wfx.wBitsPerSample/8*wfx.nChannels;
	wfx.nAvgBytesPerSec = Frequency*wfx.nBlockAlign;
	rval = lpDSB->SetFormat(&wfx);
	
	RELEASE(lpDSB);
	return rval;
}

HRESULT SoESound::ContinuousPlay(BOOL Enable)
{
	if (m_lpDS == NULL)
		return DSERR_UNINITIALIZED;
	
	LPDIRECTSOUNDBUFFER lpDSB;
    DSBUFFERDESC dsbdesc;
	HRESULT rval;
	
	// Get primary SoundBuffer
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
	rval = m_lpDS->CreateSoundBuffer(&dsbdesc, &lpDSB, NULL);
	if (FAILED(rval))
		return rval;
	
	if (Enable)
		rval = lpDSB->Play(0, 0, DSBPLAY_LOOPING);
	else 
		rval = lpDSB->Stop();
	RELEASE(lpDSB);
	
	return rval;
}

HRESULT SoESound::Enable3d(void)
{
	if (m_lpDS == NULL)
		return DSERR_UNINITIALIZED;
	
#if DIRECTDRAW_VERSION >= SOE_DDVER
	LPDIRECTSOUNDBUFFER lpDSB;
    DSBUFFERDESC dsbdesc;
	HRESULT rval;
	
	// Get primary SoundBuffer
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER; 
	rval = m_lpDS->CreateSoundBuffer(&dsbdesc, &lpDSB, NULL);
	if (FAILED(rval))
		return rval;
	
	rval = lpDSB->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&m_lp3DListener);
	if (FAILED(rval))
		return rval;
	RELEASE(lpDSB);
	
	m_3dSound = TRUE;
	return DS_OK;
#else
	return DSERR_UNSUPPORTED;
#endif
}

HRESULT SoESound::Set3dFactors(float DopplerFactor, float RolloffFactor, float DistanceFactor)
{
#if DIRECTDRAW_VERSION >= SOE_DDVER
	HRESULT rval;
	if (m_lp3DListener == NULL)
		return DSERR_UNINITIALIZED;
	
	rval = m_lp3DListener->SetDopplerFactor(DopplerFactor, DS3D_DEFERRED);
	if (FAILED(rval))
		return rval;
	
	rval = m_lp3DListener->SetRolloffFactor(RolloffFactor, DS3D_DEFERRED);
	if (FAILED(rval))
		return rval;
	
	rval = m_lp3DListener->SetDistanceFactor(DistanceFactor, DS3D_DEFERRED);
	if (FAILED(rval))
		return rval;
	
	return m_lp3DListener->CommitDeferredSettings();
#else
	return DSERR_UNSUPPORTED;
#endif
}

HRESULT SoESound::Place3dListener(float posX, float posY, float posZ, float frontX, float frontY, float frontZ, float topX, float topY, float topZ, BOOL setOrientation)
{
#if DIRECTDRAW_VERSION >= SOE_DDVER
	HRESULT rval;
	if (m_lp3DListener == NULL)
		return DSERR_UNINITIALIZED;
	
	rval = m_lp3DListener->SetPosition(posX, posY, posZ, DS3D_DEFERRED); // DS3D_IMMEDIATE
	if (FAILED(rval))
		return rval;
	
	if (setOrientation)
	{
		rval = m_lp3DListener->SetOrientation(frontX, frontY, frontZ, topX, topY, topZ, DS3D_DEFERRED);
		if (FAILED(rval))
			return rval;
	}
	
	return m_lp3DListener->CommitDeferredSettings();
#else
	return DSERR_UNSUPPORTED;
#endif
}

HRESULT SoESound::SetMIDIVolume(BYTE rvol, BYTE lvol)
{
	MIDIOUTCAPS midiCaps;
	if (FAILED(midiOutGetDevCaps(0, &midiCaps, sizeof(midiCaps)))) 
		return DSERR_NODRIVER;
	
	if (midiCaps.dwSupport&MIDICAPS_VOLUME)
		return midiOutSetVolume(0, MAKEVOL(rvol, lvol));
	
	return DSERR_UNSUPPORTED;
}

HRESULT SoESound::SetDIGIVolume(BYTE rvol, BYTE lvol)
{
	WAVEOUTCAPS waveCaps;
	if (FAILED(waveOutGetDevCaps(0, &waveCaps, sizeof(waveCaps))))
		return DSERR_NODRIVER;
	
	if (waveCaps.dwSupport&WAVECAPS_VOLUME)
		return waveOutSetVolume(0, MAKEVOL(rvol, lvol));
	
	return DSERR_UNSUPPORTED;
}

HRESULT SoESound::SetCDVolume(BYTE rvol, BYTE lvol)
{
	AUXCAPS auxCaps;
	if (FAILED(auxGetDevCaps(0, &auxCaps, sizeof(auxCaps))))
		return DSERR_NODRIVER;
	
	if (auxCaps.dwSupport&AUXCAPS_VOLUME)
		return auxSetVolume(0, MAKEVOL(rvol, lvol));
	
	return DSERR_UNSUPPORTED;
}

DWORD SoESound::GetMIDIVolume(void)
{
	DWORD vol;
	midiOutGetVolume(0, &vol);
	return vol;
}

DWORD SoESound::GetDIGIVolume(void)
{
	DWORD vol;
	waveOutGetVolume(0, &vol);
	return vol;
}

DWORD SoESound::GetCDVolume(void)
{
	DWORD vol;
	auxGetVolume(0, &vol);
	return vol;
}