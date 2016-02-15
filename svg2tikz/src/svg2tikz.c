
/***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <unistd.h>
#include <locale.h>
#include "process.h"


#define false 0
#define true !false


FILE* openInfile(char* filename);
FILE* openOutfile(char* filename);
char* chagextension(char* filename, const char* extension);
int xmlData(LINE* line);

NODE* globalArgs = NULL;
NODE* textArgs = NULL;
NODE* colors = NULL;
NODE* libraries = NULL;
int globalPadding = 0;
LIST *typelist = NULL;

int main(int argc, char *argv[])
{
	FILE *infile, *outfile;
	
	if(argc == 2)
	{
		infile = openInfile(argv[1]);
		if (infile == NULL)
		{
			return 1;
		}
		else
		{
			char exts[] = "tikz", fname[strlen(argv[1]) + strlen(exts) + 6];
			strcpy(fname, argv[1]);
			chagextension(fname, exts);
			outfile = openOutfile(fname);
		}
	}
	else if(argc == 3)
	{
		infile = openInfile(argv[1]);
		if (infile == NULL)
		{
			return 1;
		}
		else
		{
			outfile = openOutfile(argv[2]);
		}
	}
	else
	{
		printf("Incorret argument numbers\n");
		printf("Usage: %s infile [outfile]\n", argv[0]);
		return 1;
	}
	if (outfile == NULL)
	{
		return 1;
	}

	wchar_t c = fgetwc(infile);	

	while (c != WEOF)
	{
		if(c == '<')
		{
			//
			wchar_t *buffer = malloc(sizeof(wchar_t));
			buffer[0] = '\0';
			int n = 0;
			for (; c != '>'; c = fgetwc(infile))
			{
				buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
				buffer[n++] = c;
			}
			buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
			buffer[n++] = '>';
			buffer[n] = '\0';

			LINE *line = createLine(buffer);
			free(buffer);

			n = 0;
			line->content = malloc(sizeof(wchar_t));
			if (c == '>')
			{
				c = fgetwc(infile);
				for (; c != WEOF && !iswprint(c); c = fgetwc(infile));
				for (; c != WEOF && c != '<'; c = fgetwc(infile))
				{
					line->content = realloc(line->content, (n + 2) * sizeof(wchar_t));
					line->content[n++] = c;
				}
			}
			line->content[n] = '\0';

			if (wcscmp(line->tag, L"?xml") == 0)
			{
				xmlData(line);
			}
			processLine(line, infile, outfile);
			freeLine(line);
		}
		else
		{
			c = fgetwc(infile);
		}
	}

	// Free global variables
	destroyList(typelist);
	destroyNode(colors);
	destroyNode(libraries);

	// Close files
	fclose(infile);
	fclose(outfile);
	return 0;
}


/***************************/
/*** Open svg image file ***/
/***************************/
FILE* openInfile(char* filename)
{
	if (access(filename, F_OK) == -1)
	{
		printf("The file %s do not exist\n", filename);
		return NULL;
	}
	else if(access(filename, R_OK) == -1)
	{
		printf("No access to file %s\n", filename);
		return NULL;
	}
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("Error to open file %s\n", filename);
		return NULL;
	}
	else
	{
		return file;
	}
}


/**********************************/
/*** Open espesifed output file ***/
/**********************************/
FILE* openOutfile(char* filename)
{
	if (access(filename, F_OK) == 0)
	{
		printf("The file %s exist, do you want to override? (y/n)\n", filename);
		while (true)
		{
			char c = getchar();
			if (c == 'n')
			{
				return NULL;
			}
			else if (c == 'y')
			{
				if(access(filename, W_OK) == -1)
				{
					printf("No access to file %s\n", filename);
					return NULL;
				}
				break;
			}
		}
	}

	FILE *file = fopen(filename, "w");
	if (file == NULL)
	{
		printf("Error to open file %s\n", filename);
		return NULL;
	}
	else
	{
		return file;
	}
}


/**************************************************/
/*** Generate filename with especifed extension ***/
/**************************************************/
char* chagextension(char* filename, const char* extension)
{
	char* extind = strrchr(filename, '.');//, *last = strchr(filename, '\0');
	//printf("%s\n", extind + sizeof(char));
	if (strcmp(extind + sizeof(char), extension) == 0)
	{
		*extind = '\0';
		sprintf(filename, "%s_copy.%s", filename, extension);
		return filename;
	}
	if (extind != NULL)
	{
		*extind = '\0';
	}

	sprintf(filename, "%s.%s", filename, extension);
	return filename;
}


/************************/
/*** Process xml info ***/
/************************/
int xmlData(LINE* line)
{
	//
	for (int i = 0; i < line->atribc; i++)
	{
		if (wcscmp(line->atributes[i], L"encoding") == 0)
		{
			//
			char locale[wcslen(line->values[i]) + 7], tmp[wcslen(line->values[i]) + 1];
			setlocale(LC_ALL, "");
			strcpy(locale, setlocale(LC_CTYPE, NULL));
			for (int pos = 0, n = strlen(locale); pos < n; pos++)
			{
				if (locale[pos] == '.')
				{
					locale[pos] = '\0';
					break;
				}
			}
			wcstombs(tmp, line->values[i], wcslen(line->values[i]) + 1);
			strcat(locale, ".");
			strcat(locale, tmp);
			printf("Current locale: %s\n", setlocale(LC_CTYPE, locale));
		}
	}
	return 0;
}