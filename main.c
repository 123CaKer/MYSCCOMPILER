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
#if  0
static void init()// 初始化全局变量 
{
Line = 1;
Putback = '\n';
Globs = 0;  /// 符号表初始位置 为 全局变量
Locls = NSYMBOLS - 1; /// 符号表末尾 为局部变量
O_dumpAST = 0;
}
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
// 令牌清单
char* tokstr[] = { "+", "-", "*", "/", "intlit" };

#endif //  0

void* my_pre_processor(char* p)
{
#if 0
    char cmd[TEXTLEN];  // 用于存储命令行语句
   //生成预处理命令
    snprintf(cmd, TEXTLEN, "%s %s %s", CPPCMD, INCDIR, p);// p为.c 文件

   
#endif // 0

   

    Infilename = "..//needcompilefile/hello.i"; //输入文件名 (路径) 以后可做可视化更改
    FILE* fp = fopen(Infilename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s: %s\n", fp, strerror(errno));
        exit(1);
    }

#if 0

    // Open up the pre-processor pipe 进程管道执行
    if ((Infile = _popen(cmd, "r")) == NULL)
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);
    }

#endif // 0

}
   



void doer(char* p , char* q) // p为输入文件，q为输出文件 
{
   // struct token* tree;
   // init();


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

#if 0
    // For now, ensure that printint() and printchar() are defined
    addglob("printint", P_INT, S_FUNCTION, C_GLOBAL, 0, 0);
    addglob("printchar", P_VOID, S_FUNCTION, C_GLOBAL, 0, 0);
#endif

    Line = 1;			// reset scanner
    Putback = '\n';
    clear_symtable();		// 清空符号表

    // 目前使用生成的.s文件进行输出 并在汇编器中执行 最终输出值 具体参考 ch 5
    scan(&Token);			// 判断类型
    Peektoken.token = 0;   // and set there is no lookahead token
    genpreamble();		// 输出 preamble
    global_declarations();
    genpostamble(); // 输出 postable
    fclose(Outfile);
    fclose(Infile);

    exit(0);
}

int _ZZJSTART()
{

    char* p = NULL;
    p = "..//needcompilefile/hello.c"; // 此处用于之后的编译器可视化
    Infilename = "..//needcompilefile/hello.i"; //输入文件名 (路径) 以后可做可视化更改
    char* myoutpath = "..//needcompilefile/hello.s";// 生成.s
  //  my_pre_processor(p);// 预处理
    doer(p, myoutpath);// 编译
}




