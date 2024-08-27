#pragma once
// ����
enum 
{
	T_EOF,     // ��ĩβ
	T_PLUS,    // +
	T_MINUS,   // -
	T_STAR,    // *
	T_SLASH,    // /
	T_INTLIT   // ����
};

// AST �ڵ�����
enum 
{
	A_ADD,
	A_SUBTRACT, 
	A_MULTIPLY, 
	A_DIVIDE,
	A_INTLIT
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
	struct ASTnode* right;
	int intvalue;                         //��������
};