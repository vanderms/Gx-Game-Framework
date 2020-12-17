#include "Gx/Gx.h"
#include "Gx/Components/Components.h"


static void onUpdate(sEvent* e) {
    sElement* hello = e->target;
    if (nButtonHasStatus(hello, nButton_HOVER)) {
        nElemSetBackgroundColor(hello, "Red");
    }
    else {
        nElemSetBackgroundColor(hello, "Blue");
    }
}


static void onLoad(sEvent* e){
    (void) e;

    sSize size = nAppLogicalSize();
    sElement* hello = nElemCreate(&(sIni){
        .display =  nElem_DISPLAY_ABSOLUTE,
        .body = nElem_BODY_NONE,
        .position = &(sRect){80, 200, 200, 50},
        .zIndex = 1,
        .backgroundColor = "(0, 0, 0, 180)",
        .color = "White",
        .onUpdate = onUpdate,
        .fontSize = 32,
        .text = "Hello Gx!",
    });

    nButtonCreate(hello, nButton_KEYBOARD | nButton_SCREEN, SDLK_a);

    for (Uint32 i = 0; i < 100; i++) {
        int velx = rand() % 10 - 5;
        velx = velx == 0 ? 1 : velx;
        int vely = rand() % 10 - 5;
        vely = vely == 0 ? -1 : vely;
        int x = rand() % nAppLogicalSize().w;
        int y = rand() % nAppLogicalSize().h;

         sElement* particle = nElemCreate(&(sIni){
            .display = nElem_DISPLAY_RELATIVE,
            .body = nElem_BODY_DYNAMIC,
            .backgroundColor = "White",
            .position = &(sRect){x, y, 2, 2},
            .velocity = {velx, vely},
        });

        nElemSetElasticity(particle, 1.0);
    }
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



