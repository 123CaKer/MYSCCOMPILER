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

// Add a global symbol to the symbol table.
// Return the slot number in the symbol table
int addglob(char* name) 
{
    int y;

    // If this is already in the symbol table, return the existing slot
    if ((y = findglob(name)) != -1)
        return y;

    // Otherwise get a new slot, fill it in and
    // return the slot number
    y = newglob();
    Gsym[y].name = strdup(name);
    return y;
}