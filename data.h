#pragma once
#include<stdio.h>

#ifndef myextern_
#define myextern_ extern

#endif

#define TEXTLEN		    1024	        // ����������Ϊ1024
#define NSYMBOLS        1024            // �����ű�������
#define NOREG	-1		// genAST ����غ��������üĴ��� ����-1�� ʱ
#define NOLABEL	 0		// Use NOLABEL when we have no label to pass to genAST()

int   Line;     // ��ǰ�������ڵĶ�ȡ����
int	  Putback;  // ��Ҫ������������ֵ һ���ǲ���Ҫ��
FILE* Infile;   // �ļ�����ȡ��ָ��
FILE* Outfile;  // ����ļ�ָ��
struct token	Token;  // ȫ������
char Text[TEXTLEN + 1];		// ��ʶ���洢 buff
struct symtable Gsym[NSYMBOLS]; // ȫ�ַ��ű� �洢��������
int Functionid;         // ���ű��е�ǰ�����±�ʶ
int O_dumpAST;
//int Globsq;  // Position of next free global symbol slot  arm