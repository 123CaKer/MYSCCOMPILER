#pragma once
#include"defs.h"
#include<stdlib.h>
 

// misc.c
void semi(void);
// 声明变量
// 匹配关键字
void match(int t, char* what);
// 匹配分号
void semi(void);
// 匹配非关键字
void ident(void);
// 错误信息
void fatal(char* s);
void fatals(char* s1, char* s2);
void fatald(char* s, int d);
void fatalc(char* s, int c);





int scan(struct token* t); // 判断令牌内容
struct ASTnode* mkastnode(int op, int type, struct ASTnode* left, struct ASTnode* mid, struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int type, int intvalue);// 生成叶子节点
struct ASTnode* mkastunary(int op, int type, struct ASTnode* left, int intvalue);// 生成左子树AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);

// cg.c
//void generatecode(struct ASTnode* n);
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgloadint(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);
int cgloadglob(int id);
// Store a register's value into a variable
int cgstorglob(int r, int id);
//生成全局符号
void genglobsym(int id);
void cgfuncpreamble(int);
void cgfuncpostamble(int id);
void cgreturn(int reg, int id);
void reject_token(struct token* t);


// 判断
int cgequal(int r1, int r2);
int cgnotequal(int r1, int r2);
int cglessthan(int r1, int r2);
int cggreaterthan(int r1, int r2);
int cglessequal(int r1, int);
int cggreaterequal(int r1, int r2);

void cglabel(int l);
void cgjump(int l);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);



//decl.c
// 声明变量
void var_declaration(int type);

//expr.c
//将 表达式符号转换为AST对应符号
int arithop(int tokentype);
// 获取运算符优先级
static int op_precedence(int tokentype);
// 生成语法树 返回root为+ - * /的ast树  其中p为之前的优先级
struct ASTnode* binexpr(int p);

struct ASTnode* funccall(void); //函数调用


//expr2.c
// 返回乘性表达式
//struct ASTnode* multiplicative_expr(void);
// 加性表达式
//struct ASTnode* additive_expr(void);


// gen.c
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(int id);
int genprimsize(int type);


//interp.c
// 解释AST值   可改为汇编接口代码在gen.c
//int interpretAST(struct ASTnode* n);


// stmt.c
struct ASTnode* print_statement();
struct ASTnode* assignment_statement();
struct ASTnode* if_statement();

struct ASTnode* for_statement();
struct ASTnode* single_statement();
struct ASTnode* compound_statement();
struct ASTnode* while_statement();


// sym.c
int findglob(char* s);
int newglob(void);
int addglob(char* name, int type, int stype,int endlabel);

// types.c
int type_compatible(int* left, int* right, int onlyright);

// gen.c中的静态代码对应的是汇编中的 Label:
//下标识
struct ASTnode* primary();//解析 token 并判断其对应的ASTNode （语义）
// static 每个文件
static struct ASTnode* primary()
{
    struct ASTnode* n = NULL;
    int id;
    // 将token类型为T_INTLIT 变为 AST叶子节点 否则异常
    switch (Token.token)
    {
    case T_INTLIT: //值

        if ((Token.intvalue) >= 0 && (Token.intvalue < 256))//char型
            n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
        else                                                // int型
            n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
        break;

    case T_IDENT:
        id = findglob(Text);
        if (id == -1)
            fatals("Unknown variable", Text);
        scan(&Token);
        if (Token.token == T_LPAREN)
            return funccall();
        reject_token(&Token);


        n = mkastleaf(A_IDENT, Gsym[id].type, id);
        break;
    default:
        fatald("Syntax error, token", Token.token);
    }

    scan(&Token);//扫描下一个令牌并返回叶子节点
    return n;
}


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
    genAST(n->mid, NOREG, n->op);
    genfreeregs();


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //若假存在 生成假分支语句 并跳转
    if (n->right)
    {
        genAST(n->right, NOREG, n->op);
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

    genAST(n->right, NOREG, n->op);
    genfreeregs();

    cgjump(Lstart);
    cglabel(Lend);  // 跳出while
    return NOREG;




}


// interpretAST的汇编接口版本
static int genAST(struct ASTnode* n, int reg, int parentASTop)  // reg为最近使用寄存器对应下标
{
    int  leftreg;
    int  midreg;
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
        genAST(n->left, NOREG, n->op);
        genfreeregs();
        genAST(n->right, NOREG, n->op);
        genfreeregs();
        return NOREG;


    case A_FUNCTION:
        cgfuncpreamble(n->v.id);  // 类似之前的cgpreamble生成函数前置码
        genAST(n->left, NOREG, n->op);
        cgfuncpostamble(n->v.id); // 类似之前的cgpostamble生成函数前置码
        return NOREG;

    }

    if (n->left)
        leftreg = genAST(n->left, NOREG, n->op);
    if (n->right)
        rightreg = genAST(n->right, leftreg, n->op);

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
        return cgloadint(n->v.intvalue); // 返回分配的寄存器下标号
    case A_IDENT:
        return cgloadglob(n->v.id);
    case A_LVIDENT:
        // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, n->v.id);

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
        // 当前表达式已经执行完成，需要返回
        return rightreg; // 返回寄存器结果下标值
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
        return (cgderef(leftreg, n->left->type));
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
}