#include "defs.h"
#include "data.h"
#include "decl.h"

// 解析多个语句
void statements(void) 
{
    struct ASTnode* tree;
    int reg;

    while (1)
    {
        //匹配第一个为print
        match(T_PRINT, "print");

        // 生成汇编代码
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