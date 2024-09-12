#include "defs.h"
#include "data.h"
#include "decl.h"

// 判断两者是否合适，若不合适返回0 合适返回1 left right 为值类型
int type_compatible(int* left, int* right, int onlyright)
{// onlyright 为1 代表赋值语句

    // 二者若有一个是void返回0
    if (*left == P_VOID || *right == P_VOID)
        return 0;

    // 相同且均不为void 返回1
    if (*left == *right) 
    {
        *left = *right = 0;
        return 1;
    }

    // Widen P_CHARs to P_INTs as required
    if (*left == P_CHAR && *right == P_INT)
    {
        *left = A_WIDEN;
        *right = 0; 
        return 1;
    }
    if (*left == P_INT && *right == P_CHAR)
    {

          
        if (onlyright) // 禁止 char  = int
            return 0;
        *left = 0;
        *right = A_WIDEN; 
        return 1;
    }
    // Anything remaining is compatible
    *left = *right = 0;
    return 1;
}