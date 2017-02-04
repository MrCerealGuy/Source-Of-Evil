/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Input.h"

class SoEMouse
{
public:
	SoEMouse(SoEScreen* Scr,SoEInput* Input,const char *Filename);
	~SoEMouse();
	void SetBoundries(int nx1,int nx2,int ny1,int ny2);
	void SetCur(const char *Filename);
	const char* GetCur();
	void Show(SoESurface* SURF);
	bool Refresh(); //returns true if the mouse was moved.
	bool Click(int n); //1-left, 2-middle, 3-right, 4 - any
	long GetX();
	long GetY();
private: 
	void Init();
	char* Cur; //Current state of the cursor
	SoEResource* Pointers;
	SoESurface* TempSurface;
	SoEInput* InputDevice;

	SoEScreen* Screen;
};