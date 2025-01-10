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




// �������� ����ɲ���scan()
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

// AST �ڵ�����
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
// �������� ����ƥ��
// Primitive types. The bottom 4 bits is an integer
// value that represents the level of indirection,
// e.g. 0= no pointer, 1= pointer, 2= pointer pointer etc.
enum 
{
	P_NONE//��ǰAST�ڵ㲢���Ǳ��ʽ����һ�ֱ��� ���� A_GLUE 
	
	, P_VOID = 16, P_CHAR = 32, P_INT = 48, P_LONG = 64, P_STRUCT = 80,
	P_UNION=96
};


// Structural types
enum
{
	S_VARIABLE, // ����
	S_FUNCTION,  // ����
	S_ARRAY    // ����
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
	int type;
	struct ASTnode* left;
	struct ASTnode* mid;
	struct ASTnode* right;
	int rvalue;                   // �Ƿ�Ϊ��ֵ 1 Ϊ�ұ�
	struct symtable* sym;		// For many AST nodes, the pointer to
	union
	{
		int intvalue;                         //��������
	//	int id;                     // For A_IDENT, ���ű��±�
		int size;
	};

};

// �洢���� 
enum {
	C_GLOBAL = 1,		// ȫ��
	C_LOCAL	,		//  �ֲ�
	C_STRUCT,			// �ṹ��
	C_PARAM,          // Locally visible function parameter
	C_MEMBER,			// Member of a struct or union
	C_UNION			// A union
};

// ���ű�
struct symtable
{
	char* name;                   // ������
	int type;                     // ���Ȼ�������
	struct symtable* ctype;	    // ��Ϊ struct/union, ָ��������  struct/union,
	int stype;                    // �������Ǻ���			
	int class;                    // ���ű�洢���� ȫ�ֻ��Ǿֲ�
	union 
	{
		int nelems;                 // For functions, # of params
		int posn;                  //  �ֲ������ڷ��ű��λ�� Ϊ-- 
	};
	union 
	{
		int size;			// ���ű��еķ�������
		int endlabel;		// For functions, the end label
	};
	struct symtable* next;	    // Next symbol in one list
	struct symtable* member;	// First member of a function, struct,
	// union or enum
	                            // �������β� �ṹ���Ա
};



