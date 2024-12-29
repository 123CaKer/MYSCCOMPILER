#include "defs.h"
#include "data.h"
#include "decl.h"
// 本文件将语句作为节点插入AST中形如


// compound_statement:          // empty, i.e. no statement
//      |      statement
//      |      statement statements
//      ;
//
// statement: print_statement
//      |     declaration
//      |     assignment_statement
//      |     if_statement
//      ;
//
// print_statement: 'print' expression ';'  ;
//
// declaration: 'int' identifier ';'  ;
//
// assignment_statement: identifier '=' expression ';'   ;
//
// if_statement: if_head
//      |        if_head 'else' compound_statement
//      ;
//
// if_head: 'if' '(' true_false_expression ')' compound_statement  ;
//
// identifier: T_IDENT ;

/*
      A_GLUE
     /      \
  A_IF      ......

*/

#if 0
// 现已更改为 赋值expr
struct ASTnode* assignment_statement()
{
    struct ASTnode* left, * right, * tree;
    int lefttype;
    int righttype;
    int id;
    int rtype; // 右类型

    ident(); // 匹配标识符

    if (Token.token == T_LPAREN)  //函数调用 下面正常执行
        return funccall();

    // reject_token(&Token);

    id = findglob(Text);  // 返回标识符在符号表下标
    if (id == -1)
    {
        fatals("Undeclared variable", Text);
    }

    right = mkastleaf(A_LVIDENT, Gsym[id].type, id);  //s生成右边赋值

    // 匹配等号 =
    match(T_ASSIGN, "=");

    // 生成ast
    left = binexpr(0);
    rtype = right->type;
    left = modify_type(left, rtype, 0);//assign
    if (left == NULL)
        fatal("Incompatible expression in assignment");

    /*
        A_WIDEN   -----intvalue = 0
       /       \           right =left
    left       right
    */

    // 生成赋值ast
    tree = mkastnode(A_ASSIGN, P_INT, left, NULL, right, 0);// 强制转型为P_INT 最大
//    genfreeregs();
   // semi();
    return tree;
}

struct ASTnode* print_statement()
{
    struct ASTnode* n;
    int reg;
    int righttype;
    int lefttype;

    //匹配第一个为print
    match(T_PRINT, "print");
    // 生成计算型AST
    n = binexpr(0);
    n = modify_type(n, P_INT, 0);
    if (n == NULL)
        fatal("Incompatible expression in print");


    // 生成
    n = mkastunary(A_PRINT, P_NONE, n, 0);

    genfreeregs();
    //semi();  
    return n;

}
#endif // 0





struct ASTnode* if_statement()
{

    struct ASTnode* condAST, * trueAST, * falseAST = NULL;

    int lefttype, righttype;

    match(T_IF, "if"); //匹配if
    lparen(); // 匹配 (
    condAST = binexpr(0); // 生成条件AST
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// 此处是保证了 if(x) {stmt}
    // 其中x为 表达式
    // 判断部分不涉及类型转换


    rparen();// 匹配 )


    trueAST = compound_statement(); // 获取条件语句
    // If we have an 'else', skip it
    // and get the AST for the compound statement
    if (Token.token == T_ELSE)
    {
        scan(&Token);
        falseAST = compound_statement();
    }

    // 生成AST节点
    return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, 0);
}


struct ASTnode* while_statement()
{
    struct ASTnode* condAST = NULL;
    struct ASTnode* bodyAST = NULL;
    struct ASTnode* ASTn = NULL;

    match(T_WHILE, "while"); // 匹配while 关键字
    lparen();// 匹配 （

    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// 此处是保证了 if(x) {stmt}
    // 其中x为 表达式

    // 判断部分不涉及类型转换
    rparen();// 匹配 )

    bodyAST = compound_statement();// {  ... }
    ASTn = mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, 0);
    return ASTn;

}

struct ASTnode* for_statement()
{
    struct ASTnode* condAST, * bodyAST;
    struct ASTnode* preopAST, * postopAST;
    struct ASTnode* tree;
    match(T_FOR, "for");
    lparen();
    preopAST = single_statement();
    semi();

    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// 此处是保证了 if(x) {stmt}
  // 其中x为 表达式

    // 判断部分不涉及类型转换
    semi();
    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement(); // 分号当前不处理


    tree = mkastnode(A_GLUE, P_NONE, bodyAST, NULL, postopAST, 0);
    tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, 0);
    return mkastnode(A_GLUE, P_NONE, preopAST, NULL, tree, 0);
}

static struct ASTnode* return_statement()
{
    struct ASTnode* tree;
    int returntype;
    int  functype;

    if (Gsym[Functionid].type == P_VOID)
        fatal("Can't return from a void function");

    match(T_RETURN, "return");
    lparen();

    tree = binexpr(0);
    functype = Gsym[Functionid].type;
    // returntype = tree->type;
    tree = modify_type(tree, functype, 0);
    if (tree == NULL)
    {
        fatal("Incompatible expression in return");
    }

    rparen();


    // 最终生成A_RETURN
    tree = mkastunary(A_RETURN, P_NONE, tree, 0);
    return tree;
}

struct ASTnode* single_statement()
{
    int type;
    switch (Token.token)
    {
        /*
    case T_PRINT:
        return print_statement();
        */
    case T_INT:
    case T_CHAR:
    case T_LONG:
        type = parse_type();
        ident();
        var_declaration(type,1); // 作者认为当前在chr 23中仅考虑符号表设计 并未考虑到
        // 作用域范围 因此默认为1 （全局）
        return NULL;

        /*
        case T_IDENT:
            return assignment_statement();
            */

    case T_IF:
        return if_statement();
    case T_WHILE:
        return while_statement();
    case T_FOR:
        return for_statement();
    case T_RETURN:
        return return_statement();
    default:
        return binexpr(0);
    }
}



// 解析多个语句 （BNF）语句块
struct ASTnode* compound_statement()
{
    struct ASTnode* left = NULL;
    struct ASTnode* tree = NULL;


    lbrace(); // 匹配左大括号
    while (1)
    {

        tree = single_statement();


        if (tree != NULL && (tree->op == A_ASSIGN || tree->op == A_RETURN || tree->op == A_FUNCCALL))
            semi();

        if (tree != NULL)
        {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, 0);
        }

        if (Token.token == T_RBRACE)
        {
            rbrace();
            return left;
        }
    }
}

