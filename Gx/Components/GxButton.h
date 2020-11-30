#ifndef GX_BUTTON_H
#define GX_BUTTON_H
#include "../Utilities/GxUtil.h"
	

GxElement* GxCreateButton(const GxIni* ini, Uint32 inputs, int keyCode);

//acessors
Uint32 GxButtonGetStatus(GxElement* elem);
bool GxButtonHasStatus(GxElement* elem, Uint32 status);

#endif // !GX_BUTTON_H

