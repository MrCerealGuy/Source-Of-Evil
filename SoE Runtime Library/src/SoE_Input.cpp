/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Input.h"

static LPDIRECTINPUTDEVICE lpDID = NULL;

BOOL FAR PASCAL EnumJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
	LPDIRECTINPUT pDI = (LPDIRECTINPUT)pvRef;
	
    // I think that if there is more then one joystick we will leak memory and
    // only find the last joystick returned ???
	if (FAILED(pDI->CreateDevice(pdinst->guidInstance, &lpDID, NULL)))
		return DIENUM_CONTINUE;
	
	return DIENUM_STOP;
}

SoEInput::SoEInput(void)
{
	m_lpDI					= NULL;
	m_lpDIDKeyboard			= NULL;
	m_lpDIDMouse			= NULL;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	m_lpDIDJoystick			= NULL;
#endif
	
	m_bMouse				= FALSE;
	m_bKeyboard				= FALSE;
	m_bJoystick				= FALSE;
    m_JoystickType          = 0;
	m_mouseSensitivity		= 1.0;
	m_joystickSensitivity	= 1.0;
	m_mouseMinX				= 0x80000000;
	m_mouseMinY				= 0x80000000; 
	m_mouseMaxX				= 0x7fffffff;
	m_mouseMaxY				= 0x7fffffff; 
	m_joystickMinX			= 0x80000000;
	m_joystickMinY			= 0x80000000; 
	m_joystickMaxX			= 0x7fffffff;
	m_joystickMaxY			= 0x7fffffff; 
	m_mouseX				= 0;
	m_mouseY				= 0;
	m_mouseFreeX			= 0;
	m_mouseFreeY			= 0;
	m_mouseDeltaX			= 0;
	m_mouseDeltaY			= 0;
	m_joystickX				= 0;
	m_joystickY				= 0;
	m_joystickFreeX			= 0;
	m_joystickFreeY			= 0;
	m_joystickDeltaX		= 0;
	m_joystickDeltaY		= 0;
    m_lastKey               = 0;
    m_mouseThreshold2       = 6;
    m_mouseThreshold1       = 2;
    m_mouseMultiplier2      = 2.0;
    m_mouseMultiplier1      = 1.5;
	
	// wipe our internal key data
	FlushKeyboardData();
}

SoEInput::~SoEInput(void)
{
	// make sure we're initialized
	if (!m_lpDI)
		return;
	
	// shutdown keyboard
	if (m_lpDIDKeyboard)
	{
		UnacquireKeyboard();
		m_lpDIDKeyboard->Release();
		m_lpDIDKeyboard = NULL;
	}
	
	// shutdown mouse
	if (m_lpDIDMouse)
	{
		UnacquireMouse();
		m_lpDIDMouse->Release();
		m_lpDIDMouse = NULL;
	}
	
	// shutdown joystick
#if DIRECTINPUT_VERSION >= SOE_DIVER
	if (m_lpDIDJoystick)
	{
		UnacquireJoystick();
		m_lpDIDJoystick->Release();
		m_lpDIDJoystick = NULL;
	}
#endif
	
	// kill directinput
	m_lpDI->Release();
	m_lpDI = NULL;
}

HRESULT SoEInput::Create(void *hInst, void *hWnd)
{
	HRESULT rval;
	
    m_hWnd = (HWND)hWnd;
	
    // Create the Direct Input object
	rval = DirectInputCreate((HINSTANCE)hInst, DIRECTINPUT_VERSION, &m_lpDI, NULL);
	if (FAILED(rval)) 
        return rval;
	
	// Create the mouse device
	rval = m_lpDI->CreateDevice(GUID_SysMouse, &m_lpDIDMouse, NULL);
	if (SUCCEEDED(rval))
	{
		m_lpDIDMouse->SetDataFormat(&c_dfDIMouse);
		m_lpDIDMouse->SetCooperativeLevel((HWND)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		rval = AcquireMouse();
		if (FAILED(rval)) 
			return rval;
	}
	
	// Create the keyboard device
	rval = m_lpDI->CreateDevice(GUID_SysKeyboard, &m_lpDIDKeyboard, NULL);
	if (SUCCEEDED(rval))
	{
		m_lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
		m_lpDIDKeyboard->SetCooperativeLevel((HWND)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		rval = AcquireKeyboard();
		if (FAILED(rval)) 
			return rval;
		
		// set up DIK_<keys> remap
		InitDIKToSoEKEY();
        InitSoEKEYToDIK();
		InitShiftedKeys();
        InitKeyText();
	}
	
	// Enumerate the joystick device (DirectX 5 or higher)
#if DIRECTINPUT_VERSION >= SOE_DIVER
	rval = m_lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystick, m_lpDI, DIEDFL_ATTACHEDONLY);
	if (lpDID != NULL)
	{
		rval = lpDID->QueryInterface(IID_IDirectInputDevice2, (LPVOID *)&m_lpDIDJoystick);
		if (FAILED(rval)) 
            return rval;
		
		RELEASE(lpDID);
		
		m_lpDIDJoystick->SetDataFormat(&c_dfDIJoystick);
		m_lpDIDJoystick->SetCooperativeLevel((HWND)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		// Set the X-axis range (-1000 to +1000)
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwObj = DIJOFS_X;
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin       = -1000;
		diprg.lMax       = +1000;
		
        rval = m_lpDIDJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
		if (FAILED(rval))
			return rval;
		
		// And again for Y-axis range
		diprg.diph.dwObj = DIJOFS_Y;
		
        rval = m_lpDIDJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
		if (FAILED(rval))
			return rval;
		
		// Set X axis dead zone to 10%
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwObj = DIJOFS_X;
		dipdw.diph.dwHow = DIPH_BYOFFSET;
		dipdw.dwData     = 1000;
		
        rval = m_lpDIDJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		if (FAILED(rval))
			return rval;
		
		dipdw.diph.dwObj = DIJOFS_Y;
		
		// Set Y axis dead zone to 10%
        rval = m_lpDIDJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		if (FAILED(rval))
			return rval;
		
		rval = AcquireJoystick();
		if (FAILED(rval)) 
			return rval;
		
        // get the joystick type
        DIDEVICEINSTANCE devinfo;
		
        ZeroMemory(&devinfo , sizeof(devinfo));
        devinfo.dwSize = sizeof(devinfo);
        m_lpDIDJoystick->GetDeviceInfo(&devinfo);
        m_JoystickType = GET_DIDEVICE_SUBTYPE(devinfo.dwDevType);
    }
#endif
	
	return 0;
}

HRESULT SoEInput::SetActiveDevices(BOOL bMouse, BOOL bKeyboard, BOOL bJoystick)
{
    HRESULT rval;
	
	if (m_bMouse = bMouse)
		rval = AcquireMouse();
	else
		rval = UnacquireMouse();
	
	if (m_bKeyboard = bKeyboard)
		rval = AcquireKeyboard();
	else 
		rval = UnacquireKeyboard();
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	if (m_bJoystick = bJoystick)
		rval = AcquireJoystick();
	else
		rval = UnacquireJoystick();
#endif
	
    return rval;
}

HRESULT SoEInput::SetMouseExculsive(BOOL ExclusiveMouseAccess)
{
	HRESULT rval;
	
	// Unacquire the Mouse
    rval = UnacquireMouse();
	
	if (SUCCEEDED(rval))
	{
		rval = m_lpDIDMouse->SetDataFormat(&c_dfDIMouse);
        if (FAILED(rval)) 
			return rval;
		
		if (ExclusiveMouseAccess == FALSE)
			rval = m_lpDIDMouse->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND);
		else
			rval = m_lpDIDMouse->SetCooperativeLevel(m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		
        if (FAILED(rval)) 
			return rval;
		
		rval = AcquireMouse();
	}
	
    return rval;
}

HRESULT SoEInput::SetKeyboardExculsive(BOOL bExculsiveKeyboardAccess)
{
    HRESULT rval;
	
	// Unacquire the keyboard
    rval = UnacquireKeyboard();
	
	if (SUCCEEDED(rval))
	{
		rval = m_lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(rval)) 
			return rval;
		
		if (bExculsiveKeyboardAccess == FALSE)
			rval = m_lpDIDKeyboard->SetCooperativeLevel((HWND)m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		else
			rval = m_lpDIDKeyboard->SetCooperativeLevel((HWND)m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		
        if (FAILED(rval)) 
			return rval;
		
		rval = AcquireKeyboard();
	}
	
    return rval;
}

HRESULT SoEInput::SetJoystickExculsive(BOOL bExculsiveJoystickAccess)
{
    HRESULT rval = -1;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	// Unacquire the keyboard
    rval = UnacquireJoystick();
	
	if (SUCCEEDED(rval))
	{
		rval = m_lpDIDJoystick->SetDataFormat(&c_dfDIJoystick);
        if (FAILED(rval)) 
			return rval;
		
		if (bExculsiveJoystickAccess == FALSE)
			rval = m_lpDIDJoystick->SetCooperativeLevel((HWND)m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		else
			rval = m_lpDIDJoystick->SetCooperativeLevel((HWND)m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		
        if (FAILED(rval)) 
			return rval;
		
		rval = AcquireJoystick();
	}
#endif
	
    return rval;
}

HRESULT SoEInput::SetMouseAbs(void)
{
    HRESULT rval;
	DIPROPDWORD dipdw;
	
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAXISMODE_ABS;
	
	rval = m_lpDIDMouse->SetProperty(DIPROP_AXISMODE, &dipdw.diph);
	
    return rval;
}

HRESULT SoEInput::SetJoystickAbs(void)
{
    HRESULT rval = -1;
#if DIRECTINPUT_VERSION >= SOE_DIVER
	DIPROPDWORD dipdw;
	
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAXISMODE_ABS;
	
	rval = m_lpDIDJoystick->SetProperty(DIPROP_AXISMODE, &dipdw.diph);
#endif
	
    return rval;
}

HRESULT SoEInput::RunMouseControlPanel(void *hWnd)
{
    HRESULT rval;
	
	rval = m_lpDIDMouse->RunControlPanel((HWND)hWnd, 0);
	
    return rval;
}

HRESULT SoEInput::RunJoystickControlPanel(void *hWnd)
{
    HRESULT rval = -1;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	rval = m_lpDIDJoystick->RunControlPanel((HWND)hWnd, 0);
#endif
	
    return rval;
}

void SoEInput::FlushKeyboardData(void)
{
	memset(m_keyStates, 0, sizeof(BYTE) * SOEKEY_NUMKEYS);
	memset(m_keyRaw, 0, sizeof(BYTE) * SOEKEY_NUMKEYS);
    memset(m_oldKeyRaw, 0, sizeof(BYTE) * SOEKEY_NUMKEYS);
	memset(m_keyPressTimes, 0, sizeof(DWORD) * SOEKEY_NUMKEYS);
	memset(m_keyDragStartPositions, 0, sizeof(DWORD) * SOEKEY_NUMKEYS * 2);
}

void SoEInput::SetMouseLimits(LONG x1, LONG y1, LONG x2, LONG y2)
{
    if (x1 < x2)
    {
		m_mouseMinX = x1;
        m_mouseMaxX = x2;
    }
    else
    {
        m_mouseMinX = x2;
        m_mouseMaxX = x1;
    }
	
    if (y1 < y2)
    {
		m_mouseMinY = y1;
		m_mouseMaxY = y2;
    }
    else
    {
        m_mouseMinY = y2;
		m_mouseMaxY = y1;
    }
}

void SoEInput::SetJoystickLimits(LONG x1, LONG y1, LONG x2, LONG y2)
{
    if (x1 < x2)
    {
		m_joystickMinX = x1;
        m_joystickMaxX = x2;
    }
    else
    {
        m_joystickMinX = x2;
        m_joystickMaxX = x1;
    }
	
    if (y1 < y2)
    {
		m_joystickMinY = y1;
		m_joystickMaxY = y2;
    }
    else
    {
        m_joystickMinY = y2;
		m_joystickMaxY = y1;
    }
}

void SoEInput::SetMousePos(LONG x, LONG y)
{
	// clamp non-free mouse values to limits
	if ((m_mouseX = x) >= m_mouseMaxX)
		m_mouseX = m_mouseMaxX - 1;
	
	if ((m_mouseY = y) >= m_mouseMaxY)
		m_mouseY = m_mouseMaxY - 1;
	
	if ((m_mouseX = x) <= m_mouseMinX)
		m_mouseX = m_mouseMinX + 1;
	
	if ((m_mouseY = y) <= m_mouseMinY)
		m_mouseY = m_mouseMinY + 1;
	
	// Zero out the Deltas
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
}

void SoEInput::SetMouseFreePos(LONG x, LONG y)
{
	m_mouseFreeX  = x;
	m_mouseFreeY  = y;
	
	// Zero out the Deltas
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
}

void SoEInput::SetJoystickPos(LONG x, LONG y)
{
	// clamp non-free joystick values to limits
	if ((m_joystickX = x) >= m_joystickMaxX)
		m_joystickX = m_joystickMaxX - 1;
	
	if ((m_joystickY = y) >= m_joystickMaxY)
		m_joystickY = m_joystickMaxY - 1;
	
	if ((m_joystickX = x) <= m_joystickMinX)
		m_joystickX = m_joystickMinX + 1;
	
	if ((m_joystickY = y) <= m_joystickMinY)
		m_joystickY = m_joystickMinY + 1;
	
	// Zero out the Deltas
	m_joystickDeltaX = 0;
	m_joystickDeltaY = 0;
}

void SoEInput::SetJoystickFreePos(LONG x, LONG y)
{
	m_joystickFreeX  = x;
	m_joystickFreeY  = y;
	
	// Zero out the Deltas
	m_joystickDeltaX = 0;
	m_joystickDeltaY = 0;
}

void SoEInput::SetMouseSensitivity(float factor)
{
    if (factor < 0)
        factor = 0;
	
	m_mouseSensitivity = factor;
}

float SoEInput::GetMouseSensitivity()
{
	return (m_mouseSensitivity);
}

void SoEInput::SetJoystickSensitivity(float factor)
{
    if (factor < 0)
        factor = 0;
	
	m_joystickSensitivity = factor;
}

float SoEInput::GetJoystickSensitivity()
{
	return (m_joystickSensitivity);
}

WORD SoEInput::GetShiftedKeyState(WORD key)
{
	if (m_shiftedKeyStates[key])
		return (m_shiftedKeyStates[key]);
	
	return (key);
}

WORD SoEInput::GetKeyState(WORD key)
{
    if (key >= SOEKEY_NUMKEYS)
        return 0;
	
	return m_keyStates[key];
}

WORD SoEInput::GetKeyRaw(WORD key)
{
    if (key >= NUMDIKEYS)
        return 0;
	
	return m_keyRaw[key];
}

void SoEInput::GetMousePos(LONG* x, LONG* y)
{
	*x = m_mouseX;
	*y = m_mouseY;
}

void SoEInput::GetMouseFreePos(LONG* x, LONG* y)
{
	*x = m_mouseFreeX;
	*y = m_mouseFreeY;
}

void SoEInput::GetMouseDeltas(LONG* x, LONG* y)
{
	*x = m_mouseDeltaX;
	*y = m_mouseDeltaY;
}

void SoEInput::GetJoystickPos(LONG* x, LONG* y)
{
	*x = m_joystickX;
	*y = m_joystickY;
}

void SoEInput::GetJoystickFreePos(LONG* x, LONG* y)
{
	*x = m_joystickFreeX;
	*y = m_joystickFreeY;
}

void SoEInput::GetJoystickDeltas(LONG* x, LONG* y)
{
	*x = m_joystickDeltaX;
	*y = m_joystickDeltaY;
}

HRESULT SoEInput::AcquireMouse(void)
{
    HRESULT rval = -1;
    int i;
	
	if (m_lpDIDMouse)
	{
		m_bMouse = TRUE;
        rval = m_lpDIDMouse->Acquire();
		
		if (SUCCEEDED(rval))
			return rval;
		
		// didn't get it back, try a few more times
		for (i = 0; i < 50; i++)
		{
            rval = m_lpDIDMouse->Acquire();
			if (SUCCEEDED(rval))
				return rval;
		}
		
		// still don't have it, something's up
		m_bMouse = FALSE;
	}
	
    return rval;
}

HRESULT SoEInput::UnacquireMouse(void)
{
    HRESULT rval = -1;
	
	m_bMouse = FALSE;
	
	if (m_lpDIDMouse)
		rval = m_lpDIDMouse->Unacquire();
	
    return rval;
}

HRESULT SoEInput::AcquireJoystick(void)
{
    HRESULT rval = -1;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
    int i;
	if (m_lpDIDJoystick)
	{
		m_bJoystick = TRUE;
        rval = m_lpDIDJoystick->Acquire();
		
		if (SUCCEEDED(rval))
			return rval;
		
		// didn't get it back, try a few more times
		for (i = 0; i < 50; i++)
		{
            rval = m_lpDIDJoystick->Acquire();
			
			if (SUCCEEDED(rval))
				return rval;
		}
		
		// still don't have it, something's up
		m_bJoystick = FALSE;
	}
#endif
	
    return rval;
}

HRESULT SoEInput::UnacquireJoystick(void)
{
    HRESULT rval = -1;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	m_bJoystick = 0;
	
	if (m_lpDIDJoystick)
		rval = m_lpDIDJoystick->Unacquire();
#endif
	
    return rval;
}

HRESULT SoEInput::AcquireKeyboard(void)
{
    HRESULT rval = -1;
    int i;
	
	if (m_lpDIDKeyboard)
	{
		m_bKeyboard = TRUE;
        rval = m_lpDIDKeyboard->Acquire();
		
		if (SUCCEEDED(rval))
		{
			FlushKeyboardData();
			return rval;
		}
		
		// didn't get it back, try a few more times
		for (i = 0; i < 50; i++)
		{
			rval = m_lpDIDKeyboard->Acquire();
			
			if (SUCCEEDED(rval))
			{
				FlushKeyboardData();
				return rval;
			}
		}
		
		// still don't have it, something's up
		m_bKeyboard = FALSE;
	}	
	
    return rval;
}

HRESULT SoEInput::UnacquireKeyboard(void)
{
    HRESULT rval = -1;
	
	FlushKeyboardData();
	m_bKeyboard = FALSE;
	
	if (m_lpDIDKeyboard)
		rval = m_lpDIDKeyboard->Unacquire();
	
    return rval;
}

void SoEInput::SetMouseAccelerator(BOOL onOff, DWORD threshold1, double mult1, DWORD threshold2, double mult2)
{
    int mouseAcclValues[3];
	
    if (onOff == FALSE)
    {
        // turn accleration off.
        m_mouseThreshold2  = 999;
        m_mouseThreshold1  = 999;
        m_mouseMultiplier2 = 1.0;
        m_mouseMultiplier1 = 1.0;
    }
    else
    {
        if ((threshold1 == 0) &&(threshold2 == 0))
        {
            // Use system variables.
            SystemParametersInfo(SPI_GETMOUSE, 0, mouseAcclValues, 0);
			
            if (mouseAcclValues[2] > 0)
                m_mouseThreshold1  = mouseAcclValues[0];
            else
                m_mouseThreshold1  = 999;
			
            if (mouseAcclValues[2] == 2)
                m_mouseThreshold2  = mouseAcclValues[1];
            else
                m_mouseThreshold2  = 999;
			
            m_mouseMultiplier2 = 4.0;
            m_mouseMultiplier1 = 2.0;
        }
        else
        {
            if (threshold1 > threshold2)
            {
                m_mouseThreshold2  = threshold1;
                m_mouseThreshold1  = threshold2;
                m_mouseMultiplier2 = mult1;
                m_mouseMultiplier1 = mult2;
            }
            else
            {
                m_mouseThreshold2  = threshold2;
                m_mouseThreshold1  = threshold1;
                m_mouseMultiplier2 = mult2;
                m_mouseMultiplier1 = mult1;
            }
        }
    }
}

HRESULT SoEInput::Update(void)
{
    HRESULT rval;
	DIMOUSESTATE MouseState;
	LONG oldx, oldy;
	BOOL isPressed;
	WORD i;
	DWORD key;
	
#if DIRECTINPUT_VERSION >= SOE_DIVER
	DIJOYSTATE JoyState;
#endif
	
	///////////////////////
	//  KEYBOARD
	///////////////////////
	if (1)
	{
		rval = m_lpDIDKeyboard->GetDeviceState(256, &m_keyRaw);
		if (FAILED(rval))
		{
			AcquireKeyboard();
			return rval;
		}
		else
		{
			for (i = 0; i < NUMDIKEYS; i++)
			{
				if (key = m_DIKToSoEKEY[i])
				{
					// check current key state (bit 8 flipped on means key is pressed)
					if (m_keyRaw[i] &(BYTE)0x80)
						isPressed = TRUE;
					else
						isPressed = FALSE;
					
					CheckKeyEvents(key, isPressed);
				}
			}
		}
	}
	
	///////////////////////
	//  MOUSE
	///////////////////////
	if (m_bMouse)
	{
		rval = m_lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);
		if (FAILED(rval))
		{
			AcquireMouse();
			return rval;
		}
		else
		{
            // Apply acceleration thresholds.
            if (abs(MouseState.lX) > m_mouseThreshold2)
                MouseState.lX = (LONG)((double)MouseState.lX * m_mouseMultiplier2);
            else if (abs(MouseState.lX) > m_mouseThreshold1)
                MouseState.lX = (LONG)((double)MouseState.lX * m_mouseMultiplier1);
			
            if (abs(MouseState.lY) > m_mouseThreshold2)
                MouseState.lY = (LONG)((double)MouseState.lY * m_mouseMultiplier2);
            else if (abs(MouseState.lY) > m_mouseThreshold1)
                MouseState.lY = (LONG)((double)MouseState.lY * m_mouseMultiplier1);
			
			// get new mouse position status
			oldx          = m_mouseFreeX;
			oldy          = m_mouseFreeY;
			m_mouseFreeX +=(LONG)((float)MouseState.lX * m_mouseSensitivity);
			m_mouseFreeY +=(LONG)((float)MouseState.lY * m_mouseSensitivity);
			m_mouseX     +=(LONG)((float)MouseState.lX * m_mouseSensitivity);
			m_mouseY     +=(LONG)((float)MouseState.lY * m_mouseSensitivity);
			
			// clamp non-free mouse values to limits
			if (m_mouseX >= m_mouseMaxX)
				m_mouseX = m_mouseMaxX - 1;
			
			if (m_mouseY >= m_mouseMaxY)
				m_mouseY = m_mouseMaxY - 1;
			
			if (m_mouseX <= m_mouseMinX)
				m_mouseX = m_mouseMinX;
			
			if (m_mouseY <= m_mouseMinY)
				m_mouseY = m_mouseMinY;
			
			m_mouseDeltaX = m_mouseFreeX - oldx;
			m_mouseDeltaY = m_mouseFreeY - oldy;
			
			// check the mouse buttons
			for (i = 0; i < 3; i++)
			{
				key = SOEKEY_MOUSELEFT + i;
				
				// check current key state (bit 8 flipped on means key is pressed)
				if (MouseState.rgbButtons[i] &(BYTE)0x80)
					isPressed = TRUE;
				else
					isPressed = FALSE;
				
				CheckKeyEvents(key, isPressed);
			}
		}
	}
	
	///////////////////////
	//  JOYSTICK
	///////////////////////
#if DIRECTINPUT_VERSION >= SOE_DIVER
	if (m_bJoystick)
	{
		m_lpDIDJoystick->Poll();
		
		rval = m_lpDIDJoystick->GetDeviceState(sizeof(JoyState), &JoyState);
		
		if (FAILED(rval))
		{
            AcquireJoystick();
			return rval;
		}
		else
		{
			// get new joystick position status
			oldx			= m_joystickFreeX;
			oldy			= m_joystickFreeY;
			m_joystickFreeX +=(LONG)((float)JoyState.lX * m_joystickSensitivity);
			m_joystickFreeY +=(LONG)((float)JoyState.lY * m_joystickSensitivity);
			m_joystickX		+=(LONG)((float)JoyState.lX * m_joystickSensitivity);
			m_joystickY		+=(LONG)((float)JoyState.lY * m_joystickSensitivity);
			
			// clamp non-free joystick values to limits
			if (m_joystickX >= m_joystickMaxX)
				m_joystickX = m_joystickMaxX - 1;
			
			if (m_joystickY >= m_joystickMaxY)
				m_joystickY = m_joystickMaxY - 1;
			
			if (m_joystickX <= m_joystickMinX)
				m_joystickX = m_joystickMinX;
			
			if (m_joystickY <= m_joystickMinY)
				m_joystickY = m_joystickMinY;
			
			m_joystickDeltaX = m_joystickFreeX - oldx;
			m_joystickDeltaY = m_joystickFreeY - oldy;
			
			// check the joystick buttons
			for (i = 0; i < 10; i++)
			{
				key = SOEKEY_JOYBUTN0 + i;
				
				// check current key state (bit 8 flipped on means key is pressed)
				if (JoyState.rgbButtons[i] &(BYTE)0x80)
					isPressed = TRUE;
				else
					isPressed = FALSE;
				
				CheckKeyEvents(key, isPressed);
			}
		}
	}
#endif
	
    return 0;
}

void SoEInput::CheckKeyEvents(DWORD key, BOOL isPressed)
{
	if (isPressed)
	{
		if (m_keyStates[key] == SOEKEY_NONE || m_keyStates[key] == SOEKEY_RELEASE)
		{
			// press event (key was up before but down now)
			m_keyStates[key]				= SOEKEY_PRESS;
			m_keyPressTimes[key]			= timeGetTime();
			m_keyDragStartPositions[key][0] = m_mouseFreeX;
			m_keyDragStartPositions[key][1] = m_mouseFreeY;
		}
		else if (m_keyStates[key] == SOEKEY_PRESS)
		{
			// drag event (key is still down)
			m_keyStates[key] = SOEKEY_REPEAT;
		}
	}
	else // Key is not currently pressed
	{
		if (m_keyStates[key] == SOEKEY_REPEAT || m_keyStates[key] == SOEKEY_PRESS)
		{
			// release event (key was down before but up now)
			m_keyStates[key] = SOEKEY_RELEASE;
		}
		else if (m_keyStates[key] == SOEKEY_RELEASE)
		{
			m_keyStates[key] = SOEKEY_NONE;
		}
	}
}

void SoEInput::UpdateLastKey(BOOL reset)
{
	if (reset)
		m_lastKey = 0;
	
    for (int i = 0; i < 255; i++)
    {       
        if (m_keyRaw[i] && !m_oldKeyRaw[i])
        {       
            m_lastKey = i;
			break;
        }
    }
	
    memcpy(m_oldKeyRaw, m_keyRaw, 255);
}

void SoEInput::GetKeyDesc(DWORD CheckKey, char *str)
{       
    DWORD key;
    
    key = (CheckKey << 16);
	//+(1<<30); Extended Keys
    GetKeyNameText(key, str, 50);
}

DWORD SoEInput::GetLastKeyChar(void)
{       
    return MapVirtualKey(m_lastKey, 1);    
}

DWORD SoEInput::GetNumberOfJoystickButtons(void)
{
#if DIRECTINPUT_VERSION >= SOE_DIVER
	
    DIDEVCAPS Caps;
	
    // check if joystick exists
	if (m_bJoystick)
	{
        ZeroMemory(&Caps , sizeof(Caps));
        Caps.dwSize = sizeof(Caps);    
        if (SUCCEEDED(m_lpDIDJoystick->GetCapabilities(&Caps)))
            return Caps.dwButtons;
        else
            return 0;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

BOOL SoEInput::IsGamePad(void)
{
    if (m_JoystickType == DIDEVTYPEJOYSTICK_GAMEPAD)
        return TRUE;
    else
        return FALSE;
}

HRESULT SoEInput::GetKeyName(DWORD SoEkey, char* buf, DWORD buflength)
{
    if (buf == NULL)
        return -1;
	
    strncpy(buf, &m_keyText[SoEkey][0], buflength);
	
    return 1;
}

void SoEInput::InitShiftedKeys(void)
{
	WORD i;
	
	memset(m_shiftedKeyStates, 0, SOEKEY_NUMKEYS * sizeof(BYTE));
	
	for (i = 0; i < 26; i++)
		m_shiftedKeyStates[i + 'a'] = i + 'A';
	
	m_shiftedKeyStates['0']  = ')';
	m_shiftedKeyStates['1']  = '!';
	m_shiftedKeyStates['2']  = '@';
	m_shiftedKeyStates['3']  = '#';
	m_shiftedKeyStates['4']  = '$';
	m_shiftedKeyStates['5']  = '%';
	m_shiftedKeyStates['6']  = '^';
	m_shiftedKeyStates['7']  = '&';
	m_shiftedKeyStates['8']  = '*';
	m_shiftedKeyStates['9']  = '(';
	m_shiftedKeyStates['-']  = '_';
	m_shiftedKeyStates['=']  = '+';
	m_shiftedKeyStates['[']  = '{';
	m_shiftedKeyStates[']']  = '}';
	m_shiftedKeyStates['\\'] = '|';
	m_shiftedKeyStates[';']  = ':';
	m_shiftedKeyStates['\''] = '"';
	m_shiftedKeyStates[',']  = '<';
	m_shiftedKeyStates['.']  = '>';
	m_shiftedKeyStates['/']  = '?';
	m_shiftedKeyStates['`']  = '~';
}

void SoEInput::InitSoEKEYToDIK(void)
{
	// any key mapped to zero means unmapped and should be ignored
	memset(m_SoEKEYToDIK, 0, NUMDIKEYS * sizeof(DWORD));
	
	m_SoEKEYToDIK[SOEKEY_BACKSPACE]     = DIK_BACK;
	m_SoEKEYToDIK[SOEKEY_TAB]           = DIK_TAB;
	m_SoEKEYToDIK[SOEKEY_ENTER]         = DIK_RETURN;
	m_SoEKEYToDIK[SOEKEY_ESCAPE]        = DIK_ESCAPE;
	m_SoEKEYToDIK[SOEKEY_SPACE]         = DIK_SPACE;
	m_SoEKEYToDIK[SOEKEY_BANG]          = DIK_1;
	m_SoEKEYToDIK[SOEKEY_DQUOTE]        = DIK_APOSTROPHE;
	m_SoEKEYToDIK[SOEKEY_POUND]         = DIK_3;
	m_SoEKEYToDIK[SOEKEY_DOLLAR]        = DIK_4;
	m_SoEKEYToDIK[SOEKEY_PERCENT]       = DIK_5;
	m_SoEKEYToDIK[SOEKEY_AMPERSAND]     = DIK_7;
	m_SoEKEYToDIK[SOEKEY_APOSTROPHE]    = DIK_APOSTROPHE;
	m_SoEKEYToDIK[SOEKEY_LBRACE]        = DIK_9;
	m_SoEKEYToDIK[SOEKEY_RBRACE]        = DIK_0;
	m_SoEKEYToDIK[SOEKEY_ASTERISC]      = DIK_8;
	m_SoEKEYToDIK[SOEKEY_PLUS]          = DIK_EQUALS;
	m_SoEKEYToDIK[SOEKEY_COMMA]         = DIK_COMMA;
	m_SoEKEYToDIK[SOEKEY_DASH]          = DIK_MINUS;
	m_SoEKEYToDIK[SOEKEY_PERIOD]        = DIK_PERIOD;
	m_SoEKEYToDIK[SOEKEY_SLASH]         = DIK_SLASH;
	m_SoEKEYToDIK[SOEKEY_0]             = DIK_0;
	m_SoEKEYToDIK[SOEKEY_1]             = DIK_1;
	m_SoEKEYToDIK[SOEKEY_2]             = DIK_2;
	m_SoEKEYToDIK[SOEKEY_3]             = DIK_3;
	m_SoEKEYToDIK[SOEKEY_4]             = DIK_4;
	m_SoEKEYToDIK[SOEKEY_5]             = DIK_5;
	m_SoEKEYToDIK[SOEKEY_6]             = DIK_6;
	m_SoEKEYToDIK[SOEKEY_7]             = DIK_7;
	m_SoEKEYToDIK[SOEKEY_8]             = DIK_8;
	m_SoEKEYToDIK[SOEKEY_9]             = DIK_9;
	m_SoEKEYToDIK[SOEKEY_COLON]         = DIK_COLON;
	m_SoEKEYToDIK[SOEKEY_SEMICOLON]     = DIK_SEMICOLON;
	m_SoEKEYToDIK[SOEKEY_LESSTHEN]      = DIK_COMMA;
	m_SoEKEYToDIK[SOEKEY_EQUALS]        = DIK_EQUALS;
	m_SoEKEYToDIK[SOEKEY_GREATERTHEN]   = DIK_PERIOD;
	m_SoEKEYToDIK[SOEKEY_QMARK]         = DIK_SLASH;
	m_SoEKEYToDIK[SOEKEY_AT]            = DIK_AT;
	m_SoEKEYToDIK[SOEKEY_CA]            = DIK_A;
	m_SoEKEYToDIK[SOEKEY_CB]            = DIK_B;
	m_SoEKEYToDIK[SOEKEY_CC]            = DIK_C;
	m_SoEKEYToDIK[SOEKEY_CD]            = DIK_D;
	m_SoEKEYToDIK[SOEKEY_CE]            = DIK_E;
	m_SoEKEYToDIK[SOEKEY_CF]            = DIK_F;
	m_SoEKEYToDIK[SOEKEY_CG]            = DIK_G;
	m_SoEKEYToDIK[SOEKEY_CH]            = DIK_H;
	m_SoEKEYToDIK[SOEKEY_CI]            = DIK_I;
	m_SoEKEYToDIK[SOEKEY_CJ]            = DIK_J;
	m_SoEKEYToDIK[SOEKEY_CK]            = DIK_K;
	m_SoEKEYToDIK[SOEKEY_CL]            = DIK_L;
	m_SoEKEYToDIK[SOEKEY_CM]            = DIK_M;
	m_SoEKEYToDIK[SOEKEY_CN]            = DIK_N;
	m_SoEKEYToDIK[SOEKEY_CO]            = DIK_O;
	m_SoEKEYToDIK[SOEKEY_CP]            = DIK_P;
	m_SoEKEYToDIK[SOEKEY_CQ]            = DIK_Q;
	m_SoEKEYToDIK[SOEKEY_CR]            = DIK_R;
	m_SoEKEYToDIK[SOEKEY_CS]            = DIK_S;
	m_SoEKEYToDIK[SOEKEY_CT]            = DIK_T;
	m_SoEKEYToDIK[SOEKEY_CU]            = DIK_U;
	m_SoEKEYToDIK[SOEKEY_CV]            = DIK_V;
	m_SoEKEYToDIK[SOEKEY_CW]            = DIK_W;
	m_SoEKEYToDIK[SOEKEY_CX]            = DIK_X;
	m_SoEKEYToDIK[SOEKEY_CY]            = DIK_Y;
	m_SoEKEYToDIK[SOEKEY_CZ]            = DIK_Z;
	m_SoEKEYToDIK[SOEKEY_LBRACKET]      = DIK_LBRACKET;
	m_SoEKEYToDIK[SOEKEY_BACKSLASH]     = DIK_BACKSLASH;
	m_SoEKEYToDIK[SOEKEY_RBRACKET]      = DIK_RBRACKET;
	m_SoEKEYToDIK[SOEKEY_CARETE]        = DIK_6;
	m_SoEKEYToDIK[SOEKEY_UNDERSCORE]    = DIK_UNDERLINE;
	m_SoEKEYToDIK[SOEKEY_GRAVE]         = DIK_GRAVE;
	m_SoEKEYToDIK[SOEKEY_A]             = DIK_A;
	m_SoEKEYToDIK[SOEKEY_B]             = DIK_B;
	m_SoEKEYToDIK[SOEKEY_C]             = DIK_C;
	m_SoEKEYToDIK[SOEKEY_D]             = DIK_D;
	m_SoEKEYToDIK[SOEKEY_E]             = DIK_E;
	m_SoEKEYToDIK[SOEKEY_F]             = DIK_F;
	m_SoEKEYToDIK[SOEKEY_G]             = DIK_G;
	m_SoEKEYToDIK[SOEKEY_H]             = DIK_H;
	m_SoEKEYToDIK[SOEKEY_I]             = DIK_I;
	m_SoEKEYToDIK[SOEKEY_J]             = DIK_J;
	m_SoEKEYToDIK[SOEKEY_K]             = DIK_K;
	m_SoEKEYToDIK[SOEKEY_L]             = DIK_L;
	m_SoEKEYToDIK[SOEKEY_M]             = DIK_M;
	m_SoEKEYToDIK[SOEKEY_N]             = DIK_N;
	m_SoEKEYToDIK[SOEKEY_O]             = DIK_O;
	m_SoEKEYToDIK[SOEKEY_P]             = DIK_P;
	m_SoEKEYToDIK[SOEKEY_Q]             = DIK_Q;
	m_SoEKEYToDIK[SOEKEY_R]             = DIK_R;
	m_SoEKEYToDIK[SOEKEY_S]             = DIK_S;
	m_SoEKEYToDIK[SOEKEY_T]             = DIK_T;
	m_SoEKEYToDIK[SOEKEY_U]             = DIK_U;
	m_SoEKEYToDIK[SOEKEY_V]             = DIK_V;
	m_SoEKEYToDIK[SOEKEY_W]             = DIK_W;
	m_SoEKEYToDIK[SOEKEY_X]             = DIK_X;
	m_SoEKEYToDIK[SOEKEY_Y]             = DIK_Y;
	m_SoEKEYToDIK[SOEKEY_Z]             = DIK_Z;
	m_SoEKEYToDIK[SOEKEY_LCURLY]        = DIK_LBRACKET;
	m_SoEKEYToDIK[SOEKEY_PIPE]          = DIK_BACKSLASH;
	m_SoEKEYToDIK[SOEKEY_RCURLY]        = DIK_RBRACKET;
	m_SoEKEYToDIK[SOEKEY_TILDA]         = DIK_GRAVE;
	m_SoEKEYToDIK[SOEKEY_DELETE]        = DIK_DELETE;
    m_SoEKEYToDIK[SOEKEY_LEFTSHIFT]     = DIK_LSHIFT;
    m_SoEKEYToDIK[SOEKEY_RIGHTSHIFT]    = DIK_RSHIFT;
    m_SoEKEYToDIK[SOEKEY_LEFTCTRL]      = DIK_LCONTROL;
    m_SoEKEYToDIK[SOEKEY_RIGHTCTRL]     = DIK_RCONTROL;
    m_SoEKEYToDIK[SOEKEY_LEFTALT]       = DIK_LALT;
    m_SoEKEYToDIK[SOEKEY_RIGHTALT]      = DIK_RALT;
    m_SoEKEYToDIK[SOEKEY_LEFTARROW]     = DIK_LEFTARROW;
    m_SoEKEYToDIK[SOEKEY_RIGHTARROW]    = DIK_RIGHTARROW;
    m_SoEKEYToDIK[SOEKEY_UPARROW]       = DIK_UPARROW;
    m_SoEKEYToDIK[SOEKEY_DOWNARROW]     = DIK_DOWNARROW;
    m_SoEKEYToDIK[SOEKEY_F1]            = DIK_F1;
    m_SoEKEYToDIK[SOEKEY_F2]            = DIK_F2;
    m_SoEKEYToDIK[SOEKEY_F3]            = DIK_F3;
    m_SoEKEYToDIK[SOEKEY_F4]            = DIK_F4;
    m_SoEKEYToDIK[SOEKEY_F5]            = DIK_F5;
    m_SoEKEYToDIK[SOEKEY_F6]            = DIK_F6;
    m_SoEKEYToDIK[SOEKEY_F7]            = DIK_F7;
    m_SoEKEYToDIK[SOEKEY_F8]            = DIK_F8;
    m_SoEKEYToDIK[SOEKEY_F9]            = DIK_F9;
    m_SoEKEYToDIK[SOEKEY_F10]           = DIK_F10;
    m_SoEKEYToDIK[SOEKEY_F11]           = DIK_F11;
    m_SoEKEYToDIK[SOEKEY_F12]           = DIK_F12;
    m_SoEKEYToDIK[SOEKEY_INS]           = DIK_INSERT;
    m_SoEKEYToDIK[SOEKEY_DEL]           = DIK_DELETE;
    m_SoEKEYToDIK[SOEKEY_HOME]          = DIK_HOME;
    m_SoEKEYToDIK[SOEKEY_END]           = DIK_END;
    m_SoEKEYToDIK[SOEKEY_PGUP]          = DIK_PRIOR;
    m_SoEKEYToDIK[SOEKEY_PGDN]          = DIK_NEXT;
    m_SoEKEYToDIK[SOEKEY_NUMSLASH]      = DIK_DIVIDE;
    m_SoEKEYToDIK[SOEKEY_NUMSTAR]       = DIK_NUMPADSTAR;
    m_SoEKEYToDIK[SOEKEY_NUMMINUS]      = DIK_NUMPADMINUS;
    m_SoEKEYToDIK[SOEKEY_NUMPLUS]       = DIK_NUMPADPLUS;
    m_SoEKEYToDIK[SOEKEY_NUMENTER]      = DIK_NUMPADENTER;
    m_SoEKEYToDIK[SOEKEY_NUMPERIOD]     = DIK_NUMPADPERIOD;
    m_SoEKEYToDIK[SOEKEY_NUM0]          = DIK_NUMPAD0;
    m_SoEKEYToDIK[SOEKEY_NUM1]          = DIK_NUMPAD1;
    m_SoEKEYToDIK[SOEKEY_NUM2]          = DIK_NUMPAD2;
    m_SoEKEYToDIK[SOEKEY_NUM3]          = DIK_NUMPAD3;
    m_SoEKEYToDIK[SOEKEY_NUM4]          = DIK_NUMPAD4;
    m_SoEKEYToDIK[SOEKEY_NUM5]          = DIK_NUMPAD5;
    m_SoEKEYToDIK[SOEKEY_NUM6]          = DIK_NUMPAD6;
    m_SoEKEYToDIK[SOEKEY_NUM7]          = DIK_NUMPAD7;
    m_SoEKEYToDIK[SOEKEY_NUM8]          = DIK_NUMPAD8;
    m_SoEKEYToDIK[SOEKEY_NUM9]          = DIK_NUMPAD9;
    m_SoEKEYToDIK[SOEKEY_NUMLOCK]       = DIK_NUMLOCK;
    m_SoEKEYToDIK[SOEKEY_CAPSLOCK]      = DIK_CAPITAL;
    m_SoEKEYToDIK[SOEKEY_SCROLLLOCK]    = DIK_SCROLL;
}

void SoEInput::InitDIKToSoEKEY(void)
{
	// any key mapped to zero means unmapped and should be ignored
	memset(m_DIKToSoEKEY, 0, NUMDIKEYS * sizeof(DWORD));
	
	m_DIKToSoEKEY[DIK_ESCAPE]		= SOEKEY_ESCAPE;
	m_DIKToSoEKEY[DIK_1]			= '1';
	m_DIKToSoEKEY[DIK_2]			= '2';
	m_DIKToSoEKEY[DIK_3]			= '3';
	m_DIKToSoEKEY[DIK_4]			= '4';
	m_DIKToSoEKEY[DIK_5]			= '5';
	m_DIKToSoEKEY[DIK_6]			= '6';
	m_DIKToSoEKEY[DIK_7]			= '7';
	m_DIKToSoEKEY[DIK_8]			= '8';
	m_DIKToSoEKEY[DIK_9]			= '9';
	m_DIKToSoEKEY[DIK_0]			= '0';
	m_DIKToSoEKEY[DIK_MINUS]		= '-';
	m_DIKToSoEKEY[DIK_EQUALS]		= '=';
	m_DIKToSoEKEY[DIK_BACK]			= SOEKEY_BACKSPACE;
	m_DIKToSoEKEY[DIK_TAB]			= SOEKEY_TAB;
	m_DIKToSoEKEY[DIK_Q]			= 'q';
	m_DIKToSoEKEY[DIK_W]			= 'w';
	m_DIKToSoEKEY[DIK_E]			= 'e';
	m_DIKToSoEKEY[DIK_R]			= 'r';
	m_DIKToSoEKEY[DIK_T]			= 't';
	m_DIKToSoEKEY[DIK_Y]			= 'y';
	m_DIKToSoEKEY[DIK_U]			= 'u';
	m_DIKToSoEKEY[DIK_I]			= 'i';
	m_DIKToSoEKEY[DIK_O]			= 'o';
	m_DIKToSoEKEY[DIK_P]			= 'p';
	m_DIKToSoEKEY[DIK_LBRACKET]		= '[';
	m_DIKToSoEKEY[DIK_RBRACKET]		= ']';
	m_DIKToSoEKEY[DIK_RETURN]		= SOEKEY_ENTER;
	m_DIKToSoEKEY[DIK_LCONTROL]		= SOEKEY_LEFTCTRL;
	m_DIKToSoEKEY[DIK_A]			= 'a';
	m_DIKToSoEKEY[DIK_S]			= 's';
	m_DIKToSoEKEY[DIK_D]			= 'd';
	m_DIKToSoEKEY[DIK_F]			= 'f';
	m_DIKToSoEKEY[DIK_G]			= 'g';
	m_DIKToSoEKEY[DIK_H]			= 'h';
	m_DIKToSoEKEY[DIK_J]			= 'j';
	m_DIKToSoEKEY[DIK_K]			= 'k';
	m_DIKToSoEKEY[DIK_L]			= 'l';
	m_DIKToSoEKEY[DIK_SEMICOLON]	= ';';
	m_DIKToSoEKEY[DIK_APOSTROPHE]	= '\'';
	m_DIKToSoEKEY[DIK_GRAVE]		= '`';
	m_DIKToSoEKEY[DIK_LSHIFT]		= SOEKEY_LEFTSHIFT;
	m_DIKToSoEKEY[DIK_BACKSLASH]	= '\\';
	m_DIKToSoEKEY[DIK_Z]			= 'z';
	m_DIKToSoEKEY[DIK_X]			= 'x';
	m_DIKToSoEKEY[DIK_C]			= 'c';
	m_DIKToSoEKEY[DIK_V]			= 'v';
	m_DIKToSoEKEY[DIK_B]			= 'b';
	m_DIKToSoEKEY[DIK_N]			= 'n';
	m_DIKToSoEKEY[DIK_M]			= 'm';
	m_DIKToSoEKEY[DIK_COMMA]		= ',';
	m_DIKToSoEKEY[DIK_PERIOD]		= '.';
	m_DIKToSoEKEY[DIK_SLASH]		= '/';
	m_DIKToSoEKEY[DIK_RSHIFT]		= SOEKEY_RIGHTSHIFT;
	m_DIKToSoEKEY[DIK_MULTIPLY]		= SOEKEY_NUMSTAR;
	m_DIKToSoEKEY[DIK_LMENU]		= SOEKEY_LEFTALT;
	m_DIKToSoEKEY[DIK_SPACE]		= SOEKEY_SPACE;
	m_DIKToSoEKEY[DIK_CAPITAL]		= SOEKEY_CAPSLOCK;
	m_DIKToSoEKEY[DIK_F1]			= SOEKEY_F1;
	m_DIKToSoEKEY[DIK_F2]			= SOEKEY_F2;
	m_DIKToSoEKEY[DIK_F3]			= SOEKEY_F3;
	m_DIKToSoEKEY[DIK_F4]			= SOEKEY_F4;
	m_DIKToSoEKEY[DIK_F5]			= SOEKEY_F5;
	m_DIKToSoEKEY[DIK_F6]			= SOEKEY_F6;
	m_DIKToSoEKEY[DIK_F7]			= SOEKEY_F7;
	m_DIKToSoEKEY[DIK_F8]			= SOEKEY_F8;
	m_DIKToSoEKEY[DIK_F9]			= SOEKEY_F9;
	m_DIKToSoEKEY[DIK_F10]			= SOEKEY_F10;
	m_DIKToSoEKEY[DIK_NUMLOCK]		= SOEKEY_NUMLOCK;
	m_DIKToSoEKEY[DIK_SCROLL]		= SOEKEY_SCROLLLOCK;
	m_DIKToSoEKEY[DIK_NUMPAD7]		= SOEKEY_NUM7;
	m_DIKToSoEKEY[DIK_NUMPAD8]		= SOEKEY_NUM8;
	m_DIKToSoEKEY[DIK_NUMPAD9]		= SOEKEY_NUM9;
	m_DIKToSoEKEY[DIK_SUBTRACT]		= SOEKEY_NUMMINUS;
	m_DIKToSoEKEY[DIK_NUMPAD4]		= SOEKEY_NUM4;
	m_DIKToSoEKEY[DIK_NUMPAD5]		= SOEKEY_NUM5;
	m_DIKToSoEKEY[DIK_NUMPAD6]		= SOEKEY_NUM6;
	m_DIKToSoEKEY[DIK_ADD]			= SOEKEY_NUMPLUS;
	m_DIKToSoEKEY[DIK_NUMPAD1]		= SOEKEY_NUM1;
	m_DIKToSoEKEY[DIK_NUMPAD2]		= SOEKEY_NUM2;
	m_DIKToSoEKEY[DIK_NUMPAD3]		= SOEKEY_NUM3;
	m_DIKToSoEKEY[DIK_NUMPAD0]		= SOEKEY_NUM0;
	m_DIKToSoEKEY[DIK_DECIMAL]		= SOEKEY_NUMPERIOD;
	m_DIKToSoEKEY[DIK_F11]			= SOEKEY_F11;
	m_DIKToSoEKEY[DIK_F12]			= SOEKEY_F12;
	m_DIKToSoEKEY[DIK_NUMPADENTER]  = SOEKEY_NUMENTER;
	m_DIKToSoEKEY[DIK_RCONTROL]		= SOEKEY_RIGHTCTRL;
	m_DIKToSoEKEY[DIK_DIVIDE]		= SOEKEY_NUMSLASH;
	m_DIKToSoEKEY[DIK_SYSRQ]		= SOEKEY_PRINTSCRN;
	m_DIKToSoEKEY[DIK_RMENU]		= SOEKEY_RIGHTALT;
	m_DIKToSoEKEY[DIK_HOME]			= SOEKEY_HOME;
	m_DIKToSoEKEY[DIK_UP]			= SOEKEY_UPARROW;
	m_DIKToSoEKEY[DIK_PRIOR]		= SOEKEY_PGUP;
	m_DIKToSoEKEY[DIK_LEFT]			= SOEKEY_LEFTARROW;
	m_DIKToSoEKEY[DIK_RIGHT]		= SOEKEY_RIGHTARROW;
	m_DIKToSoEKEY[DIK_END]			= SOEKEY_END;
	m_DIKToSoEKEY[DIK_DOWN]			= SOEKEY_DOWNARROW;
	m_DIKToSoEKEY[DIK_NEXT]			= SOEKEY_PGDN;
	m_DIKToSoEKEY[DIK_INSERT]		= SOEKEY_INS;
	m_DIKToSoEKEY[DIK_DELETE]		= SOEKEY_DEL;
	m_DIKToSoEKEY[DIK_LWIN]			= 0;
	m_DIKToSoEKEY[DIK_RWIN]			= 0;
	m_DIKToSoEKEY[DIK_APPS]			= 0;
}

void SoEInput::InitKeyText()
{
    strncpy(&m_keyText[SOEKEY_NULL][0],        "NULL", 12);
    strncpy(&m_keyText[SOEKEY_BACKSPACE][0],   "BACKSPACE", 12);
    strncpy(&m_keyText[SOEKEY_TAB][0],         "TAB", 12);
    strncpy(&m_keyText[SOEKEY_ENTER][0],       "ENTER", 12);
    strncpy(&m_keyText[SOEKEY_ESCAPE][0],      "ESC", 12);
    strncpy(&m_keyText[SOEKEY_SPACE][0],       "SPACE", 12);
    strncpy(&m_keyText[SOEKEY_BANG][0],        "1", 12);
    strncpy(&m_keyText[SOEKEY_DQUOTE][0],      "APOSTROPHE", 12);
    strncpy(&m_keyText[SOEKEY_POUND][0],       "3", 12);
    strncpy(&m_keyText[SOEKEY_DOLLAR][0],      "4", 12);
    strncpy(&m_keyText[SOEKEY_PERCENT][0],     "5", 12);
    strncpy(&m_keyText[SOEKEY_AMPERSAND][0],   "7", 12);
    strncpy(&m_keyText[SOEKEY_APOSTROPHE][0],  "APOSTROPHE", 12);
    strncpy(&m_keyText[SOEKEY_LBRACE][0],      "9", 12);
    strncpy(&m_keyText[SOEKEY_RBRACE][0],      "0", 12);
    strncpy(&m_keyText[SOEKEY_ASTERISC][0],    "8", 12);
    strncpy(&m_keyText[SOEKEY_PLUS][0],        "EQUALS", 12);
    strncpy(&m_keyText[SOEKEY_COMMA][0],       "COMMA", 12);
    strncpy(&m_keyText[SOEKEY_DASH][0],        "UNDERSCORE", 12);
    strncpy(&m_keyText[SOEKEY_PERIOD][0],      "PERIOD", 12);
    strncpy(&m_keyText[SOEKEY_SLASH][0],       "SLASH", 12);
    strncpy(&m_keyText[SOEKEY_0][0],           "0", 12);
    strncpy(&m_keyText[SOEKEY_1][0],           "1", 12);
    strncpy(&m_keyText[SOEKEY_2][0],           "2", 12);
    strncpy(&m_keyText[SOEKEY_3][0],           "3", 12);
    strncpy(&m_keyText[SOEKEY_4][0],           "4", 12);
    strncpy(&m_keyText[SOEKEY_5][0],           "5", 12);
    strncpy(&m_keyText[SOEKEY_6][0],           "6", 12);
    strncpy(&m_keyText[SOEKEY_7][0],           "7", 12);
    strncpy(&m_keyText[SOEKEY_8][0],           "8", 12);
    strncpy(&m_keyText[SOEKEY_9][0],           "9", 12);
    strncpy(&m_keyText[SOEKEY_COLON][0],       "SEMICOLON", 12);
    strncpy(&m_keyText[SOEKEY_SEMICOLON][0],   "SEMICOLON", 12);
    strncpy(&m_keyText[SOEKEY_LESSTHEN][0],    "COMMA", 12);
    strncpy(&m_keyText[SOEKEY_EQUALS][0],      "EQUALS", 12);
    strncpy(&m_keyText[SOEKEY_GREATERTHEN][0], "PERIOD", 12);
    strncpy(&m_keyText[SOEKEY_QMARK][0],       "SLASH", 12);
    strncpy(&m_keyText[SOEKEY_AT][0],          "2", 12);
    strncpy(&m_keyText[SOEKEY_CA][0],          "A", 12);
    strncpy(&m_keyText[SOEKEY_CB][0],          "B", 12);
    strncpy(&m_keyText[SOEKEY_CC][0],          "C", 12);
    strncpy(&m_keyText[SOEKEY_CD][0],          "D", 12);
    strncpy(&m_keyText[SOEKEY_CE][0],          "E", 12);
    strncpy(&m_keyText[SOEKEY_CF][0],          "F", 12);
    strncpy(&m_keyText[SOEKEY_CG][0],          "G", 12);
    strncpy(&m_keyText[SOEKEY_CH][0],          "H", 12);
    strncpy(&m_keyText[SOEKEY_CI][0],          "I", 12);
    strncpy(&m_keyText[SOEKEY_CJ][0],          "J", 12);
    strncpy(&m_keyText[SOEKEY_CK][0],          "K", 12);
    strncpy(&m_keyText[SOEKEY_CL][0],          "L", 12);
    strncpy(&m_keyText[SOEKEY_CM][0],          "M", 12);
    strncpy(&m_keyText[SOEKEY_CN][0],          "N", 12);
    strncpy(&m_keyText[SOEKEY_CO][0],          "O", 12);
    strncpy(&m_keyText[SOEKEY_CP][0],          "P", 12);
    strncpy(&m_keyText[SOEKEY_CQ][0],          "Q", 12);
    strncpy(&m_keyText[SOEKEY_CR][0],          "R", 12);
    strncpy(&m_keyText[SOEKEY_CS][0],          "S", 12);
    strncpy(&m_keyText[SOEKEY_CT][0],          "T", 12);
    strncpy(&m_keyText[SOEKEY_CU][0],          "U", 12);
    strncpy(&m_keyText[SOEKEY_CV][0],          "V", 12);
    strncpy(&m_keyText[SOEKEY_CW][0],          "W", 12);
    strncpy(&m_keyText[SOEKEY_CX][0],          "X", 12);
    strncpy(&m_keyText[SOEKEY_CY][0],          "Y", 12);
    strncpy(&m_keyText[SOEKEY_CZ][0],          "Z", 12);
    strncpy(&m_keyText[SOEKEY_LBRACKET][0],    "LEFT BRKT", 12);
    strncpy(&m_keyText[SOEKEY_BACKSLASH][0],   "BACKSLASH", 12);
    strncpy(&m_keyText[SOEKEY_RBRACKET][0],    "RIGHT BRKT", 12);
    strncpy(&m_keyText[SOEKEY_CARETE][0],      "6", 12);
    strncpy(&m_keyText[SOEKEY_UNDERSCORE][0],  "UNDERSCORE", 12);
    strncpy(&m_keyText[SOEKEY_GRAVE][0],       "GRAVE", 12);
    strncpy(&m_keyText[SOEKEY_A][0],           "A", 12);
    strncpy(&m_keyText[SOEKEY_B][0],           "B", 12);
    strncpy(&m_keyText[SOEKEY_C][0],           "C", 12);
    strncpy(&m_keyText[SOEKEY_D][0],           "D", 12);
    strncpy(&m_keyText[SOEKEY_E][0],           "E", 12);
    strncpy(&m_keyText[SOEKEY_F][0],           "F", 12);
    strncpy(&m_keyText[SOEKEY_G][0],           "G", 12);
    strncpy(&m_keyText[SOEKEY_H][0],           "H", 12);
    strncpy(&m_keyText[SOEKEY_I][0],           "I", 12);
    strncpy(&m_keyText[SOEKEY_J][0],           "J", 12);
    strncpy(&m_keyText[SOEKEY_K][0],           "K", 12);
    strncpy(&m_keyText[SOEKEY_L][0],           "L", 12);
    strncpy(&m_keyText[SOEKEY_M][0],           "M", 12);
    strncpy(&m_keyText[SOEKEY_N][0],           "N", 12);
    strncpy(&m_keyText[SOEKEY_O][0],           "O", 12);
    strncpy(&m_keyText[SOEKEY_P][0],           "P", 12);
    strncpy(&m_keyText[SOEKEY_Q][0],           "Q", 12);
    strncpy(&m_keyText[SOEKEY_R][0],           "R", 12);
    strncpy(&m_keyText[SOEKEY_S][0],           "S", 12);
    strncpy(&m_keyText[SOEKEY_T][0],           "T", 12);
    strncpy(&m_keyText[SOEKEY_U][0],           "U", 12);
    strncpy(&m_keyText[SOEKEY_V][0],           "V", 12);
    strncpy(&m_keyText[SOEKEY_W][0],           "W", 12);
    strncpy(&m_keyText[SOEKEY_X][0],           "X", 12);
    strncpy(&m_keyText[SOEKEY_Y][0],           "Y", 12);
    strncpy(&m_keyText[SOEKEY_Z][0],           "Z", 12);
    strncpy(&m_keyText[SOEKEY_LCURLY][0],      "LEFT BRKT", 12);
    strncpy(&m_keyText[SOEKEY_PIPE][0],        "BACKSLASH", 12);
    strncpy(&m_keyText[SOEKEY_RCURLY][0],      "RIGHT BRKT", 12);
    strncpy(&m_keyText[SOEKEY_TILDA][0],       "GRAVE", 12);
    strncpy(&m_keyText[SOEKEY_DELETE][0],      "DELETE", 12);
    strncpy(&m_keyText[SOEKEY_LEFTSHIFT][0],   "LEFT SHIFT", 12);
    strncpy(&m_keyText[SOEKEY_RIGHTSHIFT][0],  "RIGHT SHIFT", 12);
    strncpy(&m_keyText[SOEKEY_LEFTCTRL][0],    "LEFT CTRL", 12);
    strncpy(&m_keyText[SOEKEY_RIGHTCTRL][0],   "RIGHT CTRL", 12);
    strncpy(&m_keyText[SOEKEY_LEFTALT][0],     "LEFT ALT", 12);
    strncpy(&m_keyText[SOEKEY_RIGHTALT][0],    "RIGHT ALT", 12);
    strncpy(&m_keyText[SOEKEY_LEFTARROW][0],   "LEFT ARROW", 12);
    strncpy(&m_keyText[SOEKEY_RIGHTARROW][0],  "RIGHT ARROW", 12);
    strncpy(&m_keyText[SOEKEY_UPARROW][0],     "UP ARROW", 12);
    strncpy(&m_keyText[SOEKEY_DOWNARROW][0],   "DOWN ARROW", 12);
    strncpy(&m_keyText[SOEKEY_F1][0],          "F1", 12);
    strncpy(&m_keyText[SOEKEY_F2][0],          "F2", 12);
    strncpy(&m_keyText[SOEKEY_F3][0],          "F3", 12);
    strncpy(&m_keyText[SOEKEY_F4][0],          "F4", 12);
    strncpy(&m_keyText[SOEKEY_F5][0],          "F5", 12);
    strncpy(&m_keyText[SOEKEY_F6][0],          "F6", 12);
    strncpy(&m_keyText[SOEKEY_F7][0],          "F7", 12);
    strncpy(&m_keyText[SOEKEY_F8][0],          "F8", 12);
    strncpy(&m_keyText[SOEKEY_F9][0],          "F9", 12);
    strncpy(&m_keyText[SOEKEY_F10][0],         "F10", 12);
    strncpy(&m_keyText[SOEKEY_F11][0],         "F11", 12);
    strncpy(&m_keyText[SOEKEY_F12][0],         "F12", 12);
    strncpy(&m_keyText[SOEKEY_INS][0],         "INSERT", 12);
    strncpy(&m_keyText[SOEKEY_DEL][0],         "DEL", 12);
    strncpy(&m_keyText[SOEKEY_HOME][0],        "HOME", 12);
    strncpy(&m_keyText[SOEKEY_END][0],         "END", 12);
    strncpy(&m_keyText[SOEKEY_PGUP][0],        "PAGE UP", 12);
    strncpy(&m_keyText[SOEKEY_PGDN][0],        "PAGE DOWN", 12);
    strncpy(&m_keyText[SOEKEY_NUMSLASH][0],    "NUM SLASH", 12);
    strncpy(&m_keyText[SOEKEY_NUMSTAR][0],     "NUM STAR", 12);
    strncpy(&m_keyText[SOEKEY_NUMMINUS][0],    "NUM MINUS", 12);
    strncpy(&m_keyText[SOEKEY_NUMPLUS][0],     "NUM PLUS", 12);
    strncpy(&m_keyText[SOEKEY_NUMENTER][0],    "NUM ENTER", 12);
    strncpy(&m_keyText[SOEKEY_NUMPERIOD][0],   "NUM PERIOD", 12);
    strncpy(&m_keyText[SOEKEY_NUM0][0],        "NUM 0", 12);
    strncpy(&m_keyText[SOEKEY_NUM1][0],        "NUM 1", 12);
    strncpy(&m_keyText[SOEKEY_NUM2][0],        "NUM 2", 12);
    strncpy(&m_keyText[SOEKEY_NUM3][0],        "NUM 3", 12);
    strncpy(&m_keyText[SOEKEY_NUM4][0],        "NUM 4", 12);
    strncpy(&m_keyText[SOEKEY_NUM5][0],        "NUM 5", 12);
    strncpy(&m_keyText[SOEKEY_NUM6][0],        "NUM 6", 12);
    strncpy(&m_keyText[SOEKEY_NUM7][0],        "NUM 7", 12);
    strncpy(&m_keyText[SOEKEY_NUM8][0],        "NUM 8", 12);
    strncpy(&m_keyText[SOEKEY_NUM9][0],        "NUM 9", 12);
    strncpy(&m_keyText[SOEKEY_NUMLOCK][0],     "NUM LOCK", 12);
    strncpy(&m_keyText[SOEKEY_CAPSLOCK][0],    "CAPS LOCK", 12);
    strncpy(&m_keyText[SOEKEY_SCROLLLOCK][0],  "SCROLL LOCK", 12);
    strncpy(&m_keyText[SOEKEY_PRINTSCRN][0],   "PRINT SCRN", 12);
    strncpy(&m_keyText[SOEKEY_PAUSE][0],       "PAUSE", 12);
    strncpy(&m_keyText[SOEKEY_MOUSELEFT][0],   "MOUSE LEFT", 12);
    strncpy(&m_keyText[SOEKEY_MOUSERIGHT][0],  "MOUSE RIGHT", 12);
    strncpy(&m_keyText[SOEKEY_MOUSEMIDDLE][0], "MOUSE MIDLE", 12);
    strncpy(&m_keyText[SOEKEY_MOUSEBUTN4][0],  "MOUSE BUTN4", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN0][0],    "JOY BUTN 0", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN1][0],    "JOY BUTN 1", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN2][0],    "JOY BUTN 2", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN3][0],    "JOY BUTN 3", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN4][0],    "JOY BUTN 4", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN5][0],    "JOY BUTN 5", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN6][0],    "JOY BUTN 6", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN7][0],    "JOY BUTN 7", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN8][0],    "JOY BUTN 8", 12);
    strncpy(&m_keyText[SOEKEY_JOYBUTN9][0],    "JOY BUTN 9", 12);
}