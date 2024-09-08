#include "defs.h"
#include "data.h"
#include "decl.h"
/// �����ؽ��� ��֤ ������


// ���س��Ա��ʽ
struct ASTnode* multiplicative_expr(void)
{
    struct ASTnode* left, * right;
    int tokentype;

  
    left = primary();  
//���� token ���ж����Ӧ��ASTNode Ӧ��ֵ����Ϊ A_INTLIT

    tokentype = Token.token;
    if (tokentype == T_EOF)
        return left;

    // ���Ա��ʽ * / 
    while ((tokentype == T_STAR) || (tokentype == T_SLASH))
    {
      
        scan(&Token);  // ���͸�ֵ
        right = primary();

       left = mkastnode(arithop(tokentype), left,NULL, right, 0);
         
        // ������ֵ�����û���򷵻ص�ǰ��ֵ
        tokentype = Token.token;
        if (tokentype == T_EOF)
            break;
    }

    return left;
}


// ���Ա��ʽ
struct ASTnode* additive_expr(void)
{
    struct ASTnode* left, * right;
    int tokentype;

    // ������Ϊ�����ȼ��ĳ��� ������ͬ��
    left = multiplicative_expr();

    // �������left token
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

        // ���� token���Ͳ��ж�
        tokentype = Token.token;
        if (tokentype == T_EOF)
            break;
    }

    return left;
}

