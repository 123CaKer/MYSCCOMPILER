#include "defs.h"
#include "data.h"
#include "decl.h"

 
// 声明变量
void var_declaration(void)
{

	
	match(T_INT, "int");
	ident();
	addglob(Text);  // 全局符号表加入 也就是 int x; 把 x加入符号表
	genglobsym(Text);
	semi(); //;
}


// 声明函数 对应BNF参照笔记
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