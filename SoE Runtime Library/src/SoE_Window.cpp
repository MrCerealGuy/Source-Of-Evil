/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE_Window.h"

HWND SoE_CreateWindow(LPCTSTR lpWindowName, int x, int y, int width, int height)
{
	HWND hWnd;
	WNDCLASS WndClass;                                 
	WndClass.style = CS_HREDRAW | CS_VREDRAW;          
		                                               
	WndClass.lpfnWndProc = DefWindowProc;                    
	WndClass.cbClsExtra = 0; 
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = NULL;//g_hInst;                      
	WndClass.hIcon = NULL;						       
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);       
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;                         
	WndClass.lpszClassName = "SOE_WND";              
	RegisterClass(&WndClass);                          

	hWnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		"SOE_WND", 
		lpWindowName, 
		WS_POPUP,  
		x, 
		y, 
		width,  
		height,  
		NULL, 
		NULL, 
		NULL,//g_hInst,
		NULL); 

	if (!hWnd) 
		return (FALSE);
	
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);        

	return (hWnd);
}
