#include "defs.h"
#include "data.h"
#include "decl.h"


 

// 通用代码生成器
void generatecode(struct ASTnode* n)   // 生成汇编代码
{
    int reg;

    cgpreamble();
    reg = genAST(n,NOREG,-1);// 寄存器存储的是AST值
    cgprintint(reg);// 输出reg的值
    cgpostamble();
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

void genglobsym(char* s) 
{
    cgglobsym(s);
}