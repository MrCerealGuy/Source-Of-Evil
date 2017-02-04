/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include <windows.h>

class SoESurface;
class SoEScreen;

struct IMultiMediaStream; 
struct IMediaStream;
struct IDirectDrawMediaStream;
struct IDirectDrawStreamSample;
struct IDirectDrawSurface;

class SoEMovie  
{

public:
	SoEMovie(SoEScreen* pScreen, bool bFullscreen);
	virtual ~SoEMovie();

	bool Create(char* szFilename, SoESurface* pSurface);
	bool Play();
	bool Stop();
	bool Destroy();
	bool Update();

private:
	bool OpenMMStream(char* szFilename);

private:
	SoEScreen*						m_pScreen;
	SoESurface*						m_pSurface;
	IMultiMediaStream*			m_pMMStream;
	IMediaStream*					m_pPrimaryVidStream;
	IDirectDrawMediaStream*		m_pDDStream;
   IDirectDrawStreamSample*	m_pDDStreamSample;
   IDirectDrawSurface*			m_pDDSurface;
	RECT								m_rectClipping;
	bool                            m_bFullscreen;
};






