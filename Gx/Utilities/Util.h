#ifndef GX_UTIL_H
#define GX_UTIL_H
#include "../Public/GxPublic.h"

typedef struct sUtilNamespace {	
	int* (*createInt)(int value);
	Uint32* (*createUint)(Uint32 value);
	bool* (*createBool)(bool value);
	double* (*createDouble)(double value);
	char* (*createString)(const char* value);
	char* (*createStringF)(const char* format, ...);
	char* (*cloneString)(const char* str, char* buffer, unsigned int size);	
	sArray* (*split)(const char* str, const char* sep);
	char* (*trim)(const char* str, char* buffer, size_t bSize);
	int (*abs)(int value);
	int (*random)(uint32_t* seed, int start, int end);
	void (*printMask)(Uint32 mask);
	SDL_Point (*calcDistance)(const SDL_Point* pointA, const SDL_Point* pointB);
	bool (*assertNullPointer)(const void* ptr); 
	bool (*assertArgument)(bool condition);
	bool (*assertState)(bool condition);
	void* (*assertAlloc)(void* ptr);
	bool (*assertResourceNotFound)(bool condition);
	bool (*assertImplementation)(bool condition);
	bool (*assertHash)(bool condition);
	bool (*assertOutOfRange)(bool condition);
	void (*onDestroyFreeTarget)(GxEvent* e);
	void(*onDestroyDoNothing)(GxEvent* e);
	void (*splitAssetPath)(const char* path, char* folder, char* asset);
	struct sUtilHash {
		const Uint32 ELEMENT;
		const Uint32 SCENE;
		const Uint32 CONTACT;
	}* hash;
} sUtilNamespace;

extern const sUtilNamespace* nsUtil;

#endif // !UTILITIES_H
