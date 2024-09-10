#include"defs.h"
#include<stdio.h>
#include<stdlib.h>
// 创建AST节点值
 
struct ASTnode* mkastnode(int op, struct ASTnode* left,struct ASTnode* mid,struct ASTnode* right, int intvalue)
{
    struct ASTnode* n;
    n = (struct ASTnode*)malloc(sizeof(struct ASTnode));
    if (n == NULL)
        fatal("Unable to malloc in mkastnode()");

    n->op = op;
    n->left = left;
    n->mid = mid;
    n->right = right;
    n->v.intvalue = intvalue; // 抽象语法树节点赋值

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
    return mkastnode(op, NULL, NULL,NULL,intvalue);
}

// 生成度为1AST 
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue)
{
 
    return mkastnode(op, left, NULL,NULL, intvalue);
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
