#ifndef GX_MODULE_H
#define GX_MODULE_H
#include "../Util/Util.h"


extern const struct sFolderNamespace {

    void (*create)(const char* id, void(*loader)(void));

    Mix_Music* (*getMixMusic)(const char* path);

    Mix_Chunk* (*getMixChunk)(const char* path);

    sImage* (*getImage)(sFolder* self, const char* id);

    sAnimation* (*getAnimation)(sFolder* self, const char* id);

    void (*loadImage)(const char* id, const char* path, sRect* src, double proportion);

    void (*loadTileset)(const char* id, const char* pathF, int start, int end, double proportion);

    void (*createTilesetFromImage)(const char* image, sSize size, sMatrix matrix);

    void (*loadAnimation)(const char* id, const char* pathF, int start, int end, int interval, double proportion, bool continuous);

    void (*loadChunk)(const char* id, const char* path);

    void (*loadMusic)(const char* id, const char* path);

    const struct sImageNamespace {
        const char* (*name)(sImage* self);
        sFolder* (*folder)(sImage* self);
        const sSize* (*size)(sImage* self);
        SDL_Texture* (*SDLTexture)(sImage* self);
        const sRect* (*src)(sImage* self);
        double (*proportion)(sImage* self);
        sImage* (*createText)(
            const char* text, const char* font, int size, SDL_Color* color
        );
        void (*render)(sImage* self, 
            sRect* target, double angle, SDL_RendererFlip orientation, Uint8 opacity
        );
        void (*destroyText)(sImage* self);
    }* const img;


    const struct sAnimationNamespace {
        const char* (*name)(sAnimation* self);
        Uint32 (*interval)(sAnimation* self);
        Uint32 (*quantity)(sAnimation* self);
        bool (*isContinuous)(sAnimation* self);
        sImage* (*getImage)(sAnimation* self, Uint32 index);
    }* const anim;



    const struct sFolderPrivateNamespace {

        void (*destroy)(sFolder* self);
        bool(*hasStatus)(sFolder* self, int status);
        char* (*id)(sFolder* self);
        int (*getPercLoaded)(sFolder* self);
        void (*incRefCounter)(sFolder* self);
        void (*decRefCounter)(sFolder* self);

        void (*setMixChunk)(sChunk* self, Mix_Chunk* chunk);
        void (*setMixMusic)(sMusic* self, Mix_Music* music);
        void (*setSDLTexture)(sImage* self, void* resource, sSize* size);      

    }* p_;
}* const nFolder;

#endif // !GX_MODULE_H
