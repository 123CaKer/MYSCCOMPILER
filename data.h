#pragma once
#include<stdio.h>
 
#ifndef myextern_
#define myextern_ extern

#endif

#define TEXTLEN		    1024	        // 缓冲区长度为1024
#define NSYMBOLS        1024            // 最大符号表输入数
#define NOREG	-1		// Use NOREG when the AST generation functions have no register to return

int   Line;     // 当前令牌所在的读取行数
int	  Putback;  // 需要返回输入流的值 一般是不想要的
FILE*   Infile;   // 文件所读取的指针
FILE*   Outfile;  // 输出文件指针
struct token	Token;  // 全局令牌
char Text[TEXTLEN + 1];		// 标识符存储 buff
struct symtable Gsym[NSYMBOLS]; // 全局符号表 存储变量名等
int Functionid;         // 符号表中当前函数下标识