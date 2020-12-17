#ifndef GX_COMPONENT_TEXT
#define GX_COMPONENT_TEXT
#include "../../Gx.h"

/*
WARNING: this component render all text on creation and it's just fine for smaller texts up to 
2000 characters, even 10000 charactes wouldn't do much harm. However if only one component
is used to keep an entire ebook, then the computer will probably run out of RAM.
This problem can be solved, but is not a priority now, especially because this framework is not
intended to be used to create an non 2d game app.
*/

typedef struct sText {
	const char* text;
	const char* font;
	int fontSize;
	const char* color;
	double lineHeight;
	int firstLine;
	const char* alignment;	
} sText;

sElement* nTextCreate(sElement* base, const sText* config);
void nTextUpdate(sElement* base, const sText* config);
bool nTextNextPage(sElement* base);
bool nTextPreviousPage(sElement* base);
int nTextCurrentPage(sElement* base);




#endif // !GX_COMPONENT_TEXT
