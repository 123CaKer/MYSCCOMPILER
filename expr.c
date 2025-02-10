#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
// AST ����
// ���������ȼ�  
// Operator precedence for each token. Must
// match up with the order of tokens in defs.h
static int OpPrec[] =
{
  0, 10, 10,                    // T_EOF, T_ASSIGN, T_ASPLUS,
  10, 10,                       // T_ASMINUS, T_ASSTAR,
  10, 10,                       // T_ASSLASH, T_ASMOD,
  15,                           // T_QUESTION,
  20, 30,                       // T_LOGOR, T_LOGAND
  40, 50, 60,                   // T_OR, T_XOR, T_AMPER 
  70, 70,                       // T_EQ, T_NE
  80, 80, 80, 80,               // T_LT, T_GT, T_LE, T_GE
  90, 90,                       // T_LSHIFT, T_RSHIFT
  100, 100,                     // T_PLUS, T_MINUS
  110, 110, 110                 // T_STAR, T_SLASH, T_MOD
};



// �ݹ��½��﷨
#if 0
primary_expression
    : T_IDENT
    | T_INTLIT
    | T_STRLIT
    | '(' expression ')'
    ;

primary_expression
    : IDENTIFIER
    | CONSTANT
    | STRING_LITERAL
    | '(' expression ')'
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' ')'
    | postfix_expression '(' argument_expression_list ')'
    | postfix_expression '.' IDENTIFIER
    | postfix_expression '->' IDENTIFIER
    | postfix_expression '++'
    | postfix_expression '--'
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
// expression_list: <null>
//        | expression
//        | expression ',' expression_list
//        ;

/*
    һ������ ����˵ һ����ʾΪһ��list

    fun(16+5+2,313+,95,2+x/2 ) ������Ϊһ�� ���ʽ�б�

*/
struct ASTnode* expression_list(int endtoken)
{
    struct ASTnode* tree = NULL;
    struct ASTnode* child = NULL;
    int exprcount = 0;

    // Loop until the end token
    while (Token.token != endtoken)
    {

        // Parse the next expression and increment the expression count
        child = binexpr(0);
        exprcount++;

        // Build an A_GLUE AST node with the previous tree as the left child
        // and the new expression as the right child. Store the expression count.
        tree = mkastnode(A_GLUE, P_NONE, NULL, tree, NULL, child, NULL, exprcount);


        // Stop when we reach the end token
        if (Token.token == endtoken)
            break;

        // Must have a ',' at this point
        match(T_COMMA, ",");
    }

    // Return the tree of expressions
    return (tree);
}




// ����ǰ׺���ʽ������һ����Ӧ��AST����
/*
   int *a;
    x=&b;
   */
   // chr 52 ֮ǰ��prefix
#if 0
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

        tree = mkastunary(A_DEREF, value_at(tree->type), tree->ctype, tree, NULL, 0);// �������
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
#endif // 0

// �β�ptp��һ�����ڵ�ǰ���������ȼ����ݵ�һ������
/*
*  ����˵ ��ǰ��if��a==NULL||c==NULL��
* ��֤==�����ɵ���a==NULL ������ NULL||c ��Ϊ��֤ ���ɵ�Ӧ��Ϊ
* if����a==NULL��||��c==NULL���� ������if��a==��NULL||c)==NULL��(һ�ִ���)����� chr 57
*
*
* if��a==NULL||b==NULL ||c==NULL��
* ��ˣ������ǿ�������IF��亯����������
��if_statement��������binexpr��0��
binexpr��0������==�����ȼ�Ϊ40��������binexpr
binexpr��40������ǰ׺����
prefix��������postfix����
postfix��������primary����
primary�����ڣ�void*��0�Ŀ�ͷ���������ţ�������paren_expression����
paren_expression�����鿴void��ǲ�����parse_cast������һ��������ǿ��ת�������ͻ����binexpr��0��������0��
������������ڡ�NULL��ֵ����0Ӧ����Ȼ�������ȼ�40����paren_expression����ֻ�ǽ������û��㡣
����ζ���������ڽ�����NULL||b����������AST���������ǽ���a==NULL��������AST����
���������ȷ��ǰ����������ȼ�ͨ����������binexpr����һֱ���ݵ�paren_expression����������ζ�ţ�
prefix������postfix������primary������paren_expression����
������Щ���ڶ�����һ��intptp�����������䴫�ݡ�
*
*
*/
struct ASTnode* prefix(int ptp)
{
    struct ASTnode* tree;
    switch (Token.token)
    {
    case T_AMPER:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // Ensure that it's an identifier
        if (tree->op != A_IDENT)
            fatal("& operator must be followed by an identifier");

        // Prevent '&' being performed on an array
        if (tree->sym->stype == S_ARRAY)
            fatal("& operator cannot be performed on an array");

        // Now change the operator to A_ADDR and the type to
        // a pointer to the original type
        tree->op = A_ADDR;
        tree->type = pointer_to(tree->type);
        break;
    case T_STAR:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);
        tree->rvalue = 1; // ��* ��������������������������Ϊ��ֵ
        // Ensure the tree's type is a pointer
        /// ȷ����ָ������ 
        if (!ptrtype(tree->type))
            fatal("* operator must be followed by an expression of pointer type");


        // Prepend an A_DEREF operation to the tree
        // ���Ѱַ
        tree =
            mkastunary(A_DEREF, value_at(tree->type), tree->ctype, tree, NULL, 0);
        break;
    case T_MINUS:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // Prepend a A_NEGATE operation to the tree and
        // make the child an rvalue. Because chars are unsigned,
        // also widen this if needed to int so that it's signed
        tree->rvalue = 1;
        if (tree->type == P_CHAR)
            tree->type = P_INT;
        tree = mkastunary(A_NEGATE, tree->type, tree->ctype, tree, NULL, 0);
        break;
    case T_INVERT:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // Prepend a A_INVERT operation to the tree and
        // make the child an rvalue.
        tree->rvalue = 1;
        tree = mkastunary(A_INVERT, tree->type, tree->ctype, tree, NULL, 0);
        break;
    case T_LOGNOT:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // Prepend a A_LOGNOT operation to the tree and
        // make the child an rvalue.
        tree->rvalue = 1;
        tree = mkastunary(A_LOGNOT, tree->type, tree->ctype, tree, NULL, 0);
        break;
    case T_INC:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // For now, ensure it's an identifier
        if (tree->op != A_IDENT)
            fatal("++ operator must be followed by an identifier");

        // Prepend an A_PREINC operation to the tree
        tree = mkastunary(A_PREINC, tree->type, tree->ctype, tree, NULL, 0);
        break;
    case T_DEC:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix(ptp);

        // For now, ensure it's an identifier
        if (tree->op != A_IDENT)
            fatal("-- operator must be followed by an identifier");

        // Prepend an A_PREDEC operation to the tree
        tree = mkastunary(A_PREDEC, tree->type, tree->ctype, tree, NULL, 0);
        break;
    default:
        tree = postfix(ptp);
    }
    return (tree);
}



// ������׺���ʽ������һ����Ӧ��AST����
// Parse a postfix expression and return
// an AST node representing it. The
// identifier is already in Text.

// chr 50 ֮ǰpostfix����
#if 0  
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
        return (mkastleaf(A_INTLIT, P_INT, NULL, enumptr->st_posn));
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
#endif // 0  

// chr 51-52 ��postfix����
#if 0
static struct ASTnode* postfix(void)
{
    struct ASTnode* n;
    struct symtable* varptr; // ���ű�ָ��
    struct symtable* enumptr;
    int rvalue = 0;

    // If the identifier matches an enum value,
 // return an A_INTLIT node
    if ((enumptr = findenumval(Text)) != NULL)
        // ���Ѱ��ֵ�ҵ���
    {
        scan(&Token);// �Թ�
        return (mkastleaf(A_INTLIT, P_INT, NULL, enumptr->st_posn));
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

    // An identifier, check that it exists. For arrays, set rvalue to 1.
    if ((varptr = findsymbol(Text)) == NULL)
        fatals("Unknown variable", Text);
    switch (varptr->stype)
    {
    case S_VARIABLE:
        break;
    case S_ARRAY:
        rvalue = 1;//S_ARRAY����׼������ array++ ���� ����ʹ��ָ���ʼ���ڽ���ָ������ ͬʱ����&ary
        break;
    default:
        fatals("Identifier not a scalar or array variable", Text);
    }



    switch (Token.token)
    {
        // Post-increment: skip over the token
    case T_INC:
        if (rvalue == 1)//S_ARRAY����׼������ array++ ���� ����ʹ��ָ���ʼ���ڽ���ָ������ ͬʱ����&ary
            fatals("Cannot ++ on rvalue", Text);
        scan(&Token);
        n = mkastleaf(A_POSTINC, varptr->type, varptr, 0);
        break;

        // Post-decrement: skip over the token
    case T_DEC:
        if (rvalue == 1)
            fatals("Cannot -- on rvalue", Text);
        scan(&Token);
        n = mkastleaf(A_POSTDEC, varptr->type, varptr, 0);
        break;

        // Just a variable reference. Ensure any arrays
        // cannot be treated as lvalues.
    default:
        if (varptr->stype == S_ARRAY)
        {
            n = mkastleaf(A_ADDR, varptr->type, varptr, 0);
            n->rvalue = rvalue;
        }
        else
            n = mkastleaf(A_IDENT, varptr->type, varptr, 0);
    }
    return (n);

}
#endif // 0


// Parse a postfix expression and return
// an AST node representing it. The
// identifier is already in Text.
static struct ASTnode* postfix(int ptp)
{
    struct ASTnode* n;

    // Get the primary expression
    n = primary(ptp);

    // ѭ��Ѱ�Һ�׺���ʽ
    while (1)
    {
        switch (Token.token)
        {
        case T_LBRACKET:
            //���з���
            n = array_access(n);
            break;

        case T_DOT:
            // Access into a struct or union
            n = member_access(n, 0);
            break;

        case T_ARROW:
            // Pointer access into a struct or union
            n = member_access(n, 1);
            break;

        case T_INC:
            // Post-increment: skip over the token
            if (n->rvalue == 1)
                fatal("Cannot ++ on rvalue");
            scan(&Token);

            // Can't do it twice
            if (n->op == A_POSTINC || n->op == A_POSTDEC)
                fatal("Cannot ++ and/or -- more than once");

            // and change the AST operation
            n->op = A_POSTINC;
            break;

        case T_DEC:
            // Post-decrement: skip over the token
            if (n->rvalue == 1)
                fatal("Cannot -- on rvalue");
            scan(&Token);

            // Can't do it twice
            if (n->op == A_POSTINC || n->op == A_POSTDEC)
                fatal("Cannot ++ and/or -- more than once");

            // and change the AST operation
            n->op = A_POSTDEC;
            break;

        default:
            return (n);
        }
    }

    return (NULL);
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

    if (tokentype > T_EOF && tokentype <= T_MOD)
        return (tokentype);
    fatald("Syntax error, token", tokentype);
    return (0);

}

// ��ȡ��������ȼ�
static int op_precedence(int tokentype)
{
    int prec;
    if (tokentype > T_MOD)
        fatald("Token with no precedence in op_precedence:", tokentype);
    prec = OpPrec[tokentype];
    if (prec == 0)
        fatald("Syntax error, token", tokentype);
    return (prec);
}

//�ж��Ƿ�Ϊ�ҽ��ֵ   ���ҵ���ֵ z=2�� 
//�ӵ�chr 43֮�� ���� = += -= *= /= ��Ϊ�ҽ����
// ������������

/*
   a += b + c;          // needs to be parsed as
   a += (b + c);        // not
   (a += b) + c;

*/
static int rightassoc(int tokentype)
{
    if (tokentype >= T_ASSIGN && tokentype <= T_ASSLASH)
        return (1);// �����ҽ����
    return (0);// ������
}

// ����AST�﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�

struct ASTnode* binexpr(int ptp) {
    struct ASTnode* left, * right;
    struct ASTnode* ltemp, * rtemp;
    int ASTop;
    int tokentype;

    // Get the tree on the left.
    // Fetch the next token at the same time.
    left = prefix(ptp);

    // If we hit one of several terminating tokens, return just the left node
    tokentype = Token.token;
    if (tokentype == T_SEMI || tokentype == T_RPAREN ||
        tokentype == T_RBRACKET || tokentype == T_COMMA ||
        tokentype == T_COLON || tokentype == T_RBRACE) {
        left->rvalue = 1;
        return (left);
    }
    // While the precedence of this token is more than that of the
    // previous token precedence, or it's right associative and
    // equal to the previous token's precedence
    while ((op_precedence(tokentype) > ptp) ||
        (rightassoc(tokentype) && op_precedence(tokentype) == ptp)) {
        // Fetch in the next integer literal
        scan(&Token);

        // Recursively call binexpr() with the
        // precedence of our token to build a sub-tree
        right = binexpr(OpPrec[tokentype]);

        // Determine the operation to be performed on the sub-trees
        ASTop = arithop(tokentype);

        switch (ASTop) {
        case A_TERNARY:
            // Ensure we have a ':' token, scan in the expression after it
            match(T_COLON, ":");
            ltemp = binexpr(0);

            // Build and return the AST for this statement. Use the middle
            // expression's type as the return type. XXX We should also
            // consider the third expression's type.
            return (mkastnode
            (A_TERNARY, right->type, right->ctype, left, right, ltemp,
                NULL, 0));

        case A_ASSIGN:
            // Assignment
            // Make the right tree into an rvalue
            right->rvalue = 1;

            // Ensure the right's type matches the left
            right = modify_type(right, left->type, left->ctype, 0);
            if (right == NULL)
                fatal("Incompatible expression in assignment");

            // Make an assignment AST tree. However, switch
            // left and right around, so that the right expression's 
            // code will be generated before the left expression
            ltemp = left;
            left = right;
            right = ltemp;
            break;

        default:
            // We are not doing a ternary or assignment, so both trees should
            // be rvalues. Convert both trees into rvalue if they are lvalue trees
            left->rvalue = 1;
            right->rvalue = 1;

            // Ensure the two types are compatible by trying
            // to modify each tree to match the other's type.
            ltemp = modify_type(left, right->type, right->ctype, ASTop);
            rtemp = modify_type(right, left->type, left->ctype, ASTop);
            if (ltemp == NULL && rtemp == NULL)
                fatal("Incompatible types in binary expression");
            if (ltemp != NULL)
                left = ltemp;
            if (rtemp != NULL)
                right = rtemp;
        }

        // Join that sub-tree with ours. Convert the token
        // into an AST operation at the same time.
        left =
            mkastnode(arithop(tokentype), left->type, left->ctype, left, NULL,
                right, NULL, 0);

        // Some operators produce an int result regardless of their operands
        switch (arithop(tokentype)) {
        case A_LOGOR:
        case A_LOGAND:
        case A_EQ:
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            left->type = P_INT;
        }

        // Update the details of the current token.
        // If we hit a terminating token, return just the left node
        tokentype = Token.token;
        if (tokentype == T_SEMI || tokentype == T_RPAREN ||
            tokentype == T_RBRACKET || tokentype == T_COMMA ||
            tokentype == T_COLON || tokentype == T_RBRACE) {
            left->rvalue = 1;
            return (left);
        }
    }

    // Return the tree we have when the precedence
    // is the same or lower
    left->rvalue = 1;
    return (left);
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

    // �������ʽ���� ����ƥ�䣩
    tree = expression_list(T_RPAREN);

    // XXX Check type of each argument against the function's prototype

    // Build the function call AST node. Store the
    // function's return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, funcptr->type, funcptr->ctype, tree, funcptr, 0);


    // Get the ')'
    rparen();
    return (tree);
}

// Parse the index into an array and
// return an AST tree for it
#if 0

static struct ASTnode* array_access(void)
{
    struct ASTnode* left, * right;
    struct symtable* aryptr;

    // �ж��Ƿ���array
   // Check that the identifier has been defined as an array or a pointer.
    if ((aryptr = findsymbol(Text)) == NULL)
        fatals("Undeclared variable", Text);
    if (aryptr->stype != S_ARRAY &&// ����array
        (aryptr->stype == S_VARIABLE && !ptrtype(aryptr->type)))// ����ָ��
        fatals("Not an array or pointer", Text);

    left = mkastleaf(A_ADDR, aryptr->type, aryptr, 0);

    // Make a leaf node for it that points at the base of
 // the array, or loads the pointer's value as an rvalue
    // Ҳ����˵ 
    /*
    *
    *       int *p��
    *       int ar[21];
    *       p = ar // �����ַ��Ϊ��ֵ
    */
    if (aryptr->stype == S_ARRAY)
        left = mkastleaf(A_ADDR, aryptr->type, aryptr, 0);
    else {
        left = mkastleaf(A_IDENT, aryptr->type, aryptr, 0);
        left->rvalue = 1;
    }

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
#endif // 0

// Parse the index into an array and return an AST tree for it
// ����������� ��ǰ����ָ�������֮ǰ��ͬ�����ַ��һ����Ϊ��ֵ
static struct ASTnode* array_access(struct ASTnode* left)
{
    struct ASTnode* right;

    // Check that the sub-tree is a pointer
    if (!ptrtype(left->type))
        fatal("Not an array or pointer");

    // Get the '['
    scan(&Token);

    // Parse the following expression
    right = binexpr(0);

    // Get the ']'
    match(T_RBRACKET, "]");

    // Ensure that this is of int type
    if (!inttype(right->type))
        fatal("Array index is not of integer type");

    // Make the left tree an rvalue
    left->rvalue = 1;

    // Scale the index by the size of the element's type
    right = modify_type(right, left->type, left->ctype, A_ADD);

    // Return an AST tree where the array's base has the offset added to it,
    // and dereference the element. Still an lvalue at this point.
    left =
        mkastnode(A_ADD, left->type, left->ctype, left, NULL, right, NULL, 0);
    left =
        mkastunary(A_DEREF, value_at(left->type), left->ctype, left, NULL, 0);
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

#if 0
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
    right = mkastleaf(A_INTLIT, P_INT, NULL, m->st_posn);

    // Add the member's offset to the base of the struct/union
    // and dereference it. Still an lvalue at this point
    left = mkastnode(A_ADD, pointer_to(m->type), left, NULL, right, NULL, 0);
    left = mkastunary(A_DEREF, m->type, left, NULL, 0);
    return (left);
}

#endif // 0

// Parse the member reference of a struct or union
// and return an AST tree for it. If withpointer is true,
// the access is through a pointer to the member.
struct ASTnode* member_access(struct ASTnode* left, int withpointer)
{
    struct ASTnode* right;
    struct symtable* typeptr;
    struct symtable* m;

    // Check that the left AST tree is a pointer to struct or union
    if (withpointer && left->type != pointer_to(P_STRUCT)
        && left->type != pointer_to(P_UNION))
        fatal("Expression is not a pointer to a struct/union");

    // Or, check that the left AST tree is a struct or union.
    // If so, change it from an A_IDENT to an A_ADDR so that
    // we get the base address, not the value at this address.
    if (!withpointer)
    {
        if (left->type == P_STRUCT || left->type == P_UNION)
            left->op = A_ADDR;// ���е�ַ���� ������ -��
        else
            fatal("Expression is not a struct/union");
    }

    // Get the details of the composite type
    typeptr = left->ctype;

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

    // Make the left tree an rvalue
    left->rvalue = 1;

    // Build an A_INTLIT node with the offset
    right = mkastleaf(A_INTLIT, P_INT, NULL, NULL, m->st_posn);

    // Add the member's offset to the base of the struct/union
    // and dereference it. Still an lvalue at this point
    left =
        mkastnode(A_ADD, pointer_to(m->type), m->ctype, left, NULL, right, NULL,
            0);
    left = mkastunary(A_DEREF, m->type, m->ctype, left, NULL, 0);
    return (left);
}



// Parse a parenthesised expression and
// return an AST node representing it
// ���������ŵı��ʽ
static struct ASTnode* paren_expression(int ptp)
{
    struct ASTnode* n;
    int type = 0;
    struct symtable* ctype = NULL;

    // Beginning of a parenthesised expression, skip the '('.
    scan(&Token);

    // If the token after is a type identifier, this is a cast expression
    switch (Token.token)
    {
    case T_IDENT:
        // We have to see if the identifier matches a typedef.
        // If not, treat it as an expression.
        if (findtypedef(Text) == NULL)
        {
            n = binexpr(0);// ()��û��typedef' ˵����һ�����ʽ ��ʹ����Ҳ�������ʽ
                                // ����һ��������ǵ����Ǳ���typedef ʱ�� �ͻ�binexpr
                                 // ptp is zero as expression inside ( )
            break;
        }
    case T_VOID:
    case T_CHAR:
    case T_INT:
    case T_LONG:
    case T_STRUCT:
    case T_UNION:
    case T_ENUM:
        // Get the type inside the parentheses // ��ǰΪǿ������ת�� ���� ��int��a
        type = parse_cast(&ctype);

        // Skip the closing ')' and then parse the following expression
        //��int    ��rparen() var
        rparen();

    default:
        n = binexpr(ptp);		// Ĭ����һ��������ʽ
    }

    if (type == 0)// �������ǿ��ת�����˴���ƥ���������expr ����ǰΪһ�����ʽ
        rparen();  /*
                       ��x*1     ��rparen()
                   */
    else
        // Otherwise, make a unary AST node for the cast
        n = mkastunary(A_CAST, type, ctype, n, NULL, 0);
    return (n);
}

// ��52�½�֮ǰ��primary
#if 0
struct ASTnode* primary()
{
    struct ASTnode* n = NULL;
    int id;
    int type = 0;// ת��������
    int size = 0, class;//sizeof����
    struct symtable* ctype;
    // ��token����ΪT_INTLIT ��Ϊ ASTҶ�ӽڵ� �����쳣
    switch (Token.token)
    {
    case T_STATIC:
    case T_EXTERN:
        fatal("Compiler doesn't support static or extern local declarations");
    case T_SIZEOF:
        // Skip the T_SIZEOF and ensure we have a left parenthesis
        scan(&Token);
        if (Token.token != T_LPAREN)
            fatal("Left parenthesis expected after sizeof");
        scan(&Token);

        // Get the type inside the parentheses
        type = parse_stars(parse_type(&ctype, &class));
        // Get the type's size
        size = typesize(type, ctype);
        rparen();

        return (mkastleaf(A_INTLIT, P_INT, NULL, NULL, size));

    case T_STRLIT:
        // For a STRLIT token, generate the assembly for it.
        // Then make a leaf AST node for it. id is the string's label.
        id = genglobstr(Text); // ���� ���
        n = mkastleaf(A_STRLIT, pointer_to(P_CHAR), NULL, NULL, id); // ����Ҷ�ӽڵ�
        break;

    case T_INTLIT: //����ֵ

        if ((Token.intvalue) >= 0 && (Token.intvalue < 256))//char��
            n = mkastleaf(A_INTLIT, P_CHAR, NULL, NULL, Token.intvalue);
        else                                                // int��
            n = mkastleaf(A_INTLIT, P_INT, NULL, NULL, Token.intvalue);
        break;

    case T_IDENT:

        return postfix(); // ��׺���ʽ

    case T_LPAREN:// ����������Ūǿ������ת�� ��Ϊ��char* ��a

        // Beginning of a parenthesised expression, skip the '('.
        scan(&Token);


        // If the token after is a type identifier, this is a cast expression
        switch (Token.token)
        {
        case T_IDENT:
            // We have to see if the identifier matches a typedef.
            // If not, treat it as an expression.
            if (findtypedef(Text) == NULL)
            {
                n = binexpr(0); // ()��û��typedef' ˵����һ�����ʽ ��ʹ����Ҳ�������ʽ
                                // ����һ��������ǵ����Ǳ���typedef ʱ�� �ͻ�binexpr
                break;
            }
        case T_VOID:
        case T_CHAR:
        case T_INT:
        case T_LONG:
        case T_STRUCT:
        case T_UNION:
        case T_ENUM:
            // Get the type inside the parentheses
            // ��ȡǿ������ת��
            type = parse_cast();
            // Skip the closing ')' and then parse the following expression
            rparen();
            // ����Ĭ������ȥ
        default:
            n = binexpr(0); // ɨ����ʽ���Ѻ��浱���Ǹ����ʽ
        }

        // We now have at least an expression in n, and possibly a non-zero type in type
        // if there was a cast. Skip the closing ')' if there was no cast.
        if (type == 0)// �������ǿ��ת�����˴���ƥ���������expr ����ǰΪһ�����ʽ
            rparen();  /*
                           ��x*1     ��rparen()
                       */
        else
            // Otherwise, make a unary AST node for the cast
            n = mkastunary(A_CAST, type, n, NULL, 0);
        return (n);

    default:
        fatald("Expecting a primary expression, got token", Token.token);

    }

    scan(&Token);//ɨ����һ�����Ʋ�����Ҷ�ӽڵ�
    return n;
}
#endif // 0

struct ASTnode* primary(int ptp)
{
    struct ASTnode* n = NULL;
    struct symtable* enumptr;
    struct symtable* varptr;
    int id;
    int type = 0;
    int size, class;
    struct symtable* ctype;

    switch (Token.token)
    {
    case T_STATIC:
    case T_EXTERN:
        fatal("Compiler doesn't support static or extern local declarations");
    case T_SIZEOF:
        // Skip the T_SIZEOF and ensure we have a left parenthesis
        scan(&Token);
        if (Token.token != T_LPAREN)
            fatal("Left parenthesis expected after sizeof");
        scan(&Token);

        // Get the type inside the parentheses
        type = parse_stars(parse_type(&ctype, &class));

        // Get the type's size
        size = typesize(type, ctype);
        rparen();

        // Make a leaf node int literal with the size
        return (mkastleaf(A_INTLIT, P_INT, NULL, NULL, size));

    case T_INTLIT:
        // For an INTLIT token, make a leaf AST node for it.
        // Make it a P_CHAR if it's within the P_CHAR range
        if (Token.intvalue >= 0 && Token.intvalue < 256)
            n = mkastleaf(A_INTLIT, P_CHAR, NULL, NULL, Token.intvalue);
        else
            n = mkastleaf(A_INTLIT, P_INT, NULL, NULL, Token.intvalue);
        break;

    case T_STRLIT:
        // For a STRLIT token, generate the assembly for it.
        // Then make a leaf AST node for it. id is the string's label.
        id = genglobstr(Text, 0);

        // For successive STRLIT tokens, append their contents
        // to this one
        while (1)
        {
            scan(&Peektoken);
            if (Peektoken.token != T_STRLIT)
                break;
            genglobstr(Text, 1);
            scan(&Token);	// To skip it properly
        }

        // Now make a leaf AST node for it. id is the string's label.
        genglobstrend(); // char * "sas""sasas"
        n = mkastleaf(A_STRLIT, pointer_to(P_CHAR), NULL, NULL, id);
        break;


    case T_IDENT:
        // If the identifier matches an enum value,
        // return an A_INTLIT node
        if ((enumptr = findenumval(Text)) != NULL) // ����ö��
        {
            n = mkastleaf(A_INTLIT, P_INT, NULL, NULL, enumptr->st_posn);
            break;
        }
        // See if this identifier exists as a symbol. For arrays, set rvalue to 1.
        if ((varptr = findsymbol(Text)) == NULL)
            fatals("Unknown variable or function", Text);
        switch (varptr->stype)
        {
        case S_VARIABLE:
            n = mkastleaf(A_IDENT, varptr->type, varptr->ctype, varptr, 0);
            break;
        case S_ARRAY:
            n = mkastleaf(A_ADDR, varptr->type, varptr->ctype, varptr, 0);
            n->rvalue = 1;
            break;
        case S_FUNCTION:
            // Function call, see if the next token is a left parenthesis
            scan(&Token);
            if (Token.token != T_LPAREN)
                fatals("Function name used without parentheses", Text);
            return (funccall());
        default:
            fatals("Identifier not a scalar or array variable", Text);
        }
        break;

    case T_LPAREN:
        return (paren_expression(ptp));

    default:
        fatals("Expecting a primary expression, got token", Token.tokstr);
    }

    // Scan in the next token and return the leaf node
    scan(&Token);
    return (n);
}






