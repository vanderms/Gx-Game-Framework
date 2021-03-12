#include "Gx/Gx.h"
#include "Gx/Components/Components.h"



static void onLoad(sEvent* e){
    (void) e;

    sSize size = nAppLogicalSize();

    nElemCreate(&(sIni){
        .display =  nELEM_DISPLAY_ABSOLUTE,
        .body = nELEM_BODY_NONE,
        .position = &(sRect){0, 0, size.w, size.h},
        .backgroundColor = "White",
    });


    sElement* hello = nElemCreate(&(sIni){
        .display =  nELEM_DISPLAY_ABSOLUTE,
        .body = nELEM_BODY_NONE,
        .position = &(sRect){80, 200, 200, 50},
        .zIndex = 1,
        .color = "White",
        .fontSize = 32,
        .text = "Hello Gx!",
    });

    nButtonCreate(hello, nBUTTON_KEYBOARD | nBUTTON_SCREEN, SDLK_a);
    nRoundedCreate(hello, "Blue", 5);
}


int main(int argc, char** argv){
    (void) argv[argc];

    nAppCreate(&(sIni){
        .window = "Landscape|360",
        .onLoad = onLoad,
    });



    nAppRun();

    return 0;
}



