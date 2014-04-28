
//------------------------------------------------------------------------------
// 
//  Finding a path around an obstacle
// 
//------------------------------------------------------------------------------

/*
    This module implements the simple operation of finding the path around a new obstacle point,
    if such path exists. It is meant to reconnect an already chosen/set, path; which was cut at one point.
    
    In the present state, the problem is that, when twisting around, the search does not look to the sides,
    and because of that, may generate circular paths. ex:
1 1 1 1 1 1
    1     1
    1 1 1 1
*/


#include "defs.h"
#include "io.h"



int findup      (int a, int b);
int findright   (int a, int b);
int findleft    (int a, int b);
int finddown    (int a, int b);

    
/* needed here, in theory, tho in practice it seems it is 
    almost always never reached */
static int dummy (int a, int b)
{
    fprintf (stderr, "reached dummy %d %d, %d\n", a, b, calls);
    return 0;
}

static int (*fps[8][5]) (int, int) = 
    {   { finddown, findright,  findright,  findup,     dummy },
        { findup,   findright,  findright,  finddown,   dummy },
                    
        { finddown, findleft,   findleft,   findup,     dummy },
        { findup,   findleft,   findleft,   finddown,   dummy },

        { findleft, findup,     findup,     findright,  dummy },
        { findright,findup,     findup,     findleft,   dummy },

        { findleft, finddown,   finddown,   findright,  dummy },
        { findright,finddown,   finddown,   findleft,   dummy } 
    };

int (**fp) (int, int);


static point *p1, *testp1, *origp1;
static int depth;

static int markpath (int ey, int ex, int yy, int xx)
{
    if (origp1 != p1 && testp1 != p1) 
    {   /* went back in the path, so it involves erasing some path */
        ignoredrounds++;
        return 0;
    }

    /* can never happend! */
    if ( depth == 0 && trmap[ey][ex].d != DISTMAX ) 
    {
        ignoredrounds++;
        fprintf(stderr, "Ignored around depth == %d\n", depth);
    }
    
    if((trmap[yy+1][xx].v == MAPDEST && &trmap[yy+1][xx] != p1 && (yy+1 != ey || xx != ex))
    || (trmap[yy-1][xx].v == MAPDEST && &trmap[yy-1][xx] != p1 && (yy-1 != ey || xx != ex))
    || (trmap[yy][xx+1].v == MAPDEST && &trmap[yy][xx+1] != p1 && (xx+1 != ex || yy != ey))
    || (trmap[yy][xx-1].v == MAPDEST && &trmap[yy][xx-1] != p1 && (xx-1 != ex || yy != ey)))
    {   /* involves remapping the adjacencies, ignored */
        ignoredrounds++;
        dprintf(outp, "Ignored around mapdest\n");
        return 0;
    }   
        
    trmap[ey][ex].x = xx;
    trmap[ey][ex].y = yy;

    if ( trmap[ey][ex].d == DISTMAX ) 
    {
        dprintf(outp, "found result adjacent\n");
        testp1 = p1 = &trmap[trcc->y2][trcc->x2];
        p1 = &trmap[p1->y][p1->x];

        if ( p1->v != MAPWALL && p1 !=  &trmap[ey][ex] )
        {
            dprintf(outp, "clear previous path from adjacent, start at (%d, %d)\n", 
                    p1->y, p1->x);
            for (   ; trmap[p1->y][p1->x].v != MAPWALL;
                    p1 = &trmap[p1->y][p1->x], p1->v = EMPTY );

            dprintf(outp, "cleared path\n");
        }
        
        testp1->x = ex;
        testp1->y = ey;
    }

    return 1;
}



#define FINDBRANCH(st)                              \
    dprintf(outp, "find %s at (%d, %d)\n",          \
            st, ey,ex);                             \
                                                    \
    if ( trmap[ey][ex].v == MAPWALL )               \
    {   dprintf(outp, "fail at find %s (%d, %d)\n", \
                st, yy,xx);                         \
        return 0;                                   \
    }                                               \
                                                    \
    if ( trmap[ey][ex].v == MAPDEST )               \
    {   if ( p1->y != ey || p1->x != ex )           \
            return markpath(ey,ex,yy,xx);           \
        else /* it's the previous mapdest point */  \
            return 0;                               \
        /*  p1 = &trmap[p1->y][p1->x];  */          \
    }                                               \
                                                    \
    if ( fp[++depth] (ey, ex) )                     \
    {                                               \
        if (origp1->y == ey && origp1->x == ex)     \
            trmap[yy][xx].v = EMPTY;                \
        else                                        \
        {   trmap[ey][ex].v = MAPDEST;              \
            trmap[ey][ex].x = xx;                   \
            trmap[ey][ex].y = yy;                   \
        }                                           \
        return 1;                                   \
    }                                               \
    return 0;


#define transfcut(yaa,xaa,ygg,xgg)      \
    xgg = xaa;                          \
    if ((ygg = yaa) > BS && xgg > BS)   \
    /* too inefficient to consider */   \
    {   ignoredrounds++;    \
        return 0;                       \
    }


int findup (int yy, int xx)
{
    int ey, ex;
    transfcut(yy+1, xx, ey, ex)
    FINDBRANCH("up")
}

int finddown (int yy, int xx)
{
    int ey, ex;
    transfcut(yy-1, xx, ey, ex)
    FINDBRANCH("down")
}

int findright (int yy, int xx)
{
    int ey, ex;
    transfcut(yy, xx+1, ey, ex)
    FINDBRANCH("right")
}

int findleft (int yy, int xx)
{
    int ey, ex;
    transfcut(yy, xx-1, ey, ex)
    FINDBRANCH("left")
}



int findaround (unit t1y, unit t1x, unit t2y, unit t2x)
{
    p1 = &trmap[t1y][t1x];
    testp1 = &trmap[p1->y][p1->x];

    aroundcalls++;
    
    if (DEBUG) {
        dprintf(outp, "\nsimple go around: (%d, %d) => (%d, %d)\n", 
                t1y, t1x, t2y, t2x);
        printmap(trcc-colors);
        printpath(trcc-colors);
    }

    if ( t1y == t2y)    /* horizontal move */
    {   
        if  (t1x < t2x) fp = *fps;
        else            fp = fps[2];
    }
    else                /* vertical move */
    {   if(t1y < t2y)       fp = fps[4];
        else if(t1x == t2x) fp = fps[6];
        /* tru the rift */
        else                fp = *fps;
    }

    depth = 0;
    origp1 = p1;
    if ( fp[0] (t1y, t1x) ) 
        return 1;

    depth = 0;
    origp1 = p1 = &trmap[t1y][t1x];
    dprintf(outp,"try the other side\n");
    int ret = (fp += 5)[0] (t1y, t1x);
    
    if (DEBUG) printmap(trcc-colors);
    if (DEBUG) printpath(trcc-colors);
    return ret;
}


