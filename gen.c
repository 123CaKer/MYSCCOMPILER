#include "defs.h"
#include "data.h"
#include "decl.h"

int genlabel(void)
{
    static int id = 1;
    return (id++);
}
 


void genpreamble()
{
    cgpreamble();
}
void genpostamble()
{
    cgpostamble();
}
void genfreeregs()
{
    freeall_registers();
}
void genprintint(int reg) 
{
    cgprintint(reg);
}

void genglobsym(int id) 
{
    cgglobsym(id);
}

int genprimsize(int type) 
{
    return (cgprimsize(type));
}