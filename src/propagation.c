//------------------------------------------------------------------------------
// 
//  Constraint propagation
// 
//------------------------------------------------------------------------------s

#include "defs.h"
#include "propagation.h"
#include "io.h"
#include "bottlenecks.h"

extern int  remap (int y, int x);
extern void choosecolor(void);


#define CHECKCALL(st)                   \
    ++calls;                            \
    dprintf(outp, "Call %d choose %s at (%d, %d) - %d\n", \
        calls, st, yy, xx, currcolor);


#define PUTBACK(direction,rxyflag,y,x)              \
    end ## direction:                               \
    dprintf(outp, "return %s at (%d, %d) - %d\n",   \
        #direction, yy, xx, 0);                     \
                                                    \
    cube[yy][xx] = 0;                               \
    for (trcc = colortinc; trcc != colorend; trcc++)\
    {                                               \
        if (trcc->map[yy][xx].d == DISTMAX)         \
        {                                           \
            point *found[3] = {NULL,NULL,NULL};     \
            int i = 0;                              \
            trmap = trcc->map;                      \
            trmap[yy][xx].v = MAPDEST;              \
                                                    \
            if (rxyflag) {                          \
                transfsimple(y,x, yy,xx, yr,xr)     \
            }                                       \
                                                    \
            if (trmap[yy-1][xx].v == MAPDEST &&     \
                trmap[yy-1][xx].d != DISTMAX )      \
                    found[i++] = &trmap[yy-1][xx];  \
            if (trmap[yy][xx-1].v == MAPDEST &&     \
                trmap[yy][xx-1].d != DISTMAX )      \
                    found[i++] = &trmap[yy][xx-1];  \
            if (trmap[yt][xt].v == MAPDEST &&       \
                trmap[yt][xt].d != DISTMAX )        \
                    found[i++] = &trmap[yt][xt];    \
            if (trmap[yr][xr].v == MAPDEST &&       \
                trmap[yr][xr].d != DISTMAX )        \
                    found[i++] = &trmap[yr][xr];    \
                                                    \
            if (STRAIGHTEN_ALL_PATHS && i > 0)      \
                reconnect(yy,xx,found[0],found[1],found[2]);\
            else if (i > 1)                         \
                reconnect(yy,xx,found[0],found[1],found[2]);\
                                                    \
            dprintf(outp, "putback mapdest\n");     \
        }                                           \
        else                                        \
            trcc->map[yy][xx].v = EMPTY;            \
    }



void reconnect(int y, int x, point *res1, point *res2, point *res3)
{
    point *rec, *aux;
    if (DEBUG && (y != 2 && x != 8) && (y != 3 && x != 9) && (y != 5 && x != 5))
    {   printf( "Reconnect (%d,%d)\n", y, x);
        printmap    (trcc-colors);
        printpath(trcc-colors);
    }
    
    if (res3)
    {
        for(aux = &trmap[res3->y][res3->x], rec = res3;
            aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
        {   
            if (aux == res1)    break;
            if (aux == res2)    break;
        }
        
        if (aux->v != MAPDEST)
            for(aux = &trmap[res2->y][res2->x], rec = res2;
                aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
                    if (aux == res1)
                    {   rec = res1;
                        break;
                    }
    }
    
    else if (res2)
    {   /* 2 mapdest points found */
        rec = res1;
                    
        for(aux = &trmap[res1->y][res1->x];
            aux->v == MAPDEST; aux = &trmap[aux->y][aux->x])
            if (aux == res2)
            {
                rec = res2;
                break;
            }
    }
        
    else /* res1 only */
        rec = res1;
    
    /* clears mapdests erased */
    aux = &trmap[trcc->y2][trcc->x2];   /* aux = p2 */
    aux = &trmap[aux->y][aux->x];       /* aux = adj */
    for(aux = &trmap[aux->y][aux->x]; aux != rec; 
        aux->v = EMPTY, aux= &trmap[aux->y][aux->x]);

    /* reconnecting: creating new path from p2 to rec */
    trmap[trcc->y2][trcc->x2].y = y;
    trmap[trcc->y2][trcc->x2].x = x;
                    /* coords of rec */
    int offset = (void*)trmap[1] - (void*)trmap[0];
    trmap[y][x].y = ((void*)rec - (void*)trmap[0]) / offset;
    trmap[y][x].x = (((void*)rec - (void*)trmap[0]) % offset) / sizeof(point);
    
    if (DEBUG && (y != 2 && x != 8) && (y != 3 && x != 9) && (y != 5 && x != 5))
    {   printf("link: curr (%d,%d), rec (%d,%d), next (%d,%d)\n", 
            y, x, trmap[y][x].y, trmap[y][x].x, rec->y, rec->x);
        printmap (trcc-colors);
        printpath(trcc-colors);
    }
}


#define CHOOSEBRANCH(ay,ax,by,bx,cy,cx,direction)   \
    CHECKCALL(#direction)                           \
                                                    \
    if (cube[ay][ax] == currcolor ||                \
        cube[by][bx] == currcolor ||                \
        cube[cy][cx] == currcolor)                  \
        return;                                     \
                                                    \
    cube[yy][xx] = currcolor;                       \
    for (trcc = colortinc; trcc != colorend; trcc++)\
        if (trcc->map[yy][xx].v == MAPDEST)         \
            if (!remap(yy,xx))                      \
            {   cube[yy][xx] = 0;                   \
                return;                             \
            }                                       \
                                                    \
    /* mark this posit as used in all the colors' maps */   \
    for (trcc = colortinc; trcc != colorend; trcc++)\
    {   /* can never have a path, coz that's been already tested */ \
        trcc->map[yy][xx].v = MAPWALL;              \
                                                    \
        if (BOTTLENECKS)                            \
            if (trcc->lastbnset == ncall)           \
            {   fprintf(outp, "invalidate %d\n",    \
                            trcc->ascii - '0');     \
                invalidbfs++;                       \
                trcc->lastbnset = 0;                \
                detectbottlenecks();                \
            }                                       \
    }                                               \
                                                    \
    if ( area[yy][xx] )                             \
    {   /* found end point */                       \
        choosecolor();                              \
        goto end ## direction;                      \
    }



void choosel (int yy, int xx)
{
    int xt, yt, yr, xr;
    transfsimple(yy+1, xx, yy, xx, yt, xt)

    CHOOSEBRANCH(yy, xx-1, yt, xt, yy-1, xx, left)
    
    GOLEFT  (yy,xx)
    GODOWN  (yy,xx)
    GOUP    (yt, xt, yy,xx)

    PUTBACK (left, 1, yy, xx+1)
}


void chooser (int yy, int xx)
{
    int xt, yt, yr,xr;
    transfsimple(yy+1, xx, yy, xx, yt, xt)
    transfsimple(yy, xx+1, yy, xx, yr, xr)
    
    CHOOSEBRANCH(yr, xr, yt, xt, yy-1, xx, right)
    
    GORIGHT (yr,xr,yy,xx)
    GODOWN  (yy,xx)
    GOUP    (yt,xt,yy,xx)
    
    PUTBACK (right, 0, 0, 0)
}


void chooseu (int yy, int xx)
{
    int xt, yt, yr,xr;
    transfsimple(yy+1, xx, yy, xx, yt, xt)
    transfsimple(yy, xx+1, yy, xx, yr, xr)
    
    CHOOSEBRANCH(yr, xr, yt, xt, yy, xx-1, up)
    
    GOUP    (yt,xt,yy,xx)
    GORIGHT (yr,xr,yy,xx)
    GOLEFT  (yy,xx)
    
    PUTBACK (up, 0, 0, 0)
}


void choosed (int yy, int xx)
{
    int xt, yt, yr,xr;
    transfsimple(yy, xx+1, yy, xx, yt, xt)
    
    CHOOSEBRANCH(yy-1, xx, yt, xt, yy, xx-1, down)
    
    GODOWN  (yy,xx)
    GORIGHT (yt, xt, yy,xx)
    GOLEFT  (yy,xx)
    
    PUTBACK (down, 1, yy+1, xx)
}


