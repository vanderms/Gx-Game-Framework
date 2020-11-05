#include "../Utilities/GxUtil.h"
#include "../Private/GxPrivate.h"
#include "../Renderable/GxRenderable.h"
#include "../Folder/GxFolder.h"
#include "../Scene/GxScene.h"
#include "../Array/GxArray.h"
#include "../Tilemap/GxTilemap.h"
#include <string.h>

//... AUXILIARY STRUCTS
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

static Color* createColor(const char* value) {
	Color* self = calloc(1, sizeof(Color));
	GxAssertAllocationFailure(self);
	if (value) {
		self->value = malloc(sizeof(SDL_Color));
		GxAssertAllocationFailure(self->value);
		GxConvertColor(self->value, value);
		self->last = GmCreateString(value);
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
			GxAssertAllocationFailure(self->value);
		}
		GxConvertColor(self->value, value);
		self->last = GmCreateString(value);
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


typedef struct Border {
	int size;
	Color* color;
} Border;

//... MAIN TYPE
typedef struct GxRenderable {
	int type;
	int zIndex;
	SDL_RendererFlip orientation;
	char* asset;
	GxArray* folders;
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
} GxRenderable;


//... ALIGNMENT CONSTANTS
static const char* sRight = "right";
static const char* sLeft = "left";
static const char* sBottom = "bottom";
static const char* sTop = "top";
static const char* sCenter = "center";
static const char* sNum = "number";
static const char* sCenterCenter = "center|center";

//forward declaration
static void updateLabel(GxRenderable* renderable);

//constructors and destructors
GxRenderable* GxCreateRenderable_(GxElement* elem, const GxIni* ini) {

	if(!(ini->modules & (GxDisplayAbsolute | GxDisplayRelative))){
		GxAssertInvalidArgument(ini->modules & GxDisplayNone);
		return NULL;
	}

	GxRenderable* self = calloc(1, sizeof(GxRenderable));
	GxAssertAllocationFailure(self);
	self->type = ini->modules & GxDisplayAbsolute ? GxDisplayAbsolute : GxDisplayRelative;
	self->zIndex = ini->zIndex;
	elem->renderable = self;
	//... folders
	if (ini->folders) {
		self->folders = GmArraySplit(ini->folders, "|");
		for (Uint32 i = 0; i < GxArraySize(self->folders); i++) {
			GxFolder* folder = GxGetFolder_(GxArrayAt(self->folders, i));
			GxAssertInvalidArgument(folder);
			GxArrayInsert(self->folders, i, folder, NULL);
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
	self->text = ini->text ? GmCreateString(ini->text) : NULL;
	GxElemSetFont(elem, ini->font ? ini->font : "Default");
	self->shouldUpdateLabel = ini->text ? true : false;

	self->border.color = createColor(NULL);
	GxElemSetBorder(elem, ini->border);
	self->wflag = 0;
	return self;
}

void GxDestroyRenderable_(GxRenderable* self) {
	if (self) {
		if(GxAppIsRunning_() && self->folders){
			for (Uint32 i = 0; i < GxArraySize(self->folders); i++){
				GxFolder* folder = GxArrayAt(self->folders, i);
				GxFolderDecRefCounter_(folder);
			}
		}
		free(self->asset);
		free(self->alignment);
		free(self->text);
		free(self->font);
		destroyColor(self->color);
		destroyColor(self->backgroundColor);
		destroyColor(self->border.color);
		GxDestroyImage_(self->label);
		GxDestroyArray(self->folders);
		free(self);
	}
}

bool GxElemHasRelativePosition(GxElement* self) {
	validateElem(self, false, false);
	return self->renderable && self->renderable->type == GxDisplayRelative;
}

bool GxElemHasAbsolutePosition(GxElement* self) {
	validateElem(self, false, false);
	return self->renderable && self->renderable->type == GxDisplayAbsolute;
}


int GxElemGetZIndex(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->zIndex;
}

void GxElemSetZIndex(GxElement* self, int value) {
	validateElem(self, false, true);
	self->renderable->zIndex = value;
}

int GxElemGetOrientation(GxElement* self) {
	validateElem(self, false, true);
	return (int) self->renderable->orientation;
}

void GxElemSetOrientation(GxElement* self, int value) {
	validateElem(self, false, true);
	GxAssertInvalidArgument(value == GxElemForward || value == GxElemBackward);
	self->renderable->orientation = (SDL_RendererFlip) value;
}

const char* GxElemGetImage(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->image ? self->renderable->asset : NULL;
}

const char* GxElemGetAnimation(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->animation ? self->renderable->asset : NULL;
}

const char* GxElemGetAlignment(GxElement* self) {
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

void GxElemSetAlignment(GxElement* self, const char* value) {
	validateElem(self, false, true);
	if (!self->renderable->alignment) {
		self->renderable->alignment = calloc(1, sizeof(Alignment));
		GxAssertAllocationFailure(self->renderable->alignment);
	}
	if (value == sCenterCenter) {
		self->renderable->alignment->horizontal = sCenter;
		self->renderable->alignment->vertical = sCenter;
	}
	else if (strstr(self->renderable->alignment->last, value)) {
		return;
	}
	else {
		char* align = GmCreateString(value);
		char* div = strstr(align, "|");
		GxAssertInvalidArgument(div);
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

bool GxElemIsHidden(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->hidden;
}

void GxElemHide(GxElement* self) {
	validateElem(self, false, true);
	self->renderable->hidden = true;
}

void GxElemShow(GxElement* self) {
	validateElem(self, false, true);
	self->renderable->hidden = false;
}

double GxElemGetAngle(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->angle;
}

void GxElemSetAngle(GxElement* self, double angle) {
	validateElem(self, false, true);
	self->renderable->angle = angle;
}

double GxElemGetProportion(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->proportion;
}

void GxElemSetProportion(GxElement* self, double proportion) {
	validateElem(self, false, true);
	self->renderable->proportion = proportion;
}

void GxElemSetToFit(GxElement* self, const char* axis) {
	validateElem(self, false, true);
	GxAssertInvalidArgument(strcmp(axis, "horizontal") == 0 || strcmp(axis, "vertical") == 0);
	GxSize size = GxImageGetSize_(self->renderable->image);
	if (strcmp(axis, "horizontal") == 0)
		self->renderable->proportion = ((double) self->pos->w) / size.w;
	else self->renderable->proportion = ((double) self->pos->h) / size.h;
}

const SDL_Color* GxElemGetColor(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->color->value;
}

void GxElemSetColor(GxElement* self, const char* color) {
	validateElem(self, false, true);
	if(updateColor(self->renderable->color, color)){
		self->renderable->shouldUpdateLabel = true;
	}
}

const SDL_Color* GxElemGetBackgroundColor(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->backgroundColor->value;
}

void GxElemSetBackgroundColor(GxElement* self, const char* color) {
	validateElem(self, false, true);
	updateColor(self->renderable->backgroundColor, color);
}

int GxElemGetBorderSize(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->border.size;
}

const SDL_Color* GxElemGetBorderColor(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->border.color->value;
}

void GxElemSetBorder(GxElement* self, const char* border) {
	validateElem(self, false, true);

	//if null, reset border
	if (!border) {
		self->renderable->border.size = 0;
		updateColor(self->renderable->border.color, NULL);
		self->renderable->border.size = 0;
		return;
	}

	//get size and color strings
	char* size = GxCloneString(border, (char[64]){0}, 64);
	char* color = strstr(size, "|");
	GxAssertInvalidArgument(color);
	color[0] = '\0';
	color++;

	//set size
	self->renderable->border.size = atoi(size);

	//set color
	if(color[0] != '\0'){
		updateColor(self->renderable->border.color, color);
	}
}

uint32_t GxElemGetWFlag_(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->wflag;
}

void GxElemSetWFlag_(GxElement* self, uint32_t value) {
	validateElem(self, false, true);
	self->renderable->wflag = value;
}


//element render methods
static inline SDL_Rect calcAbsolutePos(GxElement* self) {
	int y = GxGetWindowSize().h - (self->pos->y + self->pos->h);
	return (SDL_Rect) { self->pos->x, y, self->pos->w, self->pos->h };
}

static inline SDL_Rect calcRelativePos(GxElement* self) {
	const SDL_Rect* cpos = GxSceneGetCamera(self->scene)->pos;
	int x = self->pos->x - cpos->x;
	int y = (cpos->y + cpos->h) - (self->pos->y + self->pos->h);
	return (SDL_Rect) { x, y, self->pos->w, self->pos->h };
}

static inline void applyWidgetData(GxElement* self, SDL_Rect* pos, GxImage* image) {

	GxSize imgsize = GxImageGetSize_(image);
	pos->w = (int) (imgsize.w * self->renderable->proportion + 0.5);
	pos->h = (int) (imgsize.h * self->renderable->proportion + 0.5);

	//horizontal alignment -> (left is default)
	if(!self->renderable->alignment ||
		self->renderable->alignment->horizontal == sCenter) {
		pos->x += (self->pos->w - pos->w) / 2;
	}
	else if (self->renderable->alignment->horizontal ==  sRight){
		pos->x += self->pos->w - pos->w;
	}
	else if(self->renderable->alignment->horizontal == sNum) {
		pos->x += (self->pos->w - pos->w) / 2; //first set default
		pos->x += self->renderable->alignment->x;
	}

	//vertical alignment (top is default)
	if (!self->renderable->alignment ||
		self->renderable->alignment->vertical == sCenter){
		pos->y += (self->pos->h - pos->h) / 2;
	}
	else if (self->renderable->alignment->vertical == sBottom) {
		pos->y += (self->pos->h - pos->h);
	}
	else if(self->renderable->alignment->vertical == sNum) {
		pos->y += (self->pos->h - pos->h) / 2; //first set default
		pos->y -= self->renderable->alignment->y;
	}
}

static inline SDL_Rect* calcAbsoluteImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	applyWidgetData(self, pos, image);
	return pos;
}

static inline SDL_Rect* calcRelativeImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	applyWidgetData(self, pos, image);
	return pos;
}

 SDL_Rect* GxElemCalcImagePos(GxElement* self, SDL_Rect* pos, GxImage* image) {
	return (
		self->renderable->type == GxDisplayAbsolute ?
		calcAbsoluteImagePos(self, pos, image) :
		calcRelativeImagePos(self, pos, image)
	);
}

SDL_Rect GxGetElemPositionOnWindow(GxElement* self) {
	validateElem(self, false, true);
	return (
		self->renderable->type == GxDisplayAbsolute ?
		calcAbsolutePos(self) :
		calcRelativePos(self)
	);
}

static inline void renderBorder(SDL_Renderer* renderer, SDL_Rect* pos, int quantity) {
	if (quantity <= 0) return;
	if (pos->w == 0 || pos->h == 0) return;	
	SDL_RenderDrawRect(renderer, pos);
	pos->x++;
	pos->y++;
	pos->w -= 2;
	pos->h -= 2;
	renderBorder(renderer, pos, quantity - 1);
}


void GxElemRender_(GxElement* self) {

	SDL_Renderer* renderer = GxGetSDLRenderer();
	SDL_Rect pos = GxGetElemPositionOnWindow(self);
	SDL_Rect labelPos = pos;
	SDL_Color* color = self->renderable->backgroundColor->value;

	//first backgrund color
	if (color && color->a != 0) {
		int bs = self->renderable->border.size;
		SDL_Rect square = {pos.x + bs, pos.y + bs, pos.w - 2*bs, pos.h - 2*bs};
		SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
		SDL_Rect* dst = GxAppCalcDest(&square, &(SDL_Rect){0});
		SDL_RenderFillRect(renderer, dst);
		//SDL_RenderDrawRect(renderer, &pos);
	}

	//then borders
	SDL_Color* borderColor = self->renderable->border.color->value;
	int bsize = self->renderable->border.size;

	if (bsize > 0 && borderColor && borderColor->a) {
		SDL_SetRenderDrawColor(renderer,
			borderColor->r, borderColor->g, borderColor->b, borderColor->a
		);
		SDL_Rect* dst = GxAppCalcDest(&pos, &(SDL_Rect){0});
		renderBorder(renderer, dst, bsize);
	}

	//then image or animation
	GxImage* image = NULL;
	if (self->renderable->image) {
		image = self->renderable->image;
	}
	else if (self->renderable->animation) {
		GxAnimation* anim = self->renderable->animation; //create alias
		self->renderable->animCounter++; //to avoid counter starting with 0.
		if (self->renderable->animCurrent >= GxAnimGetQuantity_(anim)) {
			self->renderable->animCurrent = 0;
		}
		image = GxAnimGetImage_(anim, self->renderable->animCurrent);

		if (self->renderable->animCounter % GxAnimGetInterval_(anim) == 0)
			self->renderable->animCurrent++;

			//if it's the last frame in animation and there's no repeat set renderable->animation to NULL
		if (self->renderable->animCurrent >= GxAnimGetQuantity_(anim) && !GxAnimIsContinous_(anim)) {
			self->renderable->animation = NULL;
			self->renderable->animCounter = 0;
			self->renderable->animCurrent = 0;
		}
	}

	if (image) {
		GxElemCalcImagePos(self, &pos, image);
		GxImageRender_(image, &pos, self->renderable->angle, self->renderable->orientation);
	}

	//finally, render label
	if (self->renderable->shouldUpdateLabel) {
		updateLabel(self->renderable);
	}
	if (self->renderable->label) {
		GxElemCalcImagePos(self, &labelPos, self->renderable->label);
		GxImageRender_(self->renderable->label, &labelPos, 0.0, (SDL_RendererFlip) GxElemForward);
	}
}

enum AssetType{IMAGE, ANIMATION};

static inline void* elemGetAsset(GxElement* self, const char* apath, enum AssetType type) {

	if (apath != NULL) {
		if (self->renderable->asset && (strcmp(self->renderable->asset, apath) == 0)){
			return ( type == IMAGE ?
				(void*) self->renderable->image : (void*) self->renderable->animation
			);
		}
		free(self->renderable->asset);
		self->renderable->asset = GmCreateString(apath);
		char* slash = strstr(self->renderable->asset, "/");
		GxAssertInvalidArgument(slash);

		*slash = '\0';
		GxFolder* folder = GxGetFolder_(self->renderable->asset);
		GxAssertInvalidArgument(folder);

		bool folderIsLoaded = GxFolderHasStatus_(folder, GxStatusLoading) ||
			GxFolderHasStatus_(folder, GxStatusReady);
		GxAssertInvalidOperation(folderIsLoaded);

		*slash = '/';
		void* asset = (type == IMAGE ?
			(void*) GxFolderGetImage_(folder, slash + 1) :
			(void*) GxFolderGetAnimation_(folder, slash + 1)
		);
		GxAssertInvalidArgument(asset);
		return asset;
	}
	return NULL;
}

void GxElemSetImage(GxElement* self, const char* apath) {

	validateElem(self, false, true);
	GxAssertInvalidOperation(!GxIsTilemap(self));
	GxImage* image = elemGetAsset(self, apath, IMAGE);
	if(self->renderable->image != image){
		self->renderable->animation = NULL;
		self->renderable->animCounter = 0;
		self->renderable->animCurrent = 0;
		self->renderable->image = image;
	}
}

void GxTilemapSetImage_(GxElement* self, GxImage* pallete) {
	validateElem(self, false, true);
	self->renderable->image = pallete;
}

void GxElemSetAnimation(GxElement* self, const char* apath) {
	
	validateElem(self, false, true);
	GxAssertInvalidOperation(!GxIsTilemap(self));
	GxAnimation* anim = elemGetAsset(self, apath, ANIMATION);
	
	self->renderable->image = NULL;
	self->renderable->animation = anim;
	self->renderable->animCounter = 0;
	self->renderable->animCurrent = 0;	
}


static void updateLabel(GxRenderable* renderable) {

	GxDestroyImage_(renderable->label);
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

void GxElemSetText(GxElement* self, const char* format, ...) {

	validateElem(self, false, true);

	if (format){
		static char text[1024];
		va_list args;
		va_start(args, format);
		vsnprintf(text, 1024, format, args);
		va_end(args);

		if(!self->renderable->text) {
			self->renderable->text = GmCreateString(text);
			self->renderable->shouldUpdateLabel = true;
		}
		else { // self->renderable text != NULL
			if (strcmp(text, self->renderable->text) != 0) {
				free(self->renderable->text);
				self->renderable->text = GmCreateString(text);
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


const char* GxElemGetText(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->text;
}

void GxElemSetFontSize(GxElement* self, int size) {
	validateElem(self, false, true);
	if(size <= 0) size = 16;
	if(size <= 8) size = 8;
	if (self->renderable->fontSize != size) {
		self->renderable->fontSize = size;
		self->renderable->shouldUpdateLabel = true;
	}
}

int GxElemGetFontSize(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->fontSize;
}

void GxElemSetFont(GxElement* self, const char* font) {
	validateElem(self, false, true);
	GxAssertInvalidArgument(font);
	if (self->renderable->font && strcmp(self->renderable->font, font) == 0) {
		return;
	}
	const void* fontExists = GxGetFontPath_(font);
	GxAssertInvalidArgument(fontExists);
	if (fontExists) {
		free(self->renderable->font);
		self->renderable->font = GmCreateString(font);
		self->renderable->shouldUpdateLabel = true;
	}
}

const char* GxElemGetFont(GxElement* self) {
	validateElem(self, false, true);
	return self->renderable->font;
}
