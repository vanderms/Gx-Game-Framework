#ifndef GX_TILEMAP_H
#define GX_TILEMAP_H
#include "../Utilities/GxUtil.h"


GxElement* GxCreateTileMap(const char* tilePath, const GxIni* ini);
bool GxIsTilemap(GxElement* elem);

#endif // !GX_TILEMAP_H
