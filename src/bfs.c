
// ============================================================================
// 
//  Best-first search
// 
// ============================================================================


#include "defs.h"
#include "io.h"

int timestamp;

void reset_timestamps(void)
{
    int j, k;
    point *tt;
    trcc->stamp = STAMPSTART;
    
    resetstamps++;
    if (DEBUG)printmap(trcc-colors);
    dprintf(outp, "RESET TIMESTAMPS\n");

    for (k = BS*2; k > BS; k--)
        for (j = 1; j <= BS; j++)
            if (trmap[k][j].v != MAPWALL)
                trmap[k][j].v = EMPTY;

    for (   ; k > 0; k--)
        for (j = 1; j <= BS*2; j++)
            if (trmap[k][j].v != MAPWALL)
                trmap[k][j].v = EMPTY;

    tt = &trmap[trcc->y2][trcc->x2];
    tt = &trmap[tt->y][tt->x];
    
    if (tt->v != MAPWALL)
        for ( tt = &trmap[tt->y][tt->x]; tt->v != MAPWALL;
            tt->v = MAPDEST,  tt = &trmap[tt->y][tt->x] );
    
    if ( ! *trcc->adj[0] ) *trcc->adj[0] = MAPDEST;
    if ( ! *trcc->adj[1] ) *trcc->adj[1] = MAPDEST;
    if ( ! *trcc->adj[2] ) *trcc->adj[2] = MAPDEST;
    if ( ! *trcc->adj[3] ) *trcc->adj[3] = MAPDEST;

//  rmap[trcc->y1][trcc->x1].v = trcc->stamp;   /* prolly not needed */

    if (DEBUG) printmap(trcc-colors);
}



static void reset_distances (void)
{
    int j, k;
    trcc->distance = DISTANCESTART+1;
    
    if (DEBUG)printmap(trcc-colors);
        dprintf(outp, "RESET DISTANCE STAMPS\n");
    resetdists++;

    for (k = BS*2; k > BS; k--)
        for (j = 1; j <= BS; j++)
            if ( trmap[k][j].d )
                trmap[k][j].d = DISTANCESTART;

    for (   ; k > 0; k--)
        for (j = 1; j <= BS*2; j++)
            if ( trmap[k][j].d )
                trmap[k][j].d = DISTANCESTART;

    *trcc->adjd[0] = DISTMAX;
    *trcc->adjd[1] = DISTMAX;
    *trcc->adjd[2] = DISTMAX;
    *trcc->adjd[3] = DISTMAX;
    
    trmap[trcc->y1][trcc->x1].d = DISTMAX;
    trmap[trcc->y2][trcc->x2].d = DISTMAX;
}




#define GETCOLINEAR(curry,currx,prevy,prevx,coly,colx)\
    colx = currx;                           \
    coly = curry;                           \
                                            \
    if (prevx == colx)                      \
    {   if (prevy > coly)                   \
            coly--;                         \
        else if (colx <= BS || coly+1 <= BS)\
            coly++;                         \
        else                                \
        {   colx = curry;                   \
            coly = currx;                   \
        }                                   \
    }                                       \
                                            \
    else if (prevx > colx)                  \
        colx--;                             \
    else if ( colx+1 <= BS || coly <= BS )  \
        colx++;                             \
    else                                    \
    {   colx = curry;                       \
        coly = currx;                       \
    }


#define TESTBFS(b,a)                        \
    /* timestamp is always < MAPWALL */     \
    if ( trmap[b][a].v < timestamp )        \
    {                                       \
        if ( trmap[b][a].v == MAPDEST )     \
        {        if (!res1) res1 = spf;     \
            else if (!res2) res2 = spf;     \
            else            res3 = spf;     \
        }                                   \
        else                                \
            trmap[b][a].v = timestamp;      \
        spf->y = b;                         \
        spf->x = a;                         \
        /* link to the previous point */    \
        spf->v = sp - stackbfs;             \
        spf++;                              \
    }



/*  the (y,x) coords of the stacked points have the 
    actual coords of the point */

int bfsfind(void)
{
    int ex, ey, ry, rx;
    point   *res1 = NULL,
            *res2 = NULL,
            *res3 = NULL;
    point *sp5, *tt, *p2;
    point *stackbfs;
    point *sp, *spf;
        
    bfscalls++;
    if (DEBUG)
    {   dprintf(outp,"call to bfs at (0,0). stamp = %d\n", trcc->stamp);
        printmap (trcc - colors);
        printpath(trcc - colors);
    }
    
    if ( ++trcc->stamp == STAMPMAX )
        reset_timestamps();
    trmap[trcc->y1][trcc->x1].v = timestamp = trcc->stamp;
    stackbfs = trcc->stackbfs;
    
    for (sp = stackbfs, spf = sp+1;
        !res1 && sp != spf; sp++ )
    {
        ey = sp->y-1; ex = sp->x;
        TESTBFS(ey, ex)
        ey++; ex--;
        TESTBFS(ey, ex)
        
        transfsimple(sp->y + 1, sp->x, sp->y, sp->x, ey, ex)
        TESTBFS(ey,ex)
        transfsimple(sp->y, sp->x +1, sp->y, sp->x, ey, ex)
        TESTBFS(ey,ex)
    }
    
    if ( sp == spf )
    {
        dprintf(outp,"BFS: not found connection\n");
        return 0;
    }

    dprintf(outp,"call to bfs at (0,0). stamp = %d\n", trcc->stamp);
    if (DEBUG) {
        printmap (trcc - colors);
        printpath(trcc - colors);
    }

#if 0
#define outp stdin
#endif

    if (DEBUG) {
        dprintf(outp, "bfs pockets: (%d,%d)", res1->y, res1->x);
        if (res2)
        {   dprintf(outp, ", (%d,%d)", res2->y, res2->x);
            if (res3)   dprintf(outp, ", (%d,%d)", res3->y, res3->x);
        }
        dprintf(outp, "\n");
    }

    /*********** testing pockets ***********/

    if (res3)
    {
        point *cmp1 = &trmap[res1->y][res1->x];
        point *cmp2 = &trmap[res2->y][res2->x];
        point *cmp3 = &trmap[res3->y][res3->x];

             if (cmp3->d == DISTMAX)
            spf = res3;
        else if (cmp2->d == DISTMAX)
            spf = res2;
        else if (cmp1->d == DISTMAX)
            spf = res1;
            
        else
        {   point *aux;
            point *cmp1 = &trmap[res1->y][res1->x];
            point *cmp2 = &trmap[res2->y][res2->x];
            point *cmp3 = &trmap[res3->y][res3->x];

            for(aux = &trmap[cmp3->y][cmp3->x];
                aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
            {   
                if (aux == cmp1)
                {   cmp1 = NULL;
                    if (!cmp2)  break;
                }
                
                else if (aux == cmp2)
                {   cmp2 = NULL;
                    if (!cmp1)  break;
                }
            }
            
            if (!cmp1)
            {   if (!cmp2)  /* both == 0 */
                {   spf = res3;
                    ignoredrounds++;
                }
                else
                    spf = res2;
            }
            
            else if (!cmp2)
                spf = res1;
            
            else    /* 1 e 2 > 3 */
            {   spf = res1;
                for(aux = &trmap[cmp2->y][cmp2->x];
                    aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
                    if (aux == cmp1)
                    {   spf = res2;
                        break;
                    }
            }
            dprintf(outp,"3 mapdests: (%d,%d)\n", spf->y, spf->x);
        }
    }
    
    else if (res2)
    {   /* 2 mapdest points found */
        point *aux = &trmap[res1->y][res1->x];
        point *cmp = &trmap[res2->y][res2->x];
        
        if (aux->d == DISTMAX)
            spf = res1;
        else if (cmp->d == DISTMAX)
            spf = res2;
        else
        {   
            for(aux = &trmap[aux->y][aux->x], spf = res2;
                aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
                if (aux == cmp)
                {   spf = res1;
                    break;
                }
                    
            dprintf(outp,"2 mapdests: (%d,%d)\n", spf->y, spf->x);
        }
    }
    
    else 
    {   /* normal case, only 1 mapdest point found */
        spf = res1; 
        dprintf (outp,"1 mapdests: (%d,%d)\n", spf->y, spf->x);
    }

    ex = spf->x;
    ey = spf->y;
    tt  = &trmap[ey][ex];
    p2  = &trmap[trcc->y2][trcc->x2];
    sp5 = &trmap[p2->y][p2->x]; /* auxiliar */
    
    if (tt->d == DISTMAX)   /* adjacency point, all the old path trashed */
    {
        dprintf (outp,"ajdcen point: (%d,%d)\n", spf->y, spf->x);   
        tt = sp5;
        p2->x = ex;
        p2->y = ey;
    }
    
    /* reconnecting tru the first after adjacency, replacing adjacency */
    else if(tt == &trmap[sp5->y][sp5->x] 
        &&  sp->y != p2->y && sp->x != p2->x )
    {
        GETCOLINEAR(ey,ex, sp->y,sp->x, ry,rx)
        dprintf (outp,"test reconnect: p2 - (%d,%d), rs - (%d,%d)\n", 
            p2->y, p2->x, ry, rx);
            
        if (trmap[ry][rx].v == MAPDEST 
            && trmap[ry][rx].d == DISTMAX)
        {
            p2->y = ry;
            p2->x = rx;
            trmap[ry][rx].y = ey;
            trmap[ry][rx].x = ex;
            dprintf (outp,"reconnect done\n");
        }
    }

    /* traverses the path backwards from the result found till the cut,
        to clear the global map and values of MAPDEST */
    for ( tt = &trmap[tt->y][tt->x]; 
    tt->v == MAPDEST;
        tt->v = EMPTY,              /* clears MAPDESTs */
            tt = &trmap[tt->y][tt->x] );

    /* traverses the path backwards from the result till the start to set this chosen part */
    while (spf != stackbfs )
    {   
        tt = & trmap[spf->y][spf->x];
        spf = stackbfs + spf->v;
        tt->v = MAPDEST;
        tt->x = spf->x;
        tt->y = spf->y;
    }   
    
    trmap[spf->y][spf->x].v = MAPDEST;  /* set start point */
    
#ifdef  outp
#undef  outp
#endif
    
    if (DEBUG) {
        printmap(trcc - colors);
        dprintf(outp, "changed path\n"); 
        printpath(trcc - colors);
    }
    
    return 1;
}




/* the cube position has to be tested because the choices
set in the mainboard are not passed onto the dist. maps */

#define TEST_BFS_REJECT(b,a)        \
    if ( cube[b][a] == EMPTY &&     \
        trmap[b][a].d < timestamp)  \
    {                               \
        if ( (sjf->d = sp->d+1) >   \
            DEPTHREJECT )           \
                return 0;           \
        trmap[b][a].d = timestamp;  \
        sjf->y = b;                 \
        sjf->x = a;                 \
        sjf++;                      \
    }

    
int bfsreject (void)
{
    point *sjf, *sj4; 
    point *sp;
    int ex, ey;
    
    dprintf(outp,"call to rejectbfs. color: %c \n", trcc->ascii);
    testrejects++;

    if ( ++trcc->distance == DISTOBST )
        reset_distances();
    timestamp = trcc->distance;
    sp = trcc->stackrej;

    for (sjf = sj4 = trcc->sj4stackrej; sp != sj4; sp++ )
        if ( cube[sp->y][sp->x] == EMPTY )
        {
            ey = sp->y-1; ex = sp->x;
            TEST_BFS_REJECT(ey,ex)
            ey++; ex--;
            TEST_BFS_REJECT(ey,ex)
            
            transfsimple(ey+1, ex+1, sp->y, sp->x, ey, ex)
            TEST_BFS_REJECT(ey,ex)
            transfsimple(sp->y, sp->x +1, sp->y, sp->x, ey, ex)
            TEST_BFS_REJECT(ey,ex)
        }
    
    for (   ; sp != sjf; sp++ )
    {
        ey = sp->y-1; ex = sp->x;
        TEST_BFS_REJECT(ey,ex)
        ey++; ex--;
        TEST_BFS_REJECT(ey,ex)
        
        transfsimple(ey+1, ex+1, sp->y, sp->x, ey, ex)
        TEST_BFS_REJECT(ey,ex)
        transfsimple(sp->y, sp->x +1, sp->y, sp->x, ey, ex)
        TEST_BFS_REJECT(ey,ex)
    }

    dprintf(outp,"trivially rejected\n");
//  printdistmap (trcc - colors);
    rejectsdone++;

    return 1;
}



