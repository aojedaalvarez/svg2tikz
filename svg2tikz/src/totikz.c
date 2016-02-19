
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
	wchar_t *tmpStr = malloc((padding * PADDING_SIZE + 1) * sizeof(wchar_t));
	for (int i = 0; i < padding * PADDING_SIZE; i++)
	{
		tmpStr[i] = ' ';
	}
	tmpStr[padding * PADDING_SIZE] = '\0';
	tmpStr = addStr(tmpStr, dest);
	free(dest);
	return tmpStr;
}

wchar_t* addStr(wchar_t* dest, const wchar_t* src)
{
	int n = wcslen(dest), m = wcslen(src);
	if (src != NULL && m > 0)
	{
		dest = realloc(dest, (n + m + 1) * sizeof(wchar_t));
		wcscat(dest, src);
	}
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
	if(!isInNode(colors, sColor + 1))
	{
		int lenCor = (wcslen(sColor) - 1) / 3;
		wchar_t *tmp = NULL, R[lenCor + 1], G[lenCor + 1], B[lenCor + 1];
		for (int i = 0; i < lenCor; i++)
		{
			R[i] = sColor[i + 1];
			G[i] = sColor[lenCor + i + 1];
			B[i] = sColor[2 * lenCor + i + 1];
		}
		R[lenCor] = '\0';
		G[lenCor] = '\0';
		B[lenCor] = '\0';
		int r = wcstol(R, &tmp, 16) * 255 / (pow(16, lenCor) - 1), g = wcstol(G, &tmp, 16) * 255 / (pow(16, lenCor) - 1), b = wcstol(B, &tmp, 16) * 255 / (pow(16, lenCor) - 1);
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

	int d = getValue(line, L"style");
	if (d != -1)
	{
		wchar_t *buffer = malloc(sizeof(wchar_t));
		buffer[0] = '\0';
		buffer = addStr(buffer, L"<style ");
		for (int i = 0, n = wcslen(line->values[d]); i < n; i++)
		{
			if (line->values[d][i] == ':')
			{
				buffer = addStr(buffer, L"=\"");
			}
			else if (line->values[d][i] == ';')
			{
				buffer = addStr(buffer, L"\" ");
			}
			else if (line->values[d][i] == ' ' || !iswprint(line->values[d][i]))
			{
				continue;
			}
			else
			{
				buffer = addChar(buffer, line->values[d][i]);
			}
		}
		buffer = addStr(buffer, L"\"\\>");

		LINE* tmpLine = createLine(buffer);
		tmpLine->content = NULL;

		wchar_t *tmpArgs = NULL;
		tmpArgs = shapeOptions(tmpArgs, NULL, tmpLine, outfile);
		if (wcslen(args) > 0 && wcslen(tmpArgs) > 0)
		{
			args = addStr(args, L", ");
		}
		if (wcslen(tmpArgs) > 0)
		{
			args = addStr(args, tmpArgs);
		}
		free(buffer);
		free(tmpArgs);
		freeLine(tmpLine);
	}
	d = getValue(line, L"stroke");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		if (line->values[d][0] == '#')
		{
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
	d = getValue(line, L"stroke-opacity");
	if (d != -1 && wcscmp(line->values[d], L"none") != 0)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"draw opacity=");
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
		wchar_t* tmpValue = NULL, buffer[16];
		float wd = wcstof(line->values[d], &tmpValue) * 0.75;
		swprintf(buffer, 16, L"%.4f", wd);
		args = addStr(args, buffer);
	}
	args = commonOptions(args, line);
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
			wcscpy(inherit, L"anchor=south west");
			args = addStr(args, L"anchor=south west");
		}
		if (wcscmp(line->values[d], L"end") == 0)
		{
			wcscpy(inherit, L"anchor=south east");
			args = addStr(args, L"anchor=south east");
		}
		if (wcscmp(line->values[d], L"middle") == 0)
		{
			wcscpy(inherit, L"anchor=south");
			args = addStr(args, L"anchor=south");
		}
	}
	else
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, inherit);
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
		wchar_t* tmpValue = NULL, buffer[16];
		float wd = wcstof(line->values[d], &tmpValue) * 0.75;
		swprintf(buffer, 16, L"%.4f", wd);
		args = addStr(args, buffer);
	}
	d = getValue(line, L"dy");
	if (d != -1)
	{
		if (wcslen(args) > 0)
		{
			args = addStr(args, L", ");
		}
		args = addStr(args, L"yshift=");
		wchar_t* tmpValue = NULL, buffer[16];
		float wd = - wcstof(line->values[d], &tmpValue) * 0.75;
		swprintf(buffer, 16, L"%.4f", wd);
		args = addStr(args, buffer);
	}
	args = commonOptions(args, line);
	return args;
}

wchar_t* commonOptions(wchar_t* args, LINE* line)
{
	int	d = getValue(line, L"transform");
	if (d != -1)
	{
		wchar_t *transform = line->values[d];
		while (wcslen(transform) > 0)
		{
			wchar_t type[16] = L"", value[64] = L"";
			int i = 0, j = 0;
			for (i = 0; transform[i] != '('; i++)
			{
				type[j++] = transform[i];
			}
			type[j] = '\0';
			for (; !iswdecimal(transform[i]) && transform[i] != '-'; i++);
			for (j = 0; transform[i] != ')'; i++)
			{
				value[j++] = transform[i];
			}
			value[j] = '\0';
			for (; !iswalpha(transform[i]) && transform[i] != '\0'; i++);
			transform += i;
			if (wcscmp(type, L"translate") == 0)
			{
				if (wcslen(args) > 0)
				{
					args = addStr(args, L", ");
				}
				wchar_t *tmpStr = NULL, buffer[32];
				args = addStr(args, L"shift=");
				float xs = wcstof(value, &tmpStr), ys = 0;
				for (j = 0; tmpStr[j] != '\0' && !iswdecimal(tmpStr[j]) && tmpStr[j] != '-'; j++);
				if (wcslen(tmpStr + j) > 0)
				{
					ys = - wcstof(tmpStr + j, &tmpStr);
				}
				swprintf(buffer, 32, L"{(%.4f, %.4f)}", xs, ys);
				args = addStr(args, buffer);
			}
			else if (wcscmp(type, L"rotate") == 0)
			{
				if (wcslen(args) > 0)
				{
					args = addStr(args, L", ");
				}
				wchar_t *tmpStr = NULL, buffer[64];
				float angle = - wcstof(value, &tmpStr), xs = 0, ys = 0;
				args = addStr(args, L"rotate around=");
				for (j = 0; tmpStr[j] != '\0' && !iswdecimal(tmpStr[j]) && tmpStr[j] != '-'; j++);
				if (wcslen(tmpStr + j) > 0)
				{
					xs = wcstof(tmpStr + j, &tmpStr);
				}
				for (j = 0; tmpStr[j] != '\0' && !iswdecimal(tmpStr[j]) && tmpStr[j] != '-'; j++);
				if (wcslen(tmpStr + j) > 0)
				{
					ys = - wcstof(tmpStr + j, &tmpStr);
				}
				swprintf(buffer, 64, L"{%.2f:(%.4f, %.4f)}", angle, xs, ys);
				args = addStr(args, buffer);
			}
			else if (wcscmp(type, L"scale") == 0)
			{
				if (wcslen(args) > 0)
				{
					args = addStr(args, L", ");
				}
				wchar_t *tmpStr = NULL, buffer[32];
				float fs = wcstof(value, &tmpStr);
				args = addStr(args, L"xscale=");
				swprintf(buffer, 32, L"%.4f", fs);
				args = addStr(args, buffer);

				for (j = 0; tmpStr[j] != '\0' && !iswdecimal(tmpStr[j]) && tmpStr[j] != '-'; j++);
				if (wcslen(tmpStr + j) > 0)
				{
					fs = wcstof(tmpStr + j, &tmpStr);
					args = addStr(args, L", yscale=");
					swprintf(buffer, 32, L"%.4f", fs);
					args = addStr(args, buffer);
				}
			}
		}
	}
	return args;
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
		for (int i = 0, pLen = wcslen(path); i < pLen;)
		{
			if (path[i] == 'M')
			{
				commd = addStr(commd, L" (");
				for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
				while(!iswalpha(path[i]))
				{
					i = addPoint(&commd, path, i);
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(iswdecimal(path[i]) || path[i] == '-')
					{
						commd = addStr(commd, L") -- (");
					}
					else
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(iswdecimal(path[i]) || path[i] == '-')
					{
						commd = addStr(commd, L") -- ++(");
					}
					else
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(iswdecimal(path[i]) || path[i] == '-')
					{
						commd = addStr(commd, L") -- (");
					}
					else
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(iswdecimal(path[i]) || path[i] == '-')
					{
						commd = addStr(commd, L") -- ++(");
					}
					else
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
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" -- (");
					wchar_t* tmpPoint = NULL;
					lastPos.x = wcstof(path + i, &tmpPoint);
					if (path[i] == '-')
					{
						commd = addChar(commd, '-');
						i++;
					}
					while (iswdecimal(path[i]))
					{
						commd = addChar(commd, path[i++]);
					}
					wchar_t lastY[32];
					swprintf(lastY, 32, L", %.4f", lastPos.y);
					commd = addStr(commd, lastY);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				
				lastPos.path = 'H';
				lastControl.path = '\0';
			}
			else if (path[i] == 'h')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" -- ++(");
					wchar_t* tmpPoint = NULL;
					lastPos.x += wcstof(path + i, &tmpPoint);
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				
				lastPos.path = 'h';
				lastControl.path = '\0';
			}
			else if (path[i] == 'V')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					wchar_t lastX[32];
					swprintf(lastX, 32, L"%.4f", lastPos.x);
					commd = addStr(commd, L" -- (");
					commd = addStr(commd, lastX);
					commd = addStr(commd, L", ");
					wchar_t* tmpPoint = NULL;
					lastPos.y = - wcstof(path + i, &tmpPoint);
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				lastPos.path = 'V';
				lastControl.path = '\0';
			}
			else if (path[i] == 'v')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" -- ++(0, ");
					wchar_t* tmpPoint = NULL;
					lastPos.y -= wcstof(path + i, &tmpPoint);
					
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
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
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
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
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}

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
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls ++(");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&commd, path, i);
					commd = addStr(commd, L") and ++(");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);

					wchar_t* tmpPoint = NULL;
					lastControl.x = lastPos.x + wcstof(path + i, &tmpPoint);
					int j = 0;
					for (; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastControl.y = lastPos.y - wcstof(tmpPoint + j, &tmpPoint);
					
					i = addPoint(&commd, path, i);
					commd = addStr(commd, L") .. ++(");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);

					lastPos.x += wcstof(path + i, &tmpPoint);
					for (j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastPos.y -= wcstof(tmpPoint + j, &tmpPoint);

					i = addPoint(&commd, path, i);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				lastControl.path = 'c';
				lastPos.path = 'c';
			}
			else if (path[i] == 'S')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls (");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					int j = i;
					
					float cx, cy;
					wchar_t *point = NULL;
					if(lastControl.path == 'C' || lastControl.path == 'c' || lastControl.path == 'S' || lastControl.path == 's')
					{
						cx = 2 * lastPos.x - lastControl.x;
						cy = 2 * lastPos.y - lastControl.y;
						point = malloc(64 * sizeof(wchar_t));
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
					free(point);

					wchar_t *tmpPoint;
					lastControl.x = wcstof(path + i, &tmpPoint);
					for (j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastControl.y = - wcstof(tmpPoint + j, &tmpPoint);
					lastControl.path = 'S';

					i = addPoint(&commd, path, i);
					commd = addStr(commd, L") .. (");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);

					lastPos.x = wcstof(path + i, &tmpPoint);
					for (j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastPos.y = - wcstof(tmpPoint + j, &tmpPoint);

					i = addPoint(&commd, path, i);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				
				lastPos.path = 'S';
			}
			else if (path[i] == 's')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls ");
					
					float cx, cy;
					int j = 0;
					wchar_t * point = NULL;
					if(lastControl.path == 'C' || lastControl.path == 'c' || lastControl.path == 'S' || lastControl.path == 's')
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
					free(point);

					wchar_t* tmpPoint = NULL;
					lastControl.x = lastPos.x + wcstof(path + i, &tmpPoint);
					for (j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastControl.y = lastPos.y - wcstof(tmpPoint + j, &tmpPoint);
					lastControl.path = 's';

					i = addPoint(&commd, path, i);
					commd = addStr(commd, L") .. ++(");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);

					lastPos.x += wcstof(path + i, &tmpPoint);
					for (j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastPos.y -= wcstof(tmpPoint + j, &tmpPoint);

					i = addPoint(&commd, path, i);
					commd = addChar(commd, ')');

					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}

				lastPos.path = 's';
			}
			else if (path[i] == 'Q')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls (");
					wchar_t* cpoint = malloc(sizeof(wchar_t)), *tmpPoint = NULL;
					cpoint[0] = '\0';	

					//for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&cpoint, path, i);

					int j = 0;
					float cx = wcstof(cpoint, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					float cy = wcstof(tmpPoint + j, &tmpPoint);
					wchar_t buffer[32];
					swprintf(buffer, 32, L"%.4f, %.4f", lastPos.x + 2 * (cx - lastPos.x) / 3, lastPos.y + 2 * (cy - lastPos.y) / 3);
					commd = addStr(commd, buffer);
				
					commd = addStr(commd, L") and (");
					cpoint = realloc(cpoint, sizeof(wchar_t));
					cpoint[0] = '\0';
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&cpoint, path, i);

					lastPos.x = wcstof(cpoint, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					lastPos.y = wcstof(tmpPoint + j, &tmpPoint);
					lastControl.x = lastPos.x + 2 * (cx - lastPos.x) / 3;
					lastControl.y = lastPos.y + 2 * (cy - lastPos.y) / 3;
					swprintf(buffer, 32, L"%.4f, %.4f", lastControl.x, lastControl.y);
					commd = addStr(commd, buffer);
					commd = addStr(commd, L") .. (");
					commd = addStr(commd, cpoint);
					free(cpoint);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				
				lastControl.path = 'Q';
				lastPos.path = 'Q';
			}
			else if (path[i] == 'q')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls (");
					wchar_t* cpoint = malloc(sizeof(wchar_t)), *tmpPoint = NULL;
					cpoint[0] = '\0';

					//for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&cpoint, path, i);

					int j = 0;
					float cx = wcstof(cpoint, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					float cy = wcstof(tmpPoint + j, &tmpPoint);
					wchar_t buffer[32];
					swprintf(buffer, 32, L"%.4f, %.4f", lastPos.x + 2 * cx / 3, lastPos.y + 2 * cy / 3);
					commd = addStr(commd, buffer);
				
					commd = addStr(commd, L") and (");
					cpoint = realloc(cpoint, sizeof(wchar_t));
					cpoint[0] = '\0';
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&cpoint, path, i);

					float fx = wcstof(cpoint, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					float fy = wcstof(tmpPoint + j, &tmpPoint);
					swprintf(buffer, 32, L"%.4f, %.4f", fx + lastPos.x + 2 * (cx - fx) / 3, fy + lastPos.y + 2 * (cy - fy) / 3);


					lastControl.x = fx + lastPos.x + 2 * (cx - fx) / 3;
					lastControl.y = fy + lastPos.y + 2 * (cy - fy) / 3;
					lastPos.x += fx;
					lastPos.y += fy;
					lastControl.path = 'q';

					commd = addStr(commd, buffer);
					commd = addStr(commd, L") .. ++(");
					commd = addStr(commd, cpoint);
					free(cpoint);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}
				
				lastPos.path = 'q';
			}
			else if(path[i] == 'T')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls (");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					int j = i;
				
					float cx, cy;
					wchar_t * point = NULL;
					if(lastControl.path == 'Q' || lastControl.path == 'q' || lastControl.path == 'T' || lastControl.path == 't')
					{
						cx = 2 * lastPos.x - lastControl.x;
						cy = 2 * lastPos.y - lastControl.y;
						point = malloc(32 * sizeof(wchar_t));
						swprintf(point, 32, L"%.4f, %.4f", cx, cy);
					}
					else
					{
						point = malloc(32 * sizeof(wchar_t));
						swprintf(point, 32, L"%.4f, %.4f", lastPos.x, lastPos.y);
					}

					commd = addStr(commd, point);
					//free(point);

					point = realloc(point, sizeof(wchar_t));
					point[0] = '\0';
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&point, path, i);

					wchar_t buffer[32], *tmpPoint = NULL;
					float fx = wcstof(point, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					float fy = wcstof(tmpPoint + j, &tmpPoint);
					free(point);

					commd = addStr(commd, L") and (");
					lastControl.x = cx + (fx - lastPos.x) / 3;
					lastControl.y = cy + (fy - lastPos.y) / 3;
					swprintf(buffer, 32, L"%.4f, %.4f", lastControl.x, lastControl.y);
					commd = addStr(commd, buffer);
					commd = addStr(commd, L") .. (");
					lastControl.path = 'T';
				
					swprintf(buffer, 32, L"%.4f, %.4f", fx, fy);
					lastPos.x = fx;
					lastPos.y = fy;
				
					commd = addStr(commd, buffer);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}

				lastPos.path = 'T';
			}
			else if(path[i] == 't')
			{
				for(; path[i] != '\0' && !iswdecimal(path[i]) && path[i] != '-'; i++);
				while (!iswalpha(path[i]))
				{
					commd = addStr(commd, L" .. controls (");
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					int j = i;
					
					float cx, cy;
					wchar_t * point = NULL;
					if(lastControl.path == 'Q' || lastControl.path == 'q' || lastControl.path == 'T' || lastControl.path == 't')
					{
						cx = 2 * lastPos.x - lastControl.x;
						cy = 2 * lastPos.y - lastControl.y;
						point = malloc(32 * sizeof(wchar_t));
						swprintf(point, 32, L"%.4f, %.4f", cx, cy);
					}
					else
					{
						point = malloc(32 * sizeof(wchar_t));
						swprintf(point, 32, L"%.4f, %.4f", lastPos.x, lastPos.y);
					}

					commd = addStr(commd, point);
				
					point = realloc(point, sizeof(wchar_t));
					point[0] = '\0';
					for(; !iswdecimal(path[i]) && path[i] != '-'; i++);
					i = addPoint(&point, path, i);

					wchar_t buffer[32], *tmpPoint = NULL;
					float fx = wcstof(point, &tmpPoint);
					for(j = 0; !iswdecimal(tmpPoint[j]) && tmpPoint[j] != '-'; j++);
					float fy = wcstof(tmpPoint + j, &tmpPoint);
					free(point);

					commd = addStr(commd, L") and (");
					lastControl.x = cx + fx / 3;
					lastControl.y = cy + fy / 3;
					swprintf(buffer, 32, L"%.4f, %.4f", lastControl.x, lastControl.y);
					commd = addStr(commd, buffer);
					commd = addStr(commd, L") .. ++(");
				
					swprintf(buffer, 32, L"%.4f, %.4f", fx, fy);
					lastPos.x += fx;
					lastPos.y += fy;
					lastControl.path = 't';
				
					commd = addStr(commd, buffer);
					commd = addChar(commd, ')');
					for (; i < pLen && !(path[i] != ' ' && iswprint(path[i])); i++);
					if(path[i] == '\0')
					{
						break;
					}
				}

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
		if (line->values[d][0] != '\0')
		{
			commd = addStr(commd, line->values[d]);	
		}
		else
		{
			commd = addChar(commd, '0');
		}
		commd = addStr(commd, L", ");

		if (line->values[e][0] == '\0')
		{
			commd = addChar(commd, '0');
		}
		else if (line->values[e][0] == '-')
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
	commd = addStr(commd, L"*0.75pt}{");

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

	commd = addStr(commd, L"*0.75pt}}{\\pgfpoint{");
	commd = addStr(commd, line->values[d]);
	commd = addStr(commd, L"*0.75pt}{");
	commd = addStr(commd, line->values[e]);
	commd = addStr(commd, L"*0.75pt}}\n");
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
	

	wchar_t *buffer = malloc(sizeof(wchar_t)), c = fgetwc(infile);
	buffer[0] = '\0';
	for(; c != '<'; c = fgetwc(infile))
	{
		if(c == WEOF)
		{
			return -1;
		}
	}
	int n = 0;
	for (; c != '>'; c = fgetwc(infile))
	{
		buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
		buffer[n++] = c;
	}
	c = fgetwc(infile);
	buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
	buffer[n++] = '>';
	buffer[n] = '\0';

	LINE *stop = createLine(buffer);
	free(buffer);

	d = getValue(stop, L"stop-color");
	if (d != -1)
	{
		args = addStr(args, L", left color=");
		args = addStr(args, stop->values[d]);
	}
	freeLine(stop);

	buffer = malloc(sizeof(wchar_t));
	buffer[0] = '\0';
	for(; c != '<'; c = fgetwc(infile))
	{
		if(c == WEOF)
		{
			return -1;
		}
	}
	n = 0;
	for (; c != '>'; c = fgetwc(infile))
	{
		buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
		buffer[n++] = c;
	}
	c = fgetwc(infile);
	buffer = realloc(buffer, (n + 2) * sizeof(wchar_t));
	buffer[n++] = '>';
	buffer[n] = '\0';

	stop = createLine(buffer);
	free(buffer);

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