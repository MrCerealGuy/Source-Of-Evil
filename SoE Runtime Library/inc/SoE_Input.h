/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_INPUT_H
#define SOE_INPUT_H

// SoE Input Types. Applies to keyboard keys, mouse & joystick buttons.
#define SOEKEY_NONE				0
#define SOEKEY_RELEASE			1
#define SOEKEY_PRESS			2
#define SOEKEY_REPEAT			3

// SoEInput Key constants
enum
{
	SOEKEY_NULL				= 0x00000000,
    SOEKEY_BACKSPACE		= 0x00000008,
    SOEKEY_TAB				= 0x00000009,
    SOEKEY_ENTER			= 0x0000000D,
    SOEKEY_ESCAPE			= 0x0000001B,
    SOEKEY_SPACE			= 0x00000020,

	// Printable characters
	SOEKEY_BANG,
	SOEKEY_DQUOTE,
	SOEKEY_POUND,
	SOEKEY_DOLLAR,
	SOEKEY_PERCENT,
	SOEKEY_AMPERSAND,
	SOEKEY_APOSTROPHE,
	SOEKEY_LBRACE,
	SOEKEY_RBRACE,
	SOEKEY_ASTERISC,
	SOEKEY_PLUS,
	SOEKEY_COMMA,
	SOEKEY_DASH,
	SOEKEY_PERIOD,
	SOEKEY_SLASH,
    SOEKEY_0,
	SOEKEY_1,
	SOEKEY_2,
	SOEKEY_3,
	SOEKEY_4,
	SOEKEY_5,
	SOEKEY_6,
	SOEKEY_7,
	SOEKEY_8,
	SOEKEY_9,
	SOEKEY_COLON,
	SOEKEY_SEMICOLON,
	SOEKEY_LESSTHEN,
	SOEKEY_EQUALS,
	SOEKEY_GREATERTHEN,
	SOEKEY_QMARK,
	SOEKEY_AT,
	SOEKEY_CA,
	SOEKEY_CB,
	SOEKEY_CC,
	SOEKEY_CD,
	SOEKEY_CE,
	SOEKEY_CF,
	SOEKEY_CG,
	SOEKEY_CH,
	SOEKEY_CI,
	SOEKEY_CJ,
	SOEKEY_CK,
	SOEKEY_CL,
	SOEKEY_CM,
	SOEKEY_CN,
	SOEKEY_CO,
	SOEKEY_CP,
	SOEKEY_CQ,
	SOEKEY_CR,
	SOEKEY_CS,
	SOEKEY_CT,
	SOEKEY_CU,
	SOEKEY_CV,
	SOEKEY_CW,
	SOEKEY_CX,
	SOEKEY_CY,
	SOEKEY_CZ,
	SOEKEY_LBRACKET,
	SOEKEY_BACKSLASH,
	SOEKEY_RBRACKET,
	SOEKEY_CARETE,
	SOEKEY_UNDERSCORE,
	SOEKEY_GRAVE,
	SOEKEY_A,
	SOEKEY_B,
	SOEKEY_C,
	SOEKEY_D,
	SOEKEY_E,
	SOEKEY_F,
	SOEKEY_G,
	SOEKEY_H,
	SOEKEY_I,
	SOEKEY_J,
	SOEKEY_K,
	SOEKEY_L,
	SOEKEY_M,
	SOEKEY_N,
	SOEKEY_O,
	SOEKEY_P,
	SOEKEY_Q,
	SOEKEY_R,
	SOEKEY_S,
	SOEKEY_T,
	SOEKEY_U,
	SOEKEY_V,
	SOEKEY_W,
	SOEKEY_X,
	SOEKEY_Y,
	SOEKEY_Z,
	SOEKEY_LCURLY,
	SOEKEY_PIPE,
	SOEKEY_RCURLY,
	SOEKEY_TILDA,
	SOEKEY_DELETE,

	// modifier keys
    SOEKEY_LEFTSHIFT		= 0x00000080,
    SOEKEY_RIGHTSHIFT,
	SOEKEY_LEFTCTRL,
	SOEKEY_RIGHTCTRL,
    SOEKEY_LEFTALT,
    SOEKEY_RIGHTALT,
	
	// arrow keys
	SOEKEY_LEFTARROW,
    SOEKEY_RIGHTARROW,
    SOEKEY_UPARROW,
    SOEKEY_DOWNARROW,
    
	// function keys
	SOEKEY_F1,
    SOEKEY_F2,
    SOEKEY_F3,
    SOEKEY_F4,
    SOEKEY_F5,
    SOEKEY_F6,
    SOEKEY_F7,
    SOEKEY_F8,
    SOEKEY_F9,
    SOEKEY_F10,
    SOEKEY_F11,
    SOEKEY_F12,

	// cursor control keys
    SOEKEY_INS,
    SOEKEY_DEL,
    SOEKEY_HOME,
    SOEKEY_END,
    SOEKEY_PGUP,
    SOEKEY_PGDN,
	
	// numeric keypad
    SOEKEY_NUMSLASH,
    SOEKEY_NUMSTAR,
    SOEKEY_NUMMINUS,
    SOEKEY_NUMPLUS,
    SOEKEY_NUMENTER,
    SOEKEY_NUMPERIOD,
    SOEKEY_NUM0,
    SOEKEY_NUM1,
    SOEKEY_NUM2,
    SOEKEY_NUM3,
    SOEKEY_NUM4,
    SOEKEY_NUM5,
    SOEKEY_NUM6,
    SOEKEY_NUM7,
    SOEKEY_NUM8,
    SOEKEY_NUM9,
    
	// locks and misc keys
	SOEKEY_NUMLOCK,
    SOEKEY_CAPSLOCK,
    SOEKEY_SCROLLLOCK,
    SOEKEY_PRINTSCRN,
    SOEKEY_PAUSE,
	
	// windows keys deliberately not listed

	// characters 256 and up used for mouse and joystick buttons etc.
	SOEKEY_MOUSELEFT		= 0x00000100,
	SOEKEY_MOUSERIGHT,
	SOEKEY_MOUSEMIDDLE,
	SOEKEY_MOUSEBUTN4,
	SOEKEY_JOYBUTN0,
	SOEKEY_JOYBUTN1,
	SOEKEY_JOYBUTN2,
	SOEKEY_JOYBUTN3,
	SOEKEY_JOYBUTN4,
	SOEKEY_JOYBUTN5,
	SOEKEY_JOYBUTN6,
	SOEKEY_JOYBUTN7,
	SOEKEY_JOYBUTN8,
	SOEKEY_JOYBUTN9,

	// last keytype defined
	SOEKEY_NUMKEYS
};

#define NUMDIKEYS 256

class SOEAPI SoEInput
{
public:
	SoEInput(void);
	virtual ~SoEInput(void);

	HRESULT Create(void *hInst, void *hWnd);
	HRESULT Update(void);
	void    UpdateLastKey( BOOL reset );
	void    FlushKeyboardData();
    void    SetJoystickLimits(LONG x1, LONG y1, LONG x2, LONG y2);
	void    SetMouseLimits(LONG x1, LONG y1, LONG x2, LONG y2);
	void    SetMouseSensitivity(float factor);
	void    SetJoystickSensitivity(float factor);
    void    SetMousePos(LONG x, LONG y);
	void    SetMouseFreePos(LONG x, LONG y);
	void    SetJoystickPos(LONG x, LONG y);
	void    SetJoystickFreePos(LONG x, LONG y);
    void    GetKeyDesc( DWORD Checkkey, char *str );
	void    GetMousePos(LONG* x, LONG* y);
	void    GetMouseFreePos(LONG* x, LONG* y);
	void    GetMouseDeltas(LONG* x, LONG* y);
	void    GetJoystickPos(LONG* x, LONG* y);
	void    GetJoystickFreePos(LONG* x, LONG* y);
	void    GetJoystickDeltas(LONG* x, LONG* y);
	HRESULT RunMouseControlPanel(void *hWnd = NULL);
	HRESULT RunJoystickControlPanel(void *hWnd = NULL);
    HRESULT SetActiveDevices(BOOL bMouse, BOOL bKeyboard, BOOL bJoystick);
	HRESULT SetMouseAbs(void);
	HRESULT SetJoystickAbs(void);
    HRESULT SetMouseExculsive(BOOL bExculsive);
    HRESULT SetKeyboardExculsive(BOOL bExculsive);
    HRESULT SetJoystickExculsive(BOOL bExculsive);
    BYTE    GetLastKey( void ) { return m_lastKey; };
	WORD    GetShiftedKeyState(WORD key);
	WORD    GetKeyState(WORD key);
	WORD    GetKeyRaw(WORD key);
	float   GetMouseSensitivity();
	float   GetJoystickSensitivity();
    BOOL    HasJoystick( void ) { return m_bJoystick; }
    BOOL    HasMouse( void )    { return m_bMouse; }
    BOOL    IsGamePad( void );
    DWORD   GetNumberOfJoystickButtons( void );
    DWORD   GetJoystickType( void ) { return m_JoystickType; }
    DWORD   GetLastKeyChar( void );
    DWORD   SoEKEYToDIK(DWORD SoEkey) {if(SoEkey > NUMDIKEYS) return 0; return m_SoEKEYToDIK[SoEkey];}
    HRESULT GetKeyName(DWORD SoEkey, char* buf, DWORD buflength);
    void    SetMouseAccelerator(BOOL onOff, DWORD threshold1 = 0, double mult1 = 1.0, DWORD threshold2 = 0, double mult2 = 1.0);
    
    

private:
	void    InitDIKToSoEKEY();
    void    InitSoEKEYToDIK();
    void    InitShiftedKeys();
    void    InitKeyText();
    void    CheckKeyEvents(DWORD key, BOOL isPressed);
	HRESULT AcquireMouse();
	HRESULT UnacquireMouse();
	HRESULT AcquireKeyboard();
	HRESULT UnacquireKeyboard();
	HRESULT AcquireJoystick();
	HRESULT UnacquireJoystick();

private:
	LPDIRECTINPUT        m_lpDI;
	LPDIRECTINPUTDEVICE  m_lpDIDKeyboard;
	LPDIRECTINPUTDEVICE  m_lpDIDMouse;

#if DIRECTINPUT_VERSION >= SOE_DIVER
	LPDIRECTINPUTDEVICE2 m_lpDIDJoystick;
#endif

	BOOL   m_bMouse;
	BOOL   m_bKeyboard;
	BOOL   m_bJoystick;
    DWORD  m_JoystickType;
	float  m_mouseSensitivity;
	float  m_joystickSensitivity;
	LONG   m_mouseMinX;
	LONG   m_mouseMinY;
	LONG   m_mouseMaxX;
	LONG   m_mouseMaxY;
	LONG   m_joystickMinX;
	LONG   m_joystickMinY;
	LONG   m_joystickMaxX;
	LONG   m_joystickMaxY;
	LONG   m_mouseX, m_mouseY;
	LONG   m_mouseFreeX, m_mouseFreeY;
	LONG   m_mouseDeltaX, m_mouseDeltaY;
	LONG   m_joystickX, m_joystickY;
	LONG   m_joystickFreeX, m_joystickFreeY;
	LONG   m_joystickDeltaX, m_joystickDeltaY;
	BYTE   m_keyStates[SOEKEY_NUMKEYS];
	BYTE   m_keyRaw[NUMDIKEYS];
	BYTE   m_oldKeyRaw[NUMDIKEYS];
	DWORD  m_keyPressTimes[SOEKEY_NUMKEYS];
	DWORD  m_keyDragStartPositions[SOEKEY_NUMKEYS][2];
    BYTE   m_lastKey;
	BYTE   m_shiftedKeyStates[SOEKEY_NUMKEYS];
	DWORD  m_DIKToSoEKEY[NUMDIKEYS];
    HWND   m_hWnd;
    DWORD  m_SoEKEYToDIK[NUMDIKEYS];
    char   m_keyText[SOEKEY_NUMKEYS][12];
    LONG   m_mouseThreshold1;
    LONG   m_mouseThreshold2;
    double m_mouseMultiplier1;
    double m_mouseMultiplier2;
};

#endif
