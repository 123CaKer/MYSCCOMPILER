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
		id = addglob(Text, type, S_VARIABLE);// 向符号表添加
		genglobsym(id);// 生成全局符号（）
		semi();
	
}


// 声明函数 对应BNF参照笔记
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


// 解析变量声明
int parse_type(int t)
{
	if (t == T_CHAR) return P_CHAR;
	if (t == T_INT)  return P_INT;
	if (t == T_VOID) return P_VOID;
	fatald("Illegal type, token", t);
}

// 解析变量声明，将当前类型

