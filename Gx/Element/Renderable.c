#include "../Utilities/Util.h"
#include "Element.h"
#include "../Folder/GxFolder.h"
#include "../Scene/GxScene.h"
#include "../Array/Array.h"
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
	SDL_RendererFlip orientation;
	char* asset;
	sArray* folders;
	Uint8 opacity;
	GxImage* image;
	GxAnimation* animation;
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
	GxImage* label;

	//...
	Uint32 wflag;
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
		nApp->convertColor(self->value, value);
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
		nApp->convertColor(self->value, value);
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
static sElemRenderable* pCreate(sElement* elem, const sIni* ini) {
	
	if(ini->display != GxElemAbsolute && ini->display != GxElemRelative){
		nUtil->assertArgument(ini->display == GxElemNone);
		return NULL;
	}

	sElemRenderable* self = calloc(1, sizeof(sElemRenderable));
	nUtil->assertAlloc(self);
	self->type = ini->display == GxElemAbsolute ? GxElemAbsolute : GxElemRelative;
	self->zIndex = ini->zIndex;
	nElem->p->setRenderable(elem, self);
	self->opacity = 255;
	//... folders
	if (ini->folders) {
		self->folders = nUtil->split(ini->folders, "|");
		for (Uint32 i = 0; i < nArr->size(self->folders); i++) {
			GxFolder* folder = nApp->prv->getFolder(nArr->at(self->folders, i));
			nUtil->assertArgument(folder);
			nArr->insert(self->folders, i, folder, NULL);
			GxFolderIncRefCounter_(folder);
		}
	}

	//...
	if (ini->orientation){
		GxElemSetOrientation(elem, ini->orientation);
	}
	else {
		self->orientation = SDL_FLIP_NONE;
	}
	//...
	if (ini->image){
		GxElemSetImage(elem, ini->image);
	}
	//...
	if (ini->animation){
		GxElemSetAnimation (elem, ini->animation);
	}
	//...
	if (ini->alignment) {
		GxElemSetAlignment(elem, ini->alignment);
	}
	//...
	self->hidden = ini->hidden;
	self->angle = ini->angle;
	self->proportion = ini->proportion != 0.0 ? ini->proportion : 1.0;
	self->backgroundColor = createColor(ini->backgroundColor);

	//label
	self->color = ini->color ? createColor(ini->color) : createColor("Black");
	GxElemSetFontSize(elem, ini->fontSize);
	self->text = ini->text ? nUtil->createString(ini->text) : NULL;
	GxElemSetFont(elem, ini->font ? ini->font : "Default");
	self->shouldUpdateLabel = ini->text ? true : false;

	self->border.color = createColor(NULL);
	GxElemSetBorder(elem, ini->border);
	self->wflag = 0;
	return self;
}

static void pDestroy(sElemRenderable* self) {
	if (self) {
		if(nApp->isRunning() && self->folders){
			for (Uint32 i = 0; i < nArr->size(self->folders); i++){
				GxFolder* folder = nArr->at(self->folders, i);
				GxFolderDecRefCounter_(folder);
			}
		}
		free(self->asset);
		free(self->alignment);
		free(self->text);
		free(self->font);
		if(self->color) destroyColor(self->color);
		if(self->backgroundColor) destroyColor(self->backgroundColor);
		if(self->border.color) destroyColor(self->border.color);
		if(self->label) GxDestroyImage_(self->label);
		if(self->folders) nArr->destroy(self->folders);
		free(self);
	}
}

static bool hasRelativePosition(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable && renderable->type == GxElemRelative;
}

static bool hasAbsolutePosition(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable && renderable->type == GxElemAbsolute;
}


static int zIndex(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->zIndex;
}

static void setZIndex(sElement* self, int value) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->zIndex = value;
}

static Uint8 opacity(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->opacity;
}

static void setOpacity(sElement* self, Uint8 value) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->opacity = value;
}

static int orientation(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return (int) renderable->orientation;
}

static void setOrientation(sElement* self, int value) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	nUtil->assertArgument(value == GxElemForward || value == GxElemBackward);
	renderable->orientation = (SDL_RendererFlip) value;
}

static const char* image(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->image ? renderable->asset : NULL;
}

static const char* animation(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->animation ? renderable->asset : NULL;
}

static const char* alignment(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
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

static void setAlignment(sElement* self, const char* value) {
	sElemRenderable* renderable = nElem->p->renderable(self);
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

static bool isHidden(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->hidden;
}

static void hide(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->hidden = true;
}

static void show(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->hidden = false;
}

static double angle(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->angle;
}

static void setAngle(sElement* self, double angle) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->angle = angle;
}

static double proportion(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->proportion;
}

static void setProportion(sElement* self, double proportion) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->proportion = proportion;
}

static void setToFit(sElement* self, const char* axis) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	nUtil->assertArgument(strcmp(axis, "horizontal") == 0 || strcmp(axis, "vertical") == 0);
	GxSize size = GxImageGetSize_(renderable->image);
	const SDL_Rect* pos = nElem->position(self);
	if (strcmp(axis, "horizontal") == 0){
		renderable->proportion = ((double) pos->w) / size.w;
	}
	else {
		renderable->proportion = ((double) pos->h) / size.h;
	}
}

static const SDL_Color* color(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->color->value;
}

static void setColor(sElement* self, const char* color) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	if(updateColor(renderable->color, color)){
		renderable->shouldUpdateLabel = true;
	}
}

static const SDL_Color* backgroundColor(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->backgroundColor->value;
}

static void setBackgroundColor(sElement* self, const char* color) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	updateColor(renderable->backgroundColor, color);
}

static int borderSize(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->border.size;
}

static const SDL_Color* borderColor(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->border.color->value;
}

static void setBorder(sElement* self, const char* border) {
	sElemRenderable* renderable = nElem->p->renderable(self);

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

static uint32_t pWFlag(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->wflag;
}

static void pSetWFlag_(sElement* self, uint32_t value) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	renderable->wflag = value;
}

enum AssetType{IMAGE, ANIMATION};

static void* elemGetAsset(sElement* self, const char* apath, enum AssetType type) {
	sElemRenderable* renderable = nElem->p->renderable(self);
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
		GxFolder* folder = nApp->prv->getFolder(renderable->asset);
		nUtil->assertArgument(folder);

		bool folderIsLoaded = GxFolderHasStatus_(folder, GxStatusLoading) ||
			GxFolderHasStatus_(folder, GxStatusReady);
		nUtil->assertState(folderIsLoaded);

		*slash = '/';
		void* asset = (type == IMAGE ?
			(void*) GxFolderGetImage_(folder, slash + 1) :
			(void*) GxFolderGetAnimation_(folder, slash + 1)
		);
		nUtil->assertArgument(asset);
		return asset;
	}
	return NULL;
}

static void setImage(sElement* self, const char* apath) {

	sElemRenderable* renderable = nElem->p->renderable(self);
	GxImage* image = elemGetAsset(self, apath, IMAGE);
	if(renderable->image != image){
		renderable->animation = NULL;
		renderable->animCounter = 0;
		renderable->animCurrent = 0;
		renderable->image = image;
	}
}

static void setAnimation(sElement* self, const char* apath) {
	
	sElemRenderable* renderable = nElem->p->renderable(self);
	GxAnimation* anim = elemGetAsset(self, apath, ANIMATION);
	
	renderable->image = NULL;
	renderable->animation = anim;
	renderable->animCounter = 0;
	renderable->animCurrent = 0;	
}

static void setText(sElement* self, const char* format, ...) {

	sElemRenderable* renderable = nElem->p->renderable(self);

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


static const char* getText(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->text;
}

static void setFontSize(sElement* self, int size) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	if(size <= 0) size = 16;
	if(size <= 8) size = 8;
	if (renderable->fontSize != size) {
		renderable->fontSize = size;
		renderable->shouldUpdateLabel = true;
	}
}

static int fontSize(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->fontSize;
}

static void setFont(sElement* self, const char* font) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	nUtil->assertArgument(font);
	if (renderable->font && strcmp(renderable->font, font) == 0) {
		return;
	}
	const void* fontExists = nApp->prv->getFontPath(font);
	nUtil->assertArgument(fontExists);
	if (fontExists) {
		free(renderable->font);
		renderable->font = nUtil->createString(font);
		renderable->shouldUpdateLabel = true;
	}
}

static const char* getFont(sElement* self) {
	sElemRenderable* renderable = nElem->p->renderable(self);
	return renderable->font;
}

static void pUpdateLabel(sElemRenderable* renderable) {

	if(renderable->label){
		GxDestroyImage_(renderable->label);
	}
	renderable->shouldUpdateLabel = false;

	if(renderable->text && renderable->fontSize > 0 &&
		renderable->font && renderable->color->value
	){
		renderable->label = GxImageCreateText_(renderable->text,
			renderable->font, renderable->fontSize, renderable->color->value
		);
	}
	else {
		renderable->label = NULL;
	}
}
