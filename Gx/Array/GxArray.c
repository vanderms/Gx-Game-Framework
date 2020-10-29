#include "../Utilities/GxUtil.h"
#include "../Array/GxArray.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

//structs
typedef struct ArrData {
	void* value;
	GxDestructor dtor;
} ArrData;

typedef struct GxArray{   
	ArrData* entries;   
    Uint32 size;
    Uint32 capacity;    
} GxArray;

//constructor and destructors
GxArray* GxCreateArray(void){
	//create self
	GxArray* self = malloc(sizeof(GxArray));	
	//create attributes    
    self->entries = malloc(sizeof(ArrData) * 8);
    GxAssertAllocationFailure(self->entries);
    self->capacity = 8;  
    self->size = 0;  
    return self;
}

void GxDestroyArray(GxArray* self) {
    if (self) {
        for (Uint32 i = 0; i < self->size; i++)
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        free(self->entries);
        free(self);
    }    
}

//accessors and mutators
Uint32 GxArraySize(GxArray* self){
	return self->size;
}

Uint32 GxArrayCapacity(GxArray* self){
	return self->capacity;
}

//methods
void* GxArrayAt(GxArray* self, Uint32 index) {    
    GxAssertOutOfRange(index < self->size);
    return self->entries[index].value;
}

void* GxArrayLast(GxArray* self) {
	return self->entries[self->size - 1].value;
}

void GxArrayPush(GxArray* self, void* value, GxDestructor dtor){
    GxArrayInsert(self, self->size, value, dtor);
}

void GxArrayInsert(GxArray* self, Uint32 index, void* value, GxDestructor dtor) {
    
    if (index >= self->size) {
        index = self->size++;
    }
    else if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }

    if (index >= self->capacity) {
        GxArrayReserve(self, 2 * self->capacity);
    }
    self->entries[index].value = value;
    self->entries[index].dtor = dtor;
}


void GxArrayRemove(GxArray* self, Uint32 index) {       
    GxAssertOutOfRange(index < self->size);
    if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }
    for (Uint32 i = index; i < self->size - 1; i++) {
        self->entries[i] = self->entries[i + 1];
    }
    self->size--;
}

int GxArrayRemoveByValue(GxArray* self, void* value) {   
    int64_t index = GxArrayIndexOf(self, value);
    if (index != -1) {
        GxArrayRemove(self, (Uint32) index);
        return 1;
    } else return 0;
}

int64_t GxArrayIndexOf(GxArray* self, void* value) {   
    int64_t index = -1;
    for (Uint32 i = 0; i < self->size; i++) {
        if (value == GxArrayAt(self, i)) {
            index = i;
            break;
        }
    }
    return index;
}

void GxArrayReserve(GxArray* self, Uint32 capacity) {   
    if (self->capacity >= capacity) return;
    self->entries = realloc(self->entries, sizeof(ArrData) * capacity); 
    GxAssertAllocationFailure(self->entries);
    self->capacity = capacity;
}

void GxArrayClean(GxArray* self) {
    if(self->size > 0 || self->capacity > 8){
        for (Uint32 i = 0; i < self->size; i++) {
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        }
        free(self->entries);
        self->entries = malloc(sizeof(ArrData) * 8); 
        GxAssertAllocationFailure(self->entries);
        self->capacity = 8;
        self->size = 0;
    }
}


//This is not the most efficient neither supports multithreading. Some day
//I will implement a sort function
static GxComp arrcomp = NULL;

static inline int arraySortCallback(const ArrData* lhs, const ArrData* rhs) {    
    return arrcomp(lhs->value, rhs->value);
}

void GxArraySort(GxArray* self, GxComp compare) {
    arrcomp = compare;
    qsort(self->entries, self->size, sizeof(ArrData), (GxComp) arraySortCallback);    
}

