#include "../Utilities/Util.h"
#include "../Private/sElement.h"
#include "../Renderable/sElemRenderable.h"
#include "../Folder/GxFolder.h"
#include "../Scene/GxScene.h"
#include "../Array/Array.h"
#include <string.h>


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
sElemRenderable* GxCreateRenderable_(sElement* elem, const sIni* ini) {
	
	if(ini->display != GxElemAbsolute && ini->display != GxElemRelative){
		nUtil->assertArgument(ini->display == GxElemNone);
		return NULL;
	}

	sElemRenderable* self = calloc(1, sizeof(sElemRenderable));
	nUtil->assertAlloc(self);
	self->type = ini->display == GxElemAbsolute ? GxElemAbsolute : GxElemRelative;
	self->zIndex = ini->zIndex;
	elem->renderable = self;
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

void GxDestroyRenderable_(sElemRenderable* self) {
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

bool GxElemHasRelativePosition(sElement* self) {
	validateElem(self, false, false);
	return self->renderable && self->renderable->type == GxElemRelative;
}

bool GxElemHasAbsolutePosition(sElement* self) {
	validateElem(self, false, false);
	return self->renderable && self->renderable->type == GxElemAbsolute;
}


int GxElemGetZIndex(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->zIndex;
}

void GxElemSetZIndex(sElement* self, int value) {
	validateElem(self, false, true);
	self->renderable->zIndex = value;
}

Uint8 GxElemGetOpacity(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->opacity;
}

void GxElemSetOpacity(sElement* self, Uint8 value) {
	validateElem(self, false, true);
	self->renderable->opacity = value;
}

int GxElemGetOrientation(sElement* self) {
	validateElem(self, false, true);
	return (int) self->renderable->orientation;
}

void GxElemSetOrientation(sElement* self, int value) {
	validateElem(self, false, true);
	nUtil->assertArgument(value == GxElemForward || value == GxElemBackward);
	self->renderable->orientation = (SDL_RendererFlip) value;
}

const char* GxElemGetImage(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->image ? self->renderable->asset : NULL;
}

const char* GxElemGetAnimation(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->animation ? self->renderable->asset : NULL;
}

const char* GxElemGetAlignment(sElement* self) {
	validateElem(self, false, true);
	if (!self->renderable->alignment) {
		return sCenterCenter;
	}
	if(self->renderable->alignment->changed){
		const char* horizontal = NULL, *vertical = NULL;
		char hbuf[12], vbuf[12];

		if (self->renderable->alignment->horizontal == sNum) {
			snprintf(hbuf, 12, "%d", self->renderable->alignment->x);
			horizontal = hbuf;
		}
		else horizontal = self->renderable->alignment->horizontal;

		if (self->renderable->alignment->vertical == sNum) {
			snprintf(vbuf, 12, "%d", self->renderable->alignment->y);
			vertical = vbuf;
		}
		else horizontal = self->renderable->alignment->horizontal;

		snprintf(self->renderable->alignment->last, 25, "%s|%s", horizontal, vertical);
		self->renderable->alignment->changed = false;
	}
	return self->renderable->alignment->last;
}

void GxElemSetAlignment(sElement* self, const char* value) {
	validateElem(self, false, true);
	if (!self->renderable->alignment) {
		self->renderable->alignment = calloc(1, sizeof(Alignment));
		nUtil->assertAlloc(self->renderable->alignment);
	}
	if (value == sCenterCenter) {
		self->renderable->alignment->horizontal = sCenter;
		self->renderable->alignment->vertical = sCenter;
	}
	else if (strstr(self->renderable->alignment->last, value)) {
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
				self->renderable->alignment->horizontal = sRight;
			}
			else if (strcmp(horizontal, "left") == 0) {
				self->renderable->alignment->horizontal = sLeft;
			}
			else if(strcmp(horizontal, "center") == 0){
				self->renderable->alignment->horizontal = sCenter;
			}
			else {
				self->renderable->alignment->horizontal = sNum;
				self->renderable->alignment->x = atoi(horizontal);
			}
		}

		//set vertical
		if (vertical) {
			if (strcmp(vertical, "top") == 0) {
				self->renderable->alignment->vertical = sTop;
			}
			else if (strcmp(vertical, "bottom") == 0) {
				self->renderable->alignment->vertical = sBottom;
			}
			else if(strcmp(vertical, "center") == 0){
				self->renderable->alignment->vertical = sCenter;
			}
			else {
				self->renderable->alignment->vertical = sNum;
				self->renderable->alignment->y = atoi(vertical);
			}
		}

		//set GxElemGetAlignment flag
		self->renderable->alignment->changed = true;

		//then free align
		free(align);
	}
}

bool GxElemIsHidden(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->hidden;
}

void GxElemHide(sElement* self) {
	validateElem(self, false, true);
	self->renderable->hidden = true;
}

void GxElemShow(sElement* self) {
	validateElem(self, false, true);
	self->renderable->hidden = false;
}

double GxElemGetAngle(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->angle;
}

void GxElemSetAngle(sElement* self, double angle) {
	validateElem(self, false, true);
	self->renderable->angle = angle;
}

double GxElemGetProportion(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->proportion;
}

void GxElemSetProportion(sElement* self, double proportion) {
	validateElem(self, false, true);
	self->renderable->proportion = proportion;
}

void GxElemSetToFit(sElement* self, const char* axis) {
	validateElem(self, false, true);
	nUtil->assertArgument(strcmp(axis, "horizontal") == 0 || strcmp(axis, "vertical") == 0);
	GxSize size = GxImageGetSize_(self->renderable->image);
	if (strcmp(axis, "horizontal") == 0)
		self->renderable->proportion = ((double) self->pos->w) / size.w;
	else self->renderable->proportion = ((double) self->pos->h) / size.h;
}

const SDL_Color* GxElemGetColor(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->color->value;
}

void GxElemSetColor(sElement* self, const char* color) {
	validateElem(self, false, true);
	if(updateColor(self->renderable->color, color)){
		self->renderable->shouldUpdateLabel = true;
	}
}

const SDL_Color* GxElemGetBackgroundColor(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->backgroundColor->value;
}

void GxElemSetBackgroundColor(sElement* self, const char* color) {
	validateElem(self, false, true);
	updateColor(self->renderable->backgroundColor, color);
}

int GxElemGetBorderSize(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->border.size;
}

const SDL_Color* GxElemGetBorderColor(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->border.color->value;
}

void GxElemSetBorder(sElement* self, const char* border) {
	validateElem(self, false, true);

	//if null, reset border
	if (!border) {
		self->renderable->border.size = 0;
		updateColor(self->renderable->border.color, NULL);
		self->renderable->border.size = 0;
		return;
	}

	//get size and color strings
	char* size = nUtil->cloneString(border, (char[64]){0}, 64);
	char* color = strstr(size, "|");
	nUtil->assertArgument(color);
	color[0] = '\0';
	color++;

	//set size
	self->renderable->border.size = atoi(size);

	//set color
	if(color[0] != '\0'){
		updateColor(self->renderable->border.color, color);
	}
}

uint32_t GxElemGetWFlag_(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->wflag;
}

void GxElemSetWFlag_(sElement* self, uint32_t value) {
	validateElem(self, false, true);
	self->renderable->wflag = value;
}



enum AssetType{IMAGE, ANIMATION};

static inline void* elemGetAsset(sElement* self, const char* apath, enum AssetType type) {

	if (apath != NULL) {
		if (self->renderable->asset && (strcmp(self->renderable->asset, apath) == 0)){
			return ( type == IMAGE ?
				(void*) self->renderable->image : (void*) self->renderable->animation
			);
		}
		free(self->renderable->asset);
		self->renderable->asset = nUtil->createString(apath);
		char* slash = strstr(self->renderable->asset, "/");
		nUtil->assertArgument(slash);

		*slash = '\0';
		GxFolder* folder = nApp->prv->getFolder(self->renderable->asset);
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

void GxElemSetImage(sElement* self, const char* apath) {

	validateElem(self, false, true);	
	GxImage* image = elemGetAsset(self, apath, IMAGE);
	if(self->renderable->image != image){
		self->renderable->animation = NULL;
		self->renderable->animCounter = 0;
		self->renderable->animCurrent = 0;
		self->renderable->image = image;
	}
}

void GxTilemapSetImage_(sElement* self, GxImage* pallete) {
	validateElem(self, false, true);
	self->renderable->image = pallete;
}

void GxElemSetAnimation(sElement* self, const char* apath) {
	
	validateElem(self, false, true);	
	GxAnimation* anim = elemGetAsset(self, apath, ANIMATION);
	
	self->renderable->image = NULL;
	self->renderable->animation = anim;
	self->renderable->animCounter = 0;
	self->renderable->animCurrent = 0;	
}




void GxElemSetText(sElement* self, const char* format, ...) {

	validateElem(self, false, true);

	if (format){
		static char text[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(text, 1024, format, args);
		va_end(args);

		if(!self->renderable->text) {
			self->renderable->text = nUtil->createString(text);
			self->renderable->shouldUpdateLabel = true;
		}
		else { // self->renderable text != NULL
			if (strcmp(text, self->renderable->text) != 0) {
				free(self->renderable->text);
				self->renderable->text = nUtil->createString(text);
				self->renderable->shouldUpdateLabel = true;
			}
		}
	}
	else { // !format
		if (self->renderable->text) {
			free(self->renderable->text);
			self->renderable = NULL;
			self->renderable->shouldUpdateLabel = true;
		}
		//if not text formatted and not self->renderable->text, then do nothing.
	}
}


const char* GxElemGetText(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->text;
}

void GxElemSetFontSize(sElement* self, int size) {
	validateElem(self, false, true);
	if(size <= 0) size = 16;
	if(size <= 8) size = 8;
	if (self->renderable->fontSize != size) {
		self->renderable->fontSize = size;
		self->renderable->shouldUpdateLabel = true;
	}
}

int GxElemGetFontSize(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->fontSize;
}

void GxElemSetFont(sElement* self, const char* font) {
	validateElem(self, false, true);
	nUtil->assertArgument(font);
	if (self->renderable->font && strcmp(self->renderable->font, font) == 0) {
		return;
	}
	const void* fontExists = nApp->prv->getFontPath(font);
	nUtil->assertArgument(fontExists);
	if (fontExists) {
		free(self->renderable->font);
		self->renderable->font = nUtil->createString(font);
		self->renderable->shouldUpdateLabel = true;
	}
}

const char* GxElemGetFont(sElement* self) {
	validateElem(self, false, true);
	return self->renderable->font;
}

void GxElementUpdateLabel_(sElemRenderable* renderable) {

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
