#include "defs.h"
#include "data.h"
#include "decl.h"   

static int Globs = 0;                   // 空闲全局符号表位置
// 在符号表中找到全局符号位置并返回
int findglob(char* s)
{
    int i;

    for (i = 0; i < Globs; i++)
    {
        if (*s == *(Gsym[i].name) && !strcmp(s, Gsym[i].name))
            return i;
    }
    return -1;
}

// Get the position of a new global symbol slot, or die
// if we've run out of positions.
int newglob(void)
{
    int p;

    if ((p = Globs++) >= NSYMBOLS)
        fatal("Too many global symbols");
    return p;
}
// 向符号表增加全局符号并返回对应下标
int addglob(char* name, int type, int stype, int endab,int size)
{
    int y;

    // If this is already in the symbol table, return the existing slot
    if ((y = findglob(name)) != -1)
        return y;

    // Otherwise get a new slot, fill it in and
    // return the slot number
    y = newglob();
    Gsym[y].name = strdup(name);
    Gsym[y].type = type;
    Gsym[y].stype = stype;
    Gsym[y].endlabel = endab;
    Gsym[y].size = size;
    return y;
}

