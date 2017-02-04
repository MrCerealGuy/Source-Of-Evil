/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Surface.h"
#include "SoE_Movie.h"
#include "mmstream.h"
#include "amstream.h"
#include "ddstream.h"

SoEMovie::SoEMovie(SoEScreen* pScreen, bool bFullscreen = true)
{
	m_pScreen     = pScreen;
	m_bFullscreen = bFullscreen;
}

SoEMovie::~SoEMovie()
{
		Destroy();
}

bool SoEMovie::Create(char *szFilename, SoESurface *pSurface)
{
	CoInitialize( NULL );		// Initialize COM
	m_pSurface = pSurface;

	return OpenMMStream(szFilename);
}

bool SoEMovie::Play()
{
	HRESULT rval;

   rval = m_pMMStream->GetMediaStream(MSPID_PrimaryVideo, &m_pPrimaryVidStream);
	if(FAILED(rval)) return false;

   rval = m_pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&m_pDDStream);
	if(FAILED(rval)) return false;

   rval = m_pDDStream->CreateSample(NULL, NULL, 0, &m_pDDStreamSample);
	if(FAILED(rval)) return false;

   rval = m_pDDStreamSample->GetSurface(&m_pDDSurface, &m_rectClipping);
	if(FAILED(rval)) return false;
   rval = m_pMMStream->SetState(STREAMSTATE_RUN);
	if(FAILED(rval)) return false;

	return true;
}

bool SoEMovie::Update()
{
	HRESULT rval;

	if (m_pDDStreamSample->Update(0, NULL, NULL, 0) != S_OK)
	{
	    return false;
	}
	else
	{

		rval = m_pSurface->m_lpDDS->Blt((m_bFullscreen) ? NULL : &m_rectClipping,//NULL, // Destination rect, NULL = whole surface
		#ifndef NTDX3
										 (IDirectDrawSurface7*)
		#endif
										 m_pDDSurface,
										 &m_rectClipping,
										 DDBLT_WAIT,
										 NULL);

		if(FAILED(rval)) return false;

		return true;
    }
}

bool SoEMovie::Destroy()
{
	RELEASE(m_pMMStream);
	RELEASE(m_pPrimaryVidStream);
	RELEASE(m_pDDStream);
	RELEASE(m_pDDSurface);
	RELEASE(m_pDDStreamSample);

	CoUninitialize();

	return true;
}

bool SoEMovie::Stop()
{
	HRESULT rval;

	rval = m_pMMStream->SetState(STREAMSTATE_STOP);

	if(FAILED(rval)) return false;

	return true;
}

bool SoEMovie::OpenMMStream(char* szFilename)
{
	HRESULT					rval;
   IAMMultiMediaStream*	pAMStream;
   WCHAR	   				wPath[MAX_PATH];


	// Create COM object
	rval = CoCreateInstance(CLSID_AMMultiMediaStream,
									NULL,
									CLSCTX_INPROC_SERVER,
			  						IID_IAMMultiMediaStream,
									(void **)&pAMStream
									);

	if(FAILED(rval)) return false;

	rval = pAMStream->Initialize(STREAMTYPE_READ, 0, NULL);
	if(FAILED(rval)) return false;

	rval = pAMStream->AddMediaStream(m_pScreen->GetDD(), &MSPID_PrimaryVideo, 0, NULL);
	if(FAILED(rval)) return false;

	rval = pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL);
	if(FAILED(rval)) return false;

   if(0 == MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wPath, sizeof(wPath)/sizeof(wPath[0])))
	{
		return false;
	}

   rval = pAMStream->OpenFile(wPath, 0);
	if(FAILED(rval)) return false;

   m_pMMStream = pAMStream;

	// These two lines are unnecessary? Looks like that to me
   pAMStream->AddRef();
	RELEASE(pAMStream);

	return true;
}