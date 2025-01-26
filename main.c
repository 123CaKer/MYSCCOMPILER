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
static void init()// ��ʼ��ȫ�ֱ��� 
{
Line = 1;
Putback = '\n';
Globs = 0;  /// ���ű��ʼλ�� Ϊ ȫ�ֱ���
Locls = NSYMBOLS - 1; /// ���ű�ĩβ Ϊ�ֲ�����
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
// �����嵥
char* tokstr[] = { "+", "-", "*", "/", "intlit" };

#endif //  0

void* my_pre_processor(char* p)
{
#if 0
    char cmd[TEXTLEN];  // ���ڴ洢���������
   //����Ԥ��������
    snprintf(cmd, TEXTLEN, "%s %s %s", CPPCMD, INCDIR, p);// pΪ.c �ļ�

   
#endif // 0

   

    Infilename = "..//needcompilefile/hello.i"; //�����ļ��� (·��) �Ժ�������ӻ�����
    FILE* fp = fopen(Infilename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s: %s\n", fp, strerror(errno));
        exit(1);
    }

#if 0

    // Open up the pre-processor pipe ���̹ܵ�ִ��
    if ((Infile = _popen(cmd, "r")) == NULL)
    {
        fprintf(stderr, "Unable to open %s: %s\n", p, strerror(errno));
        exit(1);
    }

#endif // 0

}
   



void doer(char* p , char* q) // pΪ�����ļ���qΪ����ļ� 
{
   // struct token* tree;
   // init();


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

#if 0
    // For now, ensure that printint() and printchar() are defined
    addglob("printint", P_INT, S_FUNCTION, C_GLOBAL, 0, 0);
    addglob("printchar", P_VOID, S_FUNCTION, C_GLOBAL, 0, 0);
#endif

    Line = 1;			// reset scanner
    Putback = '\n';
    clear_symtable();		// ��շ��ű�

    // Ŀǰʹ�����ɵ�.s�ļ�������� ���ڻ������ִ�� �������ֵ ����ο� ch 5
    scan(&Token);			// �ж�����
    Peektoken.token = 0;   // and set there is no lookahead token
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
    Infilename = "..//needcompilefile/hello.i"; //�����ļ��� (·��) �Ժ�������ӻ�����
    char* myoutpath = "..//needcompilefile/hello.s";// ����.s
  //  my_pre_processor(p);// Ԥ����
    doer(p, myoutpath);// ����
}




