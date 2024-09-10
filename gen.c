#include "defs.h"
#include "data.h"
#include "decl.h"


 

// ͨ�ô���������
void generatecode(struct ASTnode* n)   // ���ɻ�����
{
    int reg;

    cgpreamble();
    reg = genAST(n,NOREG,-1);// �Ĵ����洢����ASTֵ
    cgprintint(reg);// ���reg��ֵ
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