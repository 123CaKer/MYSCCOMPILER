#include "defs.h"
#include "data.h"
#include "decl.h"

 
// ��������
void var_declaration()
{
		int id, type;
		type = parse_type(Token.token);// ������ǰ����
		//scan(&Token);
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
	//scan(&Token);
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
	int typer=-1;
	switch (t)
	{
	case T_CHAR:
		typer= P_CHAR;
		break;
	case T_INT:
		typer= P_INT;
		break;
	case T_VOID:
		typer= P_VOID;
		break;
	case T_LONG:
		typer =P_LONG;
		break;
	default:
		fatald("Illegal type, token", t);
	}
		
	while (1)
	{
		scan(&Token);
		if (Token.token==T_STAR)
			typer = pointer_to(typer);
		else
			break;
	}
	return typer;
	
}

// ������������������ǰ����

