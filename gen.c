#include "defs.h"
#include "data.h"
#include "decl.h"


// Generate the code for an
// A_LOGAND or A_LOGOR operation
static int gen_logandor(struct ASTnode* n) {
    // Generate two labels
    int Lfalse = genlabel();
    int Lend = genlabel();
    int reg;

    // Generate the code for the left expression
    // followed by the jump to the false label
    reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
    cgboolean(reg, n->op, Lfalse);
    genfreeregs(NOREG);

    // Generate the code for the right expression
    // followed by the jump to the false label
    reg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, 0);
    cgboolean(reg, n->op, Lfalse);
    genfreeregs(reg);

    // We didn't jump so set the right boolean value
    if (n->op == A_LOGAND) {
        cgloadboolean(reg, 1);
        cgjump(Lend);
        cglabel(Lfalse);
        cgloadboolean(reg, 0);
    }
    else {
        cgloadboolean(reg, 0);
        cgjump(Lend);
        cglabel(Lfalse);
        cgloadboolean(reg, 1);
    }
    cglabel(Lend);
    return(reg);
}

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
    genfreeregs(NOREG);

    //  否则真分支 
    genAST(n->mid, NOLABEL, looptoplabel, loopendlabel, n->op);
    genfreeregs(NOREG);


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //若假存在 生成假分支语句 并跳转

    /*
    现在，编译器可以解析其自己的每个源代码文件了。但当我试图链接它们时，我得到了一个关于丢失L0标签的警告。
     调查原因，发现我没有在gen.c的genIFAST（）中传递loopendlabel。修复程序如下：
    */
    if (n->right)
    {
        genAST(n->right, NOLABEL, NOLABEL, loopendlabel, n->op);
        genfreeregs(NOREG);
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
    genfreeregs(NOREG);

    genAST(n->right, NOLABEL, Lstart, Lend, n->op);
    genfreeregs(NOREG);

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
/*
Spill the registers first.
Copy the arguments to the function (using the registers).
Call the function.
Reload the registers before we Copy the register's return value.
*/
static int gen_funccall(struct ASTnode* n)
{
    struct ASTnode* gluetree = n->left;
    int reg;
    int numargs = 0;

    // Save the registers before we copy the arguments
    // 在进行函数调用之前寄存器数据加载到主存中
    spill_all_regs();

    // If there is a list of arguments, walk this list
    // from the last argument (right-hand child) to the
    // first
    // 复制参数 （将reg中数据复制到stack）
    while (gluetree)
    {
        // 右边生成
        reg = genAST(gluetree->right, NOLABEL, NOLABEL, NOLABEL, gluetree->op);
        // Copy this into the n'th function parameter: size is 1, 2, 3, ...
        cgcopyarg(reg, gluetree->a_size);
        // Keep the first (highest) number of arguments
        if (numargs == 0)
            numargs = gluetree->a_size;
        // genfreeregs(NOREG);
        gluetree = gluetree->left;
    }

    // Call the function, clean up the stack (based on numargs),
    // and return its result
    // 调用目标函数
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
    caseval = (int*)malloc((n->a_intvalue + 1) * sizeof(int));
    caselabel = (int*)malloc((n->a_intvalue + 1) * sizeof(int));

    // Generate labels for the top of the jump table, and the
    // end of the switch statement. Set a default label for
    // the end of the switch, in case we don't have a default.
    Ljumptop = genlabel();
    Lend = genlabel();
    defaultlabel = Lend;

    // Output the code to calculate the switch condition
    reg = genAST(n->left, NOLABEL, NOLABEL, NOLABEL, 0);
    cgjump(Ljumptop);
    genfreeregs(reg);

    // Walk the right-child linked list to
    // generate the code for each case
    for (i = 0, c = n->right; c != NULL; i++, c = c->right)
    {

        // Get a label for this case. Store it
        // and the case value in the arrays.
        // Record if it is the default case.
        caselabel[i] = genlabel();
        caseval[i] = c->a_intvalue;
        cglabel(caselabel[i]);
        if (c->op == A_DEFAULT)
            defaultlabel = caselabel[i];
        else
            casecount++;

        // Generate the case code. Pass in the end label for the breaks.
     // If case has no body, we will fall into the following body.
        if (c->left)
            genAST(c->left, NOLABEL, NOLABEL, Lend, 0);
        genfreeregs(NOREG);
    }

    // Ensure the last case jumps past the switch table
    cgjump(Lend);

    // Now output the switch table and the end label.
    cgswitch(reg, casecount, Ljumptop, caselabel, caseval, defaultlabel);
    cglabel(Lend);
    return (NOREG);
}

// Allocate a QBE temporary
static int nexttemp = 0;
static int cgalloctemp(void) {
    return (++nexttemp);
}
// 生成三元运算符表达式 类似为if

static int gen_ternary(struct ASTnode* n) {
    int Lfalse, Lend;
    int reg, expreg;

    // Generate two labels: one for the
    // false expression, and one for the
    // end of the overall expression
    Lfalse = genlabel();
    Lend = genlabel();

    // Generate the condition code followed
    // by a jump to the false label.
    genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);
    // genfreeregs(NOREG);

    // Get a register to hold the result of the two expressions
    reg = alloc_register();

    // Generate the true expression and the false label.
    // Move the expression result into the known register.
    expreg = genAST(n->mid, NOLABEL, NOLABEL, NOLABEL, n->op);
    cgmove(expreg, reg);

    /*
          chr 60 寄存器释放问题

          在之前的函数调用中，我们使用的是genfreeregs()函数，该函数的目的是释放所有的
          reg ，如果传入的reg 无法进行释放则不释放 ，原因就在于此处，传入参数后 ，剩下的寄存器
          可能由于线程优先级原因导致寄存器的赋值无法达到正确释放，倘若内部有值并且未能够传入mem中
          则可能导致值丢失现象，故我们采用cgfreereg(expreg); 来释放对应的下标为expreg的reg


          估计在之前的*p 等等地址原因，也可能与此有关 ,但解决方案在chr 57中的 expr 值传入链中引入“上一个优先级”
          防止寄存器释放错误  eg 即为if (a==NULL || b==NULL || c==NULL) 中正确生成
          if ((a==NULL) || (b==NULL) ||( c==NULL))
    */
    cgfreereg(expreg);
    cgjump(Lend);
    cglabel(Lfalse);

    // Generate the false expression and the end label.
    // Move the expression result into the known register.
    expreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
    cgmove(expreg, reg);
    cgfreereg(expreg);
    cglabel(Lend);
    return (reg);
}


// chr 63的 
#if 0
static int gen_ternary(struct ASTnode* n) {
    int Lfalse, Lend;
    int reg, expreg;
    int r, r2;

    // Generate two labels: one for the
    // false expression, and one for the
    // end of the overall expression
    Lfalse = genlabel();
    Lend = genlabel();

    // Generate the condition code
    r = genAST(n->left, Lfalse, NOLABEL, NOLABEL, n->op);
    // Test to see if the condition is true. If not, jump to the false label
    r2 = cgloadint(1, P_INT);
    cgcompare_and_jump(A_EQ, r, r2, Lfalse, P_INT);

    // Get a temporary to hold the result of the two expressions
    reg = cgalloctemp();

    // Generate the true expression and the false label.
    // Move the expression result into the known temporary.
    expreg = genAST(n->mid, NOLABEL, NOLABEL, NOLABEL, n->op);
    cgmove(expreg, reg);
    cgfreereg(expreg);
    cgjump(Lend);
    cglabel(Lfalse);

    // Generate the false expression and the end label.
    // Move the expression result into the known temporary.
    expreg = genAST(n->right, NOLABEL, NOLABEL, NOLABEL, n->op);
    cgmove(expreg, reg);
    cgfreereg(expreg);
    cglabel(Lend);
    return (reg);
}
#endif

// interpretAST的汇编接口版本  后序
int genAST(struct ASTnode* n, int iflabel, int looptoplabel, int loopendlabel, int parentASTop) // reg为最近使用寄存器对应下标
{
    int  leftreg = NOREG;
    //  int  midreg;
    int  rightreg = NOREG;

    // Empty tree, do nothing
    if (n == NULL)
        return(NOREG);


    // We now have specific AST node handling at the top
    switch (n->op)//此处填写语句类型 if fun 。。。
    {
    case A_IF:
        return genIFAST(n, looptoplabel, loopendlabel);
    case A_WHILE:
        return genWHILE(n);
    case A_SWITCH:
        return (genSWITCH(n));
    case A_TERNARY:
        return (gen_ternary(n));//  三元运算符
    case A_FUNCCALL: // 函数调用
        return (gen_funccall(n)); // 将当前函数传入（A_FUNCALL）
    case A_LOGOR:
        return (gen_logandor(n));
    case A_LOGAND:
        return (gen_logandor(n));
    case A_GLUE:
        // Do each child statement, and free the
     // registers after each child
        if (n->left != NULL) // 需判断一下是否为空
            genAST(n->left, iflabel, looptoplabel, loopendlabel, n->op);
        genfreeregs(NOREG);
        if (n->right != NULL)// 需判断一下是否为空 
            genAST(n->right, iflabel, looptoplabel, loopendlabel, n->op);
        genfreeregs(NOREG);
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
        return cgdivmod(leftreg, rightreg, A_DIVIDE);
    case A_MOD:
        return (cgdivmod(leftreg, rightreg, A_MOD));
    case A_INTLIT:
        return (cgloadint(n->a_intvalue, n->type)); // 返回分配的寄存器下标号
    case A_STRLIT:
        return (cgloadglobstr(n->a_intvalue));
    case A_AND:
        return (cgand(leftreg, rightreg));
    case A_OR:
        return (cgor(leftreg, rightreg));
    case A_XOR:
        return (cgxor(leftreg, rightreg));
#if 0
    case A_LOGOR:
        return (gen_logandor(n));
    case A_LOGAND:
        return (gen_logandor(n));
#endif // 0
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
#if 0

        if (n->rvalue || parentASTop == A_DEREF)
        {
            if (n->sym->class == C_GLOBAL || n->sym->class == C_STATIC || n->sym->class == C_EXTERN)
            {
                return (cgloadglob(n->sym, n->op));// C_GLOBAL和C_STATIC C_EXTERN
            }
            else
            {
                return (cgloadlocal(n->sym, n->op));// C_PARAM和C_LOCAL 
            }
        }
        else
            return (NOREG);

#endif // 0

        // Load our value if we are an rvalue
    // or we are being dereferenced
        if (n->rvalue || parentASTop == A_DEREF)
        {
            return (cgloadvar(n->sym, n->op));
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
        if (parentASTop == A_IF || parentASTop == A_WHILE || parentASTop == A_TERNARY)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, iflabel, n->left->type));
        else
            return (cgcompare_and_set(n->op, leftreg, rightreg, n->left->type));



        // 右结合性 
    case A_ASPLUS:
    case A_ASMINUS:
    case A_ASSTAR:
    case A_ASSLASH:
    case A_ASMOD:
    case A_ASSIGN:

        // For the '+=' and friends operators, generate suitable code
      // and get the register with the result. Then take the left child,
      // make it the right child so that we can fall into the assignment code.
        switch (n->op)
        {
        case A_ASPLUS:
            leftreg = cgadd(leftreg, rightreg);
            n->right = n->left;
            break;
        case A_ASMINUS:
            leftreg = cgsub(leftreg, rightreg);
            n->right = n->left;
            break;
        case A_ASSTAR:
            leftreg = cgmul(leftreg, rightreg);
            n->right = n->left;
            break;
        case A_ASSLASH:
            leftreg = cgdivmod(leftreg, rightreg, A_ASSLASH);
            n->right = n->left;
            break;
        case A_ASMOD:
            leftreg = cgdivmod(leftreg, rightreg, A_MOD);
            n->right = n->left;
            break;
        }
        //在这里进行赋值（*p类型）
        // Are we assigning to an identifier or through a pointer?
        switch (n->right->op) // *结合右值
        {
        case A_IDENT:
            if (n->right->sym->class == C_GLOBAL || n->right->sym->class == C_EXTERN ||
                n->right->sym->class == C_STATIC)
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
        // If we have a symbol, get its address. Otherwise,
    // the left register already has the address because
    // it's a member access
        //这样就形成 a.b.c....... 访问链
        if (n->sym != NULL)
            return (cgaddress(n->sym));
        else
            return (leftreg);
    case A_DEREF:

        //  return (cgderef(leftreg, n->left->type));

        if (n->rvalue)
            return cgderef(leftreg, n->left->type);
        else
            return (leftreg);

    case A_SCALE:
        // Small optimisation: use shift if the
        // scale value is a known power of two
        switch (n->a_size)
        {
        case 2:
            return(cgshlconst(leftreg, 1));
        case 4:
            return(cgshlconst(leftreg, 2));
        case 8:
            return(cgshlconst(leftreg, 3));
        default:
            // 分配数组int a[10]  int * 10
            rightreg = cgloadint(n->a_size, P_INT);
            return cgmul(leftreg, rightreg); //l = l * r;
        }
    case A_POSTINC:
        // Load the variable's value into a register,
        // then increment it
#if 0

        if (n->sym->class == C_GLOBAL || n->sym->class == C_STATIC)
            return (cgloadglob(n->sym, n->op));
        else
            return (cgloadlocal(n->sym, n->op));

#endif // 0
        // Load and decrement the variable's value into a register
      // and post increment/decrement it
        return (cgloadvar(n->sym, n->op));

    case A_POSTDEC:

#if 0
        // Load the variable's value into a register,
               // then decrement it
        if (n->sym->class == C_GLOBAL || n->sym->class == C_STATIC)
            return (cgloadglob(n->sym, n->op));
        else
            return (cgloadlocal(n->sym, n->op));

#endif // 0

        // Load and decrement the variable's value into a register
   // and post increment/decrement it
        return (cgloadvar(n->sym, n->op));
    case A_PREINC:
#if 0
        // Load and increment the variable's value into a register
        if (n->left->sym->class == C_GLOBAL || n->left->sym->class == C_STATIC)
            return (cgloadglob(n->left->sym, n->op));
        else
            return (cgloadlocal(n->left->sym, n->op));
#endif // 0
        // Load and decrement the variable's value into a register
     // and pre increment/decrement it
        return (cgloadvar(n->left->sym, n->op));

    case A_PREDEC:
#if 0
        // Load and decrement the variable's value into a register
        if (n->left->sym->class == C_GLOBAL || n->left->sym->class == C_STATIC)
            return (cgloadglob(n->left->sym, n->op));
        else
            return (cgloadlocal(n->left->sym, n->op));
#endif // 0

        // Load and decrement the variable's value into a register
       // and pre increment/decrement it
        return (cgloadvar(n->left->sym, n->op));
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
    case A_CAST:
        return (leftreg);		// Not much to do
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
void genfreeregs(int keepreg)
{

    freeall_registers(keepreg);

}

#if 0
void genprintint(int reg)
{
    cgprintint(reg);
}
#endif
void genglobsym(struct symtable* node)
{
    cgglobsym(node);
}
// 生成字符串的汇编代码
// If append is true, append to
// previous genglobstr() call.
int genglobstr(char* strvalue, int append)
{
    int l = genlabel();
    cgglobstr(l, strvalue, append);
    return (l);
}

// char *c= "sa" "asasas"; 
void genglobstrend(void)// 字符拼接末尾标识
{
    cgglobstrend();
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