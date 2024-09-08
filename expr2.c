#include "defs.h"
#include "data.h"
#include "decl.h"
/// 普拉特解析 保证 乘优先


// 返回乘性表达式
struct ASTnode* multiplicative_expr(void)
{
    struct ASTnode* left, * right;
    int tokentype;

  
    left = primary();  
//解析 token 并判断其对应的ASTNode 应赋值类型为 A_INTLIT

    tokentype = Token.token;
    if (tokentype == T_EOF)
        return left;

    // 乘性表达式 * / 
    while ((tokentype == T_STAR) || (tokentype == T_SLASH))
    {
      
        scan(&Token);  // 类型赋值
        right = primary();

       left = mkastnode(arithop(tokentype), left,NULL, right, 0);
         
        // 更新左值，如果没有则返回当前左值
        tokentype = Token.token;
        if (tokentype == T_EOF)
            break;
    }

    return left;
}


// 加性表达式
struct ASTnode* additive_expr(void)
{
    struct ASTnode* left, * right;
    int tokentype;

    // 左子树为高优先级的乘性 右子树同理
    left = multiplicative_expr();

    // 返回最近left token
    tokentype = Token.token;
    if (tokentype == T_EOF)
        return left;

    // Cache the '+' or '-' token type

    // Loop working on token at our level of precedence
    while (1) 
    {
        // Fetch in the next integer literal
        scan(&Token);

        right = multiplicative_expr();
    
        left = mkastnode(arithop(tokentype), left,NULL ,right, 0);

        // 更新 token类型并判断
        tokentype = Token.token;
        if (tokentype == T_EOF)
            break;
    }

    return left;
}

