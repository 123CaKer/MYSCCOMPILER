#pragma once
// 令牌
enum 
{
	T_EOF,     // 树末尾
	T_PLUS,    // +
	T_MINUS,   // -
	T_STAR,    // *
	T_SLASH,    // /
	T_INTLIT   // 整型
};

// AST 节点类型
enum 
{
	A_ADD,
	A_SUBTRACT, 
	A_MULTIPLY, 
	A_DIVIDE,
	A_INTLIT
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
	struct ASTnode* right;
	int intvalue;                         //整形数字
};