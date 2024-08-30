#pragma once
#include<stdio.h>
#ifndef extern_
#define extern_ extern
#endif
#define TEXTLEN		1024	// 缓冲区长度为1024


extern_ int     Line;     // 当前令牌所在的读取行数
extern_ int	    Putback;  // 需要返回输入流的值 一般是不想要的
extern_ FILE*   Infile;   // 文件所读取的指针
extern_ FILE*   Outfile;  // 输出文件指针
extern_ struct token	Token;  // 全局令牌
extern_ char Text[TEXTLEN + 1];		// Last identifier scanned