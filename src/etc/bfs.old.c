
// ============================================================================
// 
//	Best-first search
// 
// ============================================================================


#include "defs.h"
#include "io.h"


static void reset_timestamps(void)
{
	int j, k;
	point *tt;
	trcc->stamp = STAMPSTART;
	
	resetstamps++;
//	dprintf(outp, "RESET TIMESTAMPS\n");

	for (k = BS*2; k > BS; k--)
		for (j = 1; j <= BS; j++)
			if (trmap[k][j].v != MAPWALL)
				trmap[k][j].v = EMPTY;

	for (	; k > 0; k--)
		for (j = 1; j <= BS*2; j++)
			if (trmap[k][j].v != MAPWALL)
				trmap[k][j].v = EMPTY;

	tt = &trmap[trcc->y2][trcc->x2];
	tt = &trmap[tt->y][tt->x];
	
	if (tt->v != MAPWALL)
		for ( tt = &trmap[tt->y][tt->x]; tt->v != MAPWALL;
			tt->v = MAPDEST,  tt = &trmap[tt->y][tt->x] );
	
	if ( ! *trcc->adj1 ) *trcc->adj1 = MAPDEST;
	if ( ! *trcc->adj2 ) *trcc->adj2 = MAPDEST;
	if ( ! *trcc->adj3 ) *trcc->adj3 = MAPDEST;
	if ( ! *trcc->adj4 ) *trcc->adj4 = MAPDEST;

//	rmap[trcc->y1][trcc->x1].v = trcc->stamp;	/* prolly not needed */

	if (DEBUG) printmap(trcc-colors);
}


static void reset_distances (void)
{
	int j, k;
	trcc->distance = DISTANCESTART+1;
	
//	dprintf(outp, "RESET DISTANCE STAMPS\n");
	resetdists++;

	for (k = BS*2; k > BS; k--)
		for (j = 1; j <= BS; j++)
			if ( trmap[k][j].d )
				trmap[k][j].d = DISTANCESTART;

	for (	; k > 0; k--)
		for (j = 1; j <= BS*2; j++)
			if ( trmap[k][j].d )
				trmap[k][j].d = DISTANCESTART;

	*trcc->adj1d = DISTMAX; 
	*trcc->adj2d = DISTMAX; 
	*trcc->adj3d = DISTMAX; 
	*trcc->adj4d = DISTMAX; 
	trmap[trcc->y1][trcc->x1].d = DISTMAX;
	trmap[trcc->y2][trcc->x2].d = DISTMAX;
}




#define GETCOLINEAR(curry,currx,prevy,prevx,coly,colx)\
	colx = currx;							\
	coly = curry;							\
											\
	if (prevx == colx)						\
	{	if (prevy > coly)					\
			coly--;							\
		else if (colx <= BS || coly+1 <= BS)\
			coly++;							\
		else 								\
		{	colx = curry;					\
			coly = currx;					\
		}									\
	}										\
											\
	else if (prevx > colx)					\
		colx--;								\
	else if ( colx+1 <= BS || coly <= BS )	\
		colx++;								\
	else									\
	{	colx = curry;						\
		coly = currx;						\
	}


#define	TESTBFS(b,a)						\
	/* timestamp is always < MAPWALL */		\
	if ( trmap[b][a].v < timestamp)			\
	{										\
		if ( trmap[b][a].v == MAPDEST )		\
			break;							\
		trmap[b][a].v = timestamp;			\
		spf->y = b;							\
		spf->x = a;							\
		/* link to the previous point */	\
		spf->v = sp - stackbfs;				\
		spf++;								\
	}



/*	the (y,x) coords of the stacked points have the 
	actual coords of the point */

int bfsfind(void)
{
	int ex, ey, ry, rx, cy, cx;
	point *sp5, *tt, *p2;
	point *stackbfs;
	point *sp, *spf;
	int timestamp;
		
	bfscalls++;
	if (DEBUG)
	{	dprintf(outp,"call to bfs at (0,0). stamp = %d\n", trcc->stamp);
		printmap(trcc - colors);
		printpath(trcc - colors);
	}
	
	if ( ++trcc->stamp == STAMPMAX )
		reset_timestamps();
	trmap[trcc->y1][trcc->x1].v = timestamp = trcc->stamp;
	stackbfs = trcc->stackbfs;

	for (sp = stackbfs, spf = sp+1; sp != spf; sp++ )
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
	
	if ( sp == spf)
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
#define	outp stdin
#endif

/* (ey, ex) = point Mapdest found */
/* sp	= last point found */
/* tt	= previous point found */

	tt = stackbfs + sp->v;
	
	GETCOLINEAR(sp->y,sp->x,tt->y,tt->x,ry,rx,cy,cx)
	
	dprintf (outp,"test pocket: sp (%d,%d), rs (%d,%d), es (%d,%d)\n", 
		sp->y, sp->x, ry, rx, ey, ex);
	
	if ( (ex != rx || ey != ry) &&
		trmap[ry][rx].v == MAPDEST )
	{	/*	pocket case (f = bfs find):
			f f f 1 1 1 etc
				1 1
		*/
		dprintf (outp,"pocket hit\n");
		ey = ry;
		ex = rx;
	}
	
	else
	{
	/*	pocket case (f = bfs find):
		f
		f
		f 1 1 1 etc
		1 1 
	*/
		// falta, nao sei pq/onde esta
		
	}
	
	spf->y = ey;
	spf->x = ex;
	spf->v = sp - stackbfs;
	tt = &trmap[ey][ex];
	p2 = &trmap[trcc->y2][trcc->x2];
	sp5 = &trmap[p2->y][p2->x];	/* auxiliar */
	
	if (tt->d == DISTMAX)	/* adjacency point, all the old path trashed */
	{
		dprintf (outp,"ajdcen point: (%d,%d)\n", spf->y, spf->x);	
		tt = sp5;
		p2->x = ex;
		p2->y = ey;
	}
	
	/* reconnecting tru the first after adjacency, replacing adjacency */
	else if(tt == &trmap[sp5->y][sp5->x] 
		&&	sp->y != p2->y && sp->x != p2->x )
	{
		GETCOLINEAR(ey,ex, sp->y,sp->x, ry,rx)
		dprintf (outp,"test reconnect: p2 - (%d,%d), rs - (%d,%d)\n", 
			p2->y, p2->x, ry, rx);

/*		if (trmap[ry][rx].v == MAPDEST &&
			trmap[ry][rx].d != DISTMAX )
			fprintf(outp, "SPECIAL CASE RECONNECT REMOVED\n"); //debug
		else
*/
		if (trmap[ry][rx].v == MAPDEST && trmap[ry][rx].d == DISTMAX)
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
	for ( tt = &trmap[tt->y][tt->x]; tt->v == MAPDEST;
		tt->v = EMPTY, 				/* clears MAPDESTs */
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
	
	trmap[spf->y][spf->x].v = MAPDEST; 	/* set start point */
	
#ifdef	outp
#undef	outp
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

#define	TEST_BFS_REJECT(b,a)		\
	if ( cube[b][a] == EMPTY &&		\
		trmap[b][a].d < timestamp)	\
	{								\
		if ( (sjf->d = sp->d+1) >	\
			DEPTHREJECT )			\
				return 0;			\
		trmap[b][a].d = timestamp;	\
		sjf->y = b;					\
		sjf->x = a;					\
		sjf++;						\
	}
	
int bfsreject (void)
{
	point *sjf, *sj4; 
	point *sp;
	int ex, ey;
	int timestamp;
	
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
	
	for (	; sp != sjf; sp++ )
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
//	printdistmap (trcc - colors);
	rejectsdone++;

	return 1;
}



