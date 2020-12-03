#include "../Utilities/Util.h"
#include "../List/List.h"

typedef struct ListNode {
	void* value;
	void(*dtor)(void*);
	struct ListNode* next;
	struct ListNode* prev;
} ListNode;

static inline ListNode* createNode(void* value, sDtor dtor) {
	ListNode* node = malloc(sizeof(ListNode));
	node->value = value;
	node->dtor = dtor;
	node->next = NULL;
	node->prev = NULL;
	return node;
}

static inline void destroyNode(ListNode* node) {
	if (node->dtor) node->dtor(node->value);
	free(node);
}

typedef struct sList {	
	int size;
	ListNode* first;
	ListNode* last;
	ListNode* previousNode;	
} sList;


static sList* create() {
	sList* self = malloc(sizeof(sList));	
	nUtil->assertAlloc(self);
	self->size = 0;
	self->first = NULL;
	self->last = NULL;
	self->previousNode = NULL;	
	return self;
}

static void destroy(sList* self) {
	if (self) {
		nList->clean(self);
		free(self);
	}
}

static int size(sList* self) {
	return self->size;
}

static void* first(sList* self) {
	return self->first->value;
}

static void* last(sList* self) {
	return self->last->value;
}

static ListNode* listGetNodeByIndex(sList* self, int index) {
	nUtil->assertOutOfRange(index >= 0 && index < self->size);	
	 ListNode* node = NULL;
	 if (index < (self->size / 2)) {
		 node = self->first;
		 while (index-- > 0) {
			 node = node->next;
		 }
	 }	 
	 else {
		 node = self->last;
		 while (++index < self->size) {
			 node = node->prev;
		 }
	 }	
	 return node;
}

static void* at(sList* self, int index) {
	ListNode* node = listGetNodeByIndex(self, index);
	return node->value;
}

static bool contains(sList* self, void* value) {	
	ListNode* node = self->first;
	while (node && node->value != value) {
		node = node->next;
	}
	return (bool) node;
}

static void* begin(sList* self) {
	if (self) {
		self->previousNode = self->first;
		return self->first ? self->first->value : NULL;
	}
	return NULL;	
}

static void* next(sList* self) {	
	if (self->previousNode) {
		self->previousNode = self->previousNode->next;
		if (self->previousNode) {
			return self->previousNode->value;
		}		
	}
	return NULL;
}

static void push(sList* self, void* value, sDtor dtor) {
	ListNode* node = createNode(value, dtor);	
	if (self->last) {
		node->prev = self->last;
		self->last->next = node;		
	}
	else {
		self->first = node;		
	}	
	self->last = node;
	self->size++;
}

static void insert(sList* self, int index, void* value, sDtor dtor) {
	ListNode* node = createNode(value, dtor);

	if (index >= self->size) {
		nList->push(self, value, dtor);
	}
	else if (index == 0) {
		node->next = self->first;
		self->first->prev = node;		
		self->first = node;
	}
	else {
		ListNode* next = listGetNodeByIndex(self, index);
		next->prev->next = node;
		node->prev = next->prev;
		next->prev = node;
		node->next = next;		
	}
	self->size++;
}

static bool replace(sList* self, void* oldValue, void* newValue, sDtor dtor) {
	
	ListNode* node = (self->previousNode && 
		oldValue == self->previousNode->value) ? self->previousNode : self->first;

	while (node && node->value != oldValue) {
		node = node->next;
	}

	if (!node) return false;

	if (node->dtor) node->dtor(node->value);
	node->value = newValue;
	node->dtor = dtor;
	return true;
}

static bool listRemoveNode(sList* self, ListNode* node) {
	
	if (node) {
		if (node == self->first || node == self->last) {
			if (node == self->first) {
				if(node->next) node->next->prev = NULL;
				self->first = node->next;
			}
			if (node == self->last) {
				if(node->prev) node->prev->next = NULL;
				self->last = node->prev;
			}
		}
		else {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		if (node == self->previousNode) {
			self->previousNode = node->prev;
		}		
		
		destroyNode(node);
		self->size--;
		return true;
	}
	else return false;
}

static bool removeByValue(sList* self, void* value) {

	//tests if the value is equal to self->nextNode->prev or self->last
	//not being equal to anyone sets node equals to self->first
	ListNode* node = (self->previousNode && value == self->previousNode->value) 
		? self->previousNode
		: ((self->last && self->last->value == value) ? self->last : self->first);

	//search in crescent order
	while (node && node->value != value) {
		node = node->next;
	}

	//then returns null if not found or (if found) the return of listRemoveNode() method.
	return (bool) (node && listRemoveNode(self, node));	
}

static bool removeByIndex(sList* self, int index) {
	nUtil->assertOutOfRange(index >= 0 && index < self->size);	
	 return listRemoveNode(self, listGetNodeByIndex(self, index));
}

static void clean(sList* self) {
	while (nList->size(self)) {
		listRemoveNode(self, self->last);
	}
	self->size = 0;
}


const struct sListNamespace* nList = &(struct sListNamespace){
	//constructor and destructor
	.create = create,
	.destroy = destroy,
	.size = size,
	.first = first,
	.last = last,
	.at = at,
	.contains = contains,
	.begin = begin,
	.next = next,
	.push = push,
	.insert = insert,
	.replace = replace,
	.remove = removeByValue,
	.removeByIndex = removeByIndex,
	.clean = clean,
};