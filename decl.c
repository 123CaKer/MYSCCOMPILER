#include "defs.h"
#include "data.h"
#include "decl.h"


// 声明变量 type 为变量类型 int char 。。。。
void var_declaration(int type, int islocal,int isparam)
{
	int id;
	// 如果匹配[
	if (Token.token == T_LBRACKET)
	{
		// Skip past the '['
		scan(&Token);

		 //检查 []中是否有 数字
		if (Token.token == T_INTLIT)
		{
			if (islocal) 
			{
				//addlocl(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
				fatal("For now, declaration of local arrays is not implemented");
			}
			else 
			{
				addglob(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
			}
		}

		// 确保匹配 ]
		scan(&Token);
		match(T_RBRACKET, "]");
	}
	else
	{
		if (islocal) 
		{
			if (addlocl(Text, type, S_VARIABLE, isparam, 1) == -1)
				fatals("Duplicate local variable declaration", Text);
		}
		else 
		{
			addglob(Text, type, S_VARIABLE, 0, 1);
		}
	}

	// 分号
 //	semi();
}

// param_declaration: <null>
//           | variable_declaration
//           | variable_declaration ',' param_declaration
//
// Parse the parameters in parentheses after the function name.
// Add them as symbols to the symbol table and return the number
// of parameters.
static int param_declaration(void)
{
	int type;
	int paramcnt = 0;

	// Loop until the final right parentheses
	//  循环取值 （，，，，，，，............. ）
	while (Token.token != T_RPAREN)
	{
		// Get the type and identifier
		// and add it to the symbol table
		type = parse_type();
		ident();
		var_declaration(type, 1, 1);
		paramcnt++;

		// Must have a ',' or ')' at this point
		switch (Token.token) 
		{
		case T_COMMA: scan(&Token); break;
		case T_RPAREN: break;
		default:
			fatald("Unexpected token in parameter list", Token.token);
		}
	}

	// 返回函数参数个数
	return(paramcnt);
}


// 声明函数 对应BNF参照笔记
#if 0
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree, * finalstmt;
	int nameslot, endlabel;


	/*
	type = parse_type(Token.token);//获取当前函数声明类型 eg int add（）  为int
	//scan(&Token);
	ident();
	*/
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel);
	Functionid = nameslot;//当前函数下标
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
	int nameslot, endlabel,paramcnt;

	// Text now has the identifier's name.
	// Get a label-id for the end label, add the function
	// to the symbol table, and set the Functionid global
	// to the function's symbol-id
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel,0);
	Functionid = nameslot;

//	genresetlocals();		// 重新设置局部变量的位置

	 // Scan in the parentheses and any parameters
  // Update the function symbol entry with the number of parameters
	lparen();
	paramcnt = param_declaration();
	Gsym[nameslot].nelems = paramcnt;
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


// 解析变量声明 符号表
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

	while (1) //  *****************p
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

			// Now free the symbols associated  with this function
			freeloclsyms();
		}
		else
		{

			// 解析全局变量声明 并且匹配分号
			var_declaration(type, 0, 0);
			semi();
		}
		if (Token.token == T_EOF)
			break;
	}
}

