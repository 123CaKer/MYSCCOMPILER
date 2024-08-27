#include"defs.h"
#include<stdio.h>
#include<stdlib.h>
// ����AST�ڵ�ֵ
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
    n->intvalue = intvalue;  // �����﷨���ڵ㸳ֵ

    /*
          -
        /   \
      2       3      2-3
    
    */
    return n;
}


//����ASTҶ�ӽڵ�
struct ASTnode* mkastleaf(int op, int intvalue) 
{
    return mkastnode(op, NULL, NULL, intvalue);
}

// ����������AST 
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue)
{
 
    return mkastnode(op, left, NULL, intvalue);
}

// AST�ڵ��ͷ�
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
