
// ============================================================================
// 
//  Detection of Bottlenecks
// 
// ============================================================================

#include "defs.h"


extern void reset_timestamps(void);

extern void printmap(int);

extern int timestamp;

/*
    values of pathtab[i].v :
        1 - bottleneck found, but has possible free points reachable around
        2 - same found, but with not traversable points around
*/

#define MARKPOINT           \
{   MARKPREV                \
    MARKTURN                \
    MARKAHEAD               \
}

/* complete blocked point */
#define MARKTURN            \
{   if ( !pathtab[pn].v )   \
        ++npoints;          \
    pathtab[pn].v = 2;      \
}

#define MARKAHEAD           \
    pathtab[pn+1].v = 1;    \
    ++npoints;  

#define MARKPREV            \
    if ( !pathtab[pn-1].v ) \
    {   pathtab[pn-1].v = 1;\
        ++npoints;          \
    }

#define MARKCURVE           \
    if ( !pathtab[pn].v )   \
    {   ++npoints;          \
        pathtab[pn].v = 1;  \
    }


#define SIZEOFLINE  16*sizeof(point)
#define BNSETSTART 2

point* pathtab;
bool** tnmap;

FILE *nullp;
//int addsh[2][2];
//int addsv[2][2];

typedef struct _incr
{   int y;
    int x;
} incr;


int bndetects   = 0,
    invalidbfs  = 0,
    bn_not_used = 0,
    detects1    = 0,
    bnhits      = 0,
    detects2    = 0;

unsigned int ncall =  BNSETSTART+1;

int bfsbn (int y, int x);

#define CHANGE_ADDS \
{   if ( adds == addsh)     \
        adds = addsv;   \
    else                \
        adds = addsh;   \
}


void printbnmap (int k)
{
    bool** cub;
    int i, j;

    if (CUTOUTPUT)
        return;

    cub = colors[k].bnmap;
    for (i = BS*2; i > BS; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS; j++)
            fprintf(outp, "%2d ", cub[i][j] );
    

    for (   ; i > 0; i--, fprintf(outp, "\n"))
        for (j = 1; j <= BS*2; j++)
            fprintf(outp, "%2d ", cub[i][j] );

        fprintf(outp, "Ncall %d\n", colors[k].lastbnset);
}


void clearbottlenecks(void)
{
    tnmap = trcc->bnmap;
    for (pathtab = trcc->bnpath+1; pathtab->x;
        tnmap[pathtab->y][pathtab->x] = 0, pathtab++);

/*  absolutely necessary to set at 0 the values of .v, the flags of bottleneck points
    which will be set ahead */
    memset(trcc->bnpath, 0, 16* sizeof(point));
    
    for (pathtab = trcc->bnpath+16;
        pathtab->x; memset(pathtab, 0, sizeof(point)), pathtab++);
}


void detectbottlenecks(void)
{
    point *tt, *tp;
    int b, a;
    int zy, zx;
    int inc, ins;
    int minbn, pn = 2, npoints = 0;
    incr addsh[3] = {{1,0}, {-1,0}, {0,-1}} ;
    incr addsv[3] = {{0,1}, {0,-1}, {-1,0}} ;
    incr *adds;

    int jj = trcc->ascii - '0';
    if (DEBUG)
    {
        fprintf (outp, "Call to detect bottlenecks, color %c\n", trcc->ascii);
        printmap(jj);
        printbnmap(jj);
    }
    
    trcc->lastbnset = ncall;    
    bndetects++;
    trmap = trcc->map;
    trmap[trcc->y1][trcc->x1].d = 0;
    clearbottlenecks();
    pathtab = trcc->bnpath;
    
    tp = &trmap[trcc->y2][trcc->x2];
    tt = &trmap[tp->y][tp->x];  /* adjacency */

    pathtab[pn].y = b = tt->y;
    pathtab[pn].x = a = tt->x;
    tnmap[b][a] = pn;

    tnmap[tp->y][tp->x] = 1;
    pathtab[1].y = tp->y;
    pathtab[1].x = tp->x;
    
    if ( tp->y == b )       /* horizontal path */
        adds = addsh;
    else
        adds = addsv;

    for ( tp = &trmap[b][a]; tp->x; tp = &trmap[b][a] )
    {
        int ex, ey, ry, rx;

        transfsimple (b+adds[0].y, a+adds[0].x, b, a, ey, ex)

#if 0
        if ( adds == addsh)
            fprintf(outp,"horiz test point (%d,%d)\n", b, a);
        else
            fprintf(outp,"vert test point (%d,%d)\n", b, a);
#endif      
        /* turn */
        inc = 0;
        if( trmap[b+adds[1].y][a+adds[1].x].v == MAPDEST &&
            trmap[b+adds[1].y][a+adds[1].x].d != DISTMAX )
            inc = -1;
            
        else
        if( trmap[ey][ex].v == MAPDEST &&
            trmap[ey][ex].d != DISTMAX )
            inc = 1;
            
        if ( inc )
        {
            if (trmap[b+adds[2].y][a+adds[2].x].v == MAPDEST )
                ins = 1;
            else
                ins = -1;
                
            if (adds == addsv)
            {   minbn = inc;
                inc = -ins;
                ins = -minbn;
            }
            
            transfcomplex (b+inc, a-ins, b, a, ey, ex)
            
            if (trmap[ey][ex].v == MAPWALL)
            {
                if ( adds == addsh )
                {   transfsimple (b, a+ins, b, a, ry, rx)
                    transfsimple (b-inc, a, b, a, zy, zx)
                }
                else
                {   transfsimple (b-inc, a, b, a, ry, rx)
                    transfsimple (b, a+ins, b, a, zy, zx)
                }
                
                transfcomplex (b-inc, a+ins, b, a, ey, ex) 
                
                if (trmap[zy][zx].v == MAPWALL)
                {   MARKPREV    
                    if (trmap[ry][rx].v == MAPWALL)
                    {   MARKTURN
                        MARKAHEAD
                    }
                    else    
                        MARKCURVE
                }
                else if (trmap[ry][rx].v == MAPWALL)
                {   MARKCURVE
                    MARKAHEAD
                }
                else if (trmap[ey][ex].v == MAPWALL)
                    MARKCURVE
            }
            CHANGE_ADDS
            goto testrift;
        }
            
        if( trmap[ey][ex].v == MAPWALL 
        &&  trmap[b+adds[1].y][a+adds[1].x].v == MAPWALL )
            MARKPOINT

testrift:tt = tp;
        if ( b != tt->y && a != tt->x )     /* change of quadrant */
            CHANGE_ADDS

        pn++;
        pathtab[pn].y = b = tt->y;
        pathtab[pn].x = a = tt->x;
        tnmap[b][a] = pn;
    }

    trmap[trcc->y1][trcc->x1].d = DISTMAX;
    pathtab[pn+1].x = 0;
        
    if ( pathtab[1].v != 0)
    {   --npoints;
/*      pathtab[1].v = 0;   can be removed !? yes, a BN adj. point is considered. 
        The npoints is decr. because it's not needed to test if this BN is unique */
    }

    if ( !npoints )
    {   
        fprintf(outp, "no bottlenecks\n\n");
        for (pathtab = trcc->bnpath+1; pathtab->x;
            tnmap[pathtab->y][pathtab->x] = 0, pathtab++);
        return;         /* no bottleneck points */
    }

    if ( ++trcc->stamp == DISTMAX )
        reset_timestamps();
    timestamp = trcc->stamp;

    /* pn has currrently P1 */
    for ( ; npoints; pn-- )
    {
        if ( pathtab[pn].v )
            --npoints;
        
        if  ( pathtab[pn].v != 2    && DISTMAX !=
            (minbn = bfsbn(pathtab[pn].y, pathtab[pn].x)))
                for ( a = pn-1; a > minbn; a--)
                    if ( pathtab[a].v )
                    {
                        fprintf(outp,"ERASED bottleneck: %d - (%d,%d)\n", 
                            a, pathtab[a].y, pathtab[a].x);
                        pathtab[a].v = 0;
                        --npoints;
                    }
    }

    for ( pathtab++; pathtab->x ; pathtab++ )
        if ( !pathtab->v )
            tnmap[pathtab->y][pathtab->x] = 0;

    /* not needed, but put here by correctness of dominion */
    if (tnmap[trcc->y1][trcc->x1])
        tnmap[trcc->y1][trcc->x1] = 0;
            
    if (DEBUG)
    {   printmap(jj);
        fprintf(outp, "bottleneck points: ");

        pathtab = trcc->bnpath;
        for ( pn = 1; pathtab[pn].x != trcc->x1 || pathtab[pn].y != trcc->y1; pn++)
            if ( pathtab[pn].v )
                fprintf (outp, "(%d,%d) ", pathtab[pn].y, pathtab[pn].x);
        fprintf(outp, "\n");
        printbnmap(jj);
    }
}


point *stackbn;


#ifdef  TESTBFS
#undef  TESTBFS
#endif

#define TESTBFS(b,a)                    \
    if (trmap[b][a].v < timestamp)      \
    {                                   \
        if (trmap[b][a].v == MAPDEST)   \
        {   if (tnmap[b][a] < min)      \
                min = tnmap[b][a];      \
        }                               \
        else                            \
        {   trmap[b][a].v = timestamp;  \
            sjf->y = b;                 \
            sjf->x = a;                 \
            sjf++;                      \
        }                               \
    }

int bfsbn (int y, int x)
{
    point *sp, *sjf;
    int min = DISTMAX;
    
#if 0
    printf("in bfsbn\n");
    fprintf(outp,"call to bfs bottlenecks: (%d,%d)\n", y, x);
    printmap (trcc - colors);
#endif

    sp = stackbn;
    sp->x = x;
    sp->y = y;

    for (sjf = sp+1; sp != sjf; sp++ )
    {
        int ex, ey, ry, rx;

        ey = sp->y-1; ex = sp->x;
        TESTBFS(ey,ex)
        ey++; ex--;
        TESTBFS(ey,ex)
        ex++;

        transfsimple(ey+1, ex, sp->y, sp->x, ey, ex)
        TESTBFS(ey,ex)
        transfsimple(sp->y, sp->x +1, sp->y, sp->x, ey, ex)
        TESTBFS(ey,ex)
    }

#if 0
    printf("out bfsbn\n");
    fprintf(outp,"bfs bottlenecks, min found: %d\n", min);
#endif

    return min;
}




void initbottlenecks()
{
    pathtab = calloc (STACKSIZE, sizeof(point));
    pathtab[0].v = 1;
    stackbn = calloc (STACKSIZE, sizeof(point));

    for (trcc = colors+2; trcc != colorend; trcc++)
    {
        int i;
        trcc->lastbnset = BNSETSTART;
        trcc->bnmap = (bool**) malloc ((BS*2 +2)*sizeof(bool*));
        for (i = 0; i <= BS*2+1; i++)
            trcc->bnmap[i] = calloc ((BS*2 +2), sizeof(bool));

//      trcc->bnmap[0][0] = 1;   special case overlimit flag 
        
        trcc->bnpath = (point*) calloc ((BS*2)*(BS*2) ,sizeof(point));
    }
}



/*  
    code to be included in the various functions 
    
    end of choosecolor:
        trcc = colortinc;
        bnclears++;
        clearbottlenecks();
        

    remap function:

        if ( trcc->lastbnset && trcc->bnmap[y][x] ) 
        {   // bottleneck point
            dprintf(outp,"bottleneck hit %d: %d %d\n", jj, y, x);
            if (DEBUG) printbnmap(jj);
            bnhits++;
            return 0;
        }

            
    propagation:

            if ( trcc->lastbnset == ncall)              \
            {   fprintf(outp, "invalidate %d\n", jj);   \
                invalidbfs++;           \
                trcc->lastbnset = 0;                    \
            }                                           \


*/