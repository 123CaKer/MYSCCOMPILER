#include "defs.h"
#include "data.h"
#include "decl.h"

// �ж������Ƿ���ʣ��������ʷ���0 ���ʷ���1 left right Ϊֵ����
int type_compatible(int* left, int* right, int onlyright)
{// �˴������Ҳ�����������ң�������ʽ������
    int leftsize, rightsize;

    // ��ͬƥ��
    if (*left == *right)
    {
        *left = *right = 0;
        return 1;
    }
    //��ȡÿ�����͵Ĵ�С
    leftsize = genprimsize(*left);
    rightsize = genprimsize(*right);

    /// void ����
    if ((leftsize == 0) || (rightsize == 0))
        return (0);

    // ����ƥ��
    if (leftsize < rightsize)
    {
        *left = A_WIDEN;
        *right = 0;
        return (1);
    }
    if (rightsize < leftsize)
    {
        if (onlyright)
            return 0;
        *left = 0;
        *right = A_WIDEN;
        return 1;
    }
    // Anything remaining is the same size
    // and thus compatible
    *left = *right = 0;
    return 1;
}

// int *a ��ַ
int pointer_to(int type)
{
    int newtype;
    switch (type)
    {
    case P_VOID:
        newtype = P_VOIDPTR;
        break;
    case P_CHAR:
        newtype = P_CHARPTR;
        break;
    case P_INT:
        newtype = P_INTPTR;
        break;
    case P_LONG:
        newtype = P_LONGPTR;
        break;
    default:
        fatald("Unrecognised in pointer_to: type", type);
    }
    return newtype;
}

//  *a ������
int value_at(int type)
{
    int newtype;
    switch (type)
    {
    case P_VOIDPTR:
        newtype = P_VOID;
        break;
    case P_CHARPTR:
        newtype = P_CHAR;
        break;
    case P_INTPTR:
        newtype = P_INT;
        break;
    case P_LONGPTR:
        newtype = P_LONG;
        break;
    default:
        fatald("Unrecognised in value_at: type", type);
    }
    return newtype;
}

//�ж��Ƿ�Ϊ int ������� ��Ϊ1
int inttype(int type)
{
    if (type == P_CHAR || type == P_INT || type == P_LONG)
        return 1;
    return 0;
}

//�ж��Ƿ�Ϊָ��������� ��Ϊ1
int ptrtype(int type)
{
    if (type == P_VOIDPTR || type == P_CHARPTR || type == P_INTPTR || type == P_LONGPTR)
        return 1;
    return 0;
}

//��ǰ��tree����rtype opΪ��ǰtree������
struct ASTnode* modify_type(struct ASTnode* tree, int rtype, int op)
{
    int ltype;
    int lsize, rsize;

    ltype = tree->type;

    // Compare scalar int types
    if (inttype(ltype) && inttype(rtype))
    {

        // Both types same, nothing to do
        if (ltype == rtype)
            return (tree);

        // Get the sizes for each type
        lsize = genprimsize(ltype);
        rsize = genprimsize(rtype);

        // Tree's size is too big
        if (lsize > rsize)
            return (NULL);

        // Widen to the right
        if (rsize > lsize)
            return (mkastunary(A_WIDEN, rtype, tree, 0));
    }

    // For pointers on the left
    if (ptrtype(ltype))
    {
        // OK is same type on right and not doing a binary op
        if (op == 0 && ltype == rtype)
            return (tree);
    }

    // We can scale only on A_ADD or A_SUBTRACT operation
    if (op == A_ADD || op == A_SUBTRACT)
    {

        // Left is int type, right is pointer type and the size
        // of the original type is >1: scale the left
        if (inttype(ltype) && ptrtype(rtype))
        {
            rsize = genprimsize(value_at(rtype));// ��ȡ��С������  int *a sizeof��*a��
            if (rsize > 1)
                return (mkastunary(A_SCALE, rtype, tree, rsize));//��ǰ��rsizeΪ�������е�size
        }
    }


    return NULL;
}