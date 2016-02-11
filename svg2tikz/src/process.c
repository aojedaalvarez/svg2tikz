////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include "totikz.h"
#include "process.h"


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
		if (!isNode(libraries, L"patterns"))
		{
			wchar_t *padding = malloc(sizeof(wchar_t));
			padding[0] = '\0';
			padding = addPadding(padding, globalPadding);
			fwprintf(outfile, L"%ls\\usetikzlibrary{patterns}\n", padding);
			free(padding);
			addNode(&libraries, L"patterns");
		}
		tikzPattern(line, outfile);
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
		destroyNode(colors);
		colors = NULL;
		return 0;
	}
	if (wcscmp(line->tag, L"linearGradient") == 0)
	{
		tikzlinearGradient(line, infile, outfile);
		return 0;
	}

	if (wcscmp(line->tag, L"svg") == 0)
	{
		fwprintf(outfile, L"\\begin{tikzpicture}[x=0.026458cm, y=0.026458cm]\n");
		globalPadding++;
		return 0;
	}
	if (wcscmp(line->tag, L"/svg") == 0)
	{
		fwprintf(outfile, L"\\end{tikzpicture}");
		globalPadding--;
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

void push(NODE** stack, wchar_t* opts)
{
	NODE* node = malloc(sizeof(NODE));
	node->opts = malloc((wcslen(opts) + 1) * sizeof(wchar_t));
	wcscpy(node->opts, opts);
	node->next = *stack;
	*stack = node;
}

void pop(NODE** stack)
{
	if (stack != NULL)
	{
		NODE* tmp = *stack;
		*stack = (**stack).next;
		free(tmp->opts);
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
		(*node)->next = NULL;
		return 0;
	}
	NODE* pos = *node;
	for(; pos->next != NULL; pos = pos->next);
	pos->next = malloc(sizeof(NODE));
	pos->next->opts = malloc((wcslen(opts) + 1) * sizeof(wchar_t));
	wcscpy(pos->next->opts, opts);
	pos->next->next = NULL;
	return 0;
}

int isNode(NODE* node, wchar_t* opts)
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

int destroyNode(NODE* node)
{
	while (node != NULL)
	{
		free(node->opts);
		NODE* tmp = node;
		node = node->next;
		free(tmp);
	}
	return 0;
}

LINE* createLine(FILE* file)
{
	//
	LINE* line = malloc(sizeof(LINE));
	line->tag = malloc(1 * sizeof(wchar_t));
	line->atributes = malloc(sizeof(wchar_t*));
	line->values = malloc(sizeof(wchar_t*));
	line->tag[0] = '\0';
	line->atribc = 0;
	int n = 0;
	wchar_t c = fgetwc(file);
	if (c == '<')
	{
		c = fgetwc(file);
	}
	for (; c != '>' && c != ' '; c = fgetwc(file))
	{
		line->tag = realloc(line->tag, (n + 2) * sizeof(wchar_t));
		line->tag[n++] = c;
	}
	line->tag = realloc(line->tag, (n + 2) * sizeof(wchar_t));
	line->tag[n] = '\0';

	if (c == ' ')
	{
		c = fgetwc(file);
		n = 0;
		line->atribc++;
		line->atributes[0] = malloc(sizeof(wchar_t));
		line->values[0] = malloc(sizeof(wchar_t));
		line->atributes[0][0] = '\0';
		line->values[0][0] = '\0';
		
		while (c != '>' && c != '/')
		{
			if ((c == '?') || (c == '='))
			{
				c = fgetwc(file);
				continue;
			}
			if (c == ' ')
			{
				line->atribc++;
				line->atributes = realloc(line->atributes, (line->atribc) * sizeof(wchar_t*));
				line->values = realloc(line->values, (line->atribc) * sizeof(wchar_t*));
				line->atributes[line->atribc - 1] = malloc(sizeof(wchar_t));
				line->values[line->atribc - 1] = malloc(sizeof(wchar_t));
				line->atributes[line->atribc - 1][0] = '\0';
				line->values[line->atribc - 1][0] = '\0';
				n = 0;
				c = fgetwc(file);
				continue;
			}
			if (c == '\"')
			{
				c = fgetwc(file); // skip (=")
				n = 0;
				for(; c != '\"'; c = fgetwc(file))
				{
					line->values[line->atribc - 1] = realloc(line->values[line->atribc - 1], (n + 2) * sizeof(wchar_t));
					line->values[line->atribc - 1][n++] = c;
				}
				line->values[line->atribc - 1] = realloc(line->values[line->atribc - 1], (n + 2) * sizeof(wchar_t));
				line->values[line->atribc - 1][n++] = '\0';
				if (c == '\"')
				{
					c = fgetwc(file);
					continue;
				}
			}
			for (; c != '=' && c != '?' && c != '>' && c != '/' && c != ' '; c = fgetwc(file))
			{
				line->atributes[line->atribc - 1] = realloc(line->atributes[line->atribc - 1], (n + 2) * sizeof(wchar_t));
				line->atributes[line->atribc - 1][n++] = c;
			}
			line->atributes[line->atribc - 1] = realloc(line->atributes[line->atribc - 1], (n + 2) * sizeof(wchar_t));
			line->atributes[line->atribc - 1][n++] = '\0';
		}
	}
	long current = ftell(file);

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

	fseek(file, current, SEEK_SET);
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