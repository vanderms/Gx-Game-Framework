#ifndef GX_TILEMAP_H
#define GX_TILEMAP_H
#include "../Utilities/GxUtil.h"


GxElement* GxCreateTileMap(const char* tilePath, const GxIni* ini);
void GxTilemapUpdate(GxElement* elem, int* sequence);





#endif // !GX_TILEMAP_H
