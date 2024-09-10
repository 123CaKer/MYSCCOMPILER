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
    int id;
    
    ident(); // 匹配标识符

    id = findglob(Text);  // 返回标识符在符号表下标
    if (id == -1)
    {
        fatals("Undeclared variable", Text);
    }
    right = mkastleaf(A_LVIDENT, id);

    // 匹配等号 =
    match(T_ASSIGN, "=");

    // 生成ast
    left = binexpr(0);

    // 生成赋值ast
    tree = mkastnode(A_ASSIGN, left, NULL,right, 0);//r=f
    genfreeregs();
   // semi();
    return tree;
}

 


struct ASTnode* print_statement()
{
    struct ASTnode* tree;
    struct ASTnode* n;
    int reg;
    //匹配第一个为print
    match(T_PRINT, "print");

    // 生成汇编代码
    n = binexpr(0);
    tree = mkastunary(A_PRINT, n, 0);
    genfreeregs();
    //semi();  
    return tree;

}



struct ASTnode *  if_statement() 
{

    struct ASTnode* condAST, * trueAST, * falseAST = NULL;
    match(T_IF, "if"); //匹配if
    lparen(); // 匹配 (
    condAST = binexpr(0); // 生成条件AST 包括 )
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
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
    return mkastnode(A_IF, condAST, trueAST, falseAST, 0);
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
    rparen();// 匹配 )

    bodyAST = compound_statement();// {  ... }
    ASTn = mkastnode(A_WHILE,condAST,NULL,bodyAST,0);
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
    semi();
    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement(); // 分号当前不处理

   
    tree = mkastnode(A_GLUE, bodyAST, NULL, postopAST, 0);
    tree = mkastnode(A_WHILE, condAST, NULL, tree, 0);
    return mkastnode(A_GLUE, preopAST, NULL, tree, 0);
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
    case T_IDENT:
        return assignment_statement();
    case T_IF:
        return if_statement();
    case T_WHILE:
        return while_statement();
    case T_FOR:
        return for_statement();
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

       
        if (tree != NULL && (tree->op == A_PRINT || tree->op == A_ASSIGN))
            semi();

        if (tree != NULL) 
        {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, left, NULL, tree, 0);
        }
       
        if (Token.token == T_RBRACE)
        {
            rbrace();
            return left;
        }
    }
}

