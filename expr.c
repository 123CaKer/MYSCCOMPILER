#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>
// AST 解析
// 操作符优先级  
static int OpPrec[] =
{
  0, 10, 10,                    // T_EOF, T_PLUS, T_MINUS
  20, 20,                       // T_STAR, T_SLASH
  30, 30,                       // T_EQ, T_NE
  40, 40, 40, 40                // T_LT, T_GT, T_LE, T_GE
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

// 生成语法树 返回root为+ - * /的ast树  其中p为之前的优先级
struct ASTnode* binexpr(int p) 
{
    struct ASTnode* n, * left, * right;
    int lefttype, righttype;
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

    if (Token.token == T_EOF||Token.token==T_SEMI|| Token.token == T_RPAREN)// 匹配)
        /*  AST树
        * 
        *          A_IF
        *     /          \
        * (condition)
        *   
        */
        return left;

    while (op_precedence(Token.token)>p)
    {

        // 将 表达式符号转换为AST对应符号 （高优先级）
        tokentype = arithop(Token.token);

        //设置当前Token类型
        scan(&Token);

        // 右递归调用生成右子树
        right = binexpr(OpPrec[tokentype]);

        lefttype = left->type;
        righttype = right->type;
        if (type_compatible(&(lefttype), &(righttype), 0)==0)
            fatal("Incompatible types");


        // 将类型改为宽适配，小类型适配大类型 即生成A_WIDEN节点
        if (lefttype)//char 和 int 小的改为 A_WIDEN 另一个为0
            left = mkastunary(lefttype, right->type, left, 0);
        if (righttype)
            right = mkastunary(righttype, left->type, right, 0);

        // 生成ast节点
        left = mkastnode(arithop(tokentype), left->type, left, NULL, right, 0);
        

        tokentype = Token.token;  // 更新 token
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN)// 匹配)
            return left;
    }
   
    return left; //返回创建
} 

struct ASTnode* funccall(void)
{
    struct ASTnode* tree;
    int id;

    // Check that the identifier has been defined,
    // then make a leaf node for it. XXX Add structural type test
    if ((id = findglob(Text)) == -1)
    {
        fatals("Undeclared function", Text);
    }
    // Get the '('
    lparen();

    // Parse the following expression
    tree = binexpr(0);

    // Build the function call AST node. Store the
    // function's return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, Gsym[id].type, tree, id);

    // Get the ')'
    rparen();
    return (tree);
}