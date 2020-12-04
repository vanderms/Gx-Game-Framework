#ifndef GX_APP_H
#define GX_APP_H
#include "../Utilities/Util.h"
#include <SDL_ttf.h>


typedef struct sAppNamespace {
	sScene* (*create)(const sIni* ini);	
	void (*run)(void);
	bool (*isCreated)(void);
	bool (*isRunning)(void);	
	SDL_Window* (*SDLWindow)(void);
	SDL_Renderer* (*SDLRenderer)(void);	
	sScene* (*getScene)(const char* id);
	sSize (*logicalSize)(void);
	void (*loadScene)(sScene* scene);
	void (*addFont)(const char* name, const char* path);
	sScene* (*getRunningScene)(void);
	sScene* (*getMainScene)(void);
	void (*alert)(const char* message);
	void (*runtimeError)(const char* message);
	void (*playMusic)(const char* path, int loops);
	void (*playChunk)(const char* path, int loops);
	int (*stopMusic)(void);
	void (*pauseMusic)(void);
	void (*resumeMusic)(void);
	int (*isPlayingMusic)(void);
	void (*convertColor)(SDL_Color* dest, const char* color);	
	sArray* (*tokenize)(const char* str, const char* sep);	
	char* (*sf)(const char* format, ...);
	SDL_Rect* (*calcDest)(SDL_Rect* src, SDL_Rect* dest);
	SDL_Rect* (*calcLabelDest)(SDL_Rect* src, SDL_Rect* dest);
	struct sAppPrivate {
		void (*addScene)(sScene* scene);
		void (*addFolder)(sFolder* handler);
		sFolder* (*getFolder)(const char* id);
		const char*(*getFontPath)(const char* name);
		void (*loadSDLSurface)(sImage* image, const char* path);
		void(*loadMixChunk)(sChunk* sound, const char* path);
		void (*loadMixMusic)(sMusic* music, const char* path);
	}* prv;
} sAppNamespace;

extern const sAppNamespace* nApp;

#endif // !GX_APP_H
