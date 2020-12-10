#include "../../Util/Util.h"
#include "../Element.h"
#include "../../Folder/Folder.h"
#include "../../Scene/Scene.h"
#include "../../Containers/Array/Array.h"
#include "../../Containers/Qtree/Qtree.h"
#include <string.h>

//...RENDERABLE STRUCTS 
typedef struct Alignment{
	bool changed;
	char last[25];
	const char* horizontal;
	const char* vertical;
	int x;
	int y;
} Alignment;

typedef struct Color {
	char* last;
	SDL_Color* value;
} Color;

typedef struct Border {
	int size;
	Color* color;
} Border;

//... MAIN TYPE
typedef struct sElemRenderable {
	int type;
	int zIndex;
	const sRect* pos;
	sElement* elem;
	SDL_RendererFlip orientation;
	char* asset;
	sArray* folders;
	Uint8 opacity;
	sImage* image;
	sAnimation* animation;
	uint32_t animCounter;
	Uint32 animCurrent;
	Alignment* alignment;
	bool hidden;
	double angle;
	double proportion;
	Border border;
	Color* backgroundColor;
	//label
	char* font;
	char* text;
	int fontSize;
	Color* color;
	bool shouldUpdateLabel;
	sImage* label;

	//...
	sQtreeElem* qtreeElem;
} sElemRenderable;

//... ALIGNMENT CONSTANTS
static const char* sRight = "right";
static const char* sLeft = "left";
static const char* sBottom = "bottom";
static const char* sTop = "top";
static const char* sCenter = "center";
static const char* sNum = "number";
static const char* sCenterCenter = "center|center";

//...COLOR 
static Color* createColor(const char* value) {
	Color* self = calloc(1, sizeof(Color));
	nUtil->assertAlloc(self);
	if (value) {
		self->value = malloc(sizeof(SDL_Color));
		nUtil->assertAlloc(self->value);
		nAppConvertColor(self->value, value);
		self->last = nUtil->createString(value);
	}
	return self;
}

static bool updateColor(Color* self, const char* value) {
	if (!value && !self->last) {
		return false;
	}
	else if (!value && self->last) {
		free(self->last);
		free(self->value);
		self->last = NULL;
		self->value = NULL;
		return true;
	}
	else if (!self->last || strcmp(value, self->last) != 0){
		free(self->last);
		if (!self->value) {
			self->value = malloc(sizeof(SDL_Color));
			nUtil->assertAlloc(self->value);
		}
		nAppConvertColor(self->value, value);
		self->last = nUtil->createString(value);
		return true;
	}
	return false;
}

static void destroyColor(Color* color) {
	if(color){
		free(color->last);
		free(color->value);
		free(color);
	}
}

//constructors and destructors
sElemRenderable* nElemCreateRenderable_(sElement* elem, const sIni* ini) {
	
	if(ini->display !=  nElem_DISPLAY_ABSOLUTE && ini->display !=  nElem_DISPLAY_RELATIVE){
		nUtil->assertArgument(ini->display == nElem_DISPLAY_NONE);
		return NULL;
	}

	sElemRenderable* self = nUtil->assertAlloc(calloc(1, sizeof(sElemRenderable)));	
	self->type = (ini->display ==  nElem_DISPLAY_ABSOLUTE ? 
		 nElem_DISPLAY_ABSOLUTE :  nElem_DISPLAY_RELATIVE
	);
	self->pos = nElemPosition(elem);
	self->zIndex = ini->zIndex;
	self->elem = elem;
	nElemSetRenderable_(elem, self);
	self->opacity = 255;
	//... folders
	if (ini->folders) {
		self->folders = nUtil->split(ini->folders, "|");
		for (Uint32 i = 0; i < nArraySize(self->folders); i++) {
			sFolder* folder = nAppGetFolder(nArrayAt(self->folders, i));
			nUtil->assertArgument(folder);
			nArrayInsert(self->folders, i, folder, NULL);
			nFolderIncRefCounter_(folder);
		}
	}

	//...
	if (ini->orientation){
		nElemSetOrientation(elem, ini->orientation);
	}
	else {
		self->orientation = SDL_FLIP_NONE;
	}
	//...
	if (ini->image){
		nElemSetImage(elem, ini->image);
	}
	//...
	if (ini->animation){
		nElemSetAnimation (elem, ini->animation);
	}
	//...
	if (ini->alignment) {
		nElemSetAlignment(elem, ini->alignment);
	}
	//...
	self->hidden = ini->hidden;
	self->angle = ini->angle;
	self->proportion = ini->proportion != 0.0 ? ini->proportion : 1.0;
	self->backgroundColor = createColor(ini->backgroundColor);

	//label
	self->color = ini->color ? createColor(ini->color) : createColor("Black");
	nElemSetFontSize(elem, ini->fontSize);
	self->text = ini->text ? nUtil->createString(ini->text) : NULL;
	nElemSetFont(elem, ini->font ? ini->font : "Default");
	self->shouldUpdateLabel = ini->text ? true : false;

	self->border.color = createColor(NULL);
	nElemSetBorder(elem, ini->border);
	self->qtreeElem = nQtreeCreateElem(elem, nElemPosGetter_);
	return self;
}

void nElemDestroyRenderable_(sElemRenderable* self) {
	if (self) {
		if(nAppIsRunning() && self->folders){
			for (Uint32 i = 0; i < nArraySize(self->folders); i++){
				sFolder* folder = nArrayAt(self->folders, i);
				nFolderDecRefCounter_(folder);
			}
		}
		free(self->qtreeElem);
		free(self->asset);
		free(self->alignment);
		free(self->text);
		free(self->font);
		if(self->color) destroyColor(self->color);
		if(self->backgroundColor) destroyColor(self->backgroundColor);
		if(self->border.color) destroyColor(self->border.color);
		if(self->label) nImageDestroyText(self->label);
		if(self->folders) nArrayDestroy(self->folders);
		free(self);
	}
}

bool nElemHasRelativeDisplay(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable && renderable->type ==  nElem_DISPLAY_RELATIVE;
}

bool nElemHasAbsoluteDisplay(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable && renderable->type ==  nElem_DISPLAY_ABSOLUTE;
}


int nElemZIndex(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->zIndex;
}

void nElemSetZIndex(sElement* self, int value) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->zIndex = value;
}

Uint8 nElemOpacity(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->opacity;
}

void nElemSetOpacity(sElement* self, Uint8 value) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->opacity = value;
}

int nElemOrientation(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return (int) renderable->orientation;
}

void nElemSetOrientation(sElement* self, int value) {
	sElemRenderable* renderable = nElemRenderable_(self);
	nUtil->assertArgument(value ==  nElem_FORWARD || 
		value ==  nElem_BACKWARD
	);
	renderable->orientation = (SDL_RendererFlip) value;
}

const char* nElemImage(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->image ? renderable->asset : NULL;
}

const char* nElemAnimation(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->animation ? renderable->asset : NULL;
}

const char* nElemAlignment(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if (!renderable->alignment) {
		return sCenterCenter;
	}
	if(renderable->alignment->changed){
		const char* horizontal = NULL, *vertical = NULL;
		char hbuf[12], vbuf[12];

		if (renderable->alignment->horizontal == sNum) {
			snprintf(hbuf, 12, "%d", renderable->alignment->x);
			horizontal = hbuf;
		}
		else horizontal = renderable->alignment->horizontal;

		if (renderable->alignment->vertical == sNum) {
			snprintf(vbuf, 12, "%d", renderable->alignment->y);
			vertical = vbuf;
		}
		else horizontal = renderable->alignment->horizontal;

		snprintf(renderable->alignment->last, 25, "%s|%s", horizontal, vertical);
		renderable->alignment->changed = false;
	}
	return renderable->alignment->last;
}

void nElemSetAlignment(sElement* self, const char* value) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if (!renderable->alignment) {
		renderable->alignment = calloc(1, sizeof(Alignment));
		nUtil->assertAlloc(renderable->alignment);
	}
	if (value == sCenterCenter) {
		renderable->alignment->horizontal = sCenter;
		renderable->alignment->vertical = sCenter;
	}
	else if (strstr(renderable->alignment->last, value)) {
		return;
	}
	else {
		char* align = nUtil->createString(value);
		char* div = strstr(align, "|");
		nUtil->assertArgument(div);
		div[0] = '\0';
		char* horizontal = div != align ? align : NULL;
		char* vertical = *(div + 1) != '\0'? div + 1 : NULL;

		//set horizontal
		if (horizontal) {
			if (strcmp(horizontal, "right") == 0) {
				renderable->alignment->horizontal = sRight;
			}
			else if (strcmp(horizontal, "left") == 0) {
				renderable->alignment->horizontal = sLeft;
			}
			else if(strcmp(horizontal, "center") == 0){
				renderable->alignment->horizontal = sCenter;
			}
			else {
				renderable->alignment->horizontal = sNum;
				renderable->alignment->x = atoi(horizontal);
			}
		}

		//set vertical
		if (vertical) {
			if (strcmp(vertical, "top") == 0) {
				renderable->alignment->vertical = sTop;
			}
			else if (strcmp(vertical, "bottom") == 0) {
				renderable->alignment->vertical = sBottom;
			}
			else if(strcmp(vertical, "center") == 0){
				renderable->alignment->vertical = sCenter;
			}
			else {
				renderable->alignment->vertical = sNum;
				renderable->alignment->y = atoi(vertical);
			}
		}

		//set GxElemGetAlignment flag
		renderable->alignment->changed = true;

		//then free align
		free(align);
	}
}

bool nElemIsHidden(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->hidden;
}

void nElemHide(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->hidden = true;
}

void nElemShow(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->hidden = false;
}

double nElemAngle(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->angle;
}

void nElemSetAngle(sElement* self, double angle) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->angle = angle;
}

double nElemProportion(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->proportion;
}

void nElemSetProportion(sElement* self, double proportion) {
	sElemRenderable* renderable = nElemRenderable_(self);
	renderable->proportion = proportion;
}

void nElemSetToFit(sElement* self, const char* axis) {
	sElemRenderable* renderable = nElemRenderable_(self);
	nUtil->assertArgument(strcmp(axis, "horizontal") == 0 || strcmp(axis, "vertical") == 0);
	const sSize* size = nImageSize(renderable->image);
	const sRect* pos = nElemPosition(self);
	if (strcmp(axis, "horizontal") == 0){
		renderable->proportion = ((double) pos->w) / size->w;
	}
	else {
		renderable->proportion = ((double) pos->h) / size->h;
	}
}

const SDL_Color* nElemColor(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->color->value;
}

void nElemSetColor(sElement* self, const char* color) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if(updateColor(renderable->color, color)){
		renderable->shouldUpdateLabel = true;
	}
}

const SDL_Color* nElemBackgroundColor(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->backgroundColor->value;
}

void nElemSetBackgroundColor(sElement* self, const char* color) {
	sElemRenderable* renderable = nElemRenderable_(self);
	updateColor(renderable->backgroundColor, color);
}

int nElemBorderSize(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->border.size;
}

const SDL_Color* nElemBorderColor(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->border.color->value;
}

void nElemSetBorder(sElement* self, const char* border) {
	sElemRenderable* renderable = nElemRenderable_(self);

	//if null, reset border
	if (!border) {
		renderable->border.size = 0;
		updateColor(renderable->border.color, NULL);
		renderable->border.size = 0;
		return;
	}

	//get size and color strings
	char* size = nUtil->cloneString(border, (char[64]){0}, 64);
	char* color = strstr(size, "|");
	nUtil->assertArgument(color);
	color[0] = '\0';
	color++;

	//set size
	renderable->border.size = atoi(size);

	//set color
	if(color[0] != '\0'){
		updateColor(renderable->border.color, color);
	}
}

sQtreeElem* nElemGetRenderableQtreeElem_(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->qtreeElem;
}

enum AssetType{IMAGE, ANIMATION};

static void* elemGetAsset(sElement* self, const char* apath, enum AssetType type) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if (apath != NULL) {
		if (renderable->asset && (strcmp(renderable->asset, apath) == 0)){
			return ( type == IMAGE ?
				(void*) renderable->image : (void*) renderable->animation
			);
		}
		free(renderable->asset);
		renderable->asset = nUtil->createString(apath);
		char* slash = strstr(renderable->asset, "/");
		nUtil->assertArgument(slash);

		*slash = '\0';
		sFolder* folder = nAppGetFolder(renderable->asset);
		nUtil->assertArgument(folder);

		bool folderIsLoaded = nFolderHasStatus(folder, nUtil->status->LOADING) ||
			nFolderHasStatus(folder, nUtil->status->READY);
		nUtil->assertState(folderIsLoaded);

		*slash = '/';
		void* asset = (type == IMAGE ?
			(void*) nFolderGetImage(folder, slash + 1) :
			(void*) nFolderGetAnimation(folder, slash + 1)
		);
		nUtil->assertArgument(asset);
		return asset;
	}
	return NULL;
}

void nElemSetImage(sElement* self, const char* apath) {

	sElemRenderable* renderable = nElemRenderable_(self);
	sImage* image = elemGetAsset(self, apath, IMAGE);
	if(renderable->image != image){
		renderable->animation = NULL;
		renderable->animCounter = 0;
		renderable->animCurrent = 0;
		renderable->image = image;
	}
}

void nElemSetAnimation(sElement* self, const char* apath) {
	
	sElemRenderable* renderable = nElemRenderable_(self);
	sAnimation* anim = elemGetAsset(self, apath, ANIMATION);
	
	renderable->image = NULL;
	renderable->animation = anim;
	renderable->animCounter = 0;
	renderable->animCurrent = 0;	
}

void nElemSetText(sElement* self, const char* format, ...) {

	sElemRenderable* renderable = nElemRenderable_(self);

	if (format){
		static char text[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(text, 1024, format, args);
		va_end(args);

		if(!renderable->text) {
			renderable->text = nUtil->createString(text);
			renderable->shouldUpdateLabel = true;
		}
		else { // renderable text != NULL
			if (strcmp(text, renderable->text) != 0) {
				free(renderable->text);
				renderable->text = nUtil->createString(text);
				renderable->shouldUpdateLabel = true;
			}
		}
	}
	else { // !format
		if (renderable->text) {
			free(renderable->text);
			renderable = NULL;
			renderable->shouldUpdateLabel = true;
		}
		//if not text formatted and not renderable->text, then do nothing.
	}
}


const char* nElemText(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->text;
}

void nElemSetFontSize(sElement* self, int size) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if(size <= 0) size = 16;
	if(size <= 8) size = 8;
	if (renderable->fontSize != size) {
		renderable->fontSize = size;
		renderable->shouldUpdateLabel = true;
	}
}

int nElemFontSize(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->fontSize;
}

void nElemSetFont(sElement* self, const char* font) {
	sElemRenderable* renderable = nElemRenderable_(self);
	nUtil->assertArgument(font);
	if (renderable->font && strcmp(renderable->font, font) == 0) {
		return;
	}
	const void* fontExists = nAppGetFontPath_(font);
	nUtil->assertArgument(fontExists);
	if (fontExists) {
		free(renderable->font);
		renderable->font = nUtil->createString(font);
		renderable->shouldUpdateLabel = true;
	}
}

const char* nElemFont(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->font;
}

static void pUpdateLabel(sElemRenderable* renderable) {

	if(renderable->label){
		nImageDestroyText(renderable->label);
	}
	renderable->shouldUpdateLabel = false;

	if(renderable->text && renderable->fontSize > 0 &&
		renderable->font && renderable->color->value
	){
		renderable->label = nImageCreateText(renderable->text,
			renderable->font, renderable->fontSize, renderable->color->value
		);
	}
	else {
		renderable->label = NULL;
	}
}

//...ELEMENT RENDER METHODS
static sRect calcAbsolutePos(sElemRenderable* ren) {
	int y = nAppLogicalSize().h - (ren->pos->y + ren->pos->h);
	return (sRect) { ren->pos->x, y, ren->pos->w, ren->pos->h };
}

static sRect calcRelativePos(sElemRenderable* ren) {
	const sRect* cpos = nElemPosition(nScene->getCamera(nElemScene(ren->elem)));
	int x = ren->pos->x - cpos->x;
	int y = (cpos->y + cpos->h) - (ren->pos->y + ren->pos->h);
	return (sRect) { x, y, ren->pos->w, ren->pos->h };
}

static void pApplyProportion(sElemRenderable* ren, sRect* pos, sSize size) {
	pos->w = (int) (size.w * ren->proportion + 0.5);
	pos->h = (int) (size.h * ren->proportion + 0.5);
}

static void pApplyAlignment(sElemRenderable* ren, sRect* pos) {

	//horizontal alignment -> (left is default)
	if(!ren->alignment ||
		ren->alignment->horizontal == sCenter) {
		pos->x += (ren->pos->w - pos->w) / 2;
	}
	else if (ren->alignment->horizontal ==  sRight){
		pos->x += ren->pos->w - pos->w;
	}
	else if(ren->alignment->horizontal == sNum) {
		pos->x += (ren->pos->w - pos->w) / 2; //first set default
		pos->x += ren->alignment->x;
	}

	//vertical alignment (top is default)
	if (!ren->alignment ||
		ren->alignment->vertical == sCenter){
		pos->y += (ren->pos->h - pos->h) / 2;
	}
	else if (ren->alignment->vertical == sBottom) {
		pos->y += (ren->pos->h - pos->h);
	}
	else if(ren->alignment->vertical == sNum) {
		pos->y += (ren->pos->h - pos->h) / 2; //first set default
		pos->y -= ren->alignment->y;
	}
}

sRect* nElemCalcImagePosOnCamera(sElement* self, sRect* pos, sImage* image) {
	sElemRenderable* renderable = nElemRenderable_(self);
	const sSize* size = nImageSize(image);
	pApplyProportion(renderable, pos, *size);
	pApplyAlignment(renderable, pos);
	return pos;
}

sRect nElemCalcPosOnCamera(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if(renderable->type ==  nElem_DISPLAY_ABSOLUTE){
		return calcAbsolutePos(renderable);
	}
	return calcRelativePos(renderable);	
}

sImage* nElemGetImageRef_(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	sImage* image = NULL;
	if (renderable->image) {
		image = renderable->image;
	}
	else if (renderable->animation) {
		sAnimation* anim = renderable->animation; //create alias
		renderable->animCounter++; //to avoid counter starting with 0.
		if (renderable->animCurrent >= nAnimQuantity(anim)) {
			renderable->animCurrent = 0;
		}
		image = nAnimGetImage(anim, renderable->animCurrent);

		if (renderable->animCounter % nAnimInterval(anim) == 0)
			renderable->animCurrent++;

			//if it's the last frame in animation and there's no repeat set renderable->animation to NULL
		if (renderable->animCurrent >= nAnimQuantity(anim) && !nAnimIsContinuous(anim)) {
			renderable->animation = NULL;
			renderable->animCounter = 0;
			renderable->animCurrent = 0;
		}
	}
	return image;
}

void nElemOnRender_(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	if (renderable->shouldUpdateLabel) {
		pUpdateLabel(renderable);
	}
	nElemExecuteHandler_(self, &(sEvent){
		.type = nComponent->ON_RENDER
	});
}

sImage* nElemLabel_(sElement* self) {
	sElemRenderable* renderable = nElemRenderable_(self);
	return renderable->label;
}
