#include "defs.h"
#include "data.h"
#include "decl.h"

 
// 声明变量 type 为变量类型 int char 。。。。
void var_declaration(int type)
{
	int id;
	while (1)
	{
		//scan(&Token);
		id = addglob(Text, type, S_VARIABLE, 0);// 向符号表添加
		genglobsym(id);// 生成全局符号
      // 匹配下一个token
		if (Token.token == T_SEMI)
		{
			scan(&Token);
			return;
		}
		if (Token.token == T_COMMA)
		{
			scan(&Token);
			ident();
			continue;
		}
		fatal("Missing , or ; after identifier");
	}	
}


// 声明函数 对应BNF参照笔记
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree,* finalstmt;
	int nameslot,endlabel;
	

	/*
	type = parse_type(Token.token);//获取当前函数声明类型 eg int add（）  为int
	//scan(&Token);
	ident();
	*/
	endlabel = genlabel();
	nameslot = addglob(Text,type,S_FUNCTION,endlabel);
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

	return mkastunary(A_FUNCTION,type, tree, nameslot);

}


// 解析变量声明
int parse_type()
{
	int typer=-1;
	switch (Token.token)
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
		fatald("Illegal type, token", Token.token);
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


void global_declarations(void)
{
	struct ASTnode* tree;
	int type;

	while (1) 
	{
		type = parse_type();
		ident();
		if (Token.token == T_LPAREN) //如果是（ 则为函数
		{
			tree = function_declaration(type);
			genAST(tree, NOREG, 0);
		}
		else // 变量
		{
			var_declaration(type);
		}

		if (Token.token == T_EOF)
			break;
	}
}

