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