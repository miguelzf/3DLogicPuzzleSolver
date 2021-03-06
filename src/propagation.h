//------------------------------------------------------------------------------
// 
//  Constraint propagation
// 
//------------------------------------------------------------------------------

void chooser(int yy, int xx);

void choosel(int yy, int xx);

void chooseu(int yy, int xx);

void choosed(int yy, int xx);

    
#define GORIGHT(yt,xt,yy,xx)        \
    if (cube[yt][xt] == EMPTY)      \
    {                               \
        if (yt == xx && xt == yy)   \
            choosed (yt, xt);       \
        else                        \
            chooser (yt, xt);       \
    }

#define GOUP(yt,xt,yy,xx)           \
    if (cube[yt][xt] == EMPTY)      \
    {                               \
        if ( yt == xx && xt == yy)  \
            choosel (yt, xt);       \
        else                        \
            chooseu (yt, xt);       \
    }

#define GOLEFT(y,x)                 \
    if (cube[y][x-1] == EMPTY)      \
        choosel (y, x-1);

#define GODOWN(y,x)                 \
    if (cube[y-1][x] == EMPTY)      \
        choosed (y-1, x);   



