/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Log file handling */

BOOL Log(const char *szMessage);
BOOL NetLog(const char *szMessage);
void CloseLog(const char *szFileName);
void LogProcess(int iValue);
BOOL LogProcessCallback(const char *szFormat, int iValue);
