#include "defs.h"
#include "data.h"
#include "decl.h"

 
// ��������
void var_declaration(void)
{

	
	match(T_INT, "int");
	ident();
	addglob(Text);  // ȫ�ַ��ű���� Ҳ���� int x; �� x������ű�
	genglobsym(Text);
	semi(); //;
}


// �������� ��ӦBNF���ձʼ�
struct ASTnode* function_declaration()
{
	struct ASTnode* tree;
	int nameslot;
	match(T_VOID,"void");
	ident();
	nameslot = addglob(Text);
	lparen();
	rparen();
	tree = compound_statement();
	return mkastunary(A_FUNCTION, tree, nameslot);

}