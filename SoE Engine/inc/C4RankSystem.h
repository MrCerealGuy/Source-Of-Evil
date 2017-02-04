/*****************************************************************/
/*					      Source Of Evil Engine                  */
/*																 */
/* Copyright (C) 2000-2001  Andreas Zahnleiter GreenByte Studios */
/*																 */
/*****************************************************************/


/* Rank list for players or crew members */

class C4RankSystem
{
public:
    C4RankSystem();

protected:
	char Register[256+1];
	char RankName[C4MaxName+1];
	int RankBase;    

public:    
    void Default();
    void Clear();
    int Init(const char *szRegister, const char *szDefRanks, int iRankBase);
    int Experience(int iRank);
    const char *Name(int iRank);
	BOOL Check(int iRank, const char  *szDefRankName);
};
