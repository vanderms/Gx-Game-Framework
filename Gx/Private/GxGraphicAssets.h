#include "../Utilities/GxUtil.h"

typedef enum ImageType{
	Fill,
	Draw,
	Texture,
	Text,
	Palette,
	Opaque,
	Blank,
} ImageType;

typedef struct GxImage {

    char* id;
    GxFolder* folder;
	ImageType type;
    GxSize size;

    //texture
    SDL_Texture* resource;
    SDL_Rect* src;
    double proportion;

    //opaque
    GxImage* source;

     //palette
    GxArray* children;
    GxMatrix matrix;
} GxImage;


typedef struct GxAnimation {
    char* id;
    GxArray* images;
    Uint32 interval;
    Uint32 quantity;
    bool continuous;
} GxAnimation;