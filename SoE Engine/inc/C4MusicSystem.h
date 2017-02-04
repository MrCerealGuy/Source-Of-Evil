/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Handles Music.c4g and randomly plays songs */

class C4MusicSystem  
{
public:
	C4MusicSystem();
	~C4MusicSystem();

protected:
	C4Group MusicFile;
	C4Group *pMusicFile;
	BOOL fPlaying;

public:
	void Default();
	void Clear();
	void Execute();
	void NotifySuccess();
	BOOL Init();
	BOOL Play(const char *szSongname = NULL);
	BOOL Stop();
};
