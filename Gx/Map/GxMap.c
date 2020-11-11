#include "../Utilities/GxUtil.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../List/GxList.h"
#include "../Array/GxArray.h"
#include "../Map/GxMap.h"
#define SWAP(a, b) {void* temp = a; a = b; b = temp; }

typedef struct GxInt { int value; }GxInt;

static inline GxInt* createInt(int value) {
	GxInt* self = malloc(sizeof(GxInt));	
	GxAssertAllocationFailure(self);
	self->value = value;
	return self;
}

/*************************************************************************************************************
 *  *** AUXILIARY METHODS ***
 *************************************************************************************************************/ 
 typedef struct Entry {
	//data
	void* value;
	GxDestructor dtor;
	//cross reference	
	GxInt* reference;
} Entry;


typedef struct GxMap {
	Uint32 size;
	Uint32 capacity;	
	char** keys;
	Entry* entries;
	GxList** table;
} GxMap;

static inline int calcHash(const char* str, Uint32 max) {	
	uint64_t hash = 5381;
	int c;
	while ((c = *str++)) hash = ((hash << 5) + hash) + (uint64_t) c;
	return (int) (hash % (uint64_t) max);
}

/*************************************************************************************************************
 *  *** CONSTRUCTOR AND DESTRUCTOR ***
 *************************************************************************************************************/

GxMap* GmCreateMap() {
	GxMap* self = malloc(sizeof(GxMap));
	GxAssertAllocationFailure(self);
	self->size = 0;
	self->capacity = 16;
	
	//create table
	self->table = malloc(self->capacity * sizeof(GxList*));	
	GxAssertAllocationFailure(self->table);
	self->entries = malloc(self->capacity * sizeof(Entry));
	GxAssertAllocationFailure(self->entries);
	self->keys = malloc(self->capacity * sizeof(char*));
	GxAssertAllocationFailure(self->keys);
	
	for (Uint32 i = 0; i < self->capacity; i++) {
		self->table[i] = GxCreateList();
	}
	//then return
	return self;
}


void GxDestroyMap(GxMap* self) {
	if (self) {
		for (Uint32 i = 0; i < self->capacity; i++) {
			GxDestroyList(self->table[i]);
			if(i >= self->size) continue;
			free(self->keys[i]);
			if(self->entries[i].dtor){
				self->entries[i].dtor(self->entries[i].value);
			}
			free(self->entries[i].reference);
		}

		free(self->table);
		free(self->keys);
		free(self->entries);
		free(self);
	}	
}

/*************************************************************************************************************
 *  *** MAP METHODS ***
 *************************************************************************************************************/

Uint32 GxMapSize(GxMap* self) {
	return self->size;
}

Uint32 GxMapCapacity(GxMap* self) {
	return self->capacity;
}

void* GxMapGet(GxMap* self, const char* key) {	
	
	GxList* bucket = self->table[calcHash(key, self->capacity)];
	for (char* k = GxListBegin(bucket); k != NULL; k = GxListNext(bucket)) {
		GxInt* reference = GxListNext(bucket);
		if (strcmp(k, key) == 0) {
			return self->entries[reference->value].value;
		}
	}
	return NULL;
}

void* GxMapAt(GxMap* self, Uint32 index) {
	GxAssertOutOfRange(index < self->size);
	return self->entries[index].value;
}

void GxMapSet(GxMap* self, const char* key, void* value, GxDestructor dtor) {
	
	if (self->size >= self->capacity){
		GxMapRehash(self, self->capacity * 2);
	}	

	bool contains = false;
	GxList* bucket = self->table[calcHash(key, self->capacity)];
	
	for (char* lsKey = GxListBegin(bucket); lsKey != NULL; 
		lsKey = GxListNext(bucket))
	{		
		GxInt* index = GxListNext(bucket);
		if (strcmp(lsKey, key) == 0) {
			Entry* entry = &self->entries[index->value];
			if(entry->dtor) entry->dtor(entry->value);
			entry->value = value;
			entry->dtor = dtor;
			contains = true;				
			break;
		}
	}	

	if (!contains) {	
		//fill bucket
		char* k = GmCreateString(key);
		GxInt* index = createInt(self->size);
		GxListPush(bucket, k, NULL);
		GxListPush(bucket, index, NULL);

		//fill entries and keys
		self->entries[self->size].value = value;
		self->entries[self->size].dtor = dtor;		
		self->entries[self->size].reference = index;
		self->keys[self->size] = k;
				
		//change size
		self->size++;
	}
}


void GxMapRehash(GxMap* self, Uint32 capacity) {

	if (self->capacity >= capacity) return;

	//create replacement table
	GxList** table = malloc(capacity * sizeof(GxList*));
	GxAssertAllocationFailure(table);
	
	for (Uint32 i = 0; i < capacity; i++) {
		table[i] = GxCreateList();
	}

	for (Uint32 i = 0; i < self->capacity; i++) {

		GxList* source = self->table[i];		
		
		for (char* key = GxListBegin(source); key != NULL; key = GxListNext(source)) {
			int* index = GxListNext(source);
			GxList* bucket = table[calcHash(key, capacity)];
			GxListPush(bucket, key, NULL);
			GxListPush(bucket, index, NULL);
		}		
	}
	
	//swap table and realloc keys and entries
	SWAP(self->table, table)	
	self->keys = realloc(self->keys, capacity * sizeof(char*));
	GxAssertAllocationFailure(self->keys);
	self->entries = realloc(self->entries, capacity* sizeof(Entry));
	GxAssertAllocationFailure(self->entries);
	
	//destroy previous table
	for (Uint32 i = 0; i < self->capacity; i++) {
		GxDestroyList(table[i]);
	}
	free(table);
	//set new capacity
	self->capacity = capacity;		
}

void GxMapRemove(GxMap* self, const char* key) {
	
	GxInt* index = NULL;
	bool contains = false;
	GxList* bucket = self->table[calcHash(key, self->capacity)];
	
	for (char* k = GxListBegin(bucket); k != NULL; k = GxListNext(bucket)){		
		index = GxListNext(bucket);
		if (strcmp(k, key) == 0) {	
			contains = true;			
			GxListRemove(bucket, k);
			GxListRemove(bucket, index);
			break;
		}	
	}

	if (contains) {
		//remove value, and free key 
		if (self->entries[index->value].dtor) {
 			self->entries[index->value].dtor(self->entries[index->value].value);
		}
		free(self->keys[index->value]);

		//set new size
		--self->size;

		//update entries and keys
		for (Uint32 i = index->value; i < self->size; i++) {
			self->keys[i] = self->keys[i + 1];
			self->entries[i] = self->entries[i + 1];
			self->entries[i].reference->value = i;
		}

		//free index
		free(index);
	}
}

void GxMapRemoveByIndex(GxMap* self, Uint32 index) {
	GxAssertOutOfRange(index < self->size);
	char* key = self->keys[index];
	GxMapRemove(self, key);
}

void  GxMapClean(GxMap* self) {
	
	//first create a empty map
	GxMap* empty = GmCreateMap();	
		
	//then, swap table and keys
	SWAP(self->table, empty->table)
	SWAP(self->keys, empty->keys)
	SWAP(self->entries, empty->entries)

	//Set capacity and size
	Uint32 temp = self->capacity;
	self->capacity = empty->capacity;
	empty->capacity = temp;
	empty->size = self->size;
	self->size = 0;

	//finally destroy empty
	GxDestroyMap(empty);
}
