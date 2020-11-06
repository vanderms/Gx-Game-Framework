#ifndef GX_APP_H
#define GX_APP_H
#include "../Utilities/GxUtil.h"
#include <SDL_ttf.h>

//forward declaration
typedef struct GxApp GxApp;
typedef struct GxIni GxIni;

//constructor, destructor, interface
GxScene* GxCreateApp(const GxIni* ini);

//acessors and mutators
bool GxAppIsCreated_(void);
SDL_Window* GxGetSDLWindow(void);
SDL_Renderer* GxGetSDLRenderer(void);
void GxAddScene_(GxScene* scene);
GxScene* GxGetScene(const char* id);
void GxAddFolder_(GxFolder* handler);
GxFolder* GxGetFolder_(const char* id);
GxSize GxGetWindowSize(void);
bool GxAppIsRunning_(void);
SDL_Rect* GxAppCalcDest(SDL_Rect* src, SDL_Rect* dest);
SDL_Rect* GxAppCalcLabelDest(SDL_Rect* src, SDL_Rect* dest);
//...
void GxLoadScene(GxScene* scene);
GxScene* GxGetRunningScene(void);
GxScene* GxGetMainScene(void);
void GxAppRun(void);

void GxAddFont(const char* name, const char* path);
const char* GxGetFontPath_(const char* name);

//... Folder functions
void GxPushTextureToLoad_(GxImage* image, const char* path);
void GxPushChunkToLoad_(GxSound* sound, const char* path);
void gxPushMusicToLoad_(GxMusic* music, const char* path);

//... Error handling
void GxAlert(const char* message);
void GxFatalError(const char* message);

//audio
void GxPlayMusic(const char* path, int loops);
void GxPlayChunk(const char* path, int loops);

//... Utilities
void GxConvertColor(SDL_Color* destination, const char* color);
char* GxF(const char* format, ...);
int* GxI(int value);
double* GxD(double value);
bool* GxB(bool value);
Uint32* GxU(Uint32 value);
GxArray* GxTokenize(const char* str, const char* sep);

//...Data buffers
GxData* GxDataI(const int value);
GxData* GxDataU(const Uint32 value);
GxData* GxDataF(const double value);
GxData* GxDataB(const bool value);
GxData* GxDataC(const char value);
GxData* GxDataSF(const char* format, ...);
GxData* GxDataPtr(const void* value);
GxData* GxDataList(void);
GxData* GxDataArray(void);
GxData* GxDataMap(void);
GxData* GxDataRect(const SDL_Rect* rect);
GxData* GxDataVector(const GxVector* vector);
GxData* GxDataPoint(const SDL_Point* point);
GxData* GxDataSize(const GxSize* size);
GxData* GxDataMatrix(const GxMatrix* matrix);
#endif // !GX_APP_H
