/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_SOUND_H
#define SOE_SOUND_H

// Fixes the compile problem with DirectX7 SDK
#ifndef DSBCAPS_CTRLDEFAULT
	#define DSBCAPS_CTRLDEFAULT DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY
#endif

// Fixes problem with DX6 SDK
#ifndef DSERR_ACCESSDENIED
	#define DSERR_ACCESSDENIED	E_ACCESSDENIED
#endif

class SOEAPI SoESound
{
public:
	SoESound(void);
	virtual ~SoESound(void);

	HRESULT Create( void *hWnd, BOOL Exclusive=FALSE );
	HRESULT Enable3d( void );
	HRESULT GetCaps( void );
	HRESULT SetFormat( long Frequency, BOOL Bit16=TRUE, BOOL stereo=TRUE );
	HRESULT ContinuousPlay( BOOL Enable=TRUE );
	LPDIRECTSOUND GetDS( void ) { return m_lpDS; }
#if DIRECTDRAW_VERSION >= SOE_DDVER
	LPDIRECTSOUND3DLISTENER Get3dListener( void ) { return m_lp3DListener; }
#endif

	HRESULT Set3dFactors( float DopplerFactor=1.0, float RolloffFactor=1.0, float DistanceFactor=1.0 );
	HRESULT Place3dListener( float posX, float posY, float posZ, float frontX=0.0, float frontY=0.0, float frontZ=1.0, float topX=0.0, float topY=1.0, float topZ=0.0, BOOL setOrientation=FALSE );

	void SetSoundOn( BOOL sw ) { if( m_lpDS!=NULL ) m_SoundOff=sw; }
	BOOL GetSoundOn(void) { return m_SoundOff; };
	HRESULT SetMIDIVolume( BYTE rvol, BYTE lvol );
	HRESULT SetDIGIVolume( BYTE rvol, BYTE lvol );
	HRESULT SetCDVolume( BYTE rvol, BYTE lvol );
	DWORD GetMIDIVolume(void);
	DWORD GetDIGIVolume(void);
	DWORD GetCDVolume(void);

public: // private:
	BOOL m_SoundOff;		// Stores if Sound Playback is on or off
	BOOL m_3dSound;			// Stores if 3d Sound is enabled
	LPDIRECTSOUND m_lpDS;	// The DirectSound Object
	DSCAPS m_DSCaps;		// Direct sound capabilities

#if DIRECTDRAW_VERSION >= SOE_DDVER
	LPDIRECTSOUND3DLISTENER m_lp3DListener;	// The DIRECTSOUND3D Listener
#endif
};

//////////////////////////////////////////////////////////////////////////////////
// A simple wrapper for a DirectSoundBuffer object. It is used to load and play 
// WAVE files for the sound effects in your game.
//////////////////////////////////////////////////////////////////////////////////
class SOEAPI SoESoundBuffer
{
public:
	SoESoundBuffer( void );
	~SoESoundBuffer( void );

	HRESULT EnableStreaming( int size=4096, int events=4 );
	HRESULT Enable3d( void );

	HRESULT Load( SoESound* pDS, const char* Filename, int Num = 1 );
	HRESULT Load( SoESound* pDS, LONG lSize, FILE* fptr, int Num = 1 );
    HRESULT Load( SoESound* pDS, LONG lSize, fstream* fptr, int Num = 1 );
    HRESULT Load( SoESound* pDS, LONG lSize, CHAR* lpCache, int Num = 1 );
	HRESULT LoadFromRes( SoESound* pDS, const char* Filename, int Num = 1 );
	HRESULT Extract( SoESound* pDS, LONG lSize, char *ptr, int Num = 1 );
	HRESULT Play( DWORD dwFlags = 0 );
	HRESULT PlayExtended( DWORD dwFlags=0, int pan=0, int Volume=0, DWORD freq=DSBFREQUENCY_ORIGINAL );
	HRESULT PlayExtended( float x, float y, float z, DWORD dwFlags=0 );
	HRESULT Stop( void );
	void SetVolume( LONG Volume );
	void SetPan( LONG Pan );
	DWORD Playing( void );
	HRESULT SetCone( float x=0, float y=1.0, float z=0, DWORD InsideAngle=DS3D_DEFAULTCONEANGLE, DWORD OutsideAngle=DS3D_DEFAULTCONEANGLE, long OutsideVolume=DS3D_DEFAULTCONEOUTSIDEVOLUME );
	HRESULT Set3dProperties( DWORD Mode=DS3DMODE_NORMAL, float MinDistance=DS3D_DEFAULTMINDISTANCE, float MaxDistance=DS3D_DEFAULTMAXDISTANCE, float VelX=1.0, float VelY=1.0, float VelZ=1.0 );
	HRESULT UpdateStream( BOOL FillAll=FALSE );
	LPDIRECTSOUNDBUFFER* GetDirectSoundBufferObject(void);
	DWORD GetCurrentPlayPosition(void);
	HRESULT SetCurrentPlayPosition(DWORD pos);
	BOOL IsEnded(void) { return m_sStop; };

protected:
	void Release( void );
	HRESULT CreateSoundBuffer( DWORD dwBufSize, PCMWAVEFORMAT *pcmwf, int Num);
	HRESULT ReadData( FILE* fp, DWORD dwSize );
	HRESULT ReadDataFromMemory( char *ptr, DWORD dwSize );
	LPDIRECTSOUNDBUFFER GetFreeBuffer( void );
	HRESULT FillStreamBuffer( int nr );
	HRESULT	ACMDecode( WAVEFORMATEX *wfm, BYTE *srcbuf, DWORD srclen );

private:
	int m_nBuffers;					// The number of sound buffers created
	int m_Current;					// The currently selected buffer
	const char* m_Filename;			// The name of the WAVE file
	SoESound* m_pDS;				// Contains a pointer to the SoESound object
	LPDIRECTSOUNDBUFFER *m_lpDSB;	// The IDirectSoundBuffer object
#if DIRECTDRAW_VERSION >= SOE_DDVER
	LPDIRECTSOUND3DBUFFER *m_lp3dBuffer;
#endif

	BOOL m_Streamed;				// The playback will be streamed	
	BOOL m_sLoop;
	BOOL m_sStop;
	BYTE m_sDone;
	WORD m_sSize;					// size of each stream block
	BYTE m_sBlocks, m_sCurrent;		// Stream Block Count and Current position
	DWORD m_sWaveLength;
	FILE *m_sFile;					// Filepointer for Streaming
	BOOL m_3d;						// Enables 3d functions for this Buffer
};

#endif
