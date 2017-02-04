/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Gamepad control */

class C4GamePadControl
{
public:
    C4GamePadControl();

public:
    BOOL Status;
    int Plr;
    int GamePad;
	BOOL Extended;
    DWORD Pos,Buttons;
    DWORD LastPos,LastButtons;
    BOOL CallOkay;

public:
    BOOL AnyButtonDown();
    BOOL Initialize(int iPlr, int iGamepad);
    void Execute();
};
