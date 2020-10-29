#include "Gx/Gx.h"


static void onLoad(Event* e){

    Size size = app->getWindowSize();

    elem->create(&(Ini){
        .modules = DisplayAbsolute | BodyNone,
        .position = &(Rect){0, 0, size.w, size.h},
        .text = "Hello Gx!",
        .color = "White",
        .fontSize = 32
    });
}

int main(int argc, char** argv){

    app->create(&(Ini){
        .onLoad = onLoad
    });

    return 0;
}
