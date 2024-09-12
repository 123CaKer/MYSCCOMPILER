#include "defs.h"
#include "data.h"
#include "decl.h"

 
// ��������
void var_declaration()
{
		int id, type;
		type = parse_type(Token.token);// ������ǰ����
		scan(&Token);
		ident();
		id = addglob(Text, type, S_VARIABLE);// ����ű����
		genglobsym(id);// ����ȫ�ַ��ţ���
		semi();
	
}


// �������� ��ӦBNF���ձʼ�
struct ASTnode* function_declaration()
{
	struct ASTnode* tree;
	int nameslot;
	match(T_VOID,"void");
	ident();
	nameslot = addglob(Text,P_VOID,S_FUNCTION);
	lparen();
	rparen();
	tree = compound_statement();
	return mkastunary(A_FUNCTION,P_VOID, tree, nameslot);

}


// ������������
int parse_type(int t)
{
	if (t == T_CHAR) return P_CHAR;
	if (t == T_INT)  return P_INT;
	if (t == T_VOID) return P_VOID;
	fatald("Illegal type, token", t);
}

// ������������������ǰ����

