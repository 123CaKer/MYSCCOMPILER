#define _CRT_SECURE_NO_WARNINGS
#include "defs.h"
#define extern_
#include "data.h"
#undef extern_
#include "decl.h"
#include<stdio.h>
#include<string.h>
#include <errno.h>
#define _ZZJSTART main

static void init()// ��ʼ��ȫ�ֱ��� 
{
    Line = 1;
    Putback = '\n';
}
// �����嵥
char* tokstr[] = { "+", "-", "*", "/", "intlit" };


static void scanfile()
{
    struct token T;

    while (scan(&T)) 
    {
        printf("Token is %s", tokstr[T.token]);
        if (T.token == T_INTLIT)
            printf(", value %d", T.intvalue);
        printf("\n");
    }
}

void doer(char *p) 
{
    struct token* n;
    init();

    if ((Infile = fopen(p, "r")) == NULL) // δ���ļ�
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);

    }

    scan(&Token);			// �ж�����

   // scanfile();
    n = binexpr(0);		   // ����ast ����ʼ�����ȼ�Ϊ0
    printf("%d\n", interpretAST(n));	// �����﷨��
    fclose(Infile); // �հ���
    exit(0);
}

int _ZZJSTART()
{
    char* p = NULL;
    p = "..//needcompilefile/hello.c"; // �˴�����֮��ı��������ӻ�
    doer(p);
}