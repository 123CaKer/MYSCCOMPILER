#pragma once
int scan(struct token* t); // �ж���������
struct ASTnode* mkastnode(int op, struct ASTnode* left,struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int intvalue);// ����Ҷ�ӽڵ�
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue);// ����������AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);
static struct ASTnode* primary();//���� token ���ж����Ӧ��ASTNode Ӧ��ֵ����Ϊ A_INTLIT 
// static ÿ���ļ�