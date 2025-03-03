#include "defs.h"
#include "data.h"
#include "decl.h"

// AST操作符
static char* ASTop[] = { "+", "-", "*", "/" };


// 解释AST值   可改为汇编接口代码为getAST（）
int interpretAST(struct ASTnode* n)
{
    int leftval;
    int rightval;

    //获取左子树 递归
    if (n->left)
        leftval = interpretAST(n->left);
    if (n->right)
        rightval = interpretAST(n->right);

    if (n->op == A_INTLIT)
        printf("int %d\n", n->v.intvalue);
    else
        printf("%d %s %d\n", leftval, ASTop[n->op], rightval);

    switch (n->op)// 子树计算方法 返回值
  {
    case A_ADD:
        return leftval + rightval; 
    case A_SUBTRACT:
        return leftval - rightval;
    case A_MULTIPLY:
        return leftval * rightval;
    case A_DIVIDE:
        return leftval / rightval;
    case A_INTLIT:
        return  n->v.intvalue;
    default:
        fprintf(stderr, "Unknown AST operator %d\n", n->op);
        exit(1);
  }
}