#ifndef GX_I_EVENT_HANDLER_H
#define GX_I_EVENT_HANDLER_H
#include "../Utilities/GxUtil.h"


void GxEventSetHandlers_(GxHandler* ihandlers, const GxIni* ini);
bool GxEventIniHasHandler_(const GxIni* ini);
void GxFreeTarget(GxEvent* e);

#endif // !GX_IEVENT_HANDLER_H
