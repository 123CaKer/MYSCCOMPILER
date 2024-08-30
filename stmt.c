#include "defs.h"
#include "data.h"
#include "decl.h"

// ����������
void statements(void) 
{
    struct ASTnode* tree;
    int reg;

    while (1)
    {
        //ƥ���һ��Ϊprint
        match(T_PRINT, "print");

        // ���ɻ�����
        tree = binexpr(0);
        reg = genAST(tree);
        genprintint(reg);
        genfreeregs();

        // Match the following semicolon
        // and stop if we are at EOF
        semi();
        if (Token.token == T_EOF)
            return;
    }
}