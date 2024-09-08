#include "defs.h"
#include "data.h"
#include "decl.h"
// ���ļ��������Ϊ�ڵ����AST������


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
    
    ident(); // ƥ���ʶ��

    id = findglob(Text);  // ���ر�ʶ���ڷ��ű��±�
    if (id == -1)
    {
        fatals("Undeclared variable", Text);
    }
    right = mkastleaf(A_LVIDENT, id);

    // ƥ��Ⱥ� =
    match(T_ASSIGN, "=");

    // ����ast
    left = binexpr(0);

    // ���ɸ�ֵast
    tree = mkastnode(A_ASSIGN, left, NULL,right, 0);//r=f
    genfreeregs();
    semi();
    return tree;
}




struct ASTnode* print_statement()
{
    struct ASTnode* tree;
    struct ASTnode* n;
    int reg;
    //ƥ���һ��Ϊprint
    match(T_PRINT, "print");

    // ���ɻ�����
    n = binexpr(0);
    tree = mkastunary(A_PRINT, n, 0);
    genfreeregs();
    semi();  
    return tree;

}



struct ASTnode *  if_statement() 
{

    struct ASTnode* condAST, * trueAST, * falseAST = NULL;
    match(T_IF, "if"); //ƥ��if
    lparen(); // ƥ�� (
    condAST = binexpr(0); // ��������AST ���� )
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
    rparen();// ƥ�� )

    
    trueAST = compound_statement(); // ��ȡ�������
    // If we have an 'else', skip it
    // and get the AST for the compound statement
    if (Token.token == T_ELSE)
    {
        scan(&Token);
        falseAST = compound_statement();
    }

    // ����AST�ڵ�
    return mkastnode(A_IF, condAST, trueAST, falseAST, 0);
}



// ���������� ��BNF��
struct ASTnode* compound_statement()
{
    struct ASTnode* left = NULL;
    struct ASTnode* tree = NULL;


    lbrace(); // ƥ���������
    while (1)
    {
        switch (Token.token)
        {
        case T_PRINT: // print
            tree = print_statement();
            break;
        case T_INT:  // int
            var_declaration();
            tree = NULL;
            break;
        case T_IDENT: // ��ֵ ������ʽ
            tree = assignment_statement();
            break;
        case T_IF:
            tree = if_statement();
            break;
        case T_RBRACE: // ƥ���Ҵ�����
            rbrace();
            return left;
        case T_EOF:  // ����ɨ��ֱ���ļ�β��
            return NULL;
        default:
            fatald("Syntax error, token", Token.token);
        }

        if (tree!=NULL)
        {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, left, NULL, tree, 0);
        }

    }
}