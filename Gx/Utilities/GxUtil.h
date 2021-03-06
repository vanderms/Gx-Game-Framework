#ifndef UTILITIES_H
#define UTILITIES_H
#include "../Public/GxPublic.h"


int* GxUtilCreateInt(int value);
Uint32* GxUtilCreateUint(Uint32 value);
bool* GxUtilCreateBool(bool value);
double* GxUtilCreateDouble(double value);

char* GmCreateString(const char* value);
char* GmCreateStringF(const char* format, ...);
char* GxCloneString(const char* str, char* buffer, unsigned int size);
char* GxF(const char* format, ...);
GxArray* GmArraySplit(const char* str, const char* sep);
char* GxTrim(const char* str, char* buffer, size_t bSize);

int GxAbs(int value);

static inline int GxRandom(uint32_t* seed, int start, int end) {	
	*seed = *seed * 1103515245 + 12345;
	uint32_t partial = *seed % (uint32_t)(end - start + 1);
	int response = (int) partial + start;
	return response;
}

void GxPrintMask(Uint32 mask);

typedef enum GxHash {
	GxHashElement_ = 1111251919,	
	GxHashScene_ = 2066184690,
	GxHashContact_ = 804125936,
} GxHash;


SDL_Point GxCalcDistance(const SDL_Point* pointA, const SDL_Point* pointB);

//...ASSERTIONS
bool GxAssertNullPointer(const void* ptr); 
bool GxAssertInvalidArgument(bool condition);
void* GxAssertAllocationFailure(void* ptr);
bool GxAssertNotImplemented(bool condition);
bool GxAssertInvalidHash(bool condition);
bool GxAssertOutOfRange(bool condition);
bool GxAssertInvalidOperation(bool condition);
bool GxAssertResourceNotFound(bool condition);

bool GxUserAssertNullPointer(const void* ptr);
bool GxUserAssertArgumnent(bool condition);
bool GxUserAssertState(bool condition);
void* GxUserAssertAlloc(void* pointer);
bool GxUserAssertOutOfRange(bool condition);

#endif // !UTILITIES_H
