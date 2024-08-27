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

static void init()// 初始化全局变量 
{
    Line = 1;
    Putback = '\n';
}
// 令牌清单
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

    if ((Infile = fopen(p, "r")) == NULL) // 未打开文件
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);

    }

    scan(&Token);			// 判断类型

   // scanfile();
    n = binexpr(0);		   // 生成ast ，初始化优先级为0
    printf("%d\n", interpretAST(n));	// 解释语法树
    fclose(Infile); // 闭包性
    exit(0);
}

int _ZZJSTART()
{
    char* p = NULL;
    p = "..//needcompilefile/hello.c"; // 此处用于之后的编译器可视化
    doer(p);
}