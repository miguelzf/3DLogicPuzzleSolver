#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE			1024

typedef int bool;

#define INITIAL_LINES	1023
#define LINE_SIZE		1023

#define BLANKS		isblankc
#define DIGITS		isdig
#define LETTERS		isletter
#define LOWCASE		islowcase
#define HIGHCASE	ishighcase
#define ALFANUM		isalfanum
#define ASCII		isasciic
#define NOTASCII	isnotascii
#define HIGHASCII	ishighascii

#define SPECIAL_FLAG	"-s"


bool isdefault(char* sfrom, char* a, int size)
{
	return (!strncmp(sfrom, a, size));
}


bool (*checkf) (char* c, char *a, int b) = isdefault;


bool isblankc(char* d1, char* a, int d2)
{
	return (
		*a == ' ' ||
		*a == '\t' ||
		*a == '\n' ||
		*a == '\r' );
}

bool isdig(char* d1, char *a, int d2)
{
	return (*a >= '0' && *a <= '9');
}

bool islowcase(char* d1, char *a, int d2)
{
	return ( *a >= 'a' && *a <= 'z');
}

bool ishighcase(char* d1, char *a, int d2)
{
	return ( *a >= 'A' && *a <= 'Z');
}

bool isletter(char* d1, char *a, int d2)
{
	return	( *a >= 'a' && *a <= 'z') || 
			( *a >= 'A' && *a <= 'Z');
}

bool isalfanum(char* d1, char *a, int d2)
{
	return (isletter(d1, a, d2) || isdig(d1, a, d2));
}

bool isasciic(char* d1, char *a, int d2)
{
	return (*a > ' ' || isblankc(d1, a, d2));
}

bool isnotascii(char* d1, char *a, int d2)
{
	return !isasciic(d1, a, d2);
}

bool ishighascii(char* d1, char *a, int d2)
{
	return *a < 0;
}




char* find_arg(char* arg)
{
/* complex args: */
	if (!strcmp(arg, "blank"))
	{	checkf = BLANKS;
		return "all blanks";
	}

	else if (!strcmp(arg, "digit"))
	{	checkf = DIGITS;
		return "all digits";
	}

	else if (!strcmp(arg, "letter"))
	{	checkf = LETTERS;
		return "all letters";
	}
	
	else if (!strcmp(arg, "lowcase")
		||	!strcmp(arg, "lcase"))
	{	checkf = LOWCASE;
		return "all low case letters";
	}
	
	else if (!strcmp(arg, "highcase")
		||	!strcmp(arg, "hcase"))
	{	checkf = HIGHCASE;
		return "all high case letters";
	}
	
	else if (!strcmp(arg, "alfa")
		||	!strcmp(arg, "alfanum"))
	{	checkf = ALFANUM;
		return "all alfanumerical";
	}
	
	else if (!strcmp(arg, "ascii"))
	{	checkf = ASCII;
		return "all ascii bytes";
	}
	
	else if (!strcmp(arg, "hascii")
		||	 !strcmp(arg, "highascii"))
	{	checkf = HIGHASCII;
		return "all bytes from extended ascii";
	}
	
	else if (!strcmp(arg, "notascii")
		||	 !strcmp(arg, "nascii"))
	{	checkf = NOTASCII;
		return "all bytes not ascii";
	}
	
	else
	{
		fprintf(stderr, "Special argument not recognised\n");
		exit(0);
	}
			
	return NULL;
}



char* format_arg(char* arg, int *len)
{
	int a;
	char *itr, *formatstring = malloc(MAXLINE);
	itr = formatstring;
	
	for (--arg; *(++arg); itr++)
	{
		if (*arg == '\\')
		{
			arg++;
			switch (*arg)
			{
				case 'n':
					*itr = '\n';
					break;
				case 't':
					*itr = '\t';
					break;
				case 'r':
					*itr = '\r';
					break;
				case '\'':
					*itr = '\'';
					break;
				case '"':
					*itr = '"';
					break;
				case '0':
					*itr = '\0';
					break;
					
				default:
					a = atoi(arg);
					if (a && a < 256)
					{
						*itr = a;
						while ((a /= 10))
							arg++;
					}
					else
						*itr = '\\';
			}
		}
		else
			*itr = *arg;
	}

	*itr = '\0';
	*len = itr - formatstring;
	return formatstring;
}




int main(int num, char **argv)
{
	int size = INITIAL_LINES;
	int last, arg = 1, start = 0, counter = 0;
	int i, j = 0;
	int fromlen, tolen;
	char* to, *from;
	FILE *fp;
	char **data;
	
	if (num != 5 && num != 4)
	{
		printf("Invalid number of arguments\n");
		return 0;
	}

	if (!strcmp(argv[1], SPECIAL_FLAG))
	{
		if (num != 5)
		{	printf("Invalid number of arguments\n");
			return 0;
		}
		from = find_arg(argv[3]);
		fromlen = 1;
		arg++;
	}
	else
		from = format_arg(argv[2], &fromlen);
		
	if (fromlen == 0)
	{	fprintf(stderr, "String to search may not be null\n");
		exit(0);
	}
	
	if ((fp = fopen(argv[arg], "r")) == NULL)
	{
		printf("File %s does not exist\n", argv[arg]);
		return 0;
	}
	
	data = (char**) malloc((size+1)*sizeof(char*));
	data[0] = (char*) malloc(LINE_SIZE);
	i = 0;
	
	while (1)
	{
		for (	; i < size && LINE_SIZE == 
			(last = fread(data[i], 1, LINE_SIZE, fp)); 
				i++, data[i] = (char*) malloc(LINE_SIZE) );

		if (i < size)
			break;
		
		size *= 2;
		data = (char**) realloc(data, (size+1)*sizeof(char*));
	}

	size = i;
	fclose(fp);
		
	to = format_arg(argv[arg+2], &tolen);
	
	if (checkf == isdefault)
		fprintf(stderr, "From \'%s\' to \'%s\':\n",	argv[arg+1], argv[arg+2]);
	
	else
		fprintf(stderr, "From %s to \'%s\':\n",		argv[arg+1], argv[arg+2]);

	fp = fopen(argv[arg], "w");

	for (i = 0; i < size; i++)
	{
		for (j = start; j <= LINE_SIZE-fromlen; j++)
			if (checkf(from, data[i]+j, fromlen))
			{
				if (j-start)
					fwrite(data[i] +start, 1, j-start, fp);
				fwrite(to, 1, tolen, fp);
				j += fromlen-1;
				start = j+1;
				counter++;
			}
		
		if ( j == LINE_SIZE && fromlen > 1)
		{
			start = 0;
			continue;
		}
		
		for (	; j < LINE_SIZE; j++)
			if (checkf(from, data[i]+j, LINE_SIZE -j)
			&&	checkf(from+LINE_SIZE-j, data[i+1], fromlen - (LINE_SIZE-j)))
			{
				if (j-start)
					fwrite(data[i] +start, 1, j-start, fp);
				fwrite(to, 1, tolen, fp);
				start = fromlen - (LINE_SIZE-j);
				counter++;
				break;
			}

		if ( j == LINE_SIZE)
		{
			if (LINE_SIZE-start)
				fwrite(data[i] + start, 1, LINE_SIZE-start, fp);
			start = 0;
		}
	}

	for (j = start; j <= last-fromlen; j++)
		if (checkf(from, data[i]+j, fromlen))
		{
			if (j-start)
				fwrite(data[i] +start, 1, j-start, fp);
			fwrite(to, 1, tolen, fp);
			j += fromlen-1;
			start = j+1;
			counter++;
		}

	if (last-start)
		fwrite(data[i] + start, 1, last-start, fp);
			
	if (counter)
		fprintf(stderr, "%d ocurrences found and replaced\n", counter);
	else
		fprintf(stderr, "No ocurrences found\n");
	
	fclose(fp);
	free(to);
	if (checkf == isdefault)
		free(from);

	while (i <= size)
		free(data[i++]);
	free(data);

	return 0;
}
	
