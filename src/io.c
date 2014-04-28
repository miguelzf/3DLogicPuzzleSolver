
//------------------------------------------------------------------------------
// 
//  Input/Output
// 
//------------------------------------------------------------------------------

/* mapping of the cube:
       2nd x -->
1st,y   xxxxxxx
    |   xxxxxxx
    v   xxxxxxx
        xxxxxxx
        xxxxxxx
        xxxxxxxxxxxxxx
        xxxxxxxxxxxxxx
        xxxxxxxxxxxxxx
        xxxxxxxxxxxxxx
        xxxxxxxxxxxxxx
*/

#include <ctype.h>
#include "defs.h"
#include "io.h"


static int *mapper;     /* maps the internal Color Id (index) to the original cube character of that color */


#define INWALL(y,x)     \
( x == 1 || y == 1 || x == BS*2 || y == BS*2 )


#if WALLSWAP_OPT
#define COLORSWAP(x,y)                          \
{   memmove(&buffer, colors+y, sizeof(color));  \
    memmove(colors+y, colors+x, sizeof(color)); \
    memmove(colors+x, &buffer, sizeof(color));  \
\
    k = mapper[x];                              \
    mapper[x] = mapper[y];                      \
    mapper[y] = k;                              \
    k = (int) colors[x].area;                   \
    colors[x].area = colors[y].area;            \
    colors[y].area = (void*) k;                 \
\
    cube[colors[x].y1][colors[x].x1] = x;       \
    cube[colors[x].y2][colors[x].x2] = x;       \
    cube[colors[y].y1][colors[y].x1] = y;       \
    cube[colors[y].y2][colors[y].x2] = y;       \
}

#else
#define COLORSWAP(x,y)
#endif



static  int unique (int* vec, int val)
{
    int i = 1;
    for ( ; vec[i]; i++)
        if (vec[i] == val) 
            return i;
    return 0;
}



static void fillarea (int x, int y, int nc)
{
    unit **area = colors[nc].area;
    int i, tx, ty;
    
    if ( area == 0)
    {
        area = colors[nc].area = (unit**) malloc ((BS*2 +2)*sizeof(unit*));
        for (i = 0; i <= BS*2+1; i++)
            area[i]= (unit*) calloc ((BS*2 +2), sizeof(unit));
    }

#if !WALLSWAP_OPT
    else {
#endif
        area[y][x-1] = 1;
        area[y-1][x] = 1;

        transfsimple(y, x+1, y, x, ty, tx);
        area[ty][tx] = 1;
        transfsimple(y+1, x, y, x, ty, tx);
        area[ty][tx] = 1;
#if !WALLSWAP_OPT
    }
#endif
}



void readboard (FILE* cubecfg)
{
    int i, j, k, ic, init, id, cont;
    char buffer[STDMAX*5];

    if ( !fgets (buffer, STDMAX*5 -1, cubecfg))
    {   printf("empty file");
        exit(1);
    }

/* determine board size */  
    for ( j = 0, i = 0; i < STDMAX && buffer[i] != '\n' ; i++ )
        if ( buffer[i] != ' ' )
            j++;
    
    BS = j;
/*  if ( BS != j || j >= STDMAX )
    {   printf("invalid configuration: 1st line too long\n");
        exit(1);
    }
*/
    cube = (unit**) malloc ((BS*2 +2)*sizeof(unit*));

    for (i = 0; i <= BS*2+1; i++)
        cube[i] = (unit*) calloc ((BS*2 +2), sizeof(unit));

    ncolors = init = 0;
    k =  ic = cont = 1;
    i = BS*2;
    mapper = (int*) calloc (STDMAX*5, sizeof(int)); 
    
    while (1)
    {   /* the 1st line has already been read */
        if ( buffer[0] == '\n' )
        {   
            if ( ++cont == 3 )
                i = init = BS;
            goto fetch_line;
        }
            
        for ( j = 0, k = init+1; buffer[j] != '\n' && j < STDMAX*5; j++ )
            if ( buffer[j] != ' ' )
            {
//              printf("read #%d = %c | i = %d\n", k, buffer[j], i );
                if ( buffer[j] == '-' )
                    cube[i][k++] = 0;
                else if (buffer[j] == WALL)
                    cube[i][k++] = WALL;

                else if (!(id = unique (mapper, tolower(buffer[j]))))
                {   colors[ic].x1 = k;
                    colors[ic].y1 = i;
                    mapper[ic] = tolower(buffer[j]);    /* used lowercase to cover the paths, and uppercase for starting and ending point */
                    fillarea(k, i, ic);
                    cube[i][k++] = ic++;
                }
                else
                {   colors[id].x2 = k;
                    colors[id].y2 = i;
                    fillarea(k, i, id);
                    cube[i][k++] = id;
                }
            }

        if ( buffer[j] != '\n' )
        {   printf("invalid configuration: %dth line too long", i);
            exit(1);
        }
        i--;

fetch_line:
        if ( !fgets (buffer, STDMAX*5, cubecfg))
            break;
    }

    ncolors = ic-1;


    for (i = 1; INWALL(colors[i].y1, colors[i].x1) ||
            INWALL(colors[i].y2, colors[i].x2) ; i++);

    for (j = i+1; j <= ncolors; j++)
        if (INWALL(colors[j].y1, colors[j].x1) ||
            INWALL(colors[j].y2, colors[j].x2) )
        {
            COLORSWAP(i, j);
            i++;
        }

    /* walling of frontiers, to remove the validity tests of matrix indices */
    for (i = 0; i <= BS*2+1; i++)
    {   cube[0][i]  = WALL;
        cube[BS*2+1][i]= WALL;
        cube[i][0]  = WALL;
        cube[i][BS*2+1] = WALL;
    }

    for (i = BS*2; i > BS; i--)
    {   cube[BS+1][i]= WALL;
        cube[i][BS+1]= WALL;
    }

}



void printboard (void)
{
    int i, j; 
    
    if (CUTOUTPUT) return;

    fprintf(outp, "Cube configuration:\n");

/*  for (i = BS*2 +1; i >= 0; i--, fprintf(outp, "\n"))
        for (j = 0; j <= 2*BS+1; j++)
        {   if ( cube[i][j] < 32 )  fprintf(outp, "%d  ", cube[i][j]);
            else                fprintf(outp, "%c  ", cube[i][j]);
        }
*/
    for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS; j++)
            if ( cube[i][j] < 32 )
                fprintf(outp, "%d  ", cube[i][j]);
            else
                fprintf(outp, "%c  ", cube[i][j]);

/*  for (j = 1; j < BS; j++ ,fprintf(outp, "==")); 
    fprintf(outp, "\n");
    if ( j == BS+1) fprintf(outp, "||  ");
*/
    for (   ; i > 0; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS*2; j++)
            if ( cube[i][j] < 32 )
                fprintf(outp, "%d  ", cube[i][j]);
            else
                fprintf(outp, "%c  ", cube[i][j]);
}


void printpath(int k);


void printareas (void)
{
    int i,j,k;
    fprintf(outp, "Cube's encircling areas:\n");
    if (CUTOUTPUT) return;

    for ( k = 1; k <= ncolors; k++ )
    {
        unit **area = colors[k].area;
        
        for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
            for (j = 1; j <= BS; j++)
                fprintf(outp, "%d  ", area[i][j]);

        for (   ; i > 0; i--, fprintf(outp, "\n"))
            for (j = 1; j <= BS*2; j++)
                fprintf(outp, "%d  ", area[i][j]);
    }
}



void printcolors(void)
{
    int i;
    for (i = 1; i <= ncolors; i++ )
    {   fprintf(outp, "%c : (%d, %d) - (%d, %d)\n", mapper[i], 
            colors[i].y1, colors[i].x1, colors[i].y2, colors[i].x2 /*, colors[i].m */ );
    }
}



void printresult(void)
{
    int i,j;
    outp = stdout;
    fprintf(outp, "\nSolution:\n");

    mapper[0] = '-';

    for ( i = 1; i <= ncolors; i++ )
    {
        cube [colors[i].y1] [colors[i].x1] = mapper[i] - ' ';
        cube [colors[i].y2] [colors[i].x2] = mapper[i] - ' ';
    }

    for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS; j++)
        {   if ( cube[i][j] < 32 )  fprintf(outp, "%c  ", mapper[cube[i][j]] );
            else                    fprintf(outp, "%c  ", cube[i][j]);
        }

    for (j = 1; j < BS; j++ ,fprintf(outp, "===")); 
    fprintf(outp, "=\n");

    for (   ; i > 0; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS*2; j++)
        {
            if ( j == BS+1)
                fprintf(outp, "||  ");
            if ( cube[i][j] < 32 )
                fprintf(outp, "%c  ", mapper[cube[i][j]] );
            else
                fprintf(outp, "%c  ", cube[i][j]);
        }
}


void printdistmap(int k)
{
    point** cub;
    int i, j;
    if (CUTOUTPUT) return;

    cub = colors[k].map;
    fprintf(outp, "\nDist Map for color %d :\n", k);

    for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS; j++)
        {
            if  (cub[i][j].d == DISTMAX)    fprintf(outp, " X ");
            else                            fprintf(outp, "%2d ", cub[i][j].d );
        }

    for (   ; i > 0; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS*2; j++)
        {
            if  (cub[i][j].d == DISTMAX)    fprintf(outp, " X ");
            else                            fprintf(outp, "%2d ", cub[i][j].d );
        }
}




void printmap (int k)
{
    point** cub;
    int i, j;
    if (CUTOUTPUT) return;
    
    cub = colors[k].map;
    fprintf(outp, "Map for color %d :\n", k);

    for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS; j++)
        {
            if  (cub[i][j].v == MAPWALL)
                fprintf(outp, " X ");
            else if (cub[i][j].v > MAPDEST)
            {
                if (cub[i][j].v % 100 == MAPDEST)
                    fprintf(outp, " s ");
                else if (cub[i][j].v % 100 == 0)
                    fprintf(outp, " z ");
                else
                    fprintf(outp, "%2d ", cub[i][j].v % 100 );
            }
            else
                fprintf(outp, "%2d ", cub[i][j].v);
        }

    for (   ; i > 0; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS*2; j++)
        {
            if  (cub[i][j].v == MAPWALL)
                fprintf(outp, " X ");
            else if (cub[i][j].v > MAPDEST)
            {
                if (cub[i][j].v % 100 == MAPDEST)
                    fprintf(outp, " s ");
                else if (cub[i][j].v % 100 == 0)
                    fprintf(outp, " z ");
                else
                    fprintf(outp, "%2d ", cub[i][j].v % 100 );
            }
            else
                fprintf(outp, "%2d ", cub[i][j].v);
        }
//  printpath(k);
}

void printmaps (void)
{
    int k;
    for (k = 2; k <= ncolors; k++ )
    {   printmap(k);
        printpath(k);
    }
}


void printpath(int k)
{
    point *tt;
    point **rmap = colors[k].map;

    if (CUTOUTPUT) return;

    fprintf (outp, "path for color %d:\n", k);
    for ( tt = & rmap [colors[k].y2] [colors[k].x2];
        ( tt->y != colors[k].y1 || tt->x != colors[k].x1 ) 
            && (tt->y != 0 || tt->x != 0);
                tt = & rmap[tt->y][tt->x])  
        fprintf (outp, "(%d,%d - %d.%d) ", 
            tt->y, tt->x, rmap[tt->y][tt->x].v, trmap[tt->y][tt->x].d);

    fprintf (outp, "(%d,%d - %d.%d)\n", tt->y, tt->x, 
             rmap[tt->y][tt->x].v, rmap[tt->y][tt->x].d);
}


void printpaths(void)
{
    int i;
    for ( i = 2; i <= ncolors; i++)
        printpath(i);
}


int testpocket(int nc)
{
    return 0;
    point **rmap = colors[nc].map;
    int i, j;
        
    for (i = BS*2; i > 1; i--)
        for (j = BS*2; j > 1; j--)
            if (rmap[i][j].v    == MAPDEST
            &&  rmap[i-1][j].v  == MAPDEST
            &&  rmap[i][j-1].v  == MAPDEST
            &&  rmap[i-1][j-1].v== MAPDEST)
            {
                fprintf(stderr, "POCKET FOUND at call %d!\n", calls);
//              printmap(nc);
//              printpath(nc);
            }
            
    return -1;
}
