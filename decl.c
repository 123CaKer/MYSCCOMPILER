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