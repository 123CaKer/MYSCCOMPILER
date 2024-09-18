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
		id = addglob(Text, type, S_VARIABLE,0);// ����ű����
		genglobsym(id);// ����ȫ�ַ��ţ���
		semi();
	
}


// �������� ��ӦBNF���ձʼ�
struct ASTnode* function_declaration()
{
	struct ASTnode* tree,* finalstmt;
	int nameslot,type,endlabel;
	


	type = parse_type(Token.token);//��ȡ��ǰ������������ eg int add����  Ϊint
	scan(&Token);
	ident();
	endlabel = genlabel();
	nameslot = addglob(Text,type,S_FUNCTION,endlabel);
	Functionid = nameslot;//��ǰ�����±�
	lparen();
	rparen();
	tree = compound_statement();
	if (type != P_VOID) 
	{
		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;

		/*
		         A_FUN
				 /   \
			 A_GLUE                    tree->op 
			 /     \
	A_ASSIGN       A_PRINT
	 /   \           /   \
	2     j          j     print
		
		
		*/

		if (finalstmt == NULL || finalstmt->op != A_RETURN)
			fatal("No return for function with non-void type");
	}

	return mkastunary(A_FUNCTION,type, tree, nameslot);

}


// ������������
int parse_type(int t)
{
	if (t == T_CHAR)
		return P_CHAR;
	if (t == T_INT)
		return P_INT;
	if (t == T_VOID)
		return P_VOID;
	if (t==T_LONG)
		return P_LONG;
	fatald("Illegal type, token", t);
}

// ������������������ǰ����

