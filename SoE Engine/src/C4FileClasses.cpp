/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Some class entries in the Windows registry */

#include <C4Include.h>

#define C4FileClassContentType "application/soe"

BOOL SetProtocol(const char *szProtocol, const char *szCommand, const char *szModule)
{	
	if (!SetRegClassesRoot(szProtocol,NULL,"URL: Protocol")) return FALSE;
	if (!SetRegClassesRoot(szProtocol,"URL Protocol","")) return FALSE;

	char szCmd[_MAX_PATH+1],szKey[_MAX_PATH+1];
	sprintf(szCmd,szCommand,szModule);
	sprintf(szKey,"%s\\shell\\open\\command",szProtocol);
	if (!SetRegClassesRoot(szKey,"",szCmd)) return FALSE;

	char szIconpath[_MAX_PATH+1];
	sprintf(szIconpath,"%s,1",szModule);
	sprintf(szKey,"%s\\DefaultIcon",szProtocol);
	if (!SetRegClassesRoot(szKey,"",szIconpath)) return FALSE;

	return TRUE;
}

BOOL SetC4FileClasses(const char *szEnginePath, const char *szFrontendPath)
{

	if (!SetRegFileClass("SoE.Scenario","c4s","Source Of Evil Level",szEnginePath,1,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Group","c4g","Source Of Evil Group",szEnginePath,2,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Folder","c4f","Source Of Evil Folder",szEnginePath,3,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Player","plr","Source Of Evil Player",szEnginePath,4,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Executable","c4x","Source Of Evil Engine",szEnginePath,5,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Definition","c4d","Source Of Evil Object Definition",szEnginePath,6,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Object","c4i","Source Of Evil Object Info",szEnginePath,7,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Material","c4m","Source Of Evil Material",szEnginePath,8,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Binary","c4b","Source Of Evil Binary",szEnginePath,9,C4FileClassContentType)) return FALSE;
	if (!SetRegFileClass("SoE.Video","c4v","Source Of Evil Video",szEnginePath,10,"video/avi")) return FALSE;
	
	if (!SetProtocol("host","%s %%1 /Fullscreen",szEnginePath)) return FALSE;
	if (!SetProtocol("sourceofevil","%s /Net",szFrontendPath)) return FALSE;

	return TRUE;
}

