
/*********************************************/

#include <stdio.h>
#include <wchar.h>
#include "lines.h"

#define PADDING_SIZE 4

extern NODE* globalArgs;
extern NODE* textArgs;
extern int globalPadding;
extern LIST *typelist;
extern lineLIST* defsList;


typedef struct 
{
	float x;
	float y;
	wchar_t path;
}
POINT;


static wchar_t inherit[12] = L"anchor=south";


static inline int iswdecimal(wchar_t wchar) {
	return iswdigit(wchar) || wchar == '.';
}

wchar_t* addPadding(wchar_t* dest, int padding);

wchar_t* addStr(wchar_t* dest, const wchar_t* src);

wchar_t* addChar(wchar_t* dest, const wchar_t chr);

int addPoint(wchar_t** commd, const wchar_t* path, int i);

wchar_t* convertColor(FILE* outfile, wchar_t* sColor);

wchar_t* shapeOptions(wchar_t* args, NODE* glArgs, LINE* line, FILE* outfile);

wchar_t* textOptions(wchar_t* args, NODE* glArgs, LINE* line);

wchar_t* commonOptions(wchar_t* args, LINE* line);

int tikzPath(LINE* line, FILE* outfile);

int tikzPolyline(LINE* line, FILE* outfile);

int tikzLine(LINE* line, FILE* outfile);

int tikzRect(LINE* line, FILE* outfile);

int tikzEllipse(LINE* line, FILE* outfile);

int tikzG(LINE* line, FILE* outfile);

int tikzCloseG(LINE* line, FILE* outfile);

int tikzText(LINE* line, FILE* outfile);

int tikzPattern(LINE* line, FILE* outfile);

int tikzlinearGradient(LINE* line, FILE *infile, FILE* outfile);