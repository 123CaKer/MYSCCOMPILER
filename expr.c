#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
// AST ����
// ���������ȼ�  
static int OpPrec[] = 
{
  0, 10, 20, 30,		// T_EOF, T_ASSIGN, T_LOGOR, T_LOGAND
  40, 50, 60,			// T_OR, T_XOR, T_AMPER 
  70, 70,			// T_EQ, T_NE
  80, 80, 80, 80,		// T_LT, T_GT, T_LE, T_GE
  90, 90,			// T_LSHIFT, T_RSHIFT
  100, 100,			// T_PLUS, T_MINUS
  110, 110			// T_STAR, T_SLASH
};

// �ݹ��½��﷨
#if 0
primary_expression
    : T_IDENT
    | T_INTLIT
    | T_STRLIT
    | '(' expression ')'
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' expression ')'
    | postfix_expression '++'
    | postfix_expression '--'
    ;

prefix_expression
    : postfix_expression
    | '++' prefix_expression
    | '--' prefix_expression
    | prefix_operator prefix_expression
    ;

prefix_operator
    : '&'
    | '*'
    | '-'
    | '~'
    | '!'
    ;

multiplicative_expression
    : prefix_expression
    | multiplicative_expression '*' prefix_expression
    | multiplicative_expression '/' prefix_expression
    | multiplicative_expression '%' prefix_expression
    ;

etc.
#endif

// expression_list: <null>
//        | expression
//        | expression ',' expression_list
//        ;
// ���ƺ�������ɨ�裬ֻ�������任�ɱ��ʽ

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
static struct ASTnode* expression_list(void)
{
    struct ASTnode* tree = NULL;
    struct ASTnode* child = NULL;
    int exprcount = 0;

    // Loop until the final right parentheses
    while (Token.token != T_RPAREN) 
    {

        // Parse the next expression and increment the expression count
        child = binexpr(0);
        exprcount++;

        // Build an A_GLUE AST node with the previous tree as the left child
        // and the new expression as the right child. Store the expression count.
        tree = mkastnode(A_GLUE, P_NONE, tree, NULL, child, NULL, exprcount);// �ұ�����expr

        // �ж���һ���Ƿ�Ϊ�� ���ߣ�
        switch (Token.token)
        {
        case T_COMMA:
            scan(&Token);
            break;
        case T_RPAREN:
            break;
        default:
            fatald("Unexpected token in expression list", Token.token);
        }
    }

    //�������ձ��ʽ
    return (tree);
}



// ����ǰ׺���ʽ������һ����Ӧ��AST����
/*
   int *a;
    x=&b;
   */
struct ASTnode* prefix()
{
    struct ASTnode* tree;
    switch (Token.token)
    {
    case T_AMPER:  //& x=&b;
        scan(&Token);
        tree = prefix();

        if (tree->op != A_IDENT)
            fatal("& operator must be followed by an identifier");

        // Now change the operator to A_ADDR and the type to
        // a pointer to the original type
        tree->op = A_ADDR;
        tree->type = pointer_to(tree->type);
        break;
    case T_STAR: // *
        scan(&Token);
        tree = prefix();

        // *p  ************p
        if (tree->op != A_IDENT && tree->op != A_DEREF)//* operator must be followed by an identifier or *
            fatal("* operator must be followed by an identifier or *");

        tree = mkastunary(A_DEREF, value_at(tree->type), tree, NULL, 0);// �������
        break;


    case T_MINUS:  // ����
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // Prepend a A_NEGATE operation to the tree and
        // make the child an rvalue. Because chars are unsigned,
        // also widen this to int so that it's signed
        tree->rvalue = 1;
        tree = modify_type(tree, P_INT, 0);
        tree = mkastunary(A_NEGATE, tree->type, tree, NULL, 0);
        break;


    case T_INVERT: //��λȡ��
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // �����ӽڵ�
        tree->rvalue = 1;
        tree = mkastunary(A_INVERT, tree->type, tree, NULL, 0);
        break;
    case T_LOGNOT:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // Prepend a A_LOGNOT operation to the tree and
        // make the child an rvalue.
        tree->rvalue = 1;
        tree = mkastunary(A_LOGNOT, tree->type, tree, NULL, 0);
        break;
    case T_INC: // ����
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // For now, ensure it's an identifier
        if (tree->op != A_IDENT)
            fatal("++ operator must be followed by an identifier");

        // Prepend an A_PREINC operation to the tree
        tree = mkastunary(A_PREINC, tree->type, tree, NULL, 0);
        break;
    case T_DEC: // �Լ�
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // For now, ensure it's an identifier
        if (tree->op != A_IDENT)
            fatal("-- operator must be followed by an identifier");

        // Prepend an A_PREDEC operation to the tree
        tree = mkastunary(A_PREDEC, tree->type, tree, NULL, 0);
        break;
    default:
        tree = primary();
    }
    return tree;
}

// ������׺���ʽ������һ����Ӧ��AST����
// Parse a postfix expression and return
// an AST node representing it. The
// identifier is already in Text.
static struct ASTnode* postfix(void)
{
    struct ASTnode* n;
    struct symtable* varptr; // ���ű�ָ��
    struct symtable* enumptr;

    // If the identifier matches an enum value,
 // return an A_INTLIT node
    if ((enumptr = findenumval(Text)) != NULL)
        // ���Ѱ��ֵ�ҵ���
    {
        scan(&Token);// �Թ�
        return (mkastleaf(A_INTLIT, P_INT, NULL, enumptr->posn));
    }


    // Scan in the next token to see if we have a postfix expression
    scan(&Token);

    // ��������
    if (Token.token == T_LPAREN)
        return (funccall());

    // ���з���
    if (Token.token == T_LBRACKET)
        return (array_access());


    // Access into a struct or union
    if (Token.token == T_DOT)
        return (member_access(0));
    if (Token.token == T_ARROW)
        return (member_access(1));



    // A variable. Check that the variable exists.
    if ((varptr = findsymbol(Text)) == NULL || varptr->stype != S_VARIABLE)
        fatals("Unknown variable", Text);

    switch (Token.token)
    {
        // Post-increment: skip over the token
    case T_INC:
        scan(&Token);
        n = mkastleaf(A_POSTINC, varptr->type, varptr, 0);
        break;

        // Post-decrement: skip over the token
    case T_DEC:
        scan(&Token);
        n = mkastleaf(A_POSTDEC, varptr->type, varptr, 0);
        break;

        // Just a variable reference
    default:
        n = mkastleaf(A_IDENT, varptr->type, varptr, 0);
    }
    return (n);

}
//�� ����ת��ΪAST��Ӧ����
int arithop(int tokentype)
{
# if 0
    switch (tokentype)
    {
    case T_PLUS:
        return A_ADD;
    case T_MINUS:
        return A_SUBTRACT;
    case T_STAR:
        return A_MULTIPLY;
    case T_SLASH:
        return A_DIVIDE;
    case T_EQ:
        return A_EQ;
    case T_NE:
        return A_NE;
    case T_GE:
        return A_GE;
    case T_GT:
        return A_GT;
    case T_LE:
        return A_LE;
    case T_LT:
        return A_LT;
    case T_ASSIGN:
        return A_ASSIGN;
   









    default:
        fatald("Syntax error, token", tokentype);
    }
#endif

    if (tokentype > T_EOF && tokentype <= T_SLASH)
        return (tokentype);
    fatald("Syntax error, token", tokentype);
    return (0);			

}

// ��ȡ��������ȼ�
static int op_precedence(int tokentype)
{
    int prec;
    if (tokentype > T_SLASH)
        fatald("Token with no precedence in op_precedence:", tokentype);
    prec = OpPrec[tokentype];
    if (prec == 0)
        fatald("Syntax error, token", tokentype);
    return (prec);
}

//�ж��Ƿ�Ϊ�ҽ��ֵ   ���ҵ���ֵ z=2��
static int rightassoc(int tokentype)
{
    if (tokentype == T_ASSIGN)
        return 1;
    return 0;
}

// ����AST�﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�
struct ASTnode* binexpr(int p)
{
    struct ASTnode* left, * right;
    struct ASTnode* ltemp, * rtemp;
    int ASTop;
    int tokentype;
    // ��ȡ�������������� 
    left = prefix();


    /*
               +
             /   \
            4     +
                 /   \          4+4+4 T_EOF
                 4     4
                      /
                    T_EOF
    */

    if (Token.token == T_EOF || Token.token == T_SEMI ||
        Token.token == T_RPAREN|| Token.token == T_RBRACKET || 
        Token.token == T_COMMA|| Token.token == T_COLON)// ƥ�� ������

    {
        /*  AST��
      *
      *          A_IF
      *     /          \
      * (condition)
      *
      */
        left->rvalue = 1;
        return left;

    }

    tokentype = Token.token;


    // ��һ������Ϊ���㣨�����ȼ����ڶ�������Ϊ��ֵ
    while (op_precedence(tokentype) > p || ((op_precedence(tokentype) == p) && rightassoc(tokentype)))
    {
        //���õ�ǰToken����
        scan(&Token);

        // �ҵݹ��������������
        right = binexpr(OpPrec[tokentype]);


        ASTop = arithop(tokentype);
        if (ASTop == A_ASSIGN)
        {

            right->rvalue = 1;

            //ƥ��
            right = modify_type(right, left->type, 0);
            if (left == NULL)
                fatal("Incompatible expression in assignment");


            // ȷ�� ���ʽ��������
            ltemp = left;
            left = right;
            right = ltemp;

        }
        else
        {


            // ������������

            /*
              +
            /   \           �ڽ����ж�ʱ��Ҫ���䵱ǰΪ��������
        int 2   char 5

              +
            /   \           �ڽ����ж�ʱ��Ҫ���䵱ǰΪ��������
        char 2   int 5

        ������ʵ������У���Ϊ���� ����Ҫ��������ƥ��
      */
            left->rvalue = 1;
            right->rvalue = 1;


            ltemp = modify_type(left, right->type, ASTop);  // ��������
            rtemp = modify_type(right, left->type, ASTop);  // ��������
            if (ltemp == NULL && rtemp == NULL)
                fatal("Incompatible types in binary expression");
            if (ltemp != NULL)
                left = ltemp;
            if (rtemp != NULL)
                right = rtemp;



        }
        // ����ast�ڵ� ��֤һ��
        left = mkastnode(arithop(tokentype), left->type, left, NULL, right, NULL, 0);


        tokentype = Token.token;  // ���� token����
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN 
            || Token.token == T_RBRACKET || Token.token == T_COMMA || Token.token == T_COLON)// ƥ�� ������
        {
            left->rvalue = 1; //�������Ϊ��ֵ
            return left;
        }


    }
    left->rvalue = 1;
    return left; //���ش���
}

static struct ASTnode* funccall()
{
    struct ASTnode* tree;
    struct symtable* funcptr;

    // Check that the identifier has been defined as a function,
    // then make a leaf node for it.
    if ((funcptr = findsymbol(Text)) == NULL || funcptr->stype != S_FUNCTION)
    {
        fatals("Undeclared function", Text);
    }
    // Get the '('
    lparen();

    // �������ʽ����
    tree = expression_list();

    // XXX Check type of each argument against the function's prototype

    // Build the function call AST node. Store the
    // function's return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, funcptr->type, tree, funcptr, 0);

    // Get the ')'
    rparen();
    return (tree);
}

// Parse the index into an array and
// return an AST tree for it
static struct ASTnode* array_access(void) 
{
    struct ASTnode* left, * right;
    struct symtable* aryptr;

     // �ж��Ƿ���array
    // Check that the identifier has been defined as an array
    // then make a leaf node for it that points at the base
    if ((aryptr = findsymbol(Text)) == NULL || aryptr->stype != S_ARRAY)
    {
        fatals("Undeclared array", Text);
    }
    left = mkastleaf(A_ADDR, aryptr->type, aryptr, 0);

    // Get the '['
    scan(&Token);

    // Parse the following expression
    right = binexpr(0);

    // Get the ']'
    match(T_RBRACKET, "]");

    // Ensure that this is of int type
    if (!inttype(right->type))
        fatal("Array index is not of integer type");

    // Scale the index by the size of the element's type
    right = modify_type(right, left->type, A_ADD);

    // Return an AST tree where the array's base has the offset
    // added to it, and dereference the element. Still an lvalue
    // at this point.
    left = mkastnode(A_ADD, aryptr->type, left, NULL, right, NULL, 0);
    left = mkastunary(A_DEREF, value_at(left->type), left, NULL, 0);
    return (left);
}



// �ṹ���Ա���ʣ����withpointer==1��Ϊ->����
 /*
            A_DEREF
           /     \     t->a t.a == *��t+a��λ�ã� ���ÿ�����union ���� struct
         A_ADD
       /      \
  A_IDENT��  A_INTLIT m->posn
    A_ADDR

    ������������AST
    */

 struct ASTnode* member_access(int withpointer) 
{
    struct ASTnode* left, * right;
    struct symtable* compvar;
    struct symtable* typeptr;
    struct symtable* m;

    // Check that the identifier has been declared as a
    // struct/union or a struct/union pointer
    if ((compvar = findsymbol(Text)) == NULL)
        fatals("Undeclared variable", Text);
    if (withpointer && compvar->type != pointer_to(P_STRUCT)
        && compvar->type != pointer_to(P_UNION))
        fatals("Undeclared variable", Text);
    if (!withpointer && compvar->type != P_STRUCT && compvar->type != P_UNION)
        fatals("Undeclared variable", Text);

    // If a pointer to a struct/union, get the pointer's value.
    // Otherwise, make a leaf node that points at the base
    // Either way, it's an rvalue
    if (withpointer) {
        left = mkastleaf(A_IDENT, pointer_to(compvar->type), compvar, 0);
    }
    else
        left = mkastleaf(A_ADDR, compvar->type, compvar, 0);
    left->rvalue = 1;

    // Get the details of the composite type
    typeptr = compvar->ctype;

    // Skip the '.' or '->' token and get the member's name
    scan(&Token);
    ident();

    // Find the matching member's name in the type
    // Die if we can't find it
    for (m = typeptr->member; m != NULL; m = m->next)
        if (!strcmp(m->name, Text))
            break;

    if (m == NULL)
        fatals("No member found in struct/union: ", Text);

    // Build an A_INTLIT node with the offset
    right = mkastleaf(A_INTLIT, P_INT, NULL, m->posn);

    // Add the member's offset to the base of the struct/union
    // and dereference it. Still an lvalue at this point
    left = mkastnode(A_ADD, pointer_to(m->type), left, NULL, right, NULL, 0);
    left = mkastunary(A_DEREF, m->type, left, NULL, 0);
    return (left);
}



 struct ASTnode* primary()
 {
     struct ASTnode* n = NULL;
     int id;
     // ��token����ΪT_INTLIT ��Ϊ ASTҶ�ӽڵ� �����쳣
     switch (Token.token)
     {

     case T_STRLIT:
         // For a STRLIT token, generate the assembly for it.
         // Then make a leaf AST node for it. id is the string's label.
         id = genglobstr(Text); // ���� ���
         n = mkastleaf(A_STRLIT, pointer_to(P_CHAR), NULL, id); // ����Ҷ�ӽڵ�
         break;

     case T_INTLIT: //����ֵ

         if ((Token.intvalue) >= 0 && (Token.intvalue < 256))//char��
             n = mkastleaf(A_INTLIT, P_CHAR, NULL, Token.intvalue);
         else                                                // int��
             n = mkastleaf(A_INTLIT, P_INT, NULL, Token.intvalue);
         break;

     case T_IDENT:

         return postfix(); // ��׺���ʽ

     case T_LPAREN:
         // Beginning of a parenthesised expression, skip the '('.
         // Scan in the expression and the right parenthesis
         scan(&Token);
         n = binexpr(0); // ���ȼ��㣨���ڵ�
         rparen(); // ƥ��
         return (n);

     default:
         fatald("Expecting a primary expression, got token", Token.token);

     }

     scan(&Token);//ɨ����һ�����Ʋ�����Ҷ�ӽڵ�
     return n;
 }
