/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_MUSIC_H
#define SOE_MUSIC_H

class SOEAPI SoEMusic
{
public:
	SoEMusic(void *hWnd);
	virtual ~SoEMusic();

	BOOL Play(const char *Filename);
	BOOL Stop(void);
	BOOL Pause(void);
	BOOL Resume(void);
	BOOL Restart(void);

public:
	HWND m_hWnd;				// Window handle
};

class SOEAPI SoEMusicCd
{
public:
	SoEMusicCd(void);
	~SoEMusicCd(void);

	short	Read(void);
	short	GetNumberOfTracks(void)	{ return m_nNumberOfTracks; }
	void  SetNumberOfTracks(short nTracks) { m_nNumberOfTracks = nTracks; }
	short	GetTrackLength(short nTrack);
	void  SetTrackLength(short nTrack, short nNewLength);
	short	GetTotalLength(void);
	void  Play(short nTrack);
	void  Stop(void);

private:
	short	m_nNumberOfTracks;
	short	m_nTrackLength[100];
	MCI_STATUS_PARMS m_MCIStatus;
	MCI_OPEN_PARMS m_MCIOpen;
};

#endif
