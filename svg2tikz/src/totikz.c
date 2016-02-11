
/*******************************************/

#include <stdio.h>
#include <stdlib.h> 
#include <wchar.h>
#include <wctype.h>
#include <math.h>
#include "process.h"
#include "totikz.h"


wchar_t* addPadding(wchar_t* dest, int padding)
{
	//
	for (int i = 0; i < padding; i++)
	{
		dest = addStr(dest, L"    ");
	}
	return dest;
}

wchar_t* addStr(wchar_t* dest, const wchar_t* src)
{
	int n = wcslen(dest), m = wcslen(src);
	dest = realloc(dest, (n + m + 1) * sizeof(wchar_t));
	wcscat(dest, src);
	return dest;
}

wchar_t* addChar(wchar_t* dest, const wchar_t chr)
{
	int n = wcslen(dest);
	dest = realloc(dest, (n + 2) * sizeof(wchar_t));
	dest[n] = chr;
	dest[n + 1] = '\0';
	return dest;
}

int addPoint(wchar_t** commd, const wchar_t* path, int i)
{
	if (path[i] == '-')
	{
		*commd = addChar(*commd, '-');
		i++;
	}
	while (path[i] != '\0' && iswdecimal(path[i]))
	{
		*commd = addChar(*commd, path[i++]);
	}

	*commd = addStr(*commd, L", ");
	for(; !iswdecimal(path[i]) && path[i] != '-'; i++);

	if (path[i] == '-')
	{
		i++;
	}
	else if (path[i] != '0')
	{
		*commd = addChar(*commd, '-');
	}
	while (path[i] != '\0' && iswdecimal(path[i]))
	{
		*commd = addChar(*commd, path[i++]);
	}
	return i;
}

wchar_t* convertColor(FILE* outfile, wchar_t* sColor)
{
	//
	if(!isNode(colors, sColor + 1))
	{
		wchar_t *tmp = NULL, R[2], G[2], B[2];
		R[0] = sColor[1];
		R[1] = '\0';
		G[0] = sColor[2];
		G[1] = '\0';
		B[0] = sColor[3];
		B[1] = '\0';
		int r = wcstol(R, &tmp, 16) * 255 / 15, g = wcstol(G, &tmp, 16) * 255 / 15, b = wcstol(B, &tmp, 16) * 255 / 15;
		//swprintf(tColor, 28, L"red!%d!green!%d!blue!%d", r, g, b);
		wchar_t *padding = malloc(sizeof(wchar_t));
		padding[0] = '\0';
		padding = addPadding(padding, globalPadding);
		fwprintf(outfile, L"%ls\\definecolor{rgb%ls}{RGB}{%d, %d, %d}\n", padding, sColor + 1, r, g, b);
		free(padding);
		addNode(&colors, sColor + 1);
	}
	return sColor + 1;
} 

wchar_t* shapeOptions(wchar_t* args, NODE* stack, LINE* line, FILE* outfile)
{
	args = malloc(sizeof(wchar_t));
	args[0] = '\0';

	if (stack != NULL && wcslen(stack->opts) > 0)
	{
		args = addStr(args, stack->opts);
	}

	int d = getValue(line, L"stroke");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		if (line->values[d][0] == '#')
		{
			//wchar_t color[28] = L"";
			//convertColor(color, line->values[d]);
			//args = addStr(args, L"color=");
			args = addStr(args, L"rgb");
			args = addStr(args, convertColor(outfile, line->values[d]));
		}
		else
		{
			args = addStr(args, line->values[d]);
		}
	}
	d = getValue(line, L"fill");
	if (d != -1 && wmemcmp(line->values[d], L"url(#", 5) == 0)
	{
		wchar_t tmp[wcslen(line->values[d]) - 4];
		wcscpy(tmp, line->values[d] + 5);
		tmp[wcslen(tmp) - 1] = '\0';
		wchar_t* tmpa = getArgument(typelist, tmp);
		if (tmpa != NULL)
		{
			if (wcslen(args) > 0)
			{
				args = addStr(args, L", ");
			}
			args = addStr(args, tmpa);
		}
	}
	else if (d != -1 && wcscmp(line->values[d], L"none") != 0 && wcscmp(line->values[d], L"transparent") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"fill=");
		if (line->values[d][0] == '#')
		{
			//wchar_t color[28] = L"";
			//convertColor(color, line->values[d]);
			args = addStr(args, L"rgb");
			args = addStr(args, convertColor(outfile, line->values[d]));
		}
		else
		{
			args = addStr(args, line->values[d]);
		}
	}
	d = getValue(line, L"opacity");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"opacity=");
		args = addStr(args, line->values[d]);
	}
	d = getValue(line, L"fill-opacity");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"fill opacity=");
		args = addStr(args, line->values[d]);
	}
	d = getValue(line, L"stroke-width");
	if (d != -1)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"line width=");
		args = addStr(args, line->values[d]);
	}
	return args;
}

wchar_t* textOptions(wchar_t* args, NODE* stack, LINE* line)
{
	args = malloc(sizeof(wchar_t));
	args[0] = '\0';

	if (stack != NULL && wcslen(stack->opts) > 0)
	{
		args = addStr(args, stack->opts);
	}

	int d = getValue(line, L"text-anchor");
	if (d != -1)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		if (wcscmp(line->values[d], L"start") == 0)
		{
			args = addStr(args, L"anchor=south west");
		}
		if (wcscmp(line->values[d], L"end") == 0)
		{
			args = addStr(args, L"anchor=south east");
		}
		if (wcscmp(line->values[d], L"middle") == 0)
		{
			args = addStr(args, L"anchor=south");
		}
	}
	d = getValue(line, L"font-weight");
	if (d != -1 && wcscmp(line->values[d], L"bold") == 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"font=\\bf");
	}
	d = getValue(line, L"dx");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"xshift=");
		args = addStr(args, line->values[d]);
	}
	d = getValue(line, L"dy");
	if (d != -1)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"yshift=");
		if (line->values[d][0] == '-')
		{
			wchar_t* tmpy = line->values[d] + 1;
			args = addStr(args, tmpy);
		}
		else
		{
			args = addChar(args, '-');
			args = addStr(args, line->values[d]);
		}
	}
	return args;
}

wchar_t * currentPos(wchar_t* path)
{
	return NULL;
}


int tikzPath(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t)), *args = NULL;
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\draw");

	POINT lastPos, lastControl;
	
	lastPos.x = 0;
	lastPos.y = 0;
	lastPos.path = '\0';

	lastControl.x = 0;
	lastControl.y = 0;
	lastControl.path = '\0';

	args = shapeOptions(args, globalArgs, line, outfile);
	if (wcslen(args) > 0)
	{
		commd = addChar(commd, '[');
		commd = addStr(commd, args);
		commd = addChar(commd, ']');
	}
	free(args);

	// path
	int d = getValue(line, L"d");
	if (d != -1)
	{
		//
		wchar_t* path = line->values[d];
		for (int i = 0; path[i] != '\0';)
		{
			if (path[i] == 'M')
			{
				commd = addStr(commd, L" (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				while(!iswalpha(path[i]))
				{
					i = addPoint(&commd, path, i);
					if(path[i] == ' ')
					{
						for(; path[i] == ' '; i++);
						if (iswdecimal(path[i]) || path[i] == '-')
						{
							commd = addStr(commd, L") (");
						}
					}
					else if(path[i] == '-')
					{
						commd = addStr(commd, L") (");
					}
					else if(path[i] == '\0')
					{
						break;
					}
				}
				commd = addChar(commd, ')');

				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'M';
				lastControl.path = '\0';
			}
			else if (path[i] == 'm')
			{
				if (commd[wcslen(commd)] == ')')
				{
					commd = addStr(commd, L" ++(");
				}
				else
				{
					commd = addStr(commd, L" (");
				}
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				while(!iswalpha(path[i]))
				{
					i = addPoint(&commd, path, i);
					if(path[i] == ' ')
					{
						for(; path[i] == ' '; i++);
						if(iswdecimal(path[i]) || path[i] == '-')
						{
							commd = addStr(commd, L") ++(");
						}
					}
					else if(path[i] == '-')
					{
						commd = addStr(commd, L") ++(");
					}
					else if(path[i] == '\0')
					{
						break;
					}
				}
				commd = addChar(commd, ')');

				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'm';
				lastControl.path = '\0';
			}
			else if (path[i] == 'L')
			{
				commd = addStr(commd, L" -- (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				while(!iswalpha(path[i]))
				{
					i = addPoint(&commd, path, i);
					if(path[i] == ' ')
					{
						for(; path[i] == ' '; i++);
						if(iswdecimal(path[i]) || path[i] == '-')
						{
							commd = addStr(commd, L") -- (");	
						}
					}
					else if(path[i] == '-')
					{
						commd = addStr(commd, L") -- (");
					}
					else if(path[i] == '\0')
					{
						break;
					}
				}
				commd = addChar(commd, ')');
				
				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'L';
				lastControl.path = '\0';
			}
			else if (path[i] == 'l')
			{
				commd = addStr(commd, L" -- ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				while(!iswalpha(path[i]))
				{
					i = addPoint(&commd, path, i);
					if(path[i] == ' ')
					{
						for(; path[i] == ' '; i++);
						if(iswdecimal(path[i]) || path[i] == '-')
						{
							commd = addStr(commd, L") -- ++(");
						}
					}
					else if(path[i] == '-')
					{
						commd = addStr(commd, L") -- ++(");
					}
					else if(path[i] == '\0')
					{
						break;
					}
				}
				commd = addChar(commd, ')');
				
				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'l';
				lastControl.path = '\0';
			}
			else if (path[i] == 'H')
			{
				int j = wcslen(commd);
				wchar_t lastY[j];
				swprintf(lastY, j, L"%4.4f", lastPos.y);
				commd = addStr(commd, L" -- (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				if (path[i] == '-')
				{
					commd = addChar(commd, '-');
					i++;
				}
				while (iswdecimal(path[i]))
				{
					commd = addChar(commd, path[i++]);
				}
				commd = addStr(commd, L", ");
				commd = addStr(commd, lastY);
				commd = addChar(commd, ')');
				
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.path = 'H';
				lastControl.path = '\0';
			}
			else if (path[i] == 'h')
			{
				commd = addStr(commd, L" -- ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				if (path[i] == '-')
				{
					commd = addChar(commd, '-');
					i++;
				}
				while (iswdecimal(path[i]))
				{
					commd = addChar(commd, path[i++]);
				}
				commd = addStr(commd, L", 0)");
				
				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.path = 'h';
				lastControl.path = '\0';
			}
			else if (path[i] == 'V')
			{
				int j = wcslen(commd);
				wchar_t lastX[j];
				swprintf(lastX, j, L"%4.4f", lastPos.x);
				commd = addStr(commd, L" -- (");
				commd = addStr(commd, lastX);
				commd = addStr(commd, L", ");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				if (path[i] == '-')
				{
					i++;
				}
				else if(path[i] != '0')
				{
					commd = addChar(commd, '-');
				}
				while (iswdecimal(path[i]))
				{
					commd = addChar(commd, path[i++]);
				}
				commd = addChar(commd, ')');
				
				for(j = wcslen(commd); j > 0 && commd[j] != ','; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.y = wcstof(commd + j + 2, &tmpPoint);
				lastPos.path = 'V';
				lastControl.path = '\0';
			}
			else if (path[i] == 'v')
			{
				commd = addStr(commd, L" -- ++(0, ");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				if (path[i] == '-')
				{
					i++;
				}
				else if(path[i] != '0')
				{
					commd = addChar(commd, '-');
				}
				while (iswdecimal(path[i]))
				{
					commd = addChar(commd, path[i++]);
				}
				commd = addChar(commd, ')');
				
				int j;
				for(j = wcslen(commd); j > 0 && commd[j] != ','; j--);
				wchar_t* tmpPoint = NULL;
				lastPos.y += wcstof(commd + j + 2, &tmpPoint);
				lastPos.path = 'v';
				lastControl.path = '\0';
			}
			else if (path[i] == 'z' || path[i] == 'Z')
			{
				commd = addStr(commd, L" -- cycle");
				i++;

				int j = 0;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				wchar_t* tmpPoint = NULL;
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'z';
				lastControl.path = '\0';
			}
			else if (path[i] == 'C')
			{
				commd = addStr(commd, L" .. controls (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") and (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") .. (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				int j;
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'C';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'C';
			}
			else if (path[i] == 'c')
			{
				commd = addStr(commd, L" .. controls ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") and ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") .. ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				int j;
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = lastPos.x + wcstof(commd + j + 5, &tmpPoint);
				lastControl.y = lastPos.y + wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'c';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'c';
			}
			else if (path[i] == 'S')
			{
				commd = addStr(commd, L" .. controls (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				int j = i;
				
				float cx, cy;
				wchar_t * point = NULL;
				if(lastControl.path != '\0')
				{
					cx = 2 * lastPos.x - lastControl.x;
					cy = 2 * lastPos.y - lastControl.y;
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"%4.4f, %4.4f", cx, cy);
				}
				else
				{
					point = malloc(sizeof(wchar_t));
					point[0] = '\0';
					j = addPoint(&point, path, j);
				}

				commd = addStr(commd, point);
				commd = addStr(commd, L") and (");
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") .. (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				free(point);
				
				wchar_t *tmpPoint;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'S';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'S';
			}
			else if (path[i] == 's')
			{
				commd = addStr(commd, L" .. controls ");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				int j = i;
				
				float cx, cy;
				wchar_t * point = NULL;
				if(lastControl.path != '\0')
				{
					cx = 2 * lastPos.x - lastControl.x;
					cy = 2 * lastPos.y - lastControl.y;
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"(%4.4f, %4.4f", cx, cy);
				}
				else
				{
					point = malloc(sizeof(wchar_t));
					point[0] = '\0';
					point = addStr(point, L"++(");
					j = addPoint(&point, path, j);
				}

				commd = addStr(commd, point);
				commd = addStr(commd, L") and ++(");
				i = addPoint(&commd, path, i);
				commd = addStr(commd, L") .. ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				free(point);
				
				wchar_t *tmpPoint;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = lastPos.x + wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = lastPos.y + wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 's';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 's';
			}
			else if (path[i] == 'Q')
			{
				//
				commd = addStr(commd, L" .. controls ");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				wchar_t* cpoint = malloc(sizeof(wchar_t));
				cpoint[0] = '\0';
				cpoint = addChar(cpoint, '(');
				i = addPoint(&cpoint, path, i);
				cpoint = addChar(cpoint, ')');
				commd = addStr(commd, cpoint);
				commd = addStr(commd, L" and ");
				commd = addStr(commd, cpoint);
				free(cpoint);
				commd = addStr(commd, L" .. ");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				commd = addChar(commd, '(');
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				int j;
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'Q';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'Q';
			}
			else if (path[i] == 'q')
			{
				//
				commd = addStr(commd, L" .. controls ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');

				int j;
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != '+'; j--);
				lastControl.x = lastPos.x + wcstof(commd + j + 2, &tmpPoint);
				lastControl.y = lastPos.y + wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'q';

				commd = addStr(commd, L" and ++(0,0)");
				commd = addStr(commd, L" .. ++(");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				for(j = wcslen(commd); j > 0 && commd[j] != '('; j--);
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'q';
			}
			else if(path[i] == 'T')
			{
				commd = addStr(commd, L" .. controls (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				int j = i;
				
				float cx, cy;
				wchar_t * point = NULL;
				if(lastControl.path != '\0')
				{
					cx = 2 * lastPos.x - lastControl.x;
					cy = 2 * lastPos.y - lastControl.y;
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"%4.4f, %4.4f", cx, cy);
				}
				else
				{
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"%4.4f, %4.4f", lastPos.x, lastPos.y);
				}

				commd = addStr(commd, point);
				commd = addStr(commd, L") and (");
				commd = addStr(commd, point);
				commd = addStr(commd, L") .. (");
				free(point);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 'T';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x = wcstof(commd + j + 1, &tmpPoint);
				lastPos.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 'T';
			}
			else if(path[i] == 't')
			{
				commd = addStr(commd, L" .. controls (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				int j = i;
				
				float cx, cy;
				wchar_t * point = NULL;
				if(lastControl.path != '\0')
				{
					cx = 2 * lastPos.x - lastControl.x;
					cy = 2 * lastPos.y - lastControl.y;
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"%4.4f, %4.4f", cx, cy);
				}
				else
				{
					point = malloc(21 * sizeof(wchar_t));
					swprintf(point, 64, L"%4.4f, %4.4f", lastPos.x, lastPos.y);
				}

				commd = addStr(commd, point);
				commd = addStr(commd, L") and (");
				commd = addStr(commd, point);
				commd = addStr(commd, L") .. ++(");
				free(point);
				i = addPoint(&commd, path, i);
				commd = addChar(commd, ')');
				
				wchar_t* tmpPoint = NULL;
				for(j = wcslen(commd); j > 0 && commd[j] != 'd'; j--);
				lastControl.x = wcstof(commd + j + 3, &tmpPoint);
				lastControl.y = wcstof(tmpPoint + 2, &tmpPoint);
				lastControl.path = 't';

				j += 3;
				for(int n = wcslen(commd); j < n && commd[j] != '('; j++);
				lastPos.x += wcstof(commd + j + 1, &tmpPoint);
				lastPos.y += wcstof(tmpPoint + 2, &tmpPoint);
				lastPos.path = 't';
			}/*
			else if (path[i] == 'A')
			{
				//
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				wchar_t* tmpPoint;
				int j = 0;
				float rx = wcstof(path, &tmpPoint);
				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				float ry = wcstof(tmpPoint + j, &tmpPoint);

				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				float angle = wcstof(tmpPoint + j, &tmpPoint);

				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				int largeFlag = wcstod(tmpPoint + j, &tmpPoint);

				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				int sweepFlag = wcstod(tmpPoint + j, &tmpPoint);

				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				float x = wcstof(tmpPoint + j, &tmpPoint);
				for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
				float y = - wcstof(tmpPoint + j, &tmpPoint);

				for(; !iswalpha(path[i]) && path[i] != '\0'; i++);

				float A1 = 	(y - lastPos.y) / (lastPos.x - x);
				float B1 = (lastPos.y * lastPos. y - x * x) / 2 / (lastPos.x - x);

				float D = (A1 * (lastPos.x - B1) - lastPos.y) * (A1 * (lastPos.x - B1) - lastPos.y) - (A1 * A1 + 1) * ((lastPos.x - B1) * (lastPos.x - B1) + lastPos.y * lastPos.y - rx * rx);
				float cy = 	(A1 * (lastPos.x - B1) - lastPos.y + sqrt(D)) / (A1 * A1 + 1);
				float cx = A1 * cy + B1;

				float angIni = acos((lastPos.x - cx) / cx);
				float angEnd = acos((x - cx) / cx);	

				wchar_t arc[255];
				swprintf(arc, 255, L" [rotate = %4.4f] arc (%4.4f:%4.4f:%4.4f and %4.4f)", angle, angIni, angEnd, rx, ry);
				commd = addStr(commd, arc);

				lastPos.x = x;
				lastPos.y = y;
				lastPos.path = 'A';
				lastControl.path = '\0';
			}*/
			else if (path[i] == '\0')
			{
				break;
			}
			else
			{
				i++;
			}
		}
		commd = addChar(commd, ';');
		fwprintf(outfile, L"%ls\n", commd);
	}
	free(commd);
	return 0;
}

int tikzPolyline(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t)), *args = NULL;
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\draw");

	args = shapeOptions(args, globalArgs, line, outfile);
	if (wcslen(args) > 0)
	{
		commd = addChar(commd, '[');
		commd = addStr(commd, args);
		commd = addChar(commd, ']');
	}
	free(args);

	int ps = getValue(line, L"points");

	if (ps != -1)
	{
		wchar_t* path = line->values[ps];
		int i = 0;
		for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
		commd = addStr(commd, L" (");
		i = addPoint(&commd, path, i);
		commd = addStr(commd, L")");
		while (path[i] != '\0')
		{
			for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
			if (path[i] == '\0')
			{
				break;
			}
			commd = addStr(commd, L" -- (");
			i = addPoint(&commd, path, i);
			commd = addStr(commd, L")");
		}
		if (wcscmp(line->tag, L"polygon") == 0)
		{
			commd = addStr(commd, L" -- cycle");
		}
		commd = addChar(commd, ';');
		fwprintf(outfile, L"%ls\n", commd);
	}
	free(commd);
	return 0;
}

int tikzLine(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t)), *args = NULL;
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\draw");

	args = shapeOptions(args, globalArgs, line, outfile);
	if (wcslen(args) > 0)
	{
		commd = addChar(commd, '[');
		commd = addStr(commd, args);
		commd = addChar(commd, ']');
	}
	free(args);

	int d = getValue(line, L"x1");
	int e = getValue(line, L"y1");
	if (d != -1 && e != -1)
	{
		commd = addStr(commd, L" (");
		commd = addStr(commd, line->values[d]);
		commd = addStr(commd, L", ");
		if (line->values[e][0] == '-')
		{
			wchar_t* tmpy = line->values[e] + 1;
			commd = addStr(commd, tmpy);
		}
		else if (wcscmp(line->values[e], L"0") != 0)
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[e]);
		}
		else
		{
			commd = addStr(commd, line->values[e]);
		}
		commd = addStr(commd, L") --");

		d = getValue(line, L"x2");
		e = getValue(line, L"y2");

		commd = addStr(commd, L" (");
		commd = addStr(commd, line->values[d]);
		commd = addStr(commd, L", ");
		if (line->values[e][0] == '-')
		{
			wchar_t* tmpy = line->values[e] + 1;
			commd = addStr(commd, tmpy);
		}
		else
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[e]);
		}
		commd = addStr(commd, L");");
	}
	fwprintf(outfile, L"%ls\n", commd);
	free(commd);
	return 0;
}

int tikzRect(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t)), *args = NULL;
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\draw");

	args = shapeOptions(args, globalArgs, line, outfile);
	if (wcslen(args) > 0)
	{
		commd = addChar(commd, '[');
		commd = addStr(commd, args);
		commd = addChar(commd, ']');
	}
	free(args);

	int d = getValue(line, L"x");
	int e = getValue(line, L"y");
	if (d != -1 && e != -1)
	{
		commd = addStr(commd, L" (");
		commd = addStr(commd, line->values[d]);
		commd = addStr(commd, L", ");
		if (line->values[e][0] == '-')
		{
			wchar_t* tmpy = line->values[e] + 1;
			commd = addStr(commd, tmpy);
		}
		else if (wcscmp(line->values[e], L"0") != 0)
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[e]);
		}
		else
		{
			commd = addStr(commd, line->values[e]);
		}
		commd = addStr(commd, L") rectangle");

		d = getValue(line, L"width");
		e = getValue(line, L"height");

		commd = addStr(commd, L" ++(");
		commd = addStr(commd, line->values[d]);
		commd = addStr(commd, L", ");
		if (line->values[e][0] == '-')
		{
			wchar_t* tmpy = line->values[e] + 1;
			commd = addStr(commd, tmpy);
		}
		else
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[e]);
		}
		commd = addStr(commd, L");");
	}
	fwprintf(outfile, L"%ls\n", commd);
	free(commd);
	return 0;
}

int tikzEllipse(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t)), *args = NULL;
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\draw");

	args = shapeOptions(args, globalArgs, line, outfile);
	if (wcslen(args) > 0)
	{
		commd = addChar(commd, '[');
		commd = addStr(commd, args);
		commd = addChar(commd, ']');
	}
	free(args);

	int d = getValue(line, L"cx");
	int e = getValue(line, L"cy");
	if (d != -1 && e != -1)
	{
		commd = addStr(commd, L" (");
		commd = addStr(commd, line->values[d]);
		commd = addStr(commd, L", ");
		if (line->values[e][0] == '-')
		{
			wchar_t* tmpy = line->values[e] + 1;
			commd = addStr(commd, tmpy);
		}
		else if (wcscmp(line->values[e], L"0") != 0)
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[e]);
		}
		else
		{
			commd = addStr(commd, line->values[e]);
		}

		if (wcscmp(line->tag, L"ellipse") == 0)
		{
			d = getValue(line, L"rx");
			e = getValue(line, L"ry");
			commd = addStr(commd, L") ellipse");

			commd = addStr(commd, L" (");
			commd = addStr(commd, line->values[d]);
			commd = addStr(commd, L" and ");
			commd = addStr(commd, line->values[e]);
			commd = addStr(commd, L");");
		} 
		else if (wcscmp(line->tag, L"circle") == 0)
		{
			d = getValue(line, L"r");
			commd = addStr(commd, L") circle");

			commd = addStr(commd, L" (");
			commd = addStr(commd, line->values[d]);
			commd = addStr(commd, L");");
		}
	}
	fwprintf(outfile, L"%ls\n", commd);
	free(commd);
	return 0;
}


int tikzG(LINE* line, FILE* outfile)
{
	wchar_t* args = NULL;

	args = shapeOptions(args, NULL, line, outfile);
	push(&globalArgs, args);
	
	free(args);
	return 0;
}

int tikzCloseG(LINE* line, FILE* outfile)
{
	pop(&globalArgs);
	return 0;
}

int tikzText(LINE* line, FILE* outfile)
{
	//
	wchar_t* args = NULL;
	if (wcslen(line->content) == 0)
	{
		args = textOptions(args, NULL, line);
		push(&textArgs, args);
		free(args);
		return 0;
	}
	else
	{
		args = textOptions(args, textArgs, line);
		push(&textArgs, args);
	}

	wchar_t* commd = malloc(sizeof(wchar_t));
	commd[0] = '\0';
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\node at");

	int cx = getValue(line, L"x");
	int cy = getValue(line, L"y");

	if(cx != -1 && cy != -1)
	{
		commd = addStr(commd, L" (");
		commd = addStr(commd, line->values[cx]);
		commd = addStr(commd, L", ");
		if(line->values[cy][0] == '-')
		{
			wchar_t* tmpy = line->values[cy] + 1;
			commd = addStr(commd, tmpy);
		}
		else if (wcscmp(line->values[cy], L"0") != 0)
		{
			commd = addChar(commd, '-');
			commd = addStr(commd, line->values[cy]);
		}
		else
		{
			commd = addStr(commd, line->values[cy]);
		}
		commd = addStr(commd, L")");

		if(wcslen(args) > 0)
		{
			commd = addStr(commd, L" [");
			commd = addStr(commd, args);
			commd = addChar(commd, ']');
		}

		commd = addStr(commd, L" {");
		commd = addStr(commd, line->content);
		commd = addStr(commd, L"};");

		fwprintf(outfile, L"%ls\n", commd);
	}
	free(args);
	free(commd);
	return 0;
}

int tikzPattern(LINE* line, FILE* outfile)
{
	//
	wchar_t* commd = malloc(sizeof(wchar_t));
	commd[0] = '\0';

	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"\\pgfdeclarepatterninherentlycolored{svg_");
	int d = getValue(line, L"id");
	commd = addStr(commd, line->values[d]);

	wchar_t *tmp = malloc(13 * sizeof(wchar_t));
	wcscpy(tmp, L"pattern=svg_");
	tmp = addStr(tmp, line->values[d]);
	addArgument(&typelist, line->values[d], tmp);
	free(tmp);
	
	commd = addStr(commd, L"}{\\pgfpoint{0}{0}}{\\pgfpoint{");
	d = getValue(line, L"width");
	commd = addStr(commd, line->values[d]);
	commd = addStr(commd, L"*0.026458cm}{");

	int e = getValue(line, L"height");
	if(line->values[e][0] == '-')
	{
		wchar_t* tmpy = line->values[e] + 1;
		commd = addStr(commd, tmpy);
	}
	else if (wcscmp(line->values[e], L"0") != 0)
	{
		commd = addChar(commd, '-');
		commd = addStr(commd, line->values[e]);
	}
	else
	{
		commd = addStr(commd, line->values[e]);
	}

	commd = addStr(commd, L"*0.026458cm}}{\\pgfpoint{");
	commd = addStr(commd, line->values[d]);
	commd = addStr(commd, L"*0.026458cm}{");
	commd = addStr(commd, line->values[e]);
	commd = addStr(commd, L"*0.026458cm}}\n");
	commd = addPadding(commd, globalPadding);
	commd = addStr(commd, L"{\\begin{tikzpicture}[remember picture, overlay]");
	globalPadding++;
	/*d = getValue(line, L"patternUnits");
	if (d != -1 && wcscmp(line->values[d], L"userSpaceOnUse") == 0)
	{

	}*/

	fwprintf(outfile, L"%ls\n", commd);
	free(commd);
	return 0;
}

int tikzlinearGradient(LINE* line, FILE* infile, FILE* outfile)
{
	wchar_t *args = malloc(13 * sizeof(wchar_t)), *tmp = NULL;
	
	int d = getValue(line, L"id");
	wchar_t * id = line->values[d];

	wcscpy(args, L"shading=axis");
	int x1 = getValue(line, L"x1"), x2 = getValue(line, L"x2"), y1 = getValue(line, L"y1"), y2 = getValue(line, L"y2"); 
	float angle = 90;
	if (x1 != -1 && x2 != -1 && y1 != -1 && y2 != -1)
	{
		float dy = wcstof(line->values[y2], &tmp) - wcstof(line->values[y1], &tmp);
		float dx = wcstof(line->values[x2], &tmp) - wcstof(line->values[x1], &tmp);
		if (dx == 0)
		{
			angle = 0;
		}
		else
		{
			angle += atan(dy / dx);
		}
	}
	
	LINE* stop = createLine(infile);
	d = getValue(stop, L"stop-color");
	if (d != -1)
	{
		args = addStr(args, L", left color=");
		args = addStr(args, stop->values[d]);
	}
	freeLine(stop);

	for(wchar_t c = fgetwc(infile); c != '<'; c = fgetwc(infile))
	{
		if(c == WEOF)
		{
			return -1;
		}
	}
	stop = createLine(infile);
	d = getValue(stop, L"stop-color");
	if (d != -1)
	{
		args = addStr(args, L", right color=");
		args = addStr(args, stop->values[d]);
	}
	freeLine(stop);
	
	args = addStr(args, L", shading angle=");
	tmp = malloc(64 * sizeof(wchar_t));
	swprintf(tmp, 8, L"%3.4f", angle);
	args = addStr(args, tmp);
	free(tmp);
	
	addArgument(&typelist, id, args);
	free(args);
	return 0;
}