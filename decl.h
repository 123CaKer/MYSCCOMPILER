#pragma once
int scan(struct token* t); // 判断令牌内容
struct ASTnode* mkastnode(int op, struct ASTnode* left,struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int intvalue);// 生成叶子节点
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue);// 生成左子树AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);
static struct ASTnode* primary();//解析 token 并判断其对应的ASTNode 应赋值类型为 A_INTLIT 
// static 每个文件