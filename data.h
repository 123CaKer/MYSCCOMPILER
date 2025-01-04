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
int O_dumpAST;         // �������AST�ڵ�
//int Globsq;  // Position of next free global symbol slot  arm
int Globs;		// Position of next free global symbol slot
int Locls;		// Position of next free local symbol slot


 int O_dumpAST;		// Ϊ��, dump the AST trees
 int O_keepasm;		// Ϊ��, ���ֻ���ļ�
 int O_assemble;	// Ϊ��, �������ļ�
 int O_dolink;		// Ϊ��, ����obj�ļ�
 int O_verbose;		// If true, print info on compilation stages
 char* Outfilename;	// ���򿪵�����ļ�����