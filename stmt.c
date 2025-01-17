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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// �˴��Ǳ�֤�� if(x) {stmt}
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
    return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, NULL, 0);
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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// �˴��Ǳ�֤�� if(x) {stmt}
    // ����xΪ ���ʽ

    // �жϲ��ֲ��漰����ת��
    rparen();// ƥ�� )

    Looplevel++;// ѭ���ȼ�+1
    bodyAST = compound_statement();// {  ... }
    Looplevel--;
    ASTn = mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, NULL, 0);
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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// �˴��Ǳ�֤�� if(x) {stmt}
  // ����xΪ ���ʽ

    // �жϲ��ֲ��漰����ת��
    semi();
    postopAST = single_statement();
    rparen();

    Looplevel++;// ѭ���ȼ�+1
    bodyAST = compound_statement(); // �ֺŵ�ǰ������
    Looplevel--;

    tree = mkastnode(A_GLUE, P_NONE, bodyAST, NULL, postopAST, NULL, 0);;
    tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, NULL, 0);
    return mkastnode(A_GLUE, P_NONE, preopAST, NULL, tree, NULL, 0);
}

static struct ASTnode* return_statement()
{
    struct ASTnode* tree;
    int returntype;
    int  functype;

    if (Functionid->type == P_VOID)
        fatal("Can't return from a void function");

    match(T_RETURN, "return");
    lparen();

    tree = binexpr(0);
    functype = Functionid->type;
    // returntype = tree->type;
    tree = modify_type(tree, functype, 0);
    if (tree == NULL)
    {
        fatal("Incompatible expression in return");
    }

    rparen();


    // ��������A_RETURN
    tree = mkastunary(A_RETURN, P_NONE, tree, NULL, 0);
    return tree;
}

// break_statement: 'break' ;
//
// Parse a break statement and return its AST
static struct ASTnode* break_statement(void)
{

    if (Looplevel == 0 && Switchlevel == 0)
        fatal("no loop or switch to break out from");
    scan(&Token);
    return (mkastleaf(A_BREAK, 0, NULL, 0));

}

// continue_statement: 'continue' ;
//
// Parse a continue statement and return its AST
static struct ASTnode* continue_statement(void)
{

    if (Looplevel == 0)
        fatal("no loop to continue to");
    scan(&Token);
    return (mkastleaf(A_CONTINUE, 0, NULL, 0));
}

//����switch AST
/*
* 
*         A_SWITCH
*         /      \
      condition   A_CASE
                  /    \
                ���    A_CASE
                         /   \
                       ���  A_DEAFAULT
                              /       \
*                          Ĭ�����  NULL
* 
* 
*/
static struct ASTnode* switch_statement(void) {
    struct ASTnode* left,// switch����
        * n,  // 
        * c, 
        * casetree = NULL,
        * casetail=NULL;
    int inloop = 1,
        casecount = 0;// case���������
    int seendefault = 0;
    int ASTop, 
        casevalue;// case��ֵ

    // Skip the 'switch' and '('
    scan(&Token);
    lparen();

    // Get the switch expression, the ')' and the '{'
    left = binexpr(0);
    rparen();
    lbrace();

    // Ensure that this is of int type
    if (!inttype(left->type))
        fatal("Switch expression is not of integer type");

    // Build an A_SWITCH subtree with the expression as
    // the child
    n = mkastunary(A_SWITCH, 0, left, NULL, 0);

    // Now parse the cases
    Switchlevel++;
    while (inloop) {
        switch (Token.token) {
            // Leave the loop when we hit a '}'
        case T_RBRACE: if (casecount == 0)
            fatal("No cases in switch");
            inloop = 0; break;
        case T_CASE:
        case T_DEFAULT:
            // Ensure this isn't after a previous 'default'
            if (seendefault)
                fatal("case or default after existing default");

            // Set the AST operation. Scan the case value if required
            if (Token.token == T_DEFAULT) {
                ASTop = A_DEFAULT; seendefault = 1; scan(&Token);
            }
            else {
                ASTop = A_CASE; scan(&Token);
                left = binexpr(0);
                // Ensure the case value is an integer literal
                if (left->op != A_INTLIT)
                    fatal("Expecting integer literal for case value");
                casevalue = left->intvalue;

                // Walk the list of existing case values to ensure
                // that there isn't a duplicate case value
                for (c = casetree; c != NULL; c = c->right)
                    if (casevalue == c->intvalue)
                        fatal("Duplicate case value");
            }

            // Scan the ':' and get the compound expression
            match(T_COLON, ":");
            left = compound_statement(); casecount++;

            // Build a sub-tree with the compound statement as the left child
            // and link it in to the growing A_CASE tree
            if (casetree == NULL) {
                casetree = casetail = mkastunary(ASTop, 0, left, NULL, casevalue);
            }
            else {
                casetail->right = mkastunary(ASTop, 0, left, NULL, casevalue);
                casetail = casetail->right;
            }
            break;
        default:
            fatald("Unexpected token in switch", Token.token);
        }
    }
    Switchlevel--;

    // We have a sub-tree with the cases and any default. Put the
    // case count into the A_SWITCH node and attach the case tree.
    n->intvalue = casecount;
    n->right = casetree;
    rbrace();

    return(n);
}

struct ASTnode* single_statement()
{
    int type, class = C_LOCAL;
    struct symtable* ctype;

    switch (Token.token) 
    {
    case T_IDENT:
        // We have to see if the identifier matches a typedef.
        // If not do the default code in this switch statement.
        // Otherwise, fall down to the parse_type() call.
        if (findtypedef(Text) == NULL)
            return (binexpr(0));
    case T_CHAR:
    case T_INT:
    case T_LONG:
    case T_STRUCT:
    case T_UNION:
    case T_ENUM:
    case T_TYPEDEF:
        // The beginning of a variable declaration.
        // Parse the type and get the identifier.
        // Then parse the rest of the declaration
        // and skip over the semicolon
        type = parse_type(&ctype, &class);
        ident();
        var_declaration(type, ctype, class);
        semi();
        return (NULL);		// No AST generated here
    case T_IF:
        return (if_statement());
    case T_WHILE:
        return (while_statement());
    case T_FOR:
        return (for_statement());
    case T_RETURN:
        return (return_statement());
    case T_BREAK:
        return break_statement();
    case T_CONTINUE:
        return continue_statement();
    case T_SWITCH:
        return switch_statement();
    default:
        // For now, see if this is an expression.
        // This catches assignment statements.
        return (binexpr(0));
    }
    return (NULL);		// Keep -Wall happy
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


        if (tree != NULL && (tree->op == A_ASSIGN || tree->op == A_RETURN || tree->op == A_FUNCCALL
                                  || tree->op == A_BREAK|| tree->op == A_CONTINUE))
            semi();

        if (tree != NULL)
        {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, NULL, 0);
        }

        if (Token.token == T_RBRACE)
        {
            rbrace();
            return left;
        }
    }
}

