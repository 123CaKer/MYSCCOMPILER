#pragma once
#include"defs.h"
#include<stdlib.h>


// misc.c
void semi(void);
// 声明变量
// 匹配关键字
void match(int t, char* what);
// 匹配分号
void semi(void);
// 匹配非关键字
void ident(void);
// 错误信息
void fatal(char* s);
void fatals(char* s1, char* s2);
void fatald(char* s, int d);
void fatalc(char* s, int c);
void lbrace(void);
void rbrace(void);
void lparen(void);
void rparen(void);





int scan(struct token* t); // 判断令牌内容
struct ASTnode* mkastnode(int op, int type, struct ASTnode* left, struct ASTnode* mid, struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int type, int intvalue);// 生成叶子节点
struct ASTnode* mkastunary(int op, int type, struct ASTnode* left, int intvalue);// 生成左子树AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);

// cg.c
//void generatecode(struct ASTnode* n);
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgloadint(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);
int cgloadglob(int id,int op);
// Store a register's value into a variable
int cgstorglob(int r, int id);
//生成全局符号
void genglobsym(int id);
void cgfuncpreamble(int);
void cgfuncpostamble(int id);
void cgreturn(int reg, int id);
void reject_token(struct token* t);


// 判断
int cgequal(int r1, int r2);
int cgnotequal(int r1, int r2);
int cglessthan(int r1, int r2);
int cggreaterthan(int r1, int r2);
int cglessequal(int r1, int);
int cggreaterequal(int r1, int r2);

void cglabel(int l);
void cgjump(int l);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);



//decl.c
// 声明变量
void var_declaration(int type);

//expr.c
//将 表达式符号转换为AST对应符号
int arithop(int tokentype);
// 获取运算符优先级
static int op_precedence(int tokentype);
// 生成语法树 返回root为+ - * /的ast树  其中p为之前的优先级
struct ASTnode* binexpr(int p);

struct ASTnode* funccall(void); //函数调用
struct ASTnode* array_access(void); //数组访问


//expr2.c
// 返回乘性表达式
//struct ASTnode* multiplicative_expr(void);
// 加性表达式
//struct ASTnode* additive_expr(void);


// gen.c
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(int id);
int genprimsize(int type);


//interp.c
// 解释AST值   可改为汇编接口代码在gen.c
//int interpretAST(struct ASTnode* n);



// stmt.c
struct ASTnode* print_statement();
struct ASTnode* assignment_statement();
struct ASTnode* if_statement();

struct ASTnode* for_statement();
struct ASTnode* single_statement();
struct ASTnode* compound_statement();
struct ASTnode* while_statement();


// sym.c
int findglob(char* s);
int newglob(void);
int addglob(char* name, int type, int stype, int endlabel,int size);

// types.c
int type_compatible(int* left, int* right, int onlyright);

// gen.c中的静态代码对应的是汇编中的 Label:
//下标识
 struct ASTnode* primary();//解析 token 并判断其对应的ASTNode （语义）
// static 每个文件

