#pragma once
// Commands and default filenames
#define AOUT "a.out"
#ifdef __NASM__
#define ASCMD "nasm -f elf64 -o "
#define LDCMD "cc -no-pie -fno-plt -Wall -o "
#else
#define ASCMD "as -o "
#define LDCMD "cc -o "
#endif




// 令牌类型 具体可查阅scan()
enum 
{
	T_EOF,

	// Binary operators
	T_ASSIGN, T_LOGOR, T_LOGAND,
	T_OR, T_XOR, T_AMPER,
	T_EQ, T_NE,
	T_LT, T_GT, T_LE, T_GE,
	T_LSHIFT, T_RSHIFT,
	T_PLUS, T_MINUS, T_STAR, T_SLASH,

	// Other operators
	T_INC, T_DEC, T_INVERT, T_LOGNOT,

	// Type keywords
	T_VOID, T_CHAR, T_INT, T_LONG,

	// Other keywords
	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN, T_STRUCT,

	// Structural tokens
	T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
	T_LBRACKET, T_RBRACKET,T_COMMA,T_DOT,
	T_ARROW,T_UNION
};

// AST 节点类型
enum {
	A_ASSIGN = 1, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,
	A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
	A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,
	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,
	A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,
	A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL
};
// 变量类型 类型匹配
// Primitive types. The bottom 4 bits is an integer
// value that represents the level of indirection,
// e.g. 0= no pointer, 1= pointer, 2= pointer pointer etc.
enum 
{
	P_NONE//当前AST节点并不是表达式或者一种变量 例如 A_GLUE 
	
	, P_VOID = 16, P_CHAR = 32, P_INT = 48, P_LONG = 64, P_STRUCT = 80,
	P_UNION=96
};


// Structural types
enum
{
	S_VARIABLE, // 变量
	S_FUNCTION,  // 函数
	S_ARRAY    // 数组
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
	struct symtable* sym;		// For many AST nodes, the pointer to
	union
	{
		int intvalue;                         //整形数字
	//	int id;                     // For A_IDENT, 符号表下标
		int size;
	};

};

// 存储类型 
enum {
	C_GLOBAL = 1,		// 全局
	C_LOCAL	,		//  局部
	C_STRUCT,			// 结构体
	C_PARAM,          // Locally visible function parameter
	C_MEMBER,			// Member of a struct or union
	C_UNION			// A union
};

// 符号表
struct symtable
{
	char* name;                   // 符号名
	int type;                     // 初等基本类型
	struct symtable* ctype;	    // 若为 struct/union, 指向其类型  struct/union,
	int stype;                    // 变量还是函数			
	int class;                    // 符号表存储类型 全局还是局部
	union 
	{
		int nelems;                 // For functions, # of params
		int posn;                  //  局部变量在符号表的位置 为-- 
	};
	union 
	{
		int size;			// 符号表中的符号数量
		int endlabel;		// For functions, the end label
	};
	struct symtable* next;	    // Next symbol in one list
	struct symtable* member;	// First member of a function, struct,
	// union or enum
	                            // 函数的形参 结构体成员
};



