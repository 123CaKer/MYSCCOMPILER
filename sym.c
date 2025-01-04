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

// Given a function's slot number, copy the global parameters
// from its prototype to be local parameters
// 将函数声明中的参数转换为当前函数的局部参数
void copyfuncparams(int slot)
{
    int i, id = slot + 1;

    for (i = 0; i < Gsym[slot].nelems; i++, id++) 
    {
        addlocl(Gsym[id].name, Gsym[id].type, Gsym[id].stype,Gsym[id].class, Gsym[id].size);
    }
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
// + class of the symbol
// + size: number of elements
// + endlabel: if this is a function
// Return the slot number in the symbol table
int addglob(char* name, int type, int stype, int class, int endlabel, int size) 
{
    int slot;

    // If this is already in the symbol table, return the existing slot
    if ((slot = findglob(name)) != -1)
        return (slot);

    // Otherwise get a new slot and fill it in
    slot = newglob();
    updatesym(slot, name, type, stype, class, endlabel, size, 0);
    // Generate the assembly for the symbol if it's global
    if (class == C_GLOBAL)
        genglobsym(slot);
    // Return the slot number
    return (slot);
}
// Add a local symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// Return the slot number in the symbol table, -1 if a duplicate entry
int addlocl(char* name, int type, int stype, int class, int size) 
{
    int localslot;

    // If this is already in the symbol table, return an error
    if ((localslot = findlocl(name)) != -1)
        return (-1);

    // Otherwise get a new symbol slot and a position for this local.
    // Update the local symbol table entry.
    localslot = newlocl();
    updatesym(localslot, name, type, stype, class, 0, size, 0);

    // Return the local symbol's slot
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

// Reset the contents of the symbol table
void clear_symtable(void)
{
    Globs = 0;
    Locls = NSYMBOLS - 1;
}
