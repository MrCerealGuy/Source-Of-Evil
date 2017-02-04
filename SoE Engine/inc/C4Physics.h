/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Some old constants and references */

const int StableRange=10;
const int AttachRange=5;
const int CornerRange=AttachRange+2;

#define GravAccel (Game.Landscape.Gravity)

extern FIXED FloatAccel;
extern FIXED HitSpeed1,HitSpeed2,HitSpeed3,HitSpeed4;
extern FIXED WalkAccel,SwimAccel;
extern FIXED FloatFriction;
extern FIXED RotateAccel;