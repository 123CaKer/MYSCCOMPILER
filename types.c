#include "defs.h"
#include "data.h"
#include "decl.h"

// �ж������Ƿ���ʣ��������ʷ���0 ���ʷ���1 left right Ϊֵ����
int type_compatible(int* left, int* right, int onlyright)
{// onlyright Ϊ1 ����ֵ���

    // ��������һ����void����0
    if (*left == P_VOID || *right == P_VOID)
        return 0;

    // ��ͬ�Ҿ���Ϊvoid ����1
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

          
        if (onlyright) // ��ֹ char  = int
            return 0;
        *left = 0;
        *right = A_WIDEN; 
        return 1;
    }
    // Anything remaining is compatible
    *left = *right = 0;
    return 1;
}