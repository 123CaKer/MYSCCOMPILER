#pragma once
#include<stdio.h>
#ifndef extern_
#define extern_ extern
#endif
#define TEXTLEN		1024	// ����������Ϊ1024


extern_ int     Line;     // ��ǰ�������ڵĶ�ȡ����
extern_ int	    Putback;  // ��Ҫ������������ֵ һ���ǲ���Ҫ��
extern_ FILE*   Infile;   // �ļ�����ȡ��ָ��
extern_ FILE*   Outfile;  // ����ļ�ָ��
extern_ struct token	Token;  // ȫ������
extern_ char Text[TEXTLEN + 1];		// Last identifier scanned