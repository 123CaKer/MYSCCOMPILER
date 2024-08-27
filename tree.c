#include"defs.h"
#include<stdio.h>
#include<stdlib.h>
// 创建AST节点值
struct ASTnode* mkastnode(int op, struct ASTnode* left, struct ASTnode* right, int intvalue) 
{
    struct ASTnode* n;
    n = (struct ASTnode*)malloc(sizeof(struct ASTnode));
    if (n == NULL)
    {
        fprintf(stderr, "Unable to malloc in mkastnode()\n");
        exit(1);
    }



    n->op = op;
    n->left = left;
    n->right = right;
    n->intvalue = intvalue;  // 抽象语法树节点赋值

    /*
          -
        /   \
      2       3      2-3
    
    */
    return n;
}


//生成AST叶子节点
struct ASTnode* mkastleaf(int op, int intvalue) 
{
    return mkastnode(op, NULL, NULL, intvalue);
}

// 生成左子树AST 
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue)
{
 
    return mkastnode(op, left, NULL, intvalue);
}

// AST节点释放
int mkastfree(struct ASTnode* ASTNode)
{
    if (ASTNode != NULL)
    {
        free(ASTNode);
        ASTNode = NULL;
        return 0;
    }
    else
        return 1;

}
