#include "../Utilities/GxUtil.h"
#include "../Ini/GxIni.h"
#include "../Array/GxArray.h"
#include "../App/GxApp.h"
#include <string.h>
#include <stdio.h>

void GxPrintMask(Uint32 mask) {
    for (Uint32 i = 0; i < 32; i++) {
        if(i != 0 && i % 8 == 0) putchar('.');
        putchar((1u << i) & mask ? '1' : '0');
    }
    putchar('\n');
}


char* GmCreateStringF(const char* format, ...) {
    
    static char buffer[1024]; //1kb
    //...
    va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);     
    char* value = GmCreateString(buffer);    
	return value;
}


int* GxUtilCreateInt(int value) {
	int* self = malloc(sizeof(int));
    *self = value;
    return self;
}

Uint32* GxUtilCreateUint(Uint32 value) {
	Uint32* self = malloc(sizeof(Uint32));
    GxAssertAllocationFailure(self);
    *self = value;
    return self;
}

bool* GxUtilCreateBool(bool value) {
    bool* self = malloc(sizeof(bool));
    *self = value;
    return self;
}

double* GxUtilCreateDouble(double value) {
	double* self = malloc(sizeof(double));
    GxAssertAllocationFailure(self);
    *self = value;
    return self;
}

char* GmCreateString(const char* value) {
    GxAssertNullPointer(value);
	size_t size = strlen(value) + 1;
	char* self = malloc(sizeof(char) * size);
	GxAssertAllocationFailure(self);
	//strncpy is deprecated and strncpy_s does not have portability
	for (size_t i = 0; i < size - 1; i++) self[i] = value[i];
	self[size - 1] = '\0';
	return self;
}

char* GxCloneString(const char* str, char* buffer, unsigned int size) {
    unsigned int i = 0;
    while (i < size - 1 && str[i]){
        buffer[i] = str[i];
        i++;
    }
    buffer[i] = '\0';
    return buffer;    
}

GxArray* GmArraySplit(const char* str, const char* sep) {
    
    GxArray* tokens = GxCreateArray();

    const size_t len = strlen(sep);
    char* next = NULL;
    char* token = GmCreateString(str);
    char* memory = token;

    while ((next = strstr(token, sep))){
        next[0] = '\0';
        GxArrayPush(tokens, GmCreateString(token), free);
        token = next + len;       
    }
    
    GxArrayPush(tokens, GmCreateString(token), free);
    free(memory);
    
    return tokens;
}

char* GxTrim(const char* str, char* buffer, size_t bSize) {    

    size_t len = strlen(str);
    GxAssertInvalidOperation(bSize > len);
    
    size_t start = 0, index = 0, end = len - 1;
    
    while(str[start] == ' '){ start++;}
    while(str[end] == ' '){ end--; }        
   
    while (index < (end - start) + 1) {
        buffer[index] = str[start + index];
        index++;
    } 
	buffer[index] = '\0';
    return buffer;
}

int GxAbs(int value) {
	return value > 0 ? value : - value;
}

bool GxAssertNullPointer(const void* ptr) {
    if (!ptr) {
        GxRuntimeError("In Gx: null pointer error.");
    }
    return ptr;
}

bool GxAssertInvalidArgument(bool condition){
    if (!condition) {
        GxRuntimeError("In Gx: invalid argument error.");
    }    
    return condition;
}

void* GxAssertAllocationFailure(void* ptr){
    if (!ptr) {
        GxRuntimeError("In Gx: Failed to allocate memory.");
    }
    return ptr;
}

bool GxAssertNotImplemented(bool condition) {
    if (!condition) {
        GxRuntimeError("In Gx: module not implemented error.");
    }    
    return condition;
}
bool GxAssertInvalidHash(bool condition) {
    if (!condition) {
        GxRuntimeError("In Gx: invalid hash error.");
    }    
    return condition;
}
bool GxAssertOutOfRange(bool condition){
    if (!condition) {
        GxRuntimeError("In Gx: out of range error.");
    }    
    return condition;
}

bool GxAssertInvalidOperation(bool condition) {
    if (!condition) {
        GxRuntimeError("In Gx: invalid hash error.");
    }    
    return condition;
}

bool GxAssertResourceNotFound(bool condition) {
    if (!condition) {
        GxRuntimeError("In Gx: invalid hash error.");
    }    
    return condition;
}

bool GxUserAssertNullPointer(const void* ptr) {
    if (!ptr) {
        GxRuntimeError("In Game: null pointer error.");
    }
    return ptr;
}

bool GxUserAssertArgumnent(bool condition) {    
    if (!condition) {
        GxRuntimeError("In Game: invalid argument error.");
    }    
    return condition;
}

bool GxUserAssertState(bool condition){    
    if (!condition) {
        GxRuntimeError("In Game: invalid state error.");
    }    
    return condition;
}

void* GxUserAssertAlloc(void* ptr){    
    if (!ptr) {
        GxRuntimeError("In Game: failed to allocate memory.");
    }    
    return ptr;
}

bool GxUserAssertOutOfRange(bool condition){    
    if (!condition) {
        GxRuntimeError("In Game: out of range error.");
    }    
    return condition;
}