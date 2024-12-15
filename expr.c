#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
// AST ����
// ���������ȼ�  
static int OpPrec[] =
{
   0, 10,                       // T_EOF,  T_ASSIGN
  20, 20,                       // T_PLUS, T_MINUS
  30, 30,                       // T_STAR, T_SLASH
  40, 40,                       // T_EQ, T_NE
  50, 50, 50, 50                // T_LT, T_GT, T_LE, T_GE
};

// Parse a prefix expression and return 
// a sub-tree representing it.
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
    case T_STAR:
        scan(&Token);
        tree = prefix();

        // *p  ************p
        if (tree->op != A_IDENT && tree->op != A_DEREF)//* operator must be followed by an identifier or *
            fatal("* operator must be followed by an identifier or *");

        tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);// �������
        break;
    default:
        tree = primary();
    }
    return tree;
}

//�� ���ʽ����ת��ΪAST��Ӧ����
int arithop(int tokentype)
{

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
}

// ��ȡ��������ȼ�
static int op_precedence(int tokentype)
{
    int prec = OpPrec[tokentype];
    if (prec == 0)
    {
        fprintf(stderr, "syntax error  on line %d, token %d\n", Line, tokentype);
        exit(1);
    }
    return prec;
}

//�ж��Ƿ�Ϊ�ҽ��ֵ
static int rightassoc(int tokentype)
{
    if (tokentype == T_ASSIGN)
        return 1;
    return 0;
}

// �����﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�
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

    if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN|| Token.token == T_RBRACKET)// ƥ��)

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
        left = mkastnode(arithop(tokentype), left->type, left, NULL, right, 0);


        tokentype = Token.token;  // ���� token����
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN || Token.token == T_RBRACKET)// ƥ��)
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
    int id;

    // Check that the identifier has been defined,
    // then make a leaf node for it. XXX Add structural type test
    if ((id = findglob(Text)) == -1 || Gsym[id].stype != S_FUNCTION)
    {
        fatals("Undeclared function", Text);
    }
    //  ƥ��'('
    lparen();

    // Parse the following expression
    tree = binexpr(0);

    // Build the function call AST node. Store the
    // function's return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, Gsym[id].type, tree, id);

    rparen();
    return (tree);
}

// Parse the index into an array and
// return an AST tree for it
static struct ASTnode* array_access(void) 
{
    struct ASTnode* left, * right;
    int id;
     
      /*
            a[1] *(a+1)

                        A_DEREF
                       /
                     A_ADD
                    /     \
                (a)A_ADDR  1
                     
                     
      */

    // �ж��Ƿ�Ϊ����
    if ((id = findglob(Text)) == -1 || Gsym[id].stype != S_ARRAY)
    {
        fatals("Undeclared array", Text);
    }
    left = mkastleaf(A_ADDR, Gsym[id].type, id);

    // Get the '['
    scan(&Token);

    // �ұ�Ϊ���ʽ ��������
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
    left = mkastnode(A_ADD, Gsym[id].type, left, NULL, right, 0);
    left = mkastunary(A_DEREF, value_at(left->type), left, 0);
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
         n = mkastleaf(A_STRLIT, P_CHARPTR, id);// ����Ҷ�ӽڵ�
         break;

     case T_INTLIT: //����ֵ

         if ((Token.intvalue) >= 0 && (Token.intvalue < 256))//char��
             n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
         else                                                // int��
             n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
         break;

     case T_IDENT:

         scan(&Token);


         // ��Ϊ �� ��Ϊ����
         if (Token.token == T_LPAREN)
             return (funccall());

         // ��Ϊ [ ��Ϊ����
         if (Token.token == T_LBRACKET)
             return (array_access());

         // Not a function call, so reject the new token
         reject_token(&Token);

         // �������Ƿ����
         id = findglob(Text);
         if (id == -1 || Gsym[id].stype != S_VARIABLE)
             fatals("Unknown variable", Text);


         // ����Ҷ�ӽڵ�
         n = mkastleaf(A_IDENT, Gsym[id].type, id);
         break;
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
