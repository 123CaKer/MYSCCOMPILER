#pragma once

// 令牌
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

// AST 节点类型
enum
{
	A_ADD = 1, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
	A_INTLIT,
	A_IDENT, A_LVIDENT, A_ASSIGN, A_PRINT, A_GLUE, A_IF
	
};

// 令牌结构体
struct token
{
	int token;   //上述的token类型
	int intvalue;  // 内部的值
};

// 
struct ASTnode
{
	int op;                               //  opertor 例如 3+2 中的 +  即A_ADD  A_SUBTRACT
	struct ASTnode* left; 
	struct ASTnode* mid;
	struct ASTnode* right;
	union
	{
		int intvalue;                         //整形数字
		int id;                     // For A_IDENT, the symbol slot number
	}v;
	
};


// 符号表
struct symtable
{
	char* name;                   // 符号名
};