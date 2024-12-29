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

#if 0
// ���Ѹ���Ϊ ��ֵexpr
struct ASTnode* assignment_statement()
{
    struct ASTnode* left, * right, * tree;
    int lefttype;
    int righttype;
    int id;
    int rtype; // ������

    ident(); // ƥ���ʶ��

    if (Token.token == T_LPAREN)  //�������� ��������ִ��
        return funccall();

    // reject_token(&Token);

    id = findglob(Text);  // ���ر�ʶ���ڷ��ű��±�
    if (id == -1)
    {
        fatals("Undeclared variable", Text);
    }

    right = mkastleaf(A_LVIDENT, Gsym[id].type, id);  //s�����ұ߸�ֵ

    // ƥ��Ⱥ� =
    match(T_ASSIGN, "=");

    // ����ast
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

    // ���ɸ�ֵast
    tree = mkastnode(A_ASSIGN, P_INT, left, NULL, right, 0);// ǿ��ת��ΪP_INT ���
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

    //ƥ���һ��Ϊprint
    match(T_PRINT, "print");
    // ���ɼ�����AST
    n = binexpr(0);
    n = modify_type(n, P_INT, 0);
    if (n == NULL)
        fatal("Incompatible expression in print");


    // ����
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

    match(T_IF, "if"); //ƥ��if
    lparen(); // ƥ�� (
    condAST = binexpr(0); // ��������AST
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// �˴��Ǳ�֤�� if(x) {stmt}
    // ����xΪ ���ʽ
    // �жϲ��ֲ��漰����ת��


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
    return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, 0);
}


struct ASTnode* while_statement()
{
    struct ASTnode* condAST = NULL;
    struct ASTnode* bodyAST = NULL;
    struct ASTnode* ASTn = NULL;

    match(T_WHILE, "while"); // ƥ��while �ؼ���
    lparen();// ƥ�� ��

    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// �˴��Ǳ�֤�� if(x) {stmt}
    // ����xΪ ���ʽ

    // �жϲ��ֲ��漰����ת��
    rparen();// ƥ�� )

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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);// �˴��Ǳ�֤�� if(x) {stmt}
  // ����xΪ ���ʽ

    // �жϲ��ֲ��漰����ת��
    semi();
    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement(); // �ֺŵ�ǰ������


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


    // ��������A_RETURN
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
        var_declaration(type,1); // ������Ϊ��ǰ��chr 23�н����Ƿ��ű���� ��δ���ǵ�
        // ������Χ ���Ĭ��Ϊ1 ��ȫ�֣�
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



// ���������� ��BNF������
struct ASTnode* compound_statement()
{
    struct ASTnode* left = NULL;
    struct ASTnode* tree = NULL;


    lbrace(); // ƥ���������
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

