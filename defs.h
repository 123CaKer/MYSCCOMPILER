#pragma once

// ����
enum 
{
	T_EOF,
	T_PLUS, T_MINUS,
	T_STAR, T_SLASH,
	T_EQ, T_NE,
	T_LT, T_GT, T_LE, T_GE,
	T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
	
	
	// Keywords
	T_PRINT, T_INT, T_IF, T_ELSE
};

// AST �ڵ�����
enum
{
	A_ADD = 1, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
	A_INTLIT,
	A_IDENT, A_LVIDENT, A_ASSIGN, A_PRINT, A_GLUE, A_IF
	
};

// ���ƽṹ��
struct token
{
	int token;   //������token����
	int intvalue;  // �ڲ���ֵ
};

// 
struct ASTnode
{
	int op;                               //  opertor ���� 3+2 �е� +  ��A_ADD  A_SUBTRACT
	struct ASTnode* left; 
	struct ASTnode* mid;
	struct ASTnode* right;
	union
	{
		int intvalue;                         //��������
		int id;                     // For A_IDENT, the symbol slot number
	}v;
	
};


// ���ű�
struct symtable
{
	char* name;                   // ������
};