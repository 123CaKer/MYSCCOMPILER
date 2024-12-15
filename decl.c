#include "defs.h"
#include "data.h"
#include "decl.h"


// �������� type Ϊ�������� int char ��������
void var_declaration(int type)
{
	int id;
	// ���ƥ��[
	if (Token.token == T_LBRACKET)
	{
		// Skip past the '['
		scan(&Token);

		 //��� []���Ƿ��� ����
		if (Token.token == T_INTLIT)
		{
			// Add this as a known array and generate its space in assembly.
			// We treat the array as a pointer to its elements' type
			id = addglob(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
			genglobsym(id);
		}

		// ȷ��ƥ�� ]
		scan(&Token);
		match(T_RBRACKET, "]");
	}
	else
	{
		// Add this as a known scalar
		// and generate its space in assembly
		id = addglob(Text, type, S_VARIABLE, 0, 1);// ���
		genglobsym(id);
	}

	// �ֺ�
	semi();
}


// �������� ��ӦBNF���ձʼ�
#if 0
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree, * finalstmt;
	int nameslot, endlabel;


	/*
	type = parse_type(Token.token);//��ȡ��ǰ������������ eg int add����  Ϊint
	//scan(&Token);
	ident();
	*/
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel);
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

	return mkastunary(A_FUNCTION, type, tree, nameslot);
}
#endif
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree, * finalstmt;
	int nameslot, endlabel;

	// Text now has the identifier's name.
	// Get a label-id for the end label, add the function
	// to the symbol table, and set the Functionid global
	// to the function's symbol-id
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel,0);
	Functionid = nameslot;

	// Scan in the parentheses
	lparen();
	rparen();

	// Get the AST tree for the compound statement
	tree = compound_statement();

	// If the function type isn't P_VOID ..
	if (type != P_VOID)
	{

		// Error if no statements in the function
		if (tree == NULL)
			fatal("No statements in function with non-void type");

		// Check that the last AST operation in the
		// compound statement was a return statement
		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
		if (finalstmt == NULL || finalstmt->op != A_RETURN)
			fatal("No return for function with non-void type");
	}
	// Return an A_FUNCTION node which has the function's nameslot
	// and the compound statement sub-tree
	return (mkastunary(A_FUNCTION, type, tree, nameslot));
}


// ������������ ���ű�
int parse_type()
{
	int typer = -1;
	switch (Token.token)
	{
	case T_CHAR:
		typer = P_CHAR;
		break;
	case T_INT:
		typer = P_INT;
		break;
	case T_VOID:
		typer = P_VOID;
		break;
	case T_LONG:
		typer = P_LONG;
		break;
	default:
		fatald("Illegal type, token", Token.token);
	}

	while (1)
	{
		scan(&Token);
		if (Token.token == T_STAR)
			typer = pointer_to(typer);
		else
			break;
	}
	return typer;

}


void global_declarations(void)
{
	struct ASTnode* tree;
	int type;

	while (1)
	{
		type = parse_type();
		ident();
		if (Token.token == T_LPAREN)
		{

			// Parse the function declaration and
			// generate the assembly code for it
			tree = function_declaration(type);
			if (O_dumpAST)
			{
				dumpAST(tree, NOLABEL, 0);
				fprintf(stdout, "\n\n");
			}
			genAST(tree, NOLABEL, 0);
		}
		else
		{

			// Parse the global variable declaration
			var_declaration(type);

		}
		if (Token.token == T_EOF)
			break;
	}
}

