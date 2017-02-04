/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Mouse.h"

SoEMouse::SoEMouse(SoEScreen* Scr,SoEInput* Input,const char *Filename)
{
	Screen=Scr;
	InputDevice=Input;
	InputDevice->SetActiveDevices(1,0,0); //Disables the keyboard. Whatch out!
	Pointers = new SoEResource();
	Pointers->Open(Filename);
	TempSurface=NULL;
	Cur=0;
};
SoEMouse::~SoEMouse() {
	Pointers->Close();
	delete Pointers;
	delete TempSurface;
}

void SoEMouse::Init()
{
	InputDevice->SetMousePos(Screen->GetWidth()%2,Screen->GetHeight()%2);
	SetBoundries(0,0,Screen->GetWidth(),Screen->GetHeight());

}

void SoEMouse::SetBoundries(int nx1,int nx2,int ny1,int ny2) 
{
	InputDevice->SetMouseLimits(nx1,ny1,nx2,ny2);
};
void SoEMouse::SetCur(const char *Filename)
{
	if (Cur!=Filename) {
	if (TempSurface != NULL) delete TempSurface;
	Cur=(char *)Filename;
	TempSurface=new SoESurface;
	TempSurface->Create(Screen,Pointers->GetFile(Cur),SOEMEM_VIDEOONLY);		
	TempSurface->SetColorKey();
	} 
};
const char* SoEMouse::GetCur()
{
	return Cur;
}

long SoEMouse::GetX()
{
	long a;
	InputDevice->GetMousePos(&a,NULL);
	return a;
};
long SoEMouse::GetY()
{
	long a;
	InputDevice->GetMousePos(NULL,&a);
	return a;
};

void SoEMouse::Show(SoESurface* SURF)
{
	TempSurface->DrawTrans(SURF, GetX(),GetY());
};

bool SoEMouse::Refresh()
{
	InputDevice->Update();
	return true;
};		
bool SoEMouse::Click(int n)
{
	//Need help here. returns true if button n (1-left,3-right,4-any) was clicked.
	return (false);
};