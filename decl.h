#pragma once
#include"defs.h"
#include<stdlib.h>


// misc.c
void semi(void);
// ��������
// ƥ��ؼ���
void match(int t, char* what);
// ƥ��ֺ�
void semi(void);
// ƥ��ǹؼ���
void ident(void);
// ������Ϣ
void fatal(char* s);
void fatals(char* s1, char* s2);
void fatald(char* s, int d);
void fatalc(char* s, int c);
void lbrace(void);
void rbrace(void);
void lparen(void);
void rparen(void);





int scan(struct token* t); // �ж���������
struct ASTnode* mkastnode(int op, int type, struct ASTnode* left, struct ASTnode* mid, struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int type, int intvalue);// ����Ҷ�ӽڵ�
struct ASTnode* mkastunary(int op, int type, struct ASTnode* left, int intvalue);// ����������AST 
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
//����ȫ�ַ���
void genglobsym(int id);
void cgfuncpreamble(int);
void cgfuncpostamble(int id);
void cgreturn(int reg, int id);
void reject_token(struct token* t);


// �ж�
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
// ��������
void var_declaration(int type);

//expr.c
//�� ���ʽ����ת��ΪAST��Ӧ����
int arithop(int tokentype);
// ��ȡ��������ȼ�
static int op_precedence(int tokentype);
// �����﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�
struct ASTnode* binexpr(int p);

struct ASTnode* funccall(void); //��������
struct ASTnode* array_access(void); //�������


//expr2.c
// ���س��Ա��ʽ
//struct ASTnode* multiplicative_expr(void);
// ���Ա��ʽ
//struct ASTnode* additive_expr(void);


// gen.c
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(int id);
int genprimsize(int type);


//interp.c
// ����ASTֵ   �ɸ�Ϊ���ӿڴ�����gen.c
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

// gen.c�еľ�̬�����Ӧ���ǻ���е� Label:
//�±�ʶ
 struct ASTnode* primary();//���� token ���ж����Ӧ��ASTNode �����壩
// static ÿ���ļ�

