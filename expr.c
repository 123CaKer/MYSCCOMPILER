#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>

// ���������ȼ�  EOF  +   -   *   /  INTLIT
static int OpPrec[] = { 0, 10, 10, 20, 20, 0 };


//���� token ���ж����Ӧ��ASTNode Ӧ��ֵ����Ϊ A_INTLIT
static struct ASTnode* primary( )
 {
    struct ASTnode* n;

    // ��token����ΪT_INTLIT ��Ϊ ASTҶ�ӽڵ� �����쳣
    switch (Token.token) 
    {
    case T_INTLIT:
        n = mkastleaf(A_INTLIT, Token.intvalue);
        scan(&Token);  // �ж�����
        return n;
    default:
        fprintf(stderr, "syntax error on line %d, token %d\n", Line, Token.token);
        exit(1);
    }
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
    default:
        fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);  
        exit(1);
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

// �����﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�
struct ASTnode* binexpr(int p) 
{
    struct ASTnode* n, * left, * right;
    int tokentype;

    // ��ȡ�������������� 
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

        // �� ���ʽ����ת��ΪAST��Ӧ���� �������ȼ���
        tokentype = arithop(Token.token);

        //���õ�ǰToken����
        scan(&Token);

        // �ҵݹ��������������
        right = binexpr(OpPrec[tokentype]);

        // ����ast�ڵ�
        left = mkastnode(tokentype, left, right, 0);

        tokentype = Token.token;  // ����token
        if (tokentype == T_EOF)
            return left;
    }
   
    return left; //���ش���
}