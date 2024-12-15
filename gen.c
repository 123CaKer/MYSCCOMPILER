#include "defs.h"
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
    case A_IDENT:
        /*
               A_DEREF
               /   \        *p间接寻址
               p    *
        为右值或者间接寻址
        */
        if (n->rvalue || parentASTop == A_DEREF)
            return (cgloadglob(n->v.id));
        else
            return NOREG;

        /*
    case A_LVIDENT:
        // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, n->v.id);
        */
        // 比较判断 传入的值为左右寄存器编号
    case A_EQ:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cgequal(leftreg, rightreg);
    case A_NE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cgnotequal(leftreg, rightreg);
    case A_LT:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cglessthan(leftreg, rightreg);
    case A_GT:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cggreaterthan(leftreg, rightreg);
    case A_LE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cglessequal(leftreg, rightreg);
    case A_GE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cggreaterequal(leftreg, rightreg);


    case A_ASSIGN:
        switch (n->right->op)
        {
        case A_IDENT:// 赋值变量
            return cgstorglob(leftreg, n->right->v.id);
        case A_DEREF: // 间接
            return cgstorderef(leftreg, rightreg, n->right->type);
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
void genprintint(int reg)
{
    cgprintint(reg);
}

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