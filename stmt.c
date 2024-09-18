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
struct ASTnode* assignment_statement()
{
    struct ASTnode* left, * right, * tree;
    int lefttype;
    int righttype;
    int id;
    
    ident(); // 匹配标识符
     
    if (Token.token == T_LBRACE)  //函数调用 下面正常执行
        return funccall();

   // reject_token(&Token);

    id = findglob(Text);  // 返回标识符在符号表下标
    if (id == -1)
    {
        fatals("Undeclared variable", Text);
    }

    right = mkastleaf(A_LVIDENT,Gsym[id].type, id);  //s生成右边赋值

    // 匹配等号 =
    match(T_ASSIGN, "=");
 
    // 生成ast
    left = binexpr(0);

    lefttype = left->type;
    righttype = right->type;
    if (!type_compatible(&lefttype, &righttype, 1))  
        fatal("Incompatible types");

    if (lefttype==A_WIDEN)
    {
        left = mkastunary(lefttype,right->type,left,0);// 节点类型扩充为 右节点类型
    }

    /*
        A_WIDEN   -----intvalue = 0
       /       \           right =left
    left       right
    */
   
    // 生成赋值ast
    tree = mkastnode(A_ASSIGN, P_INT, left, NULL,right, 0);// 强制转型为P_INT 最大
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

    lefttype = P_INT;
    righttype = n->type;
    if (!type_compatible(&lefttype, &righttype, 0)) //c此处并没有left树 仅为匹配函数type_compatible
        fatal("Incompatible types");

    if (righttype==A_WIDEN)
        n = mkastunary(righttype, P_INT, n, 0);

    // 生成
    n = mkastunary(A_PRINT, P_NONE, n, 0);
  
    genfreeregs();
    //semi();  
    return n;

}



struct ASTnode *  if_statement() 
{

    struct ASTnode* condAST, * trueAST, * falseAST = NULL;

    int lefttype, righttype;

    match(T_IF, "if"); //匹配if
    lparen(); // 匹配 (
    condAST = binexpr(0); // 生成条件AST
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");

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
    return mkastnode(A_IF,P_NONE ,condAST, trueAST, falseAST, 0);
}


struct ASTnode* while_statement()
{
    struct ASTnode* condAST=NULL;
    struct ASTnode* bodyAST = NULL;
    struct ASTnode* ASTn = NULL;
   
    match(T_WHILE, "while"); // 匹配while 关键字
    lparen();// 匹配 （

    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");

    // 判断部分不涉及类型转换
    rparen();// 匹配 )

    bodyAST = compound_statement();// {  ... }
    ASTn = mkastnode(A_WHILE,P_NONE,condAST,NULL,bodyAST,0);
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
        fatal("Bad comparison operator");

    // 判断部分不涉及类型转换
    semi();
    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement(); // 分号当前不处理

   
    tree = mkastnode(A_GLUE, P_NONE,  bodyAST, NULL, postopAST, 0);
    tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, 0);
    return mkastnode(A_GLUE,  P_NONE, preopAST, NULL, tree, 0);
}

 static struct ASTnode* return_statement()
 {
     struct ASTnode* tree;
     int returntype;
     int  functype;

     if (Gsym[Functionid].type == P_VOID)
         fatal("Can't return from a void function");

     match(T_RETURN,"return");
     lparen();

     tree = binexpr(0);
     functype = Gsym[Functionid].type;
     returntype = tree->type;
     if (type_compatible(&returntype, &functype,1)==0)
         fatal("Incompatible types");

     if (returntype==A_WIDEN)
         tree = mkastunary(returntype, functype, tree, 0);

     rparen();


     // 最终生成A_RETURN
     tree = mkastunary(A_RETURN, functype, tree, 0);
     return tree;
 }

struct ASTnode* single_statement() 
{
    switch (Token.token)
    {
    case T_PRINT:
        return print_statement();
    case T_INT:
        var_declaration();
        return NULL;
    case T_CHAR:
        var_declaration();
        return NULL;
    case T_LONG:
        var_declaration();
        return NULL;
    case T_IDENT:
        return assignment_statement();
    case T_IF:
        return if_statement();
    case T_WHILE:
        return while_statement();
    case T_FOR:
        return for_statement();
    case T_RETURN:
        return return_statement();
    default:
        fatald("Syntax error, token", Token.token);
    }
}



// 解析多个语句 （BNF）
struct ASTnode* compound_statement()
{
    struct ASTnode* left = NULL;
    struct ASTnode* tree = NULL;


    lbrace(); // 匹配左大括号
    while (1) 
    {
   
        tree = single_statement();

       
        if (tree != NULL && (tree->op == A_PRINT || tree->op == A_ASSIGN|| tree->op==A_RETURN))
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

