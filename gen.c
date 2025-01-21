#include "defs.h"
#include "data.h"
#include "decl.h"


// 生成 if  if_else 从句 
static int genIFAST(struct ASTnode* n, int looptoplabel, int loopendlabel)
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



     genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);;// Condition and jump to Lfalse
    genfreeregs();

    //  否则真分支 
    genAST(n->mid, NOLABEL, looptoplabel, loopendlabel, n->op);
    genfreeregs();


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //若假存在 生成假分支语句 并跳转
    if (n->right)
    {
        genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
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


    genAST(n->left, Lend, Lstart, Lend, n->op);
    genfreeregs();

    genAST(n->right, NOLABEL, Lstart, Lend, n->op);
    genfreeregs();

    cgjump(Lstart);
    cglabel(Lend);  // 跳出while
    return NOREG;




}

/*
               A_FUNCCALL
                  /
              A_GLUE
               /   \
           A_GLUE  expr4
            /   \
        A_GLUE  expr3
         /   \
     A_GLUE  expr2
     /    \
   NULL  expr1


*/
static int gen_funccall(struct ASTnode* n)
{
    struct ASTnode* gluetree = n->left;
    int reg;
    int numargs = 0;

    // If there is a list of arguments, walk this list
    // from the last argument (right-hand child) to the
    // first
    while (gluetree)
    {
        // 右边生成
        reg = genAST(gluetree->right, NOLABEL, NOLABEL, NOLABEL, gluetree->op);
        // Copy this into the n'th function parameter: size is 1, 2, 3, ...
        cgcopyarg(reg, gluetree->size);
        // Keep the first (highest) number of arguments
        if (numargs == 0)
            numargs = gluetree->size;
        genfreeregs();
        gluetree = gluetree->left;
    }

    // Call the function, clean up the stack (based on numargs),
    // and return its result
    return (cgcall(n->sym, numargs));
}

static int genSWITCH(struct ASTnode* n)
{
    int* caseval, * caselabel;
    int Ljumptop, Lend;
    int i, reg, defaultlabel = 0, casecount = 0;
    struct ASTnode* c;

    // Create arrays for the case values and associated labels.
    // Ensure that we have at least one position in each array.
    caseval = (int*)malloc((n->intvalue + 1) * sizeof(int));
    caselabel = (int*)malloc((n->intvalue + 1) * sizeof(int));

    // Generate labels for the top of the jump table, and the
    // end of the switch statement. Set a default label for
    // the end of the switch, in case we don't have a default.
    Ljumptop = genlabel();
    Lend = genlabel();
    defaultlabel = Lend;

    // Output the code to calculate the switch condition
    reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
    cgjump(Ljumptop);
    genfreeregs();

    // Walk the right-child linked list to
    // generate the code for each case
    for (i = 0, c = n->right; c != NULL; i++, c = c->right) {

        // Get a label for this case. Store it
        // and the case value in the arrays.
        // Record if it is the default case.
        caselabel[i] = genlabel();
        caseval[i] = c->intvalue;
        cglabel(caselabel[i]);
        if (c->op == A_DEFAULT)
            defaultlabel = caselabel[i];
        else
            casecount++;

        // Generate the case code. Pass in the end label for the breaks
        genAST(c->left, NOLABEL, NOLABEL, Lend, 0);
        genfreeregs();
    }

    // Ensure the last case jumps past the switch table
    cgjump(Lend);

    // Now output the switch table and the end label.
    cgswitch(reg, casecount, Ljumptop, caselabel, caseval, defaultlabel);
    cglabel(Lend);
    return (NOREG);
}


// interpretAST的汇编接口版本  后序
int genAST(struct ASTnode* n, int iflabel, int looptoplabel, int loopendlabel, int parentASTop) // reg为最近使用寄存器对应下标
{
    int  leftreg;
    //  int  midreg;
    int  rightreg;

    // We now have specific AST node handling at the top
    switch (n->op)//此处填写语句类型 if fun 。。。
    {
    case A_IF:
        return genIFAST(n, looptoplabel, loopendlabel);
    case A_WHILE:
        return genWHILE(n);
    case A_SWITCH:
        return (genSWITCH(n));
    case A_FUNCCALL: // 函数调用
        return (gen_funccall(n)); // 将当前函数传入（A_FUNCALL）
    case A_GLUE:
        // Do each child statement, and free the
     // registers after each child
        if (n->left != NULL) // 需判断一下是否为空
            genAST(n->left, iflabel, looptoplabel, loopendlabel, n->op);
        genfreeregs();
        if (n->right != NULL)// 需判断一下是否为空 
            genAST(n->right, iflabel, looptoplabel, loopendlabel, n->op);
        genfreeregs();
        return (NOREG);


    case A_FUNCTION:
        cgfuncpreamble(n->sym);  // 类似之前的cgpreamble生成函数前置码
        genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
        cgfuncpostamble(n->sym); // 类似之前的cgpostamble生成函数前置码
        return NOREG;

    }

    if (n->left)
        leftreg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, n->op);
    if (n->right)
        rightreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);

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
        return (cgloadint(n->intvalue, n->type)); // 返回分配的寄存器下标号
    case A_STRLIT:
        return (cgloadglobstr(n->intvalue));
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
            if (n->sym->class == C_GLOBAL)
            {
                return (cgloadglob(n->sym, n->op));
            }
            else
            {
                return (cgloadlocal(n->sym, n->op));// C_PARAM和C_LOCAL
            }
        }
        else
            return (NOREG);

        /*
    case A_LVIDENT:
        // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, n->sym);
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
            return (cgcompare_and_jump(n->op, leftreg, rightreg, iflabel));
        else
            return (cgcompare_and_set(n->op, leftreg, rightreg));

    case A_ASSIGN:
        // Are we assigning to an identifier or through a pointer?
        switch (n->right->op)
        {
        case A_IDENT:
            if (n->right->sym->class == C_GLOBAL)
                return (cgstorglob(leftreg, n->right->sym));
            else
                return (cgstorlocal(leftreg, n->right->sym));
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
    case A_ADDR:
        return (cgaddress(n->sym));
    case A_DEREF:

        //  return (cgderef(leftreg, n->left->type));

        if (n->rvalue)
            return cgderef(leftreg, n->left->type);
        else
            return (leftreg);

    case A_SCALE:
        // Small optimisation: use shift if the
        // scale value is a known power of two
        switch (n->size)
        {
        case 2:
            return(cgshlconst(leftreg, 1));
        case 4:
            return(cgshlconst(leftreg, 2));
        case 8:
            return(cgshlconst(leftreg, 3));
        default:
            // 分配数组int a[10]  int * 10
            rightreg = cgloadint(n->size, P_INT);
            return cgmul(leftreg, rightreg); //l = l * r;
        }
    case A_POSTINC:
        // Load the variable's value into a register,
        // then increment it
        if (n->sym->class == C_GLOBAL)
            return (cgloadglob(n->sym, n->op));
        else
            return (cgloadlocal(n->sym, n->op));
    case A_POSTDEC:
        // Load the variable's value into a register,
        // then decrement it
        if (n->sym->class == C_GLOBAL)
            return (cgloadglob(n->sym, n->op));
        else
            return (cgloadlocal(n->sym, n->op));
    case A_PREINC:
        // Load and increment the variable's value into a register
        if (n->left->sym->class == C_GLOBAL)
            return (cgloadglob(n->left->sym, n->op));
        else
            return (cgloadlocal(n->left->sym, n->op));
    case A_PREDEC:
        // Load and decrement the variable's value into a register
        if (n->left->sym->class == C_GLOBAL)
            return (cgloadglob(n->left->sym, n->op));
        else
            return (cgloadlocal(n->left->sym, n->op));
    case A_NEGATE:
        return (cgnegate(leftreg));
    case A_INVERT:
        return (cginvert(leftreg));
    case A_LOGNOT:
        return (cglognot(leftreg));
    case A_BREAK:
        cgjump(loopendlabel);
        return (NOREG);
    case A_CONTINUE:
        cgjump(looptoplabel);
        return (NOREG);
    case A_TOBOOL:
        // If the parent AST node is an A_IF or A_WHILE, generate
        // a compare followed by a jump. Otherwise, set the register
        // to 0 or 1 based on it's zeroeness or non-zeroeness
        return (cgboolean(leftreg, parentASTop, iflabel));
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

int genalign(int type, int offset, int direction) 
{
    return (cgalign(type, offset, direction));
}