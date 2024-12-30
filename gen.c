﻿#include "defs.h"
#include "data.h"
#include "decl.h"


// 生成 if  if_else 从句 
static int genIFAST(struct ASTnode* n)
{
    int Lfalse, Lend;

    /*
                   A_IF
              /      |    \
      condition   真分支  假分支

    */

    /*
生成两个标签：一个用于假复合语句，另一个用于
整个IF语句的结尾。当没有ELSE子句时，Lfalse是结束标签！
    */
    Lfalse = genlabel();
    if (n->right)      // 若假分支 存在语句
        Lend = genlabel();



    genAST(n->left, Lfalse, n->op);// Condition and jump to Lfalse
    genfreeregs();

    //  否则真分支 
    genAST(n->mid, NOLABEL, n->op);
    genfreeregs();


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //若假存在 生成假分支语句 并跳转
    if (n->right)
    {
        genAST(n->right, NOLABEL, n->op);
        genfreeregs();
        cglabel(Lend);
    }

    return NOREG;
}

static int genWHILE(struct ASTnode* n)
{

    int Lstart, Lend;

    // 生成标签
    Lstart = genlabel();
    Lend = genlabel();
    // 依照汇编语法生成汇编形式的while
    cglabel(Lstart);


    genAST(n->left, Lend, n->op);
    genfreeregs();

    genAST(n->right, NOLABEL, n->op);
    genfreeregs();

    cgjump(Lstart);
    cglabel(Lend);  // 跳出while
    return NOREG;




}


// interpretAST的汇编接口版本  后序
int genAST(struct ASTnode* n, int reg, int parentASTop)  // reg为最近使用寄存器对应下标
{
    int  leftreg;
    //  int  midreg;
    int  rightreg;

    // We now have specific AST node handling at the top
    switch (n->op)//此处填写语句类型 if fun 。。。
    {
    case A_IF:
        return genIFAST(n);
    case A_WHILE:
        return genWHILE(n);
    case A_GLUE:
        // Do each child statement, and free the
        // registers after each child
        genAST(n->left, NOLABEL, n->op);
        genfreeregs();
        genAST(n->right, NOLABEL, n->op);
        genfreeregs();
        return NOREG;


    case A_FUNCTION:
        cgfuncpreamble(n->v.id);  // 类似之前的cgpreamble生成函数前置码
        genAST(n->left, NOLABEL, n->op);
        cgfuncpostamble(n->v.id); // 类似之前的cgpostamble生成函数前置码
        return NOREG;

    }

    if (n->left)
        leftreg = genAST(n->left, NOLABEL, n->op);
    if (n->right)
        rightreg = genAST(n->right, NOLABEL, n->op);

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
        return (cgloadint(n->v.intvalue, n->type)); // 返回分配的寄存器下标号
    case A_STRLIT:
        return (cgloadglobstr(n->v.id));
    case A_AND:
        return (cgand(leftreg, rightreg));
    case A_OR:
        return (cgor(leftreg, rightreg));
    case A_XOR:
        return (cgxor(leftreg, rightreg));
    case A_LSHIFT:
        return (cgshl(leftreg, rightreg));
    case A_RSHIFT:
        return (cgshr(leftreg, rightreg));

    case A_IDENT:
        /*
               A_DEREF
               /   \        *p间接寻址
               p    *
        为右值或者间接寻址
        */
        // Load our value if we are an rvalue
      // or we are being dereferenced
        if (n->rvalue || parentASTop == A_DEREF)
        {
            if (Gsym[n->v.id].class == C_GLOBAL)
            {
                return (cgloadglob(n->v.id, n->op));
            }
            else {
                return (cgloadlocal(n->v.id, n->op));// C_PARAM和C_LOCAL
            }
        }
        else
            return (NOREG);

        /*
    case A_LVIDENT:
        // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, n->v.id);
        */
        // 比较判断 传入的值为左右寄存器编号

        // 更改部分
    case A_EQ:
    case A_NE:
    case A_LT:
    case A_GT:
    case A_LE:
    case A_GE:
        // If the parent AST node is an A_IF or A_WHILE, generate
        // a compare followed by a jump. Otherwise, compare registers
        // and set one to 1 or 0 based on the comparison.
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return (cgcompare_and_set(n->op, leftreg, rightreg));

    case A_ASSIGN:
        // Are we assigning to an identifier or through a pointer?
        switch (n->right->op) 
        {
        case A_IDENT:
            if (Gsym[n->right->v.id].class == C_GLOBAL)
                return (cgstorglob(leftreg, n->right->v.id));
            else // C_PARAM和C_LOCAL
                return (cgstorlocal(leftreg, n->right->v.id));
        case A_DEREF:
            return (cgstorderef(leftreg, rightreg, n->right->type));
        default:
            fatald("Can't A_ASSIGN in genAST(), op", n->op);
        }
#if 0
    case A_PRINT:
        genprintint(leftreg); // 打印左寄存器所存值
        /*
              A_PRINT
              /     \
            +
          /   \
         1     2
        */

        genfreeregs();
        return NOREG;
#endif    
    case A_WIDEN:
        // Widen the child's type to the parent's type
        return (cgwiden(leftreg, n->left->type, n->type));

    case A_RETURN:
        cgreturn(leftreg, Functionid);
        return (NOREG);

    case A_FUNCCALL:
        return (cgcall(leftreg, n->v.id));

    case A_ADDR:
        return (cgaddress(n->v.id));
    case A_DEREF:

        //  return (cgderef(leftreg, n->left->type));

        if (n->rvalue)
            return cgderef(leftreg, n->left->type);
        else
            return (leftreg);

    case A_SCALE:
        // Small optimisation: use shift if the
        // scale value is a known power of two
        switch (n->v.size)
        {
        case 2:
            return(cgshlconst(leftreg, 1));
        case 4:
            return(cgshlconst(leftreg, 2));
        case 8:
            return(cgshlconst(leftreg, 3));
        default:
            // 分配数组int a[10]  int * 10
            rightreg = cgloadint(n->v.size, P_INT);
            return cgmul(leftreg, rightreg); //l = l * r;
        }
    case A_POSTINC:
        // Load the variable's value into a register,
        // then increment it
        return (cgloadglob(n->v.id, n->op));
    case A_POSTDEC:
        // Load the variable's value into a register,
        // then decrement it
        return (cgloadglob(n->v.id, n->op));
    case A_PREINC:
        // Load and increment the variable's value into a register
        return (cgloadglob(n->left->v.id, n->op));
    case A_PREDEC:
        // Load and decrement the variable's value into a register
        return (cgloadglob(n->left->v.id, n->op));
    case A_NEGATE:
        return (cgnegate(leftreg));
    case A_INVERT:
        return (cginvert(leftreg));
    case A_LOGNOT:
        return (cglognot(leftreg));
    case A_TOBOOL:
        // If the parent AST node is an A_IF or A_WHILE, generate
        // a compare followed by a jump. Otherwise, set the register
        // to 0 or 1 based on it's zeroeness or non-zeroeness
        return (cgboolean(leftreg, parentASTop, reg));
    default:
        fatald("Unknown AST operator", n->op);
    }

    return NOREG;
}

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
#if 0
void genprintint(int reg)
{
    cgprintint(reg);
}
#endif
void genglobsym(int id)
{
    cgglobsym(id);
}

// 生成字符串的汇编代码
int genglobstr(char* strvalue) 
{
    int l = genlabel();
    cgglobstr(l, strvalue);
    return(l);
}

// 获取大小
int genprimsize(int type)
{
    return (cgprimsize(type));
}

#if 0
void genresetlocals(void)
{
    cgresetlocals();
}
int gengetlocaloffset(int type, int isparam) 
{
    return (cggetlocaloffset(type, isparam));
}
#endif