/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Map.h"

SoEMapCell::SoEMapCell()
{
	TileNum = 0;
}

SoEMapCell::~SoEMapCell()
{
}

void SoEMapCell::Load(FILE *fp)
{
	fread(&TileNum, sizeof(int), 1, fp);
}

void SoEMapCell::Save(FILE *fp)
{
	fwrite(&TileNum, sizeof(int), 1, fp);
}

int SoEMapCell::GetTile()
{
	return TileNum;
}

void SoEMapCell::SetTile(int Tile)
{
	TileNum = Tile;
}
