#pragma once

// 令牌类型
enum
{
	T_EOF,
	// Operators
	T_ASSIGN,
	T_PLUS, T_MINUS,
	T_STAR, T_SLASH,
	T_EQ, T_NE,
	T_LT, T_GT, T_LE, T_GE,
	// Type keywords
	T_VOID, T_CHAR, T_INT, T_LONG,
	// Structural tokens
	T_INTLIT, T_SEMI, T_IDENT,
	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
	T_AMPER, T_LOGAND,
	// Other keywords
	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN
};

// AST 节点类型
enum
{
	A_ASSIGN = 1, A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
	A_INTLIT, A_IDENT, A_GLUE,
	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE
};

// 变量类型 类型匹配
enum
{
	P_NONE,  //当前AST节点并不是表达式或者一种变量 例如 A_GLUE 
	P_VOID,
	P_CHAR,
	P_INT,
	P_LONG,
	P_VOIDPTR,
	P_CHARPTR,
	P_INTPTR,
	P_LONGPTR

};


// Structural types
enum
{
	S_VARIABLE, // 变量
	S_FUNCTION  // 函数
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
	int type;
	struct ASTnode* left;
	struct ASTnode* mid;
	struct ASTnode* right;
	int rvalue;                   // 是否为右值 1 为右边
	union
	{
		int intvalue;                         //整形数字
		int id;                     // For A_IDENT, 符号表下标
		int size;
	}v;

};


// 符号表
struct symtable
{
	char* name;                   // 符号名
	int type;                     // 初等基本类型
	int stype;                    // 变量还是函数
	int endlabel;			// For S_FUNCTIONs, the end label
};



