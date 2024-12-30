#include "defs.h"
#include "data.h"
#include "decl.h"   

//static int Globs = 0;                   // 空闲全局符号表位置
// Determine if the symbol s is in the global symbol table.
// Return its slot position or -1 if not found.
int findglob(char* s) 
{
    int i;

    for (i = 0; i < Globs; i++)
    {
        if (Gsym[i].class == C_PARAM) 
            continue;
        if (*s == *Gsym[i].name && !strcmp(s, Gsym[i].name))
            return (i);
    }
    return (-1);
}

// Clear all the entries in the
// local symbol table
void freeloclsyms(void) 
{
    Locls = NSYMBOLS - 1;
}


// Get the position of a new global symbol slot, or die
// if we've run out of positions.
static int newglob(void) 
{
    int p;

    if ((p = Globs++) >= Locls)
        fatal("Too many global symbols");
    return (p);
}

// Determine if the symbol s is in the local symbol table.
// Return its slot position or -1 if not found.
int findlocl(char* s)
{
    int i;

    for (i = Locls + 1; i < NSYMBOLS; i++)
    {
        if (*s == *Gsym[i].name && !strcmp(s, Gsym[i].name))
            return (i);
    }
    return (-1);
}

// Get the position of a new local symbol slot, or die
// if we've run out of positions.
static int newlocl(void) 
{
    int p;
    if ((p = Locls--) <= Globs)
        fatal("Too many local symbols");
    return (p);
}
static void updatesym(int slot, char* name, int type, int stype,
int class, int endlabel, int size, int posn) // 更新符号表
{
    if (slot < 0 || slot >= NSYMBOLS)
        fatal("Invalid symbol slot number in updatesym()");
    Gsym[slot].name = strdup(name);
    Gsym[slot].type = type;
    Gsym[slot].stype = stype;
    Gsym[slot].class = class;
    Gsym[slot].endlabel = endlabel;
    Gsym[slot].size = size;
    Gsym[slot].posn = posn;
}

// Add a global symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// + endlabel: if this is a function
// Return the slot number in the symbol table
int addglob(char* name, int type, int stype, int endlabel, int size) {
    int slot;

    // If this is already in the symbol table, return the existing slot
    if ((slot = findglob(name)) != -1)
        return (slot);

    // Otherwise get a new slot, fill it in and
    // return the slot number
    slot = newglob();
    updatesym(slot, name, type, stype, C_GLOBAL, endlabel, size, 0);
    genglobsym(slot);
    return (slot);
}

// Add a local symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// + isparam: if true, this is a parameter to the function
// Return the slot number in the symbol table, -1 if a duplicate entry
int addlocl(char* name, int type, int stype, int isparam, int size) 
{
    int localslot, globalslot;
    if ((localslot = findlocl(name)) != -1)
        return (-1);

    // Otherwise get a new symbol slot and a position for this local.
    // Update the local symbol table entry. If this is a parameter,
    // also create a global C_PARAM entry to build the function's prototype.
    localslot = newlocl();
    if (isparam) // 判断是否为函数参数
    {
        updatesym(localslot, name, type, stype, C_PARAM, 0, size, 0); //  更新符号表C_PARAM
        globalslot = newglob();
        updatesym(globalslot, name, type, stype, C_PARAM, 0, size, 0); //建立一个函数原型且函数原型为全局变量
    }
    else 
    {
        updatesym(localslot, name, type, stype, C_LOCAL, 0, size, 0);//  更新符号表C_LOCAL
    }

    // 返回所增加的符号表位置
    return (localslot);
}

// Determine if the symbol s is in the symbol table.
// Return its slot position or -1 if not found.
int findsymbol(char* s) 
{
    int slot;

    slot = findlocl(s);
    if (slot == -1)
        slot = findglob(s);
    return (slot);
}
