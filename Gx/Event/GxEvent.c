#include "../Event/GxEvent.h"
#include "../Ini/GxIni.h"

void GxEventSetHandlers_(GxHandler* handlers, const GxIni* ini) {
      
    //lifecycle
	handlers[GxEventOnLoad] = ini->onLoad;
	handlers[GxEventOnLoopBegin] = ini->onLoopBegin;
	handlers[GxEventOnUpdate] = ini->onUpdate;
	handlers[GxEventOnPreGraphical] = ini->onPreGraphical;
	handlers[GxEventOnPreRender] = ini->onPreRender;
	handlers[GxEventOnLoopEnd] = ini->onLoopEnd;
	handlers[GxEventOnUnload] = ini->onUnload;

    //precontact
    handlers[GxEventOnKeyboard] = ini->onKeyboard;
    handlers[GxEventMouse] = ini->onMouse;
    handlers[GxEventFinger] = ini->onFinger;
    handlers[GxEventSDLDefault] = ini->onSDLDefault;

    //contact
    handlers[GxEventPreContact] = ini->onPreContact;
    handlers[GxEventContactBegin] = ini->onContactBegin;
    handlers[GxEventContactEnd] = ini->onContactEnd;
    handlers[GxEventOnDestroy] = ini->onDestroy;    
}

bool GxEventIniHasHandler_(const GxIni* ini) {
    return ( ini->onLoad || ini->onLoopBegin || ini->onUpdate ||
        ini->onPreGraphical || ini->onPreRender || ini->onLoopEnd ||
        ini->onUnload || ini->onKeyboard || ini->onMouse ||
        ini->onFinger || ini->onSDLDefault || ini->onPreContact ||
        ini->onContactBegin || ini->onContactEnd || ini->onDestroy);
}




GxRequestData* GxCreateRequestData_(void* target,
    const char* request, GxRequestHandler handler
){
    GxRequestData* self = malloc(sizeof(GxRequestData));
   nsUtil->assertNullPointer(self);
    self->target = target;
    self->request = nsUtil->createString(request);
    self->handler = handler;
    return self;
}
void GxDestroyRequestData_(GxRequestData* self){
    free(self->request);
    free(self);
};
