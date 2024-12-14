#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
// AST 解析
// 操作符优先级  
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

        tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);// 间接引用
        break;
    default:
        tree = primary();
    }
    return tree;
}

//将 表达式符号转换为AST对应符号
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

// 获取运算符优先级
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

//判断是否为右结合值
static int rightassoc(int tokentype)
{
    if (tokentype == T_ASSIGN)
        return 1;
    return 0;
}

// 生成语法树 返回root为+ - * /的ast树  其中p为之前的优先级
struct ASTnode* binexpr(int p)
{
    struct ASTnode* left, * right;
    struct ASTnode* ltemp, * rtemp;
    int ASTop;
    int tokentype;
    // 获取整数，并给到左 
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

    if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN)// 匹配)

    {
        /*  AST树
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


    // 第一个条件为计算（高优先级）第二个条件为赋值
    while (op_precedence(tokentype) > p || ((op_precedence(tokentype) == p) && rightassoc(tokentype)))
    {
        //设置当前Token类型
        scan(&Token);

        // 右递归调用生成右子树
        right = binexpr(OpPrec[tokentype]);


        ASTop = arithop(tokentype);
        if (ASTop == A_ASSIGN)
        {

            right->rvalue = 1;

            //匹配
            right = modify_type(right, left->type, 0);
            if (left == NULL)
                fatal("Incompatible expression in assignment");


            // 确保 表达式正常生成
            ltemp = left;
            left = right;
            right = ltemp;

        }
        else
        {


            // 左右子树适配

            /*
              +
            /   \           在进行判断时需要适配当前为右适配左
        int 2   char 5

              +
            /   \           在进行判断时需要适配当前为左适配右
        char 2   int 5

        但是在实际情况中，较为复杂 故需要做到两次匹配
      */
            left->rvalue = 1;
            right->rvalue = 1;


            ltemp = modify_type(left, right->type, ASTop);  // 左适配右
            rtemp = modify_type(right, left->type, ASTop);  // 右适配左
            if (ltemp == NULL && rtemp == NULL)
                fatal("Incompatible types in binary expression");
            if (ltemp != NULL)
                left = ltemp;
            if (rtemp != NULL)
                right = rtemp;



        }
        // 生成ast节点 保证一致
        left = mkastnode(arithop(tokentype), left->type, left, NULL, right, 0);


        tokentype = Token.token;  // 更新 token类型
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN)// 匹配)
        {
            left->rvalue = 1; //树的左边为右值
            return left;
        }


    }
    left->rvalue = 1;
    return left; //返回创建
}

struct ASTnode* funccall()
{
    struct ASTnode* tree;
    int id;

    // Check that the identifier has been defined,
    // then make a leaf node for it. XXX Add structural type test
    if ((id = findglob(Text)) == -1)
    {
        fatals("Undeclared function", Text);
    }
    //  匹配'('
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