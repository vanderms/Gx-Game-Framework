#include "../Utilities/Util.h"
#include "../App/App.h"
#include "../Scene/Scene.h"
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "../Map/Map.h"
#include "../Folder/Folder.h"
#include "../Array/Array.h"
#include "../List/List.h"

#ifdef NDEBUG
    #define GX_DEV 0
#else
    #define GX_DEV 1
#endif

//... #forward declarations
static inline sMap* createColorMap(void);
static sMap* createFontMap(void);


//struct
enum AType { IMAGE, MUSIC, SOUND };

typedef struct sApp {

    //...
    int status;    
    sSize size;

    //...SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

    //... Scenes and Folders
    sScene* snMain;
    sScene* snActive;
    sScene* snRunning;

    //... containers
	sMap* scenes;
    sMap* folders;
    sMap* colors;
    sMap* fonts;

    //... temporary resources
    sArray* temporary;

    //...assets modules
    sList* aToLoad;
    sList* aLoading;
    sList* aLoaded;
    uint32_t counter;
    SDL_atomic_t atom;
} sApp;

typedef struct Asset {
    enum AType type;
    void* mod;
    void* resource;
    char* path;
} Asset;

static inline void destroyAsset(Asset* self) {
    if (self) {
        free(self->path);
        free(self);
    }
}


//static instance
static sApp* self = NULL;

//constructor and destructor
static sScene* create(const sIni* ini) {

    if(self){ return self->snMain; }
    
    self = calloc(1, sizeof(sApp));
    nUtil->assertAlloc(self);
    self->snActive = NULL;
    self->folders = nMap->create();
    self->scenes = nMap->create();
    self->colors = createColorMap();
    self->fonts = createFontMap();
    self->temporary = nArray->create();
    self->aToLoad = nList->create();
    self->aLoading = NULL;
    self->aLoaded = NULL;

    SDL_AtomicSet(&self->atom, GxStatusNone);

   
	  //init SDL modules
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
       nApp->runtimeError(SDL_GetError());
    }

    if (TTF_Init() != 0) {
         nApp->runtimeError(SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
        nApp->runtimeError(SDL_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        nApp->runtimeError(SDL_GetError());
    }

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    Uint32 bigger = mode.w > mode.h ? mode.w : mode.h;
    Uint32 smaller = mode.w > mode.h ? mode.h : mode.w;

    nUtil->assertArgument(ini->window);
    sArray* wparams = nUtil->split(ini->window, "|");
    nUtil->assertArgument(nArray->size(wparams) == 2);

    if(strstr(ini->window, "Landscape")){
	    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        self->size.h = atoi(nArray->at(wparams, 1));
        nUtil->assertArgument(self->size.h);
        self->size.w = ((double) bigger * self->size.h) / smaller;       
    }
    else if (strstr(ini->window, "Portrait")) {
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
        self->size.w = atoi(nArray->at(wparams, 1));
        nUtil->assertArgument(self->size.w);
        self->size.h = ((double) bigger * self->size.w) / smaller;       
    }
    nArray->destroy(wparams);

    const char* title = ini->title ? ini->title : "Gx";
    Uint32 flags = SDL_WINDOW_RESIZABLE;

    if (!(strcmp(SDL_GetPlatform(), "Windows") == 0 && GX_DEV) ||
        ( strcmp(SDL_GetPlatform(), "Android") == 0)) {             
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }   
      
    if (!(self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, self->size.w, self->size.h, flags))) {
        nApp->runtimeError(SDL_GetError());
    }

    if (!(self->renderer = SDL_CreateRenderer(self->window, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED))) {
         nApp->runtimeError(SDL_GetError());
    }

    //present window
    SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
    SDL_RenderClear(self->renderer);
    SDL_RenderPresent(self->renderer);  
   
    SDL_SetRenderDrawBlendMode(self->renderer, SDL_BLENDMODE_BLEND);

    self->status = GxStatusNone;
    self->snMain = GxCreateScene(ini);
    return self->snMain;
}



static SDL_Rect* calcDest(SDL_Rect* src, SDL_Rect* dest) {
#define intround(x) ((x) >= 0.0 ? (int) ((x) + 0.5) : (int) ((x) - 0.5))
    sSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nApp->SDLRenderer(), &wsize.w, &wsize.h);
    if (wsize.w == self->size.w) {
        *dest = *src;            
    }
    else {
        dest->x = intround(((double)src->x * wsize.w) / self->size.w);       
        dest->y = intround(((double)src->y * wsize.h) / self->size.h);        
        int x1 = intround(((double)(src->x + src->w) * wsize.w) / self->size.w); 
        int y1 = intround(((double)(src->y + src->h) * wsize.h) / self->size.h);
        dest->w = x1 - dest->x;
        dest->h = y1 - dest->y;
    }
    return dest;
#undef intround
}

static SDL_Rect* calcLabelDest(SDL_Rect* src, SDL_Rect* dest) {
    calcDest(src, dest);
    SDL_Point center = {dest->x + dest->w/2, dest->y + dest->h/2};
    dest->x = center.x - src->w/2;
    dest->y = center.y - src->h/2;
    dest->w = src->w;
    dest->h = src->h;
    return dest;
}

static bool isCreated() {
    return self;
}

static void destroy() {
    if (self) {
        SDL_DestroyRenderer(self->renderer);
        SDL_DestroyWindow(self->window);
        nList->destroy(self->aToLoad);
        nList->destroy(self->aLoading);
        nList->destroy(self->aLoaded);
        nMap->destroy(self->scenes);
        GxDestroyScene_(self->snMain);
        nArray->destroy(self->temporary);
        nMap->destroy(self->folders);
        nMap->destroy(self->colors);
        nMap->destroy(self->fonts);
        free(self);
        self = NULL;
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
    }
}

//acessors and mutators
static SDL_Window* SDLWindow() {
    return self->window;
}

static SDL_Renderer* SDLRenderer() {
    return self->renderer;
}

static sSize logicalSize() {
    return self->size;
}

static bool isRunning() {
    return self->status == GxStatusRunning;
}

static void addScene(sScene* scene) {
    if(self->snMain != NULL){
        nMap->set(self->scenes, GxSceneGetName(scene), scene, GxDestroyScene_);
    }
}

static sScene* getScene(const char* id) {
    return nMap->get(self->scenes, id);
}

static void addFolder(sFolder* folder) {   
   nMap->set(self->folders, nFolder->p->id(folder), folder, nFolder->p->destroy);
}

static sFolder* getFolder(const char* id) {
    return nMap->get(self->folders, id);
}

static void loadSDLSurface(sImage* image, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    nUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = IMAGE,
        .mod = image,
        .path = nUtil->createString(path)
    };
    nList->push(self->aToLoad, asset, (sDtor) destroyAsset);
}

static void loadMixChunk(sChunk* sound, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    nUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = SOUND,
        .mod = sound,
        .path = nUtil->createString(path)
    };
    nList->push(self->aToLoad, asset, (sDtor)  destroyAsset);
}

static void loadMixMusic(sMusic* music, const char* path) {
     Asset* asset = malloc(sizeof(Asset));
     nUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = MUSIC,
        .mod = music,
        .path = nUtil->createString(path)
    };
    nList->push(self->aToLoad, asset, (sDtor) destroyAsset);
}


static void mainLoadAsset(Asset* asset) {
    if (asset) {
        switch (asset->type) {
            case IMAGE: {
                SDL_Surface* surface = asset->resource;
                sSize size = { surface->w,  surface->h };
                SDL_Texture* texture = SDL_CreateTextureFromSurface(
                    self->renderer, surface
                );
                SDL_FreeSurface(surface);
                if (!texture) nApp->runtimeError(SDL_GetError());
                nFolder->p->setSDLTexture(asset->mod, texture, &size);
                break;
            }
            case SOUND: {
                nFolder->p->setMixChunk(asset->mod, asset->resource);
                break;
            }
            case MUSIC: {
                nFolder->p->setMixMusic(asset->mod, asset->resource);
                break;
            }
        }
    }
}

static int threadLoadAsset() {

    for(Asset* asset = nList->begin(self->aLoading); asset != NULL;
        asset = nList->next(self->aLoading)){

        if (asset) {
            switch (asset->type) {
                case IMAGE: {
                    asset->resource = IMG_Load(asset->path);
                    break;
                }
                case SOUND: {
                    asset->resource = Mix_LoadWAV(asset->path);
                    break;
                }
                case MUSIC: {
                    asset->resource = Mix_LoadMUS(asset->path);
                    break;
                }
            }
        }
        if (!asset->resource) nApp->runtimeError(SDL_GetError());
    }

    SDL_AtomicSet(&self->atom, GxStatusLoaded);
    return 0;
}


static void run() {
    nUtil->assertState(self);
    self->counter = SDL_GetTicks();
    
    //run
    self->status = GxStatusRunning;
    nApp->loadScene(self->snMain);

    while (self->status == GxStatusRunning) {

        bool activeIsReady =  self->snActive ?
            GxSceneGetStatus(self->snActive) == GxStatusRunning : false;

        self->counter = SDL_GetTicks();
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) GxSceneOnLoopBegin_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnLoopBegin_(self->snMain);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) self->status = GxStatusUnloading;

            self->snRunning = self->snActive;
            if (activeIsReady && self->snActive) GxSceneOnSDLEvent_(self->snActive, &e);

            self->snRunning = self->snMain;
            GxSceneOnSDLEvent_(self->snMain, &e);
        }

        //... update
        self->snRunning = self->snActive;
        if (self->snActive) GxSceneOnUpdate_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnUpdate_(self->snMain);


        //...load assets
        if (self->aLoaded == NULL && SDL_AtomicGet(&self->atom) == GxStatusLoaded) {
            self->aLoaded = self->aLoading;
            self->aLoading = NULL;
            SDL_AtomicSet(&self->atom, GxStatusNone);
        }

        if (nList->size(self->aToLoad) && SDL_AtomicGet(&self->atom) == GxStatusNone) {
            self->aLoading = self->aToLoad;
            self->aToLoad = nList->create();
            SDL_AtomicSet(&self->atom, GxStatusLoading);
            SDL_Thread* thread = SDL_CreateThread((SDL_ThreadFunction) threadLoadAsset, "loaderThread", self);
            SDL_DetachThread(thread);
        }

        if (self->aLoaded) {
            do {
                Asset* asset = nList->first(self->aLoaded);
                mainLoadAsset(asset);
                nList->remove(self->aLoaded, asset);
            } while(nList->size(self->aLoaded) && (SDL_GetTicks() - self->counter <= 12));

            if(nList->size(self->aLoaded) == 0){
                nList->destroy(self->aLoaded);
                self->aLoaded = NULL;
            }
        }
        
        {
        /* 
            Correct android screen initialization: without this block, in Android
            the first screen shows strange colored images.
            It's not clear the origin of the bug, if it occurs just in Android or if it occurs just in my old cellphone.
        */
            static int firstScreen = 0;
            if(firstScreen < 30){
                SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
                SDL_RenderClear(self->renderer);
                firstScreen++;
            }
        }

        //... present
        SDL_RenderPresent(self->renderer);

        //call loop end handlers
        self->snRunning = self->snActive;
        if (activeIsReady && self->snActive) GxSceneOnLoopEnd_(self->snActive);

        self->snRunning = self->snMain;
        GxSceneOnLoopEnd_(self->snMain);
        nArray->clean(self->temporary);

         //clear window
        SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
        SDL_RenderClear(self->renderer);
    }
    destroy();
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
}

static void loadScene(sScene* scene) {

    if (scene == self->snActive) {
        return;
    }

    self->snRunning = scene;
    GxScenePreLoad_(scene);

    if (scene != self->snMain){
        if(self->snActive){
            self->snRunning = self->snActive;
            GxSceneUnload_(self->snActive);
        }
        self->snActive = scene;
    }
}

static sScene* getRunningScene() {
    return self->snRunning;
}

static sScene* getMainScene(void) {
    return self->snMain;
}


static void alert(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", message, NULL);
}

 static void runtimeError(const char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", message, NULL);
#ifndef NDEBUG
    SDL_TriggerBreakpoint();
#endif
	exit(EXIT_FAILURE);
}

 static void playMusic(const char* path, int loops) {
    Mix_Music* music = nFolder->getMixMusic(path);
    Mix_PlayMusic(music, loops);
 }

static void playChunk(const char* path, int loops) {
    Mix_Chunk* chunk = nFolder->getMixChunk(path);
    Mix_PlayChannel(-1, chunk, loops);
 }

static void createColor(sMap* map, const char* name, SDL_Color* color) {
    SDL_Color* ncolor = malloc(sizeof(SDL_Color));
    nUtil->assertAlloc(ncolor);
    *ncolor = *color;
    nMap->set(map, name, ncolor, free);
}

static sMap* createColorMap() {

    sMap* map = nMap->create();

    createColor(map, "Black", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "White", &(SDL_Color){255, 255, 255, 255});
    createColor(map, "Blue", &(SDL_Color){0, 0, 255, 255});
    createColor(map, "Aqua", &(SDL_Color){0, 255, 255, 255});
    createColor(map, "Coral", &(SDL_Color){255, 127, 80, 255});
    createColor(map, "Crimson", &(SDL_Color){220, 20, 60, 255});   
    createColor(map, "FireBrick", &(SDL_Color){178, 34, 34, 255});    
    createColor(map, "Gold", &(SDL_Color){255, 215, 0, 255});
    createColor(map, "Green", &(SDL_Color){0, 128, 0, 255});   
    createColor(map, "Red", &(SDL_Color){255, 0, 0, 255});
    createColor(map, "Salmon", &(SDL_Color){250, 128, 114, 255});  
    createColor(map, "Teal", &(SDL_Color){0, 128, 128, 255});
    createColor(map, "Tomato", &(SDL_Color){255, 99, 71, 255});
    createColor(map, "Yellow", &(SDL_Color){0, 0, 0, 255});
    return map;
}

static void convertColor(SDL_Color* destination, const char* color) {

    SDL_Color* mapcolor = NULL;
    char* clone = nUtil->cloneString(color, (char[32]){0}, 32);
    clone = nUtil->trim(clone, (char[32]){'\0'}, 32);
    int len = (int) strlen(clone);

	if(clone[0] == '(' && clone[len - 1] == ')'){

		//remove parenthesis
		(clone++)[len - 1] = '\0';

		//get tokens
        sArray* tokens = nApp->tokenize(clone, ",");
        nUtil->assertArgument(nArray->size(tokens) == 4);

        char* r = nArray->at(tokens, 0);
        char* g = nArray->at(tokens, 1);
        char* b = nArray->at(tokens, 2);
        char* a = nArray->at(tokens, 3);

		//assign color components
		destination->r = (Uint8) atoi(r);
		destination->g = (Uint8) atoi(g);
		destination->b = (Uint8) atoi(b);
		destination->a = (Uint8) atoi(a);
	}
	else if((mapcolor = nMap->get(self->colors, color))){
        *destination = *mapcolor;
	}
    else {
        nUtil->assertArgument(false);
    }
}


static sMap* createFontMap(void) {
    sMap* fonts = nMap->create();
#define FPATH "Gx/Font/PTSerif/PTSerif-"
    nMap->set(fonts, "Default", nUtil->createString(FPATH "Regular.ttf"), free);
    nMap->set(fonts, "Italic", nUtil->createString(FPATH "Italic.ttf"), free);
    nMap->set(fonts, "Bold", nUtil->createString(FPATH "Bold.ttf"), free);
    nMap->set(fonts, "BoldItalic", nUtil->createString(FPATH "BoldItalic.ttf"), free);
#undef FPATH
    return fonts;
}

static void addFont(const char* name, const char* path){
   nUtil->assertNullPointer(name);
   nUtil->assertNullPointer(path);
    TTF_Font* teste = TTF_OpenFont(path, 16);
    if (!teste) {
        nApp->runtimeError(nApp->sf("Could not open path %s", path));
    }
    TTF_CloseFont(teste);
    nMap->set(self->fonts, name, nUtil->createString(path), free);    
}

static const char* getFontPath(const char* name) {
    return nMap->get(self->fonts, name);
}


//... TEMPORARY RESOURCES FUNCTIONS

static char* sf(const char* format, ...) {   
    static char buffer[1024]; //1kb
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
    char* value = nUtil->createString(buffer);
    nArray->push(self->temporary, value, free);
	return value;
}

static sArray* tokenize(const char* str, const char* sep){
    sArray* response = nUtil->split(str, sep);
    nArray->push(self->temporary, response, nArray->destroy);
    return response;
}


const sAppNamespace* nApp = &(sAppNamespace) { 
	.create = create,
	.run = run,
    .isCreated = isCreated,
    .isRunning = isRunning,
	.SDLWindow = SDLWindow,
    .SDLRenderer = SDLRenderer,
	.getScene = getScene,
	.logicalSize = logicalSize,
	.loadScene = loadScene,
	.addFont = addFont,
	.getRunningScene = getRunningScene,
	.getMainScene = getMainScene,
	.alert = alert,
	.runtimeError = runtimeError,
	.playMusic = playMusic,
	.playChunk = playChunk,
	.stopMusic = Mix_HaltMusic,
	.pauseMusic = Mix_PauseMusic,
	.resumeMusic = Mix_ResumeMusic,
	.isPlayingMusic = Mix_PlayingMusic,
	.convertColor = convertColor,	
	.tokenize = tokenize,
    .sf = sf,
    .calcDest = calcDest,
    .calcLabelDest = calcLabelDest,
    .prv = &(struct sAppPrivate) {
		.addScene = addScene,
		.addFolder = addFolder,
		.getFolder = getFolder,
		.getFontPath = getFontPath,
		.loadSDLSurface = loadSDLSurface,
		.loadMixChunk = loadMixChunk,
		.loadMixMusic = loadMixMusic,
	},
};


