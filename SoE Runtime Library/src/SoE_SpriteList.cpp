/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Sprite.h"

SoESpriteList::SoESpriteList(void)
{
}

SoESpriteList::~SoESpriteList(void)
{
    m_spriteList.Clear(TRUE);
}

HRESULT SoESpriteList::Draw(SoESurface* lpSoES, DWORD ScrnWorldX, DWORD ScrnWorldY, WORD BltType)
{
    HRESULT rval;
	SoESprite* Node;
	
	// Loop the list and draw the objects
	for (Node = m_spriteList.GetFirst(); Node != NULL; Node = Node->GetNext())
	{
		rval = Node->Draw(lpSoES, ScrnWorldX, ScrnWorldY, BltType);
		
        if (FAILED(rval))
            return rval;
	}
	
    return rval;
}
