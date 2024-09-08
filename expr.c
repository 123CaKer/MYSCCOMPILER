#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>

// 操作符优先级  
static int OpPrec[] =
{
  0, 10, 10,                    // T_EOF, T_PLUS, T_MINUS
  20, 20,                       // T_STAR, T_SLASH
  30, 30,                       // T_EQ, T_NE
  40, 40, 40, 40                // T_LT, T_GT, T_LE, T_GE
};

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

        // 生成ast节点
        left = mkastnode(arithop(tokentype), left, NULL, right, 0);  
        // 目前排查错误为tokentype二次转换，但依照ch8 更改 enum 顺序值

        tokentype = Token.token;  // 更新 token
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN)// 匹配)
            return left;
    }
   
    return left; //返回创建
}