#include "Gx/Gx.h"

static void onUpdate(Event* e){
    Element* btn = e->target;
    if(button->hasStatus(btn, button->ON)){
        elem->setBorder(btn, "2|White");
    }
    else{
        elem->setBorder(btn, NULL);
    }
}


static void onLoad(Event* e){


    button->create(&(Ini){
        .display = elem->ABSOLUTE,
        .body = elem->NONE,
        .position = &(Rect){270, 150, 100, 60},
        .text = "Gx!",
        .color = "White",
        .fontSize = 32,
        .onUpdate = onUpdate,
    }, button->KEYBOARD | button->FINGER | button->MOUSE, SDLK_UP);
}

int main(int argc, char** argv){

    app->create(&(Ini){
        .window = "Landscape|360",
        .onLoad = onLoad,
    });

    app->run();

    return 0;
}
