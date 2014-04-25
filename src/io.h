
// ============================================================================
// 
//	Input/Output
// 
// ============================================================================


/* mapping of the cube:
       2nd x -->
1st,y	xxxxxxx
    |	xxxxxxx
    v	xxxxxxx
		xxxxxxx
		xxxxxxx
		xxxxxxxxxxxxxx
		xxxxxxxxxxxxxx
		xxxxxxxxxxxxxx
		xxxxxxxxxxxxxx
		xxxxxxxxxxxxxx
*/


void readboard	(FILE* cubecfg);

void printboard	(void);

void printpath	(int k);

void printareas	(void);

void printcolors(void);

void printresult(void);

void printdistmap(int k);

void printmap	(int k);

void printmaps	(void);

void printpath	(int k);

void printpaths	(void);

int testpocket(int nc);

