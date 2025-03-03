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
// 宏定义相关

#if 0

/*
*  使用msvc 进行生成
   先生成.i 后生成 .s
   参考网址
   https://www.cnblogs.com/doubleconquer/p/17765267.html#%E9%A2%84%E5%A4%84%E7%90%86%E5%99%A8%E9%80%89%E9%A1%B9

   我认为最后的 msvc命令为
   cl /P  ..//needcompilefile/hello.i  /I ..//needcompilefile/tmp/include  ..//needcompilefile/hello.c

   绝对路径为
	cl /P  F:/compilering/needcompilefile/hello.i  /I  F:/compilering/needcompilefile/tmp/include  F:/compilering/needcompilefile//hello.c

*/
#define CPPCMD "cl /P  ..//needcompilefile/hello.i  /I"
#define INCDIR  "..//needcompilefile/tmp/include"   // 自定义include头文件路径 而非使用正常的 /usr/include
#define BINDIR  "..//needcompilefile/tmp"

#endif // 0



// 令牌类型 具体可查阅scan()
enum
{
	T_EOF,

	// Binary operators
	T_ASSIGN, T_ASPLUS, T_ASMINUS,
	T_ASSTAR, T_ASSLASH, T_ASMOD,
	T_QUESTION, T_LOGOR, T_LOGAND,
	T_OR, T_XOR, T_AMPER,
	T_EQ, T_NE,
	T_LT, T_GT, T_LE, T_GE,
	T_LSHIFT, T_RSHIFT,
	T_PLUS, T_MINUS, T_STAR, T_SLASH, T_MOD,

	// Other operators
	T_INC, T_DEC, T_INVERT, T_LOGNOT,

	// Type keywords
	T_VOID, T_CHAR, T_INT, T_LONG,

	// Other keywords
	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
	T_STRUCT, T_UNION, T_ENUM, T_TYPEDEF,
	T_EXTERN, T_BREAK, T_CONTINUE, T_SWITCH,
	T_CASE, T_DEFAULT, T_SIZEOF, T_STATIC,

	// Structural tokens
	T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
	T_LBRACKET, T_RBRACKET, T_COMMA, T_DOT,
	T_ARROW, T_COLON
};




// AST 节点类型
// AST node types. The first few line up
// with the related tokens
enum {
	A_ASSIGN = 1, A_ASPLUS, A_ASMINUS, A_ASSTAR,			//  1 开始
	A_ASSLASH, A_ASMOD, A_TERNARY, A_LOGOR,			//  5
	A_LOGAND, A_OR, A_XOR, A_AND, A_EQ, A_NE, A_LT,		//  9
	A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,				// 16
	A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_MOD,		// 21
	A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,				// 26
	A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,			// 30
	A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,				// 35
	A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,			// 39
	A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL, A_BREAK,		// 43
	A_CONTINUE, A_SWITCH, A_CASE, A_DEFAULT, A_CAST		// 48
};
// 变量类型 类型匹配
// Primitive types. The bottom 4 bits is an integer
// value that represents the level of indirection,
// e.g. 0= no pointer, 1= pointer, 2= pointer pointer etc.
enum
{
	P_NONE//当前AST节点并不是表达式或者一种变量 例如 A_GLUE   P_NONE 用于释放类型限制详见chr 42

	, P_VOID = 16, P_CHAR = 32, P_INT = 48, P_LONG = 64, P_STRUCT = 80,
	P_UNION = 96
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
	int token;     //上述的token类型
	char* tokstr;  // String version of the token
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
	struct symtable* ctype;	// If struct/union, ptr to that type 为了多级指针使用 即 a->next->next
	int rvalue;                   // 是否为右值 1 为右边
	struct symtable* sym;		// For many AST nodes, the pointer to
#define a_intvalue a_size	// For A_INTLIT, a_intvalue 整形数字
/*
This way, I still have two named fields sharing the same location in each struct,
but the compiler will see only the one field name in each struct
. I've given each #define a different prefix to prevent pollution of the global namespace.
*/
	int a_size;			// For A_SCALE, the size to scale by
};

// 符号表存储类型 
enum
{
	C_GLOBAL = 1,			// Globally visible symbol
	C_LOCAL,			// Locally visible symbol
	C_PARAM,			// Locally visible function parameter
	C_EXTERN,			// External globally visible symbol
	C_STATIC,			// Static symbol, visible in one file
	C_STRUCT,			// A struct
	C_UNION,			// A union
	C_MEMBER,			// Member of a struct or union
	C_ENUMTYPE,			// A named enumeration type
	C_ENUMVAL,			// A named enumeration value
	C_TYPEDEF			// A named typedef
};

// 符号表
struct symtable
{
	char* name;                   // 符号名
	int type;                     // 初等基本类型 int char long void P_***
	struct symtable* ctype;	     // 若为 struct/union, 指向其类型  struct/union, C_*** 为 符号表存储类型 
	int stype;                    // 变量还是函数			
	int class;                    // 符号表存储类型 全局还是局部
	int size;			// 符号表大小
	int nelems;			// Functions: # params. Arrays: # elements 函数和形参的个数
#define st_endlabel st_posn	// For functions, the end label
#define st_hasaddr  st_posn	// For locals, 1 if any A_ADDR operation
/*
This way, I still have two named fields sharing the same location in each struct,
but the compiler will see only the one field name in each struct
. I've given each #define a different prefix to prevent pollution of the global namespace.
*/
	int st_posn;			// For locals, the negative offset// from the stack base pointer （rbp）

	int* initlist;		// 初始化列表 eg int a，v,c
	struct symtable* next;	// Next symbol in one list
	struct symtable* member;	// First member of a function, struct, union or enum 函数的形参 结构体成员 第一个

};



