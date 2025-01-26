#pragma once
#include<stdio.h>

#ifndef myextern_
#define myextern_ extern

#endif

#define TEXTLEN		    1024	        // 缓冲区长度为1024
#define NSYMBOLS        1024            // 最大符号表输入数
#define NOREG	-1		// genAST 的相关函数不适用寄存器 即（-1） 时
#define NOLABEL	 0		// Use NOLABEL when we have no label to pass to genAST()

int   Line;     // 当前令牌所在的读取行数
int	  Putback;  // 需要返回输入流的值 一般是不想要的
FILE* Infile;   // 文件所读取的指针
FILE* Outfile;  // 输出文件指针
char* Infilename;		// Name of file we are parsing
char* Outfilename;		// Name of file we opened as Outfile
struct token	Token;  // 全局令牌
struct token Peektoken;		// 全局令牌的下一个
char Text[TEXTLEN + 1];		// 标识符存储 buff
//struct symtable Gsym[NSYMBOLS]; // 全局符号表 存储变量名等
struct symtable* Functionid; 	// 当前函数的符号表下标识符,现在修改为struct symtable* 当前为函数符号链表
int O_dumpAST;         // 调试输出AST节点
//int Globsq;  // Position of next free global symbol slot  arm
int Globs;		// Position of next free global symbol slot
int Locls;		// Position of next free local symbol slot
int Looplevel;                  // 循环深度
int Switchlevel;		// switch深度
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



 int O_dumpAST;		// 为真, dump the AST trees
 int O_keepasm;		// 为真, 保持汇编文件
 int O_assemble;	// 为真, 编译汇编文件
 int O_dolink;		// 为真, 连接obj文件
 int O_verbose;		// If true, print info on compilation stages
 char* Outfilename;	// 被打开的输出文件名字