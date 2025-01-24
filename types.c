#include "defs.h"
#include "data.h"
#include "decl.h"

// 判断两者是否合适，若不合适返回0 合适返回1 left right 为值类型
int type_compatible(int* left, int* right, int onlyright)
{// 此处的左右并不是真的左右，而是形式的左右
    int leftsize, rightsize;

    // 相同匹配
    if (*left == *right)
    {
        *left = *right = 0;
        return 1;
    }
    //获取每种类型的大小
    leftsize = genprimsize(*left);
    rightsize = genprimsize(*right);

    /// void 类型
    if ((leftsize == 0) || (rightsize == 0))
        return (0);

    // 类型匹配
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

// int *a 地址
int pointer_to(int type)
{
#if 0
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
#endif
    if ((type & 0xf) == 0xf)
        fatald("Unrecognised in pointer_to: type", type);
    return (type + 1);//eg 31 +1 =32   100000
}

//  *a 解引用
int value_at(int type)
{
#if 0
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
#endif
    if ((type & 0xf) == 0x0)
        fatald("Unrecognised in value_at: type", type);
    return (type - 1); //eg 32 -1 =31  11111 二进制 
}

//判断是否为 非指针类的类型   若是则为1
int inttype(int type)
{
#if 0
    if (type == P_CHAR || type == P_INT || type == P_LONG)
        return 1;
    return 0;
#endif
    return (((type & 0xf) == 0) && (type >= P_CHAR && type <= P_LONG));
}

//判断是否为指针类的类型 是为1
int ptrtype(int type)
{
#if 0
    if (type == P_VOIDPTR || type == P_CHARPTR || type == P_INTPTR || type == P_LONGPTR)
        return 1;
    return 0;
#endif
    return ((type & 0xf) != 0);
}

//当前的tree适配rtype op为当前tree操作符
struct ASTnode* modify_type(struct ASTnode* tree, int rtype, int op)
{
    int ltype;
    int lsize, rsize;

    ltype = tree->type;

    // XXX No idea on these yet
    // 当前不适配结构体和联合体
    if (ltype == P_STRUCT || ltype == P_UNION)
        fatal("Don't know how to do this yet");
    if (rtype == P_STRUCT || rtype == P_UNION)
        fatal("Don't know how to do this yet");


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
            return (mkastunary(A_WIDEN, rtype, tree, NULL, 0));
    }

    // For pointers （此处可用于强制类型转换）
    if (ptrtype(ltype) && ptrtype(rtype))
    {
        // We can compare them
        if (op >= A_EQ && op <= A_GE)
            return(tree);

        // A comparison of the same type for a non-binary operation is OK,
        // or when the left tree is of  `void *` type.
        // 条件1 为 强制转换类型相同 条件2为空指针转换
        if (op == 0 && (ltype == rtype || ltype == pointer_to(P_VOID)))
            return (tree);
    }

    // We can scale only on A_ADD or A_SUBTRACT operation
    if (op == A_ADD || op == A_SUBTRACT)
    {

        // Left is int type, right is pointer type and the size
    // of the original type is >1: scale the left、 即扩充左边值大小
        if (inttype(ltype) && ptrtype(rtype))
        {
            rsize = genprimsize(value_at(rtype));
            if (rsize > 1)
                return (mkastunary(A_SCALE, rtype, tree, NULL, rsize));
            else
                return (tree);		// Size 1, no need to scale
        }
    }


    return NULL;
}

// Given a type and a composite type pointer, return
// the size of this type in bytes
int typesize(int type, struct symtable* ctype)
{
    if (type == P_STRUCT || type == P_UNION)// 若为结构体或者联合体
        return(ctype->size); /// 返回符号表节点大小

    return(genprimsize(type));
}