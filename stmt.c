#include "defs.h"
#include "data.h"
#include "decl.h"
// 本文件将语句作为节点插入AST中形如


// compound_statement:          // empty, i.e. no statement
//      |      statement
//      |      statement statements
//      ;
//
// statement: declaration  //这里详细见compound_statement
//      |     expression_statement
//      |     function_call
//      |     if_statement
//      |     while_statement
//      |     for_statement
//      |     return_statement
//      ;


// if_statement: if_head
//      |        if_head 'else' statement
//      ;
//
// if_head: 'if' '(' true_false_expression ')' statement  ;
//
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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// 此处是保证了 if(x) {stmt}
    // 其中x为 表达式
    // 判断部分不涉及类型转换


    rparen();// 匹配 )


    trueAST = single_statement(); // 获取条件语句 chr 38 为单句
    // If we have an 'else', skip it
    // and get the AST for the compound statement
    if (Token.token == T_ELSE)
    {
        scan(&Token);
        falseAST = single_statement();
    }

    // 生成AST节点
    return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, NULL, 0);
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
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// 此处是保证了 if(x) {stmt}
    // 其中x为 表达式

    // 判断部分不涉及类型转换
    rparen();// 匹配 )

    Looplevel++;// 循环等级+1
    bodyAST = single_statement();// 单个句子里头套复合句 具体详见single_stmt
    Looplevel--;
    ASTn = mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, NULL, 0);
    return ASTn;

}

// for_statement: 'for' '(' expression_list ';'
//                          true_false_expression ';'
//                          expression_list ')' statement  ;
//
struct ASTnode* for_statement()
{
    struct ASTnode* condAST, * bodyAST;
    struct ASTnode* preopAST, * postopAST;
    struct ASTnode* tree;
    match(T_FOR, "for");
    lparen();
    // preopAST = single_statement();
       // Get the pre_op expression and the ';' 因为preop是匹配;
    preopAST = expression_list(T_SEMI);
    semi();

    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, NULL, 0);// 此处是保证了 if(x) {stmt}
  // 其中x为 表达式

    // 判断部分不涉及类型转换
    semi();
    postopAST = expression_list(T_RPAREN);
    rparen();

    Looplevel++;// 循环等级+1
    bodyAST = single_statement();
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

    semi(); //在此处匹配；
    // 最终生成A_RETURN
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
    semi(); //在此处匹配；
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
    semi(); //在此处匹配；
    return (mkastleaf(A_CONTINUE, 0, NULL, 0));
}

//生成switch AST
/*
*
*         A_SWITCH
*         /      \
      condition   A_CASE
                  /    \
                语句    A_CASE
                         /   \
                       语句  A_DEAFAULT
                              /       \
*                          默认语句  NULL
*
*
*/
static struct ASTnode* switch_statement(void) 
{
    struct ASTnode* left,// switch条件
        * n,  // 
        * c,
        * body ,// 当前case 下的语句体
        * casetree = NULL,
        * casetail = NULL;
    int inloop = 1,
        casecount = 0;// case的语句数量
    int seendefault = 0;
    int ASTop,
        casevalue=0;// case的值

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
    while (inloop)
    {
        switch (Token.token)
        {
            // Leave the loop when we hit a '}'
        case T_RBRACE: 
            if (casecount == 0)
            fatal("No cases in switch");
            inloop = 0;
            break;
        case T_CASE:
        case T_DEFAULT:
            // Ensure this isn't after a previous 'default'
            if (seendefault)
                fatal("case or default after existing default");

            // Set the AST operation. Scan the case value if required
            if (Token.token == T_DEFAULT) 
            {
                ASTop = A_DEFAULT; 
                seendefault = 1; 
                scan(&Token);
            }
            else
            {
                ASTop = A_CASE;
                scan(&Token);
                left = binexpr(0);
                // Ensure the case value is an integer literal
                if (left->op != A_INTLIT)
                    fatal("Expecting integer literal for case value");
                casevalue = left->a_intvalue;

                // Walk the list of existing case values to ensure
                // that there isn't a duplicate case value
                for (c = casetree; c != NULL; c = c->right)
                    if (casevalue == c->a_intvalue)
                        fatal("Duplicate case value");
            }

            // Scan the ':' and increment the casecount
            match(T_COLON, ":");
            

            // If the next token is a T_CASE, the existing case will fall
            // into the next case. Otherwise, parse the case body.
            if (Token.token == T_CASE)
                body = NULL;
            else
                body = compound_statement(1);
            casecount++;

            // Build a sub-tree with the compound statement as the left child
            // and link it in to the growing A_CASE tree
            // 连接switch语句
            if (casetree == NULL)
            {
                casetree = casetail = mkastunary(ASTop, 0, body, NULL, casevalue);
            }
            else 
            {
                casetail->right = mkastunary(ASTop, 0, body, NULL, casevalue);
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
    n->a_intvalue = casecount;
    n->right = casetree;
    rbrace();

    return(n);
}


// 保证dangling else 而且if 和 for 语句没有出现对{}的处理过程，因此在这里进行处理
struct ASTnode* single_statement()
{
    int type, class = C_LOCAL;
    struct symtable* ctype;
    struct ASTnode* stmt = NULL;

    switch (Token.token)
    {
    case T_LBRACE:
        // We have a '{', so this is a compound statement
        lbrace();
        stmt = compound_statement(0);// 符合语句
        rbrace();
        return(stmt);
    case T_IDENT:

        // We have to see if the identifier matches a typedef.
       // If not, treat it as an expression.
       // Otherwise, fall down to the parse_type() call.

        /*
            譬如你在文件中随意输入 x
            将其是为 一个变量

        */
        if (findtypedef(Text) == NULL)
        {
            stmt = binexpr(0);
            semi();
            return(stmt);
        }
    case T_CHAR:
    case T_INT:
    case T_LONG:
    case T_STRUCT:
    case T_UNION:
    case T_ENUM:
    case T_TYPEDEF:
        // The beginning of a variable declaration list.
        // 变量列表 int a ，v,c。。。
        declaration_list(&ctype, C_LOCAL, T_SEMI, T_EOF, &stmt);
        semi();
        return (stmt);// 返回语句
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
        stmt = binexpr(0);
        semi();
        return(stmt);
    }
    return (NULL);		// Keep -Wall happy
}



// 解析多个语句 （BNF）语句块 inswitch =1 说明在switch语句中需要匹配 } inswitch =0我们不需要
// and return its AST. If inswitch is true,
// we look for a '}', 'case' or 'default' token
// to end the parsing. Otherwise, look for
// just a '}' to end the parsing.
struct ASTnode* compound_statement(int inswitch)
{
    struct ASTnode* left = NULL;
    struct ASTnode* tree;

    while (1)
    {
        // Parse a single statement
        tree = single_statement();

        // For each new tree, either save it in left
        // if left is empty, or glue the left and the
        // new tree together
        if (tree != NULL) {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, NULL, 0);
        }

        // Leave if we've hit the end token
        if (Token.token == T_RBRACE)
            return(left);
        // If inswitch is true,
        // we look for a '}', 'case' or 'default' token
        // to end the parsing.
        if (inswitch && (Token.token == T_CASE || Token.token == T_DEFAULT))
            return(left);
    }
}
