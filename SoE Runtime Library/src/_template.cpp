/*********************************************************************

  Project Name : 
  Programmer   : 

 ********************************************************************/


// --- Includes ------------------------------------------------------

#define WIN32_LEAN_AND_MEAN                // Required for windows programs
#include <windows.h>                       // Required for windows programs
#define SOEINCLUDEALL                      // This define includes all headers required by SoE
#include <SoE.h>                           // Header for SoE RunLib


#undef  ERROR
#define ERROR(x) MessageBox(NULL, x, "ERROR", MB_OK)

char szAppName[]   = "YourAppName";   
char szClassName[] = "YourAppName";
HINSTANCE g_hInst;                     
HWND g_hWnd; 
BOOL bActive;  

SoEScreen  *Screen;     

//--- Init -----------------------------------------------------------
//
// Function: Handles initialization of the SoE objects
// 
//--------------------------------------------------------------------

BOOL Init() 
{	
	Screen = new SoEScreen();
	
	if (Screen == NULL) 
		return false;   	
	
	if (FAILED(Screen->CreateFullScreen(g_hWnd, 800, 600, 32))) 
		return false;

	return true; 
}

//--- DeInit -----------------------------------------------------------
//
// Function: Handles de-initialization of the SoE objects
// 
//--------------------------------------------------------------------

void DeInit()
{	

}

//--- DoFrame ----------------------------------------------------
//
// Function: Performs drawing of the current frame 
//
//--------------------------------------------------------------------

void DoFrame() 
{ 
    if (!bActive)
		return;

	if (Screen == NULL)
		return;

	Screen->GetBack()->Fill(0); 

	/* TODO: Drawing operations */

	Screen->Flip();	
}

//--- WinProc --------------------------------------------------------
//
// Function: Handles application messages
//
//--------------------------------------------------------------------

static long PASCAL WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message)
	{
		case WM_COMMAND:			
		break;

		case WM_KEYDOWN:          
		{
			switch(wParam) 
			{          
				case VK_ESCAPE:   						
					SendMessage(hWnd, WM_CLOSE, 0, 0); 
					PostQuitMessage(0);
				break; 

				case VK_F1:						
				break;
			}
		}
		return 0;

		case WM_ACTIVATEAPP:
			bActive = wParam;
		break;

		case WM_CLOSE: 
		case WM_DESTROY: 
		break;

		case WM_MOUSEMOVE:
			// x-Koord = LOWORD(lParam);
			// y-Koord = HIWORD(lParam);
		break;
		

		case WM_LBUTTONDOWN:
		break;

		case WM_RBUTTONDOWN:
		break;

		case WM_SIZE:
			if (wParam == SIZE_MAXIMIZED)						
				bActive = true;
			
			if (wParam == SIZE_MINIMIZED)			
				bActive = false;			
		break;
				
		default: 
			return DefWindowProc(hWnd, message, wParam, lParam); 	
    }

	return 0;
}

//--- InitApp --------------------------------------------------------
//
// Function: Creates the window
// 
//--------------------------------------------------------------------

static BOOL InitApp(int nCmdShow) 
{ 
	WNDCLASS WndClass;                                 
	WndClass.style = CS_HREDRAW | CS_VREDRAW;          
		                                               
	WndClass.lpfnWndProc = WinProc;                    
	WndClass.cbClsExtra = 0; 
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = g_hInst;                      
	WndClass.hIcon = NULL;						       
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);       
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;                         
	WndClass.lpszClassName = szClassName;              
	RegisterClass(&WndClass);                          

	g_hWnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		szClassName, 
		szAppName, 
		WS_POPUP,  
		0, 
		0, 
		GetSystemMetrics(SM_CXFULLSCREEN),  
		GetSystemMetrics(SM_CYFULLSCREEN),  
		NULL, 
		NULL, 
		g_hInst,
		NULL); 


	if (!g_hWnd) return FALSE;
	
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);        

	return TRUE;
} 

//--- WinMain --------------------------------------------------------
//
// Function: Inital function called by windows
//
//--------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) 
{	 
	g_hInst = hInstance;
	MSG msg;

	if (!InitApp(nCmdShow)) 
		return -1; 

	if (!Init()) 
	{        
		PostQuitMessage(0);
		return -1;
	} 

	while (1) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
		{
			if (!GetMessage(&msg, NULL, 0, 0 )) 
				return msg.wParam;
			
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
		else 
		{
			DoFrame();     
		}
	}

	return 0;
}