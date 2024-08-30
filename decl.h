#pragma once
#include<stdlib.h>
int scan(struct token* t); // �ж���������
struct ASTnode* mkastnode(int op, struct ASTnode* left,struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int intvalue);// ����Ҷ�ӽڵ�
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue);// ����������AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);
static struct ASTnode* primary();//���� token ���ж����Ӧ��ASTNode Ӧ��ֵ����Ϊ A_INTLIT 
// static ÿ���ļ�


// cg.c
void generatecode(struct ASTnode* n);
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgload(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);

// gen.c
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);



static int genAST(struct ASTnode* n);
// interpretAST�Ļ��ӿڰ汾
static int genAST(struct ASTnode* n)
{
    int  leftreg;
    int  rightreg;

    if (n->left)
        leftreg = genAST(n->left);
    if (n->right)
        rightreg = genAST(n->right);

    switch (n->op)
    {
    case A_ADD:
        return cgadd(leftreg, rightreg);
    case A_SUBTRACT:
        return cgsub(leftreg, rightreg);
    case A_MULTIPLY:
        return cgmul(leftreg, rightreg);
    case A_DIVIDE:
        return cgdiv(leftreg, rightreg);
    case A_INTLIT:
        return cgload(n->intvalue); // ���ط���ļĴ����±��
    default:
        fprintf(stderr, "Unknown AST operator %d\n", n->op);
        exit(1);
    }
}

// expr.c
struct ASTnode* binexpr(int ptp);

// stmt.c
void statements(void);

// misc.c
void match(int t, char* what);
void semi(void);