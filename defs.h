#pragma once

// ��������
enum
{
	T_EOF,
	// Operators
	T_ASSIGN, // =
	T_PLUS,  // +
	T_MINUS, // -
	T_STAR, // *
	T_SLASH, // /

	T_EQ, T_NE,
	T_LT, T_GT, T_LE, T_GE,
	// Type keywords
	T_VOID, T_CHAR, T_INT, T_LONG,

	// Structural tokens
	T_STRLIT,  // �ַ���
	T_INTLIT, // ����
	T_SEMI,  // ;
	T_IDENT, // ��ʶ�� ����
	T_LBRACE, // {
	T_RBRACE, //  }
	T_LPAREN, //  (
	T_RPAREN,//    )
	T_LBRACKET,  // [
	T_RBRACKET,  // ]
	T_AMPER,  // & &a
	T_LOGAND, // &&



	// Other keywords
	T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN

};

// AST �ڵ�����
enum
{
	A_ASSIGN = 1,
	A_ADD,
	A_SUBTRACT, 
	A_MULTIPLY,
	A_DIVIDE,
	A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,

	A_INTLIT, // ����
	A_STRLIT,// �ַ���
	A_IDENT,  // ��ʶ��
	A_GLUE,  //  ���ճ��

	A_IF,
	A_WHILE, 
	A_FUNCTION, // ����
	A_WIDEN,    // char - int 
	A_RETURN,   // return
	A_FUNCCALL, // a=fun() ��������
	A_DEREF,  // *p
	A_ADDR,  // &a
	A_SCALE  // ��С
};

// �������� ����ƥ��
enum
{
	P_NONE,  //��ǰAST�ڵ㲢���Ǳ��ʽ����һ�ֱ��� ���� A_GLUE 
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
	union
	{
		int intvalue;                         //��������
		int id;                     // For A_IDENT, ���ű��±�
		int size;
	}v;

};


// ���ű�
struct symtable
{
	char* name;                   // ������
	int type;                     // ���Ȼ�������
	int stype;                    // �������Ǻ���
	int endlabel;			      // For S_FUNCTIONs, the end label
	int size;                     // ���ű��еķ�������
};



