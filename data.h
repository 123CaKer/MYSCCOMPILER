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
char* Infilename;		// Name of file we are parsing
char* Outfilename;		// Name of file we opened as Outfile
struct token	Token;  // ȫ������
struct token Peektoken;		// ȫ�����Ƶ���һ��
char Text[TEXTLEN + 1];		// ��ʶ���洢 buff
//struct symtable Gsym[NSYMBOLS]; // ȫ�ַ��ű� �洢��������
struct symtable* Functionid; 	// ��ǰ�����ķ��ű��±�ʶ��,�����޸�Ϊstruct symtable* ��ǰΪ������������
int O_dumpAST;         // �������AST�ڵ�
//int Globsq;  // Position of next free global symbol slot  arm
int Globs;		// Position of next free global symbol slot
int Locls;		// Position of next free local symbol slot
int Looplevel;                  // ѭ�����
int Switchlevel;		// switch���
/*
     while
     {
     while      Looplevel=2
     {

     }
     }

*/
// Symbol table lists
struct symtable* Globhead, * Globtail;	  // Global variables and functions
struct symtable* Loclhead, * Locltail;	  // Local variables
struct symtable* Parmhead, * Parmtail;	  // Local parameters
struct symtable* Membhead, * Membtail;	  // Temp list of struct/union members
struct symtable* Structhead, * Structtail; // List of struct types
struct symtable* Unionhead, * Uniontail;   // List of union types
struct symtable* Enumhead, * Enumtail;    // List of enum types and values
struct symtable* Typehead, * Typetail;    // List of typedefs



 int O_dumpAST;		// Ϊ��, dump the AST trees
 int O_keepasm;		// Ϊ��, ���ֻ���ļ�
 int O_assemble;	// Ϊ��, �������ļ�
 int O_dolink;		// Ϊ��, ����obj�ļ�
 int O_verbose;		// If true, print info on compilation stages
 char* Outfilename;	// ���򿪵�����ļ�����