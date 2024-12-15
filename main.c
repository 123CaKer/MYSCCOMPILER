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
    //  Globs = 0;
    O_dumpAST = 0;
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

void doer(char* p, char* q) // pΪ�����ļ���qΪ����ļ� 
{
    struct token* tree;
    init();

    if ((Infile = fopen(p, "r")) == NULL) // δ���ļ�
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);

    }


    if ((Outfile = fopen(q, "w")) == NULL)  // ����ļ�
    {
        fprintf(stderr, "Unable to create file of ****.s: %s\n", strerror(errno));
        exit(1);
    }

    // For now, ensure that void printint()and printchar() is defined
    addglob("printint", P_CHAR, S_FUNCTION, 0,0);
    addglob("printchar", P_VOID, S_FUNCTION, 0,0);

    // Ŀǰʹ�����ɵ�.s�ļ�������� ���ڻ������ִ�� �������ֵ ����ο� ch 5
    scan(&Token);			// �ж�����
    genpreamble();		// ��� preamble
    global_declarations();
    genpostamble(); // ��� postable
    fclose(Outfile);
    fclose(Infile);

    exit(0);
}

int _ZZJSTART()
{
    char* p = NULL;
    p = "..//needcompilefile/hello.c"; // �˴�����֮��ı��������ӻ�
    char* myoutpath = "..//needcompilefile/hello.s";// ����.s
    doer(p, myoutpath);
}