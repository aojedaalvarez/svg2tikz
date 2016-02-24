
/***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>
#include <signal.h>
#include "lines.h"



FILE* openInfile(char* filename);
FILE* openOutfile(char* filename, char* infile);
_Bool isValidFileName(char* filename);
char* chageExtension(char* filename, const char* extension);
int xmlData(LINE* line);
void breakSignal(int signal);
void freeGlobals();


NODE* globalArgs = NULL;
NODE* textArgs = NULL;
NODE* libraries = NULL;
int globalPadding = 0;
LIST *typelist = NULL;
lineLIST* defsList = NULL;
_Bool isInDefs = false;

FILE *infile = NULL, *outfile = NULL;


/*********************/
/*** Main function ***/
/*********************/
int main(int argc, char *argv[])
{
	signal(SIGINT, breakSignal);
	
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
			chageExtension(fname, exts);
			outfile = openOutfile(fname, argv[1]);
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
			outfile = openOutfile(argv[2], argv[1]);
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
		fclose(infile);
		return 1;
	}

	fwide(infile, 1);
	wchar_t c = fgetwc(infile);

	while (!feof(infile))
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
			if (isInDefs && wcscmp(line->tag, L"/defs") != 0)
			{
				int haveId = getValue(line, L"id");
				if (haveId != -1)
				{
					addDefs(&defsList, line->values[haveId], line);
				}
				else
				{
					addLine(defsList, line);
				}
			}
			else
			{
				processLine(line, infile, outfile);
				freeLine(line);
			}
		}
		else if (ferror(infile))
		{
			//ungetwc(c, infile);
			//fseek(infile, ftell(infile) + 4, SEEK_SET);
			printf("read errno=%d\n", errno);
			break;
		}
		else
		{
			c = fgetwc(infile);
		}
	}

	freeGlobals();
	
	return 0;
}


void breakSignal(int signal)
{
	freeGlobals();
	exit(0);
}


inline void freeGlobals()
{
	// Free global variables
	destroyNode(globalArgs);
	destroyNode(textArgs);
	destroyNode(libraries);
	destroyList(typelist);
	destroyLineList(defsList);
	
	// Close files
	fclose(infile);
	fclose(outfile);
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
FILE* openOutfile(char* filename, char* infile)
{
	char buffer[256] = "";
	if (access(filename, F_OK) == 0)
	{
		printf("The file %s exist (o=override/r=rename/q=quit)\n", filename);
		char c = getchar();
			
		while (true)
		{
			if (c == 'q')
			{
				return NULL;
			}
			else if (c == 'o')
			{
				if(access(filename, W_OK) == -1)
				{
					printf("No access to file %s (r=rename/q=quit)\n", filename);
					c = getchar();
					continue;
				}
				else
				{
					break;
				}
			}
			else if (c == 'r')
			{
				buffer[0] = '\0';
				while (!isValidFileName(buffer) || strcmp(buffer, infile) == 0)
				{
					//for (int i = fgetc(stdin); i != EOF; i = fgetc(stdin));
					printf("\rEnter new file name: ");
					int in, n = 0;
					while ((in = fgetc(stdin)) != '\n' && in != EOF && n < 256)
					{
						buffer[n++] = in;
					}
					buffer[n] = '\0';
					if (strcmp(buffer, infile) == 0)
					{
						printf("Name can't coincide with input file\n");
					}
				}

				filename = buffer;
				if (access(filename, F_OK) == 0)
				{
					printf("The file %s exist (o=override/r=rename/q=quit)\n", filename);
					c = getchar();
					continue;
				}
				else
				{
					break;
				}
			}
			else
			{
				printf("Incorret option (o=override/r=rename/q=quit)\n");
				c = getchar();
				continue;
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


/*************************************/
/*** Comprove is filename is valid ***/
/*************************************/
_Bool isValidFileName(char* filename)
{
	return strlen(filename) > 0;
}


/**************************************************/
/*** Generate filename with especifed extension ***/
/**************************************************/
char* chageExtension(char* filename, const char* extension)
{
	char* extind = strrchr(filename, '.');

	if (extind != NULL && strcmp(extind + 1, extension) == 0)
	{
		*extind = '\0';
		sprintf(filename, "%s_copy.%s", filename, extension);
		return filename;
	}
	else if (extind != NULL)
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