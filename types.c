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