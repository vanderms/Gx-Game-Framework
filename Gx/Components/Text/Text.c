#include "Text.h"

enum sTextAlignment {
	HORIZONTAL,
	VERTICAL,
	LEFT,
	RIGHT,	
	CENTER,
	JUSTIFIED,
	TOP,
	BOTTOM,
};

typedef struct Text {
	sElement* base;	
	sArray* words;
	sSize letterSize;
	double lineHeight;
	sSize boxSize;	
	enum sTextAlignment halign;
	enum sTextAlignment valign;	
	int totalPages;
	int pageHeight;
	int currentPage;
} Text;

typedef struct WordData {
	sPoint pos;
	sImage* image;
} WordData;


static void destroyWord(WordData* self) {
	nImageDestroyText(self->image);
	free(self);
}

static void setAxisAlignment(int axis, enum sTextAlignment* align, const char* value) {	

	if (axis == HORIZONTAL) {
		if (strcmp(value, "right") == 0) {
			*align = RIGHT;
		}
		else if (strcmp(value, "left") == 0) {
			*align = LEFT;
		}
		else if (strcmp(value, "center") == 0) {
			*align = CENTER;
		}
		else if (strcmp(value, "justified") == 0) {
			*align = JUSTIFIED;
		}
		else nUtilAssertState(false);
	}
	else if (axis == VERTICAL) {
		if (strcmp(value, "top") == 0) {
			*align = TOP;
		}
		else if (strcmp(value, "bottom") == 0) {
			*align = BOTTOM;
		}
		else if (strcmp(value, "center") == 0) {
			*align = CENTER;
		}
		else nUtilAssertState(false);
	}
}


static void setAlignment(Text* self, const sText* config) {
	
	if (config->alignment) {
		sArray* tokens = nAppTokenize(config->alignment, "|");
		nUtilAssertArgument(nArraySize(tokens) <= 2);
		setAxisAlignment(HORIZONTAL, &self->halign, nArrayAt(tokens, 0));
		if (nArraySize(tokens) == 2) {
			setAxisAlignment(VERTICAL, &self->valign, nArrayAt(tokens, 1));
		}
		else {
			self->valign = TOP;
		}
	}
	else {
		self->halign = LEFT;
		self->valign = TOP;
	}
}


static void alignHorizontally(Text* self) {
		
	int baseLine = -1;
	Uint32 first = 0;
	Uint32 last = nArraySize(self->words) - 1;
	int quantity = 0;	
	int extraSpace = 0;
	int sentenceLength = 0;

	//this condition is really strange, but it is well defined behaviour, i think...
	for (Uint32 i = last; i <= last; i--) {
		WordData* data = nArrayAt(self->words, i);
		if (baseLine == -1) {
			baseLine = data->pos.y;
		}
		if (baseLine != data->pos.y) {
			first = i + 1;
			break;
		}
		sentenceLength += nImageSize(data->image)->w;
	}

	quantity = last - first + 1;
	WordData* firstWord = nArrayAt(self->words, first);
	sentenceLength += firstWord->pos.x;
	sentenceLength += ((quantity - 1) * self->letterSize.w);	
	extraSpace = self->boxSize.w - sentenceLength;

	if (extraSpace <= 0) { return; }

	for (Uint32 i = first; i <= last; i++) {
		WordData* data = nArrayAt(self->words, i);
		if (self->halign == RIGHT) {
			data->pos.x += extraSpace;
		}
		else if (self->halign == CENTER) {
			data->pos.x += (extraSpace / 2);
		}
		else if (self->halign == JUSTIFIED) {
			int space = extraSpace;
			if (i != last) {
				space = (int) (((i - first) / (double) quantity) * extraSpace);
			}			
			data->pos.x += space;			
		}
	}
}

static void alignVertically(Text* self) {

	WordData* lastWord = nArrayLast(self->words);
	if (!lastWord) {
		return;
	}
	const int textHeight = lastWord->pos.y + self->letterSize.h;
	int diff = 0;

	if (self->valign == CENTER) {
		diff = (self->boxSize.h - textHeight) / 2;
	}
	else if (self->valign == BOTTOM) {
		diff = self->boxSize.h - textHeight;
	}

	if (diff != 0) {
		for (Uint32 i = 0; i < nArraySize(self->words); i++) {
			WordData* data = nArrayAt(self->words, i);
			data->pos.y += diff;
		}
	}
}

static void setPagesAttributes(Text* self) {
	const int LS = self->letterSize.h;
	const int BS = self->boxSize.h;
	const int LH = (int) (self->lineHeight * self->letterSize.h);	
	const int lines = (BS + LH - LS) / LH;
	self->pageHeight = lines * LH;
	WordData* last = nArrayLast(self->words);
	self->totalPages = 1 + ((last->pos.y + LS) / self->pageHeight);
	self->currentPage = 1;	
}

static void updateWordArray(Text* self, const sText* config) {

	sArray* paragraphs = nAppTokenize(config->text, "\n");
	SDL_Color color = {0, 0, 0, 255};
	if (config->color) {
		nAppConvertColor(&color, config->color);
	}

	const char* font = config->font ? config->font : "Default";
	int size = config->fontSize > 4 ? config->fontSize : 16;
	sPoint pos = {0, 0};

	int firstLine = 0;
	if (config->firstLine > 0) {
		double ratio = (double) self->boxSize.w / nElemPosition(self->base)->w;
		firstLine = (int) (config->firstLine * ratio);
	}

	for (Uint32 i = 0; i < nArraySize(paragraphs); i++) {
		const char* paragraph = nArrayAt(paragraphs, i);
		sArray* words = nAppTokenize(paragraph, " ");
		pos.x = firstLine;
		for (Uint32 j = 0; j < nArraySize(words); j++) {
			char* word = nArrayAt(words, j);
			if (strcmp(word, "") == 0) {
				continue;
			}
			//create and insert wordData
			WordData* data = nUtilAssertAlloc(malloc(sizeof(WordData)));
			data->image = nImageCreateText(word, font, size, &color);

			//set pos
			sSize wordSize = *nImageSize(data->image);
			if (pos.x == firstLine || (pos.x + wordSize.w < self->boxSize.w)) {
				data->pos = pos;
			}
			else {
				if (self->halign != LEFT) {
					alignHorizontally(self);
				}
				pos.y += (int) (self->letterSize.h * self->lineHeight);
				pos.x = 0;
				data->pos = pos;
			}
			pos.x += wordSize.w + self->letterSize.w;
			nArrayPush(self->words, data, destroyWord);
		}

		//align last line of paragraph
		if (self->halign != LEFT && self->halign != JUSTIFIED) {
			alignHorizontally(self);
		}
		//set a new line for the next paragraph
		pos.y += (int) (self->letterSize.h * self->lineHeight);
	}

	if (self->valign != TOP) {
		alignVertically(self);
	}
	else {
		setPagesAttributes(self);
	}	
}


static void onRender(sEvent* e) {
	Text* self = e->target;

	sRect onCameraPos = nElemCalcPosOnCamera(self->base);
	sRect* pos = nAppCalcDest(&onCameraPos, &(sRect){0, 0, 0, 0});
	Uint8 opacity = nElemOpacity(self->base);
	int last = INT_MIN;

	for (Uint32 i = 0; i < nArraySize(self->words); i++) {
		WordData* data = nArrayAt(self->words, i);		
		if (data->pos.y < 0) {
			continue;
		}
		else if (data->pos.y + self->letterSize.h > self->boxSize.h){
			break;
		}
		else {
			
			const sSize* wsize = nImageSize(data->image);
			sRect wordPos = {pos->x + data->pos.x, pos->y + data->pos.y, wsize->w, wsize->h};
			SDL_Texture* texture = nImageSDLTexture(data->image);
			SDL_SetTextureAlphaMod(texture, opacity);
			SDL_RenderCopy(nAppSDLRenderer(), texture, NULL, &wordPos);
			SDL_SetTextureAlphaMod(texture, 255);
		}				
	}
}

static void onDestroy(sEvent* e) {
	Text* self = e->target;	
	nArrayDestroy(self->words);
	free(self);
}

void nTextUpdate(sElement* base, const sText* config) {
	
	Text* self = nElemGetComponent(base, "nText");

	nArrayClean(self->words);
	const char* font = config->font ? config->font : "Default";
	int size = config->fontSize > 4 ? config->fontSize : 16;
	sImage* temp = nImageCreateText("a", font, size, &(SDL_Color){0, 0, 0, 255});
	self->letterSize = *nImageSize(temp);
	self->lineHeight = config->lineHeight > 1.0 ? config->lineHeight : 1.0;	
	nImageDestroyText(temp);	

	setAlignment(self, config);
	updateWordArray(self, config);
}

bool nTextNextPage(sElement* base) {
	Text* self = nElemGetComponent(base, "nText");
	nUtilAssertState(self->valign == TOP);
	if (self->currentPage < self->totalPages) {
		for (Uint32 i = 0; i < nArraySize(self->words); i++) {
			WordData* data = nArrayAt(self->words, i);
			data->pos.y -= self->pageHeight;
		}
		self->currentPage++;
		return true;
	}
	return false;
}

bool nTextPreviousPage(sElement* base) {
	Text* self = nElemGetComponent(base, "nText");
	nUtilAssertState(self->valign == TOP);
	if (self->currentPage > 1) {
		for (Uint32 i = 0; i < nArraySize(self->words); i++) {
			WordData* data = nArrayAt(self->words, i);
			data->pos.y += self->pageHeight;
		}
		self->currentPage--;
		return true;
	}
	return false;	
}

int nTextCurrentPage(sElement* base) {
	Text* self = nElemGetComponent(base, "nText");
	return self->currentPage;
}

sElement* nTextCreate(sElement* base, const sText* config){

	nUtilAssertArgument(nElemIsRenderable(base));
	Text* self = nUtilAssertAlloc(malloc(sizeof(Text)));
	self->base = base;	
	self->words = nArrayCreate();
	sRect onCameraPos = nElemCalcPosOnCamera(self->base);
	sRect* basePos = nAppCalcDest(&onCameraPos, &(sRect){0, 0, 0, 0});	
	self->boxSize.w = basePos->w;
	self->boxSize.h = basePos->h;

	nElemAddComponent(base, &(sComponent){
		.name = "nText",
		.target = self,
		.onDestroy = onDestroy,
		.onRender = onRender,
	});

	nTextUpdate(self->base, config);

	return self->base;
}