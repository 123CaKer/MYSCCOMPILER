#include "defs.h"
#include "data.h"
#include "decl.h"

 
// 声明变量
void var_declaration()
{
		int id, type;
		type = parse_type(Token.token);// 解析当前类型
		scan(&Token);
		ident();
		id = addglob(Text, type, S_VARIABLE,0);// 向符号表添加
		genglobsym(id);// 生成全局符号（）
		semi();
	
}


// 声明函数 对应BNF参照笔记
struct ASTnode* function_declaration()
{
	struct ASTnode* tree,* finalstmt;
	int nameslot,type,endlabel;
	


	type = parse_type(Token.token);//获取当前函数声明类型 eg int add（）  为int
	scan(&Token);
	ident();
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
int parse_type(int t)
{
	if (t == T_CHAR)
		return P_CHAR;
	if (t == T_INT)
		return P_INT;
	if (t == T_VOID)
		return P_VOID;
	fatald("Illegal type, token", t);
}

// 解析变量声明，将当前类型

