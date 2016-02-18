///////////////////////////////////////////////

#include <wchar.h>


#ifndef PROCESS_H
#define PROCESS_H

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

static int svgCount = 0;
#endif

extern NODE* libraries;

wchar_t* getArgument(LIST* list, wchar_t* id);

int addArgument(LIST** list, wchar_t* id, wchar_t* argument);

int argumentRenew(LIST* list, wchar_t* id, wchar_t* argument);

int destroyList(LIST* list);

void push(NODE** stack, wchar_t* opts);

void pop(NODE** stack);

wchar_t* top(NODE* stack);

int addNode(NODE** node, wchar_t* opts);

int isNode(NODE* node, wchar_t* opts);

int destroyNode(NODE* node);

LINE* createLine(wchar_t* wcsLine);

int getValue(LINE* line, wchar_t* atribute);

int freeLine(LINE* line);

int processLine(LINE *line, FILE *infile, FILE *outfile);

wchar_t* svgArguments(LINE *line);