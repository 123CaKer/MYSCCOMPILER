#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>

// 操作符优先级  EOF  +   -   *   /  INTLIT
static int OpPrec[] = { 0, 10, 10, 20, 20, 0 };


//解析 token 并判断其对应的ASTNode 应赋值类型为 A_INTLIT
static struct ASTnode* primary( )
 {
    struct ASTnode* n;

    // 将token类型为T_INTLIT 变为 AST叶子节点 否则异常
    switch (Token.token) 
    {
    case T_INTLIT:
        n = mkastleaf(A_INTLIT, Token.intvalue);
        scan(&Token);  // 判断类型
        return n;
    default:
        fprintf(stderr, "syntax error on line %d, token %d\n", Line, Token.token);
        exit(1);
    }
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
    default:
        fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);  
        exit(1);
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
    int tokentype;

    // 获取整数，并给到左 
    left = primary();

    /*
               +    
             /   \
            4     +
                 /   \          4+4+4 T_EOF
                 4     4
                      /
                    T_EOF
    */
    if (Token.token == T_EOF)  // 
        return left;

    while (op_precedence(Token.token)>p)
    {

        // 将 表达式符号转换为AST对应符号 （高优先级）
        tokentype = arithop(Token.token);

        //设置当前Token类型
        scan(&Token);

        // 右递归调用生成右子树
        right = binexpr(OpPrec[tokentype]);

        // 生成ast节点
        left = mkastnode(tokentype, left, right, 0);

        tokentype = Token.token;  // 更新token
        if (tokentype == T_EOF)
            return left;
    }
   
    return left; //返回创建
}