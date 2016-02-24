
/***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include "totikz.h"
#include "lines.h"


int processLine(LINE *line, FILE *infile, FILE *outfile) // 1px = 0.02645833333333cm
{
	if (wcscmp(line->tag, L"path") == 0)
	{
		tikzPath(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"polyline") == 0 || wcscmp(line->tag, L"polygon") == 0)
	{
		tikzPolyline(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"line") == 0)
	{
		tikzLine(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"rect") == 0)
	{
		tikzRect(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"circle") == 0 || wcscmp(line->tag, L"ellipse") == 0)
	{
		tikzEllipse(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"g") == 0)
	{
		tikzG(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"/g") == 0)
	{
		tikzCloseG(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"text") == 0 || wcscmp(line->tag, L"tspan") == 0)
	{
		tikzText(line, outfile);
		return 0;
	}
	if (wcscmp(line->tag, L"/text") == 0 || wcscmp(line->tag, L"/tspan") == 0)
	{
		pop(&textArgs);
		return 0;
	}
	if (wcscmp(line->tag, L"pattern") == 0)  //page(1063) \usetikzlibrary{patterns}
	{
		if (!isInNode(libraries, L"patterns"))
		{
			wchar_t *padding = malloc(sizeof(wchar_t));
			padding[0] = '\0';
			padding = addPadding(padding, globalPadding);
			fwprintf(outfile, L"%ls\\usetikzlibrary{patterns}\n", padding);
			free(padding);
			addNode(&libraries, L"patterns");
		}
		tikzPattern(line, outfile);
		push(&globalArgs, L"");
		return 0;
	}
	if (wcscmp(line->tag, L"/pattern") == 0)
	{
		globalPadding--;
		wchar_t *tmp = malloc(sizeof(wchar_t));
		tmp[0] = '\0';
		tmp = addPadding(tmp, globalPadding);
		fwprintf(outfile, L"%ls\\end{tikzpicture}}\n", tmp);
		free(tmp);
		pop(&globalArgs);
		return 0;
	}
	if (wcscmp(line->tag, L"linearGradient") == 0)
	{
		tikzlinearGradient(line, infile, outfile);
		return 0;
	}

	if (wcscmp(line->tag, L"svg") == 0)
	{
		if (svgCount > 0)
		{
			wchar_t *args = svgArguments(line), *tmpPadding = malloc(sizeof(wchar_t));
			tmpPadding[0] = '\0';
			tmpPadding = addPadding(tmpPadding, globalPadding);
			if (args[0] == '\0')
			{
				fwprintf(outfile, L"%ls\\begin{scope}\n", tmpPadding);
			}
			else
			{
				fwprintf(outfile, L"%ls\\begin{scope}[%ls]\n", tmpPadding, args);
			}
			free(tmpPadding);
			free(args);
		}
		else
		{
			fwprintf(outfile, L"\\begin{tikzpicture}[x=0.026458cm, y=0.026458cm]\n");
		}
		svgCount++;
		globalPadding++;
		push(&globalArgs, L"");
		return 0;
	}
	if (wcscmp(line->tag, L"/svg") == 0)
	{
		globalPadding--;
		if (svgCount == 1)
		{
			fwprintf(outfile, L"\\end{tikzpicture}");
		}
		else
		{
			wchar_t *tmpPadding = malloc(sizeof(wchar_t));
			tmpPadding[0] = '\0';
			tmpPadding = addPadding(tmpPadding, globalPadding);
			fwprintf(outfile, L"%ls\\end{scope}\n", tmpPadding);
			free(tmpPadding);
		}
		svgCount--;
		pop(&globalArgs);
		return 0;
	}
	if (wcscmp(line->tag, L"defs") == 0)
	{
		isInDefs = true;
		return 0;
	}
	if (wcscmp(line->tag, L"/defs") == 0)
	{
		isInDefs = false;
		return 0;
	}
	/*fwprintf(outfile, L"TAG: %ls(", line->tag);
	fwprintf(outfile, L"%d){", line->atribc);
	for (int i = 0; i < line->atribc - 1; i++)
	{
		fwprintf(outfile, L"%ls=\"%ls\" ", line->atributes[i], line->values[i]);
	}
	if (line->atribc > 0)
	{
		fwprintf(outfile, L"%ls=\"%ls\"}{%ls}\n", line->atributes[line->atribc - 1], line->values[line->atribc - 1], line->content);
	}
	else
	{
		fwprintf(outfile, L"}{%ls}\n", line->content);
	}*/
	return 0;
}

wchar_t* svgArguments(LINE *line)
{
	float x = 0, y = 0, w = 0, h = 0, vx, vy, vw, vh;
	wchar_t *tmpStr = NULL, *args = malloc(sizeof(wchar_t));
	args[0] = '\0';
	int d = getValue(line, L"x");
	if (d != -1)
	{
		x = - wcstof(line->values[d], &tmpStr);
	}
	d = getValue(line, L"y");
	if (d != -1)
	{
		y = wcstof(line->values[d], &tmpStr);
	}

	d = getValue(line, L"width");
	if (d != -1)
	{
		w = wcstof(line->values[d], &tmpStr);
	}
	d = getValue(line, L"height");
	if (d != -1)
	{
		h = wcstof(line->values[d], &tmpStr);
	}

	d = getValue(line, L"viewBox");
	if (d != -1)
	{
		vx = - wcstof(line->values[d], &tmpStr);
		int i = 0;
		for (i = 0; !iswdecimal(tmpStr[i]) && tmpStr[i] != '-'; i++);
		vy = wcstof(tmpStr + i, &tmpStr);
		wchar_t buffer[16];
		swprintf(buffer, 16, L"%.4f", vx * 0.75);
		args = addStr(args, L"xshift=");
		args = addStr(args, buffer);
		swprintf(buffer, 16, L"%.4f", vy * 0.75);
		args = addStr(args, L", yshift=");
		args = addStr(args, buffer);
		
		
		for (i = 0; !iswdecimal(tmpStr[i]) && tmpStr[i] != '-'; i++);
		vw = wcstof(tmpStr + i, &tmpStr);
		for (i = 0; !iswdecimal(tmpStr[i]) && tmpStr[i] != '-'; i++);
		vh = wcstof(tmpStr + i, &tmpStr);
		if (w > 0 && h > 0)
		{
			swprintf(buffer, 16, L"%.4f", w / vw);
			args = addStr(args, L", xscale=");
			args = addStr(args, buffer);
			swprintf(buffer, 16, L"%.4f", h / vw);
			args = addStr(args, L", yscale=");
			args = addStr(args, buffer);
		}	
	}
	return args;
}


void push(NODE** stack, wchar_t* opts)
{
	NODE* node = malloc(sizeof(NODE));
	node->opts = malloc((wcslen(opts) + 1) * sizeof(wchar_t));
	wcscpy(node->opts, opts);
	node->colors = NULL;
	node->colorsCount = 0;
	node->next = *stack;
	*stack = node;
}

void pop(NODE** stack)
{
	if (stack != NULL)
	{
		NODE* tmp = *stack;
		*stack = (*stack)->next;
		free(tmp->opts);
		for (int i = 0; i < tmp->colorsCount; i++)
		{
			free(tmp->colors[i]);
		}
		free(tmp->colors);
		free(tmp);
	}
}

wchar_t* top(NODE* stack)
{
	if (stack != NULL)
	{
		return stack->opts;
	}
	return NULL;
}

int addNode(NODE** node, wchar_t* opts)
{
	if (*node == NULL)
	{
		*node = malloc(sizeof(NODE));
		(*node)->opts = malloc((wcslen(opts) + 1) * sizeof(wchar_t));
		wcscpy((*node)->opts, opts);
		(*node)->colorsCount = 0;
		(*node)->colors = NULL;
		(*node)->next = NULL;
		return 0;
	}
	NODE* pos = *node;
	for(; pos->next != NULL; pos = pos->next);
	pos->next = malloc(sizeof(NODE));
	pos->next->opts = malloc((wcslen(opts) + 1) * sizeof(wchar_t));
	wcscpy(pos->next->opts, opts);
	pos->next->colorsCount = 0;
	pos->next->colors = NULL;
	pos->next->next = NULL;
	return 0;
}

int addColor(NODE* node, wchar_t *color)
{
	node->colors = realloc(node->colors, (node->colorsCount + 1) * sizeof(wchar_t*));
	node->colors[node->colorsCount] = malloc((wcslen(color) + 1) * sizeof(wchar_t));
	wcscpy(node->colors[node->colorsCount++], color);
	return 0; 
}

_Bool isInNode(NODE* node, wchar_t* opts)
{
	for (; node != NULL; node = node->next)
	{
		if (wcscmp(node->opts, opts) == 0)
		{
			return 1;
		}
	}
	return 0;
}

_Bool isColor(NODE* node, wchar_t* color)
{
	for (; node != NULL; node = node->next)
	{
		if (node->colors != NULL && node->colorsCount > 0)
		{
			for (int i = 0; i < node->colorsCount; i++)
			{
				if (wcscmp(node->colors[i], color) == 0)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

int destroyNode(NODE* node)
{
	while (node != NULL)
	{
		free(node->opts);
		for (int i = 0; i < node->colorsCount; i++)
		{
			free(node->colors[i]);
		}
		free(node->colors);
		NODE* tmp = node;
		node = node->next;
		free(tmp);
	}
	return 0;
}

LINE* createLine(wchar_t* wcsLine)
{
	//
	LINE* line = malloc(sizeof(LINE));
	line->tag = malloc(1 * sizeof(wchar_t));
	line->atributes = NULL;
	line->values = NULL;
	line->tag[0] = '\0';
	line->atribc = 0;
	int n = 0, i = 0, lineLen = wcslen(wcsLine);
	//wchar_t c = fgetwc(file);
	if (wcsLine[i] == '<')
	{
		i++;
	}
	for (; i < lineLen && iswprint(wcsLine[i]) && wcsLine[i] != '>' && wcsLine[i] != ' '; i++)
	{
		line->tag = realloc(line->tag, (n + 2) * sizeof(wchar_t));
		line->tag[n++] = wcsLine[i];
	}
	line->tag = realloc(line->tag, (n + 2) * sizeof(wchar_t));
	line->tag[n] = '\0';

	for (; i < lineLen && (!iswprint(wcsLine[i]) || wcsLine[i] == ' '); i++);

	while (i < lineLen && wcsLine[i] != '>' && wcsLine[i] != '/' && wcsLine[i] != '?')
	{
		line->atribc++;
		line->atributes = realloc(line->atributes, (line->atribc) * sizeof(wchar_t*));
		line->values = realloc(line->values, (line->atribc) * sizeof(wchar_t*));
		line->atributes[line->atribc - 1] = malloc(sizeof(wchar_t));
		line->values[line->atribc - 1] = malloc(sizeof(wchar_t));
		line->atributes[line->atribc - 1][0] = '\0';
		line->values[line->atribc - 1][0] = '\0';
		n = 0;
		for (; i < lineLen &&  iswprint(wcsLine[i]) && wcsLine[i] != '=' && wcsLine[i] != '>' && wcsLine[i] != '/' && wcsLine[i] != '?' && wcsLine[i] != ' '; i++)
		{
			line->atributes[line->atribc - 1] = realloc(line->atributes[line->atribc - 1], (n + 2) * sizeof(wchar_t));
			line->atributes[line->atribc - 1][n++] = wcsLine[i];
		}
		line->atributes[line->atribc - 1] = realloc(line->atributes[line->atribc - 1], (n + 2) * sizeof(wchar_t));
		line->atributes[line->atribc - 1][n++] = '\0';

		for (; i < lineLen && (!iswprint(wcsLine[i]) || wcsLine[i] == ' ' || wcsLine[i] == '='); i++);

		if (wcsLine[i] == '\"')
		{
			i++; // skip (=")
			n = 0;
			for(; wcsLine[i] != '\0' && wcsLine[i] != '\"'; i++)
			{
				line->values[line->atribc - 1] = realloc(line->values[line->atribc - 1], (n + 2) * sizeof(wchar_t));
				line->values[line->atribc - 1][n++] = wcsLine[i];
			}
			line->values[line->atribc - 1] = realloc(line->values[line->atribc - 1], (n + 2) * sizeof(wchar_t));
			line->values[line->atribc - 1][n++] = '\0';
			if (wcsLine[i] == '\"')
			{
				i++;
			}
		}
		for (; i < lineLen && (!iswprint(wcsLine[i]) || wcsLine[i] == ' '); i++);
	}
	line->content = NULL;
	/*long current = ftell(file);

	n = 0;
	line->content = malloc(sizeof(wchar_t));
	if (c == '>')
	{
		c = fgetwc(file);
		while(c != WEOF && c != '<')
		{
			if (iswprint(c))
			{
				line->content = realloc(line->content, (n + 2) * sizeof(wchar_t));
				line->content[n++] = c;
			}
			c = fgetwc(file);
		}
	}
	line->content[n] = '\0';

	fseek(file, current, SEEK_SET);*/
	return line;
}

int getValue(LINE* line, wchar_t* atribute)
{
	for (int i = 0; i < line->atribc; i++)
	{
		if (wcscmp(line->atributes[i], atribute) == 0)
		{
			return i;
		}
	}
	return -1;
}

int freeLine(LINE *line)
{
	free(line->tag);
	for (int i = 0; i < line->atribc; i++)
	{
		free(line->atributes[i]);
		free(line->values[i]);
	}
	free(line->atributes);
	free(line->values);
	free(line->content);
	free(line);
	return 0;
}


wchar_t* getArgument(LIST* list, wchar_t* id)
{
	LIST* pos = list;
	for (;pos != NULL; pos = pos->next)
	{
		if (wcscmp(pos->id, id) == 0)
		{
			return pos->argument;
		}
	}
	return NULL;
}

int addArgument(LIST** list, wchar_t* id, wchar_t* argument)
{
	if (*list == NULL)
	{
		*list = malloc(sizeof(LIST));
		(*list)->id = malloc((wcslen(id) + 1) * sizeof(wchar_t));
		wcscpy((*list)->id, id);
		(*list)->argument = malloc((wcslen(argument) + 1) * sizeof(wchar_t));
		wcscpy((*list)->argument, argument);
		(*list)->next = NULL;
		return 0;
	}
	LIST* pos = *list;
	for(; pos->next != NULL; pos = pos->next);
	pos->next = malloc(sizeof(LIST));
	pos->next->id = malloc((wcslen(id) + 1) * sizeof(wchar_t));
	wcscpy(pos->next->id, id);
	pos->next->argument = malloc((wcslen(argument) + 1) * sizeof(wchar_t));
	wcscpy(pos->next->argument, argument);
	pos->next->next = NULL;
	return 0;
}

int argumentRenew(LIST* list, wchar_t* id, wchar_t* argument)
{
	for (; list != NULL; list = list->next)
	{
		if (wcscmp(list->id, id) == 0)
		{
			list->argument = realloc(list->argument, (wcslen(argument) + 1) * sizeof(wchar_t));
			wcscpy(list->argument, argument);
			return 0;
		}
	}
	return -1;
}

int destroyList(LIST* list)
{
	while (list != NULL)
	{
		free(list->id);
		free(list->argument);
		LIST* tmp = list;
		list = list->next;
		free(tmp);
	}
	return 0;
}



LINE** getLine(lineLIST* list, wchar_t* id)
{
	while (list != NULL)
	{
		if (wcscmp(list->id, id) == 0)
		{
			return list->lines;
		}
		list = list->next;
	}
	return NULL;
}

int addDefs(lineLIST** list, wchar_t* id, LINE* line)
{
	if (*list == NULL)
	{
		*list = malloc(sizeof(lineLIST));
		(*list)->id = malloc((wcslen(id) + 1) * sizeof(wchar_t));
		wcscpy((*list)->id, id);
		(*list)->lines = malloc(sizeof(LINE*));
		(*list)->lines[0] = line;
		(*list)->linesCount = 1;
		(*list)->next = NULL;
		return 0;
	}
	else if (wcscmp((*list)->id, id) == 0)
	{
		(*list)->lines = realloc((*list)->lines, ((*list)->linesCount + 1) * sizeof(LINE*));
		(*list)->lines[(*list)->linesCount++] = line;
		return 0;
	}
	lineLIST *pos = *list;
	while (pos->next != NULL)
	{
		if (wcscmp(pos->next->id, id) == 0)
		{
			pos->next->lines = realloc(pos->next->lines, (pos->next->linesCount + 1) * sizeof(LINE*));
			pos->next->lines[pos->next->linesCount++] = line;
			return 0;
		}
		pos = pos->next;
	}
	pos->next = malloc(sizeof(lineLIST));
	pos->next->id = malloc((wcslen(id) + 1) * sizeof(wchar_t));
	wcscpy(pos->next->id, id);
	pos->next->lines = malloc(sizeof(LINE*));
	pos->next->lines[0] = line;
	pos->next->linesCount = 1;
	pos->next->next = NULL;
	return 0;
}

int addLine(lineLIST* list, LINE *line)
{
	if (list != NULL)
	{
		for (; list->next != NULL; list = list->next);
		list->lines = realloc(list->lines, (list->linesCount + 1) * sizeof(LINE*));
		list->lines[list->linesCount++] = line;
		return 0;
	}
	return -1;
}

/*int addAtributes(lineLIST* list, wchar_t* id, wchar_t** atributes, int atribc)
{
	while (list != NULL)
	{
		if (wcscmp(list->id, id) == 0)
		{
			list->line->atributes = realloc(list->line->atributes, list->line->atribc + atribc);
			for (int i = 0; i < atribc; i++)
			{
				wcscpy(list->line->atributes[list->line->atribc + i], atributes[i]);  
			}
			list->line->atribc += atribc;
			return 0;
		}
	}
	return -1;
}*/

int destroyLineList(lineLIST* list)
{
	while (list != NULL)
	{
		lineLIST* tmp = list->next;
		free(list->id);
		for (int i = 0; i < list->linesCount; i++)
		{
			freeLine(list->lines[i]);
		}
		free(list->lines);
		free(list);
		list = tmp;
	}
	return 0;
}