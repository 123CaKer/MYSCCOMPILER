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

void doer(char *p,char * q) // p为输入文件，q为输出文件 
{
    struct token* tree;
    init();

    if ((Infile = fopen(p, "r")) == NULL) // 未打开文件
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);

    }

    
    if ((Outfile = fopen(q, "w")) == NULL)  // 输出文件
    {
        fprintf(stderr, "Unable to create file of ****.s: %s\n", strerror(errno));
        exit(1);
    }


    // 目前使用生成的.s文件进行输出 并在汇编器中执行 最终输出值 具体参考 ch 5
    scan(&Token);			// 判断类型
    genpreamble();		// 输出 preamble
    tree = compound_statement();	// 生成复合AST
    genAST(tree, NOREG, 0);	// 生成tree的汇编代码			 
    genpostamble();		// 输出 postamble

    fclose(Outfile);
    fclose(Infile); 
   
    exit(0);
}

int _ZZJSTART()
{
    char* p = NULL;
    p = "..//needcompilefile/hello.c"; // 此处用于之后的编译器可视化
    char* myoutpath= "..//needcompilefile/hello.s";// 生成.s
    doer(p,myoutpath);
}