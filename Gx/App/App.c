#include "../Utilities/Util.h"
#include "../App/App.h"
#include "../Ini/GxIni.h"
#include "../Scene/GxScene.h"
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include "../Map/GxMap.h"
#include "../Folder/GxFolder.h"
#include "../Array/Array.h"
#include "../List/GxList.h"

#ifdef NDEBUG
    #define GX_DEV 0
#else
    #define GX_DEV 1
#endif

//... #forward declarations
static inline GxMap* createColorMap(void);
static GxMap* createFontMap(void);


//struct
enum AType { IMAGE, MUSIC, SOUND };

typedef struct sApp {

    //...
    int status;    
    GxSize size;

    //...SDL
	SDL_Window* window;
	SDL_Renderer* renderer;

    //... Scenes and Folders
    GxScene* snMain;
    GxScene* snActive;
    GxScene* snRunning;

    //... containers
	GxMap* scenes;
    GxMap* folders;
    GxMap* colors;
    GxMap* fonts;

    //... temporary resources
    sArray* temporary;

    //...assets modules
    GxList* aToLoad;
    GxList* aLoading;
    GxList* aLoaded;
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
static GxScene* create(const GxIni* ini) {

    if(self){ return self->snMain; }
    
    self = calloc(1, sizeof(sApp));
    nsUtil->assertAlloc(self);
    self->snActive = NULL;
    self->folders = GmCreateMap();
    self->scenes = GmCreateMap();
    self->colors = createColorMap();
    self->fonts = createFontMap();
    self->temporary = nsArr->create();
    self->aToLoad = GxCreateList();
    self->aLoading = NULL;
    self->aLoaded = NULL;

    SDL_AtomicSet(&self->atom, GxStatusNone);

   
	  //init SDL modules
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
       nsApp->runtimeError(SDL_GetError());
    }

    if (TTF_Init() != 0) {
         nsApp->runtimeError(SDL_GetError());
    }

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
        nsApp->runtimeError(SDL_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        nsApp->runtimeError(SDL_GetError());
    }

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    Uint32 bigger = mode.w > mode.h ? mode.w : mode.h;
    Uint32 smaller = mode.w > mode.h ? mode.h : mode.w;

    nsUtil->assertArgument(ini->window);
    sArray* wparams = nsUtil->split(ini->window, "|");
    nsUtil->assertArgument(nsArr->size(wparams) == 2);

    if(strstr(ini->window, "Landscape")){
	    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        self->size.h = atoi(nsArr->at(wparams, 1));
        nsUtil->assertArgument(self->size.h);
        self->size.w = ((double) bigger * self->size.h) / smaller;       
    }
    else if (strstr(ini->window, "Portrait")) {
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
        self->size.w = atoi(nsArr->at(wparams, 1));
        nsUtil->assertArgument(self->size.w);
        self->size.h = ((double) bigger * self->size.w) / smaller;       
    }
    nsArr->destroy(wparams);

    const char* title = ini->title ? ini->title : "Gx";
    Uint32 flags = SDL_WINDOW_RESIZABLE;

    if (!(strcmp(SDL_GetPlatform(), "Windows") == 0 && GX_DEV) ||
        ( strcmp(SDL_GetPlatform(), "Android") == 0)) {             
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }   
      
    if (!(self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, self->size.w, self->size.h, flags))) {
        nsApp->runtimeError(SDL_GetError());
    }

    if (!(self->renderer = SDL_CreateRenderer(self->window, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED))) {
         nsApp->runtimeError(SDL_GetError());
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
    GxSize wsize = {0, 0};
    SDL_GetRendererOutputSize(nsApp->SDLRenderer(), &wsize.w, &wsize.h);
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
        GxDestroyList(self->aToLoad);
        GxDestroyList(self->aLoading);
        GxDestroyList(self->aLoaded);
        GxDestroyMap(self->scenes);
        GxDestroyScene_(self->snMain);
        nsArr->destroy(self->temporary);
        GxDestroyMap(self->folders);
        GxDestroyMap(self->colors);
        GxDestroyMap(self->fonts);
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

static GxSize logicalSize() {
    return self->size;
}

static bool isRunning() {
    return self->status == GxStatusRunning;
}

static void addScene(GxScene* scene) {
    if(self->snMain != NULL){
        GxMapSet(self->scenes, GxSceneGetName(scene), scene, (GxDestructor) GxDestroyScene_);
    }
}

static GxScene* getScene(const char* id) {
    return GxMapGet(self->scenes, id);
}

static void addFolder(GxFolder* handler) {
   void GxDestroyFolder_(GxFolder* self);
   GxMapSet(self->folders, GxFolderGetId_(handler), handler, (GxDestructor) GxDestroyFolder_);
}

static GxFolder* getFolder(const char* id) {
    return GxMapGet(self->folders, id);
}

static void loadSDLSurface(GxImage* image, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    nsUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = IMAGE,
        .mod = image,
        .path = nsUtil->createString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor) destroyAsset);
}

static void loadMixChunk(GxSound* sound, const char* path) {
    Asset* asset = malloc(sizeof(Asset));
    nsUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = SOUND,
        .mod = sound,
        .path = nsUtil->createString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor)  destroyAsset);
}

static void loadMixMusic(GxMusic* music, const char* path) {
     Asset* asset = malloc(sizeof(Asset));
     nsUtil->assertAlloc(asset);
    *asset = (Asset) {
        .type = MUSIC,
        .mod = music,
        .path = nsUtil->createString(path)
    };
    GxListPush(self->aToLoad, asset, (GxDestructor) destroyAsset);
}


static void mainLoadAsset(Asset* asset) {
    if (asset) {
        switch (asset->type) {
            case IMAGE: {
                SDL_Surface* surface = asset->resource;
                GxSize size = { surface->w,  surface->h };
                SDL_Texture* texture = SDL_CreateTextureFromSurface(
                    self->renderer, surface
                );
                SDL_FreeSurface(surface);
                if (!texture) nsApp->runtimeError(SDL_GetError());
                GxImageTextureSetResource_(asset->mod, texture, &size);
                break;
            }
            case SOUND: {
                GxSoundSetChunk_(asset->mod, asset->resource);
                break;
            }
            case MUSIC: {
                GxMusicSetMixMusic_(asset->mod, asset->resource);
                break;
            }
        }
    }
}

static int threadLoadAsset() {

    for(Asset* asset = GxListBegin(self->aLoading); asset != NULL;
        asset = GxListNext(self->aLoading)){

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
        if (!asset->resource) nsApp->runtimeError(SDL_GetError());
    }

    SDL_AtomicSet(&self->atom, GxStatusLoaded);
    return 0;
}


static void run() {
    nsUtil->assertState(self);
    self->counter = SDL_GetTicks();
    
    //run
    self->status = GxStatusRunning;
    nsApp->loadScene(self->snMain);

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

        if (GxListSize(self->aToLoad) && SDL_AtomicGet(&self->atom) == GxStatusNone) {
            self->aLoading = self->aToLoad;
            self->aToLoad = GxCreateList();
            SDL_AtomicSet(&self->atom, GxStatusLoading);
            SDL_Thread* thread = SDL_CreateThread((SDL_ThreadFunction) threadLoadAsset, "loaderThread", self);
            SDL_DetachThread(thread);
        }

        if (self->aLoaded) {
            do {
                Asset* asset = GxListFirst(self->aLoaded);
                mainLoadAsset(asset);
                GxListRemove(self->aLoaded, asset);
            } while(GxListSize(self->aLoaded) && (SDL_GetTicks() - self->counter <= 12));

            if(GxListSize(self->aLoaded) == 0){
                GxDestroyList(self->aLoaded);
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
        nsArr->clean(self->temporary);

         //clear window
        SDL_SetRenderDrawColor(self->renderer, 0, 0, 0, 255);
        SDL_RenderClear(self->renderer);
    }
    destroy();
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
}

static void loadScene(GxScene* scene) {

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

static GxScene* getRunningScene() {
    return self->snRunning;
}

static GxScene* getMainScene(void) {
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
    Mix_Music* music = GxFolderGetMusic(path);
    Mix_PlayMusic(music, loops);
 }

static void playChunk(const char* path, int loops) {
    Mix_Chunk* chunk = GxFolderGetChunk(path);
    Mix_PlayChannel(-1, chunk, loops);
 }

static void createColor(GxMap* map, const char* name, SDL_Color* color) {
    SDL_Color* ncolor = malloc(sizeof(SDL_Color));
    nsUtil->assertAlloc(ncolor);
    *ncolor = *color;
    GxMapSet(map, name, ncolor, free);
}

static GxMap* createColorMap() {

    GxMap* map = GmCreateMap();

    createColor(map, "Black", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "White", &(SDL_Color){255, 255, 255, 255});
    createColor(map, "Blue", &(SDL_Color){0, 0, 255, 255});
    createColor(map, "Aqua", &(SDL_Color){0, 255, 255, 255});
    createColor(map, "Coral", &(SDL_Color){255, 127, 80, 255});
    createColor(map, "Crimson", &(SDL_Color){220, 20, 60, 255});
    createColor(map, "DarkBlue", &(SDL_Color){0, 0, 139, 255});
    createColor(map, "DarkGreen", &(SDL_Color){0, 100, 0, 255});
    createColor(map, "DimGrey", &(SDL_Color){105, 105, 105, 255});
    createColor(map, "FireBrick", &(SDL_Color){178, 34, 34, 255});
    createColor(map, "ForestGreen", &(SDL_Color){34, 139, 34, 255});
    createColor(map, "Gold", &(SDL_Color){255, 215, 0, 255});
    createColor(map, "Green", &(SDL_Color){0, 128, 0, 255});
    createColor(map, "IndianRed", &(SDL_Color){205, 92, 92, 255});
    createColor(map, "LimeGreen", &(SDL_Color){50, 205, 50, 255});
    createColor(map, "OrangeRed", &(SDL_Color){255, 69, 0, 255});
    createColor(map, "Red", &(SDL_Color){255, 0, 0, 255});
    createColor(map, "Salmon", &(SDL_Color){250, 128, 114, 255});
    createColor(map, "SeaGreen", &(SDL_Color){46, 139, 87, 255});
    createColor(map, "SteelBlue", &(SDL_Color){70, 130, 180, 255});
    createColor(map, "Teal", &(SDL_Color){0, 128, 128, 255});
    createColor(map, "Tomato", &(SDL_Color){255, 99, 71, 255});
    createColor(map, "Yellow", &(SDL_Color){0, 0, 0, 255});
    createColor(map, "YellowGreen", &(SDL_Color){154, 205, 50, 255});

    return map;
}

static void convertColor(SDL_Color* destination, const char* color) {

    SDL_Color* mapcolor = NULL;
    char* clone = nsUtil->cloneString(color, (char[32]){0}, 32);
    clone = nsUtil->trim(clone, (char[32]){'\0'}, 32);
    int len = (int) strlen(clone);

	if(clone[0] == '(' && clone[len - 1] == ')'){

		//remove parenthesis
		(clone++)[len - 1] = '\0';

		//get tokens
        sArray* tokens = nsApp->tokenize(clone, ",");
        nsUtil->assertArgument(nsArr->size(tokens) == 4);

        char* r = nsArr->at(tokens, 0);
        char* g = nsArr->at(tokens, 1);
        char* b = nsArr->at(tokens, 2);
        char* a = nsArr->at(tokens, 3);

		//assign color components
		destination->r = (Uint8) atoi(r);
		destination->g = (Uint8) atoi(g);
		destination->b = (Uint8) atoi(b);
		destination->a = (Uint8) atoi(a);
	}
	else if((mapcolor = GxMapGet(self->colors, color))){
        *destination = *mapcolor;
	}
    else {
        nsUtil->assertArgument(false);
    }
}


static GxMap* createFontMap(void) {
    GxMap* fonts = GmCreateMap();
#define FPATH "Gx/Font/PTSerif/PTSerif-"
    GxMapSet(fonts, "Default", nsUtil->createString(FPATH "Regular.ttf"), free);
    GxMapSet(fonts, "Italic", nsUtil->createString(FPATH "Italic.ttf"), free);
    GxMapSet(fonts, "Bold", nsUtil->createString(FPATH "Bold.ttf"), free);
    GxMapSet(fonts, "BoldItalic", nsUtil->createString(FPATH "BoldItalic.ttf"), free);
#undef FPATH
    return fonts;
}

static void addFont(const char* name, const char* path){
   nsUtil->assertNullPointer(name);
   nsUtil->assertNullPointer(path);
    TTF_Font* teste = TTF_OpenFont(path, 16);
    if (!teste) {
        nsApp->runtimeError(nsApp->sf("Could not open path %s", path));
    }
    TTF_CloseFont(teste);
    GxMapSet(self->fonts, name, nsUtil->createString(path), free);    
}

static const char* getFontPath(const char* name) {
    return GxMapGet(self->fonts, name);
}


//... TEMPORARY RESOURCES FUNCTIONS

static char* sf(const char* format, ...) {   
    static char buffer[1024]; //1kb
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
    char* value = nsUtil->createString(buffer);
    nsArr->push(self->temporary, value, free);
	return value;
}

static sArray* tokenize(const char* str, const char* sep){
    sArray* response = nsUtil->split(str, sep);
    nsArr->push(self->temporary, response, nsArr->destroy);
    return response;
}


const sAppNamespace* nsApp = &(sAppNamespace) { 
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


