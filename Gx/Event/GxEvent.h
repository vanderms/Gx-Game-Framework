#ifndef GX_I_EVENT_HANDLER_H
#define GX_I_EVENT_HANDLER_H
#include "../Utilities/GxUtil.h"

typedef struct GxRequestData {
	char* request;
	void* target;
	GxRequestHandler handler;
}GxRequestData;


void GxEventSetHandlers_(GxHandler* ihandlers, const GxIni* ini);
bool GxEventIniHasHandler_(const GxIni* ini);
void GxOnDestroyFreeTarget(GxEvent* e);
void GxOnDestroyDoNothing(GxEvent* e);

GxRequestData* GxCreateRequestData_( void* target,
	const char* request, GxRequestHandler handler
);
void GxDestroyRequestData_(GxRequestData* self);

#endif // !GX_IEVENT_HANDLER_H
