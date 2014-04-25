
// ============================================================================
// 
//  Solver for the 3DLogic puzzle
// 
// ============================================================================

/*
    It was used an incrementing path-finding strategy:
        each color, from the start point, tries to the reach is end point.
        The path-finding strategy is random, and many heuristics may be used.
    
    Each color is first assigned an ideal path, ignoring the other colors.
    These paths are in the trmap matrixes. These matrixes are updated for 
    each update in the main cube matrix.
    
    For each color setting, it's tested if it conflicts with the following 
    colors trmap paths. If there is, the trmap path is remapped, changed, 
    to find a new possible path for the color. 
    A conflict is found if there's no possible path.
*/


#include "defs.h"
#include "io.h"


#include "propagation.h"
#include "bfs.h"
#include "bottlenecks.h"

unit currcolor  = 0;
unit cmpcolor   = 0;        /* ascii value of color, used to mark the 2 points to be connected */

int findaround (unit t1y, unit t1x, unit t2y, unit t2x);


/* debug and benchmarking */
unsigned int 
    calls       = 0,
    remapcalls  = 0,
    
    bfscalls    = 0,
    bfsfails    = 0,
    resetstamps = 0,
    resetdists  = 0,

    testblocks  = 0,
    blocksfound = 0,
    blocksgood  = 0,
    
    testrejects = 0,
    rejectsdone = 0,

    aroundcalls = 0,
    aroundfound = 0,
    ignoredrounds=0;




void finalize(void)
{
    long int tt = (clock() - t1) * (1000.0 / CLOCKS_PER_SEC);
    
    fprintf(stderr, "test blocks:\t%d\nblocks found:\t%d\nblocks adjs:\t%d\n", 
            testblocks - blocksgood, blocksfound, blocksgood);
    fprintf(stderr, "test reject:\t%d\nrejects done:\t%d\n", testrejects, rejectsdone);
    fprintf(stderr, "calls remaps:\t%d\ncalls bfs:\t%d\nbfs fails:\t%d\n",
            remapcalls - blocksgood, bfscalls, bfsfails); 
    fprintf(stderr, "reset stamps:\t%d\nreset dists:\t%d\n", resetstamps, resetdists);
    fprintf(stderr, "calls to go around: %d, found: %d, ignored: %d\n",
        aroundcalls, aroundfound, ignoredrounds );
    
    fprintf(stderr, "total chamadas:\t%d\n", calls);
    fprintf(stderr, "total tempo: %ldm%2ld.%03lds\n",tt/60000,tt/1000,tt%1000);
        
/*  bottlenecks info: 
    printf ("bnhits: %d, bndetects %d, bn's not used %d\n", bnhits, bndetects, bn_not_used);
    printf("det1: %d, det2: %d. invalids: %d\n", detects1, detects2, invalidbfs);
*/
    exit(0);
}



int remap (int y, int x)
{
    point *tt;
    remapcalls++;
    
    trmap = trcc->map;
    tt = &trmap[y][x];
        
    /* IF close to destiny, check for simple rejections */
    if (tt->d == DISTMAX )
    {
        testblocks++;
        
        if (trmap[trcc->y2][trcc->x2].x != x ||
            trmap[trcc->y2][trcc->x2].y != y )
        {   /* point is an adjacency not in the path.
            these have always v == MAPDEST */
            blocksgood++;
            return 1;
        }
        
        tt->v = MAPWALL;
        
        /* trivial rejection */
        if (*trcc->adj[0] == MAPWALL &&
            *trcc->adj[1] == MAPWALL &&
            *trcc->adj[2] == MAPWALL &&
            *trcc->adj[3] == MAPWALL )
        {
            tt->v = MAPDEST;
            blocksfound++;
            return 0;
        }
        
        tt->v = MAPDEST;
    }   
        
    /* test area around destiny for rejection */
    if ( tt->d )
        if ( bfsreject() )
            return 0;
        
    if (DEBUG) printboard();
    dprintf(outp,"\nREMAP color %d, at (%d, %d)\n", trcc - colors, y, x);
        
    tt->v = MAPWALL;
    
    // TODO remove this?
    if ( findaround (tt->y, tt->x, y, x ))  goto remapdone;
        
    /* traverses the path backwards from the cut to the start, and clears it */
    for (tt = &trmap[tt->y][tt->x]; tt->d != DISTMAX;
        tt->v = EMPTY, tt = &trmap[tt->y][tt->x]); 
        
    if ( !bfsfind() )
    {   /* traverses the path backwards from the cut to the start */
        for ( tt = &trmap[trmap[y][x].y][trmap[y][x].x];
            tt->d != DISTMAX; tt->v = MAPDEST, tt = &trmap[tt->y][tt->x]);
        trmap[y][x].v = MAPDEST;
        bfsfails++;
        testpocket(trcc-colors);
        return 0;
    }

    // bottlenecks
    if ( trcc->lastbnset && trcc->bnmap[y][x] ) 
    {   // bottleneck point
        int jj = trcc->ascii - '0';
        dprintf(outp,"bottleneck hit %d: %d %d\n", jj, y, x);
        if (DEBUG) printbnmap(jj);
        bnhits++;
        return 0;
    }

remapdone:
    testpocket(trcc-colors);
    
    /* the tested point is still not considered final - therefore, 
        either it's empty, or it's an adj to be kept */
    if (trmap[y][x].d == DISTMAX)
        trmap[y][x].v = MAPDEST;
    else
        trmap[y][x].v = EMPTY;
    return 1;
}



static point dummypoint = {4,5,5,5};

void preparecolors (void)
{
    point temps[4];
    int i, j, y, x, tx, ty, ry, rx;
    point *sp2, *sp1;
    
    /* the 1st color does not need a map and related stuff */
    colors[1].ascii = '1';
    cube [colors[1].y2] [colors[1].x2] = '1';
//  timestamp = STAMPSTART;

    for (trcc = colors+2; trcc != colorend; trcc++)
    {
        sp1 = trcc->stackbfs = (point*) malloc (STACKSIZE*sizeof(point));
        sp2 = trcc->stackrej = (point*) malloc (STACKSIZE*sizeof(point));

        trmap = trcc->map = (point**) malloc ((BS*2 +2)*sizeof(point*));
        for (i = 0; i <= BS*2+1; i++)
            trmap[i] = (point*) calloc ((BS*2 +2), sizeof(point));

        for (i = 0; i <= BS*2+1; i++)
            for (j = 0; j <= BS*2 +1; j++)
                if ( cube[i][j] )
                {   trmap[i][j].v = MAPWALL;
                    trmap[i][j].d = DISTOBST;
                }

        /* settings for the end point */
        y = trcc->y2;
        x = trcc->x2;
        trcc->ascii = trcc-colors + '0';
        cube[y][x] = trcc->ascii;
        trcc->distance = trcc->stamp = STAMPSTART;
        
        trmap[y][x].v = EMPTY;
        trmap[y][x].d = DISTMAX;
        
        transfsimple(y, x+1, y, x, ty, tx)
        transfsimple(y+1, x, y, x, ry, rx)
        
        /* temporary stacking to automatize
            the loop operations */
        temps[0].y = y;     temps[1].y = y-1;
        temps[2].y = ty;    temps[3].y = ry;
        temps[0].x = x-1;   temps[1].x = x;
        temps[2].x = tx;    temps[3].x = rx;
        
        for (i = 0; i < 4; i++)
        {   /* if this adj is not a wall..!! */
            if (trmap[temps[i].y][temps[i].x].v != MAPWALL)
            {
                /* mark adjacencies, value flags */
                trmap[temps[i].y][temps[i].x].v = MAPDEST;
                /* mark adjacencies, dist flags */
                trmap[temps[i].y][temps[i].x].d = DISTMAX;
                /* memorize pointers to adjs values */
                trcc->adj[i]  = &trmap[temps[i].y][temps[i].x].v;
                /* memorize pointers to adjs dists */
                trcc->adjd[i] = &trmap[temps[i].y][temps[i].x].d;
                
                /* preemptively push dest adjs points to the stack */
                sp2->y = temps[i].y;
                sp2->x = temps[i].x;
                (sp2++)->d = MAPDEST;
            }
            
            else
            {   /* dummies, for safety */
                trcc->adj[i]  = &dummypoint.v;
                trcc->adjd[i] = &dummypoint.d;
            }
        }
        
        trcc->sj4stackrej = sp2;

        /* settings for the start point */
        y = trcc->y1;
        x = trcc->x1;
        trmap[y][x].v = STAMPSTART;
        trmap[y][x].d = DISTMAX;
//      trmap[y][x].x = 0;
        trmap[y][x].y = 0;

        sp1->v = 0; sp1->y = y; sp1->x = x; 
        
        dprintf(outp, "\nCall to color %d:\n", trcc - colors);
        
        trmap[0][0].d = DISTMAX;
        bfsfind();
    }

    for (trcc = colors+2; trcc != colorend; trcc++)
    {   trmap = trcc->map;
        bfsreject();
    }
}



void choosecolor (void)
{
    int xt, yt, xr, yr;
    int xx, yy;

    if ( currcolor == ncolors )   /* last color has been connected */
    {   /* sucess ! */
        printresult();
        finalize();
    }

    ++currcolor; ++colort; ++colortinc;
    cmpcolor= colort->ascii;
    area    = colors[currcolor].area;

    xx = colort->x1;
    yy = colort->y1;

    if (DEBUG) printboard();
    dprintf(outp, "NEW COLOR: %d, (%d, %d)\n", currcolor, yy, xx);

    transfsimple(yy+1, xx, yy, xx, yt, xt)
    transfsimple(yy, xx+1, yy, xx, yr, xr)

    GOLEFT  (yy,xx)
    GORIGHT (yr,xr,yy,xx)
    GODOWN  (yy,xx)
    GOUP    (yt,xt,yy,xx)

    --currcolor; --colort; --colortinc;
    cmpcolor= colort->ascii;
    area    = colors[currcolor].area;
    
    // Bottlenecks
    trcc = colortinc;
    clearbottlenecks();
}





int main (int argc, char** argv)
{
    t1 = clock();
    outp  = stdout; // fopen("res.txt", "w");       /* out file */
    
    if (argc != 2)
        printf("Wrong number of arguments\n");

    colors  = (color*) calloc (STDMAX, sizeof(color)); 

    readboard(fopen(argv[1], "r")); /* in file */

    colors[0].x1 = colors[0].y1 = colors[0].y2 = colors[0].x2 = BS*2;

    colorend = colors+ncolors+1;
    colort = colors;
    colortinc = colors+1;

    initbottlenecks();
    
    printboard();
//  printareas();
//  printmaps();
    
    preparecolors();

    printcolors();
    
    dprintf(outp, "\nBEGIN\n\n");

    choosecolor();
    fprintf(stderr, "\nSolution not found!\n");

    return 0;
}


/*  
    // Not needed now, replaced 

    #define dist_source(yq,xq,res)              \
    {                                           \
        if ( trcc->x2 > BS && yq > BS )         \
            res = 2*BS+1 - trcc->y2 - xq + abs (yq - trcc->x2); \
    \
        else if ( trcc->y2 > BS && xq > BS )    \
            res = 2*BS+1 - trcc->x2 - yq + abs (xq - trcc->y2); \
    \
        else                                    \
            res = abs (yq - trcc->y2) + abs (trcc->x2 - xq);    \
    }

*/