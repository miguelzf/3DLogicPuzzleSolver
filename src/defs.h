//------------------------------------------------------------------------------
// 
//  Global solver definitions
// 
//------------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEFT    0
#define DOWN    1
#define RIGHT   2
#define UP      3

#define EMPTY   0
#define WALL    'X'
#define STDMAX  20          // default maximum of BS, cannot be exceded ever 
//#define BS        5       // board size, now determined in runtime

#define STACKSIZE       1024
#define STAMPMAX        ((hi_unit) -1)
#define STAMPSTART      2
#define DISTANCESTART   1
#define DISTMAX         STAMPMAX
#define DISTOBST        (STAMPMAX -1)
#define DEPTHREJECT     3           // IMPORTANT: Must be smaller than the shortest
                                    //  distance between any 2 color points 
#define MAPDEST         1           // value of adjacency to dest. color (p2)
#define MAPWALL         STAMPMAX    // taken points, in each color personal map 

#define BOTTLENECKS     1

#define GOAROUND    0

/* flag for the reconnecting proceedure when backtracking.
use this to avoid useless paths when a new closer adj arrives:
    a 1 1 1     a' = new adj
      a   1         the whole path to the right may 
    a'    1         be erased when using this flag.
1 1 1 1 1 1     but it's much slower.
*/
#define STRAIGHTEN_ALL_PATHS    0

/* in pratice, not useful: */
#define WALLSWAP_OPT    0       // optimization of changing the order of colors 
                                // in its very array, to put first the ones in walls 

                                // the validity/emptiness of positions, is tested by: " < MAPWALL "
                                // to include the cases of EMPTY (0) and mapcmpcol (1, unified), 
                                // for which the function must also be called upon */

#define CUTOUTPUT   1

#define DEBUG       0



typedef unsigned char uchar;
typedef unsigned int  uint;

typedef unsigned short int hi_unit;

typedef uchar unit;
typedef uchar bool;

#define dprintf     if(DEBUG) fprintf


/*
    OPTIMIZATIONS TO DO:
    
    - test WALLSWAP_OPT, swapping colors
    
    - remove Adjacencies! greatly simplifies operations
    
    - use a direction-selection-based system for the main board
        try different heuristics
    
    - switching the use of coords (x,y) with pointers to positions 
        way of getting the coords from the pointer:
            tnmap [(tt - trmap[0]) / 16] [(tt - trmap[0]) % 16] = 0;
        
    - try to speed up the proceeding of the BFS (the stack pushes, and erasing of paths)

    - replace the BFS with a recursive DFS with min. direction choice
        => USELESS, because the free area tends to be pathway-like: sinuous and straight
*/


/*  This Coordinate transformation is necessary because we are mapping a 
    3d half point in a 2d plane. For each move, it's necessary to test if
    the we are jumping over a frontier */

/* used when only one tested coord may differ from the original values */
#define transfsimple(ytest,xtest,yorig,xorig,yres,xres) \
    if ( xtest <= BS || ytest <= BS )   \
    {   xres = xtest;                   \
        yres = ytest;                   \
    }                                   \
    else  /* xaa > BS && yaa > BS*/     \
    {   xres = yorig;                   \
        yres = xorig;                   \
    }

#if 1
/* used when both tested coords may differ from the original values 
    this corresponds to rotations around the cube's center point */
// 
#define transfcomplex(ytest,xtest,yorig,xorig,yres,xres)\
    if (xtest <= BS || ytest <= BS)     \
    {   xres = xtest;                   \
        yres = ytest;                   \
    }                                   \
                                        \
/* xtest > BS && ytest > BS*/           \
                                        \
/* point (BS+1, BS+1) from (BS, BS) */  \
    else if ( xorig == yorig )          \
    { /*not necessary to consider it */ \
        yres = BS*2;                    \
        xres = BS*2;                    \
/* so that the (0,0) point is not assigned a valid */   \
/* value because it could be tested (-1,-1 transf) */ \
    }                                   \
    else if(yorig == BS)                \
    {   xres = BS;                      \
        yres = xtest;                   \
    }                                   \
    else                                \
    {   xres = ytest;                   \
        yres = BS;                      \
    }
#endif




/* if changed to an unsigned value, compatibility must be checked in the 'incrs' matrix */

typedef struct _point
{
    hi_unit v;      /* value, status, timestamp. used as the SP of the predecessor in the stack, 
                    by the BFS. the stack must never grow beyond 255 if unit = uchar */
    hi_unit d;      /* flag, used in various circuntances. d = DISTMAX indicates an adjacency point, 
                    or P1 (start point), or a cut point in the middle of a path */
    /* predecessor point: */    
    unit y;
    unit x;
} point;


typedef struct _color 
{   /* start and end coords: */
    unit x1;
    unit y1;
    unit x2;
    unit y2;
    
    unit    **area;     /* optimization: boolean matrix with the adjs marked */
    point   **map;
    unit    ascii;
    hi_unit stamp;
    hi_unit distance;

    /* BFS stucff */
    point   *stackbfs;
    point   *stackrej;
    point   *sj4stackrej;       /* memoization */

/*  simple memoization little optimizations */
    /* pointers to the values of adjs of endpoint: */
    hi_unit *adj[4];
    /* pointers to the flags of adjs of endpoint: */
    hi_unit *adjd[4];
    
    unsigned int lastbnset;
    bool **bnmap;           /* bottlenecks map */
    point* bnpath;
} color;



/******* debug and benchmarking ********/

time_t t1;

extern unsigned int 
    calls, remapcalls,
    bfscalls, bfsfails,
    resetstamps, resetdists, 
    testblocks, blocksfound, blocksgood,
    testrejects, rejectsdone, 
    aroundcalls, aroundfound, ignoredrounds;
    
/****************************************/


unit **cube;    /* main board, 2D matrix */

int BS;         /* board side size. size of cube = BS*BS */

FILE *outp;     /* output stream */

color *colors;  /* main data struct. indexed by color */

int ncolors;    /* num of colors */

unit** area;

color *trcc;    /* optimization: current trmap being used */
/* temp. variables for optimization */

color *colort, *colortinc, *colorend;

extern unit currcolor;
extern unit cmpcolor;       /* ascii value of color, used to mark the 2 points to be connected */

point **trmap;

