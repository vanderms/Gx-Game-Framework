#include "Gx/Gx.h"
#include "Gx/Components/Components.h"



static void onUpdate(sEvent* e) {
    sElement* hello = e->target;
    if (nButton->hasStatus(hello, nButton->HOVER)) {
        nElem->style->setBackgroundColor(hello, "Red");
    }
    else {
        nElem->style->setBackgroundColor(hello, "Blue");
    }
}

static void onLoad(sEvent* e){
    (void) e;


    sSize size = nApp->logicalSize();
    sElement* hello = nElem->create(&(sIni){
        .display =  nElem->display->ABSOLUTE,
        .body = nElem->body->NONE,
        .position = &(sRect){80, 200, 200, 50},
        .zIndex = 1,
        .backgroundColor = "(0, 0, 0, 180)",
        .color = "White",
        .onUpdate = onUpdate,
        .fontSize = 32,
        .text = "Hello Gx!",
    });

    nButton->implement(hello, nButton->KEYBOARD | nButton->SCREEN, SDLK_a);

    for (Uint32 i = 0; i < 100; i++) {
        int velx = rand() % 10 - 5;
        velx = velx == 0 ? 1 : velx;
        int vely = rand() % 10 - 5;
        vely = vely == 0 ? -1 : vely;
        int x = rand() % nApp->logicalSize().w;
        int y = rand() % nApp->logicalSize().h;

         sElement* particle = nElem->create(&(sIni){
            .display = nElem->display->RELATIVE,
            .body = nElem->body->DYNAMIC,
            .backgroundColor = "White",
            .position = &(sRect){x, y, 2, 2},
            .velocity = {velx, vely},
        });
        nElem->body->setElasticity(particle, 1.0);
    }
}


int main(int argc, char** argv){
    (void) argv[argc];

    nApp->create(&(sIni){
        .window = "Landscape|360",
        .onLoad = onLoad,
    });

    nApp->run();
    const char* const text = "Hello World";

    return 0;
}
