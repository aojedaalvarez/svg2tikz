
/******************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include <wchar.h>

#define false 0
#define true !false

typedef struct
{
	wchar_t* tag;
	wchar_t** atributes;
	wchar_t** values;
	int atribc;
	wchar_t* content;
}
LINE;

typedef struct NODE
{
	wchar_t* opts;
	wchar_t** colors;
	int colorsCount;
	struct NODE* next;
}
NODE;

typedef struct LIST
{
	/* data */
	wchar_t* id;
	wchar_t* argument;
	struct LIST* next;
}
LIST;

typedef struct lineLIST
{
	/* data */
	wchar_t* id;
	LINE** lines;
	int linesCount;
	struct lineLIST* next;
}
lineLIST;

static int svgCount = 0;

extern _Bool isInDefs;

extern NODE* libraries;


wchar_t* getArgument(LIST* list, wchar_t* id);

int addArgument(LIST** list, wchar_t* id, wchar_t* argument);

int argumentRenew(LIST* list, wchar_t* id, wchar_t* argument);

int destroyList(LIST* list);


LINE** getLine(lineLIST* list, wchar_t* id);

int addDefs(lineLIST** list, wchar_t* id, LINE* line);

int addLine(lineLIST* list, LINE *line);

//int addAtributes(lineLIST* list, wchar_t* id, wchar_t** atributes, int atribc);

int destroyLineList(lineLIST* list);



void push(NODE** stack, wchar_t* opts);

void pop(NODE** stack);

wchar_t* top(NODE* stack);

int addNode(NODE** node, wchar_t* opts);

int addColor(NODE* node, wchar_t *color);

_Bool isInNode(NODE* node, wchar_t* opts);

_Bool isColor(NODE* node, wchar_t* color);

int destroyNode(NODE* node);


LINE* createLine(wchar_t* wcsLine);

int getValue(LINE* line, wchar_t* atribute);

int freeLine(LINE* line);



int processLine(LINE *line, FILE *infile, FILE *outfile);

wchar_t* svgArguments(LINE *line);

#endif