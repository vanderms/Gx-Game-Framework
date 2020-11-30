#include "../Utilities/Util.h"
#include "../Array/Array.h"
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

typedef struct sArray{   
	ArrData* entries;   
    Uint32 size;
    Uint32 capacity;    
} sArray;

//constructor and destructors
static sArray* create(void){
	//create self
	sArray* self = malloc(sizeof(sArray));	
	//create attributes    
    self->entries = malloc(sizeof(ArrData) * 8);
    nsUtil->assertAlloc(self->entries);
    self->capacity = 8;  
    self->size = 0;  
    return self;
}
//...prototypes
static void reserve(sArray* self, Uint32 capacity);
static int64_t indexOf(sArray* self, void* value);

//...methods
static void destroy(sArray* self) {
    if (self) {
        for (Uint32 i = 0; i < self->size; i++)
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        free(self->entries);
        free(self);
    }    
}

//accessors and mutators
static Uint32 getSize(sArray* self){
	return self->size;
}

static Uint32 getCapacity(sArray* self){
	return self->capacity;
}

//methods
static void* at(sArray* self, Uint32 index) {    
   nsUtil->assertOutOfRange(index < self->size);
    return self->entries[index].value;
}

static void* getLast(sArray* self) {
	return self->entries[self->size - 1].value;
}

static void insert(sArray* self, Uint32 index, void* value, GxDestructor dtor) {
    
    if (index >= self->size) {
        index = self->size++;
    }
    else if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }

    if (index >= self->capacity) {
        reserve(self, 2 * self->capacity);
    }
    self->entries[index].value = value;
    self->entries[index].dtor = dtor;
}

static void push(sArray* self, void* value, GxDestructor dtor){
    insert(self, self->size, value, dtor);
}


static void removeByIndex(sArray* self, Uint32 index) {       
   nsUtil->assertOutOfRange(index < self->size);
    if (self->entries[index].dtor) {
        self->entries[index].dtor(self->entries[index].value);
    }
    for (Uint32 i = index; i < self->size - 1; i++) {
        self->entries[i] = self->entries[i + 1];
    }
    self->size--;
}

static int removeByValue(sArray* self, void* value) {   
    int64_t index = indexOf(self, value);
    if (index != -1) {
        removeByIndex(self, (Uint32) index);
        return 1;
    } else return 0;
}

static int64_t indexOf(sArray* self, void* value) {   
    int64_t index = -1;
    for (Uint32 i = 0; i < self->size; i++) {
        if (value == nsArr->at(self, i)) {
            index = i;
            break;
        }
    }
    return index;
}

static void reserve(sArray* self, Uint32 capacity) {   
    if (self->capacity >= capacity) return;
    self->entries = realloc(self->entries, sizeof(ArrData) * capacity); 
    nsUtil->assertAlloc(self->entries);
    self->capacity = capacity;
}

static void clean(sArray* self) {
    if(self->size > 0 || self->capacity > 8){
        for (Uint32 i = 0; i < self->size; i++) {
            if (self->entries[i].dtor) self->entries[i].dtor(self->entries[i].value);
        }
        free(self->entries);
        self->entries = malloc(sizeof(ArrData) * 8); 
        nsUtil->assertAlloc(self->entries);
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

void sort(sArray* self, GxComp compare) {
    arrcomp = compare;
    qsort(self->entries, self->size, sizeof(ArrData), (GxComp) arraySortCallback);    
}





const sArrayNamespace* nsArr = &(sArrayNamespace){
	.create = create,
	.destroy = destroy,
	.size = getSize,
	.capacity = getCapacity,
	.at = at,
    .last = getLast,
	.push = push,
	.insert = insert,
	.remove = removeByIndex,
	.removeByValue = removeByValue,
	.indexOf = indexOf,
	.reserve = reserve,
	.clean = clean,
	.sort = sort	
};
