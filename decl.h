#pragma once
#include<stdlib.h>
int scan(struct token* t); // 判断令牌内容
struct ASTnode* mkastnode(int op, struct ASTnode* left,struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int intvalue);// 生成叶子节点
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue);// 生成左子树AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);
static struct ASTnode* primary();//解析 token 并判断其对应的ASTNode 应赋值类型为 A_INTLIT 
// static 每个文件


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
// interpretAST的汇编接口版本
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
        return cgload(n->intvalue); // 返回分配的寄存器下标号
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