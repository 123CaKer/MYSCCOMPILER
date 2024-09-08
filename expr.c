#include "defs.h"
#include "data.h"
#include "decl.h"
#include <stdio.h>

// ���������ȼ�  
static int OpPrec[] =
{
  0, 10, 10,                    // T_EOF, T_PLUS, T_MINUS
  20, 20,                       // T_STAR, T_SLASH
  30, 30,                       // T_EQ, T_NE
  40, 40, 40, 40                // T_LT, T_GT, T_LE, T_GE
};

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

    if (Token.token == T_EOF||Token.token==T_SEMI|| Token.token == T_RPAREN)// ƥ��)
        /*  AST��
        * 
        *          A_IF
        *     /          \
        * (condition)
        *   
        */
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
        left = mkastnode(arithop(tokentype), left, NULL, right, 0);  
        // Ŀǰ�Ų����Ϊtokentype����ת����������ch8 ���� enum ˳��ֵ

        tokentype = Token.token;  // ���� token
        if (Token.token == T_EOF || Token.token == T_SEMI || Token.token == T_RPAREN)// ƥ��)
            return left;
    }
   
    return left; //���ش���
}