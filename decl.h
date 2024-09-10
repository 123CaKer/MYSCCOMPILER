#pragma once
#include"defs.h"
#include<stdlib.h>
 

// misc.c
void match(int t, char* what);
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





int scan(struct token* t); // �ж���������
struct ASTnode* mkastnode(int op, struct ASTnode* left, struct ASTnode* mid,struct ASTnode* right, int intvalue);
struct ASTnode* mkastleaf(int op, int intvalue);// ����Ҷ�ӽڵ�
struct ASTnode* mkastunary(int op, struct ASTnode* left, int intvalue);// ����������AST 
int mkastfree(struct ASTnode* ASTNode);
struct ASTnode* binexpr(int);
int interpretAST(struct ASTnode* n);

// cg.c
void generatecode(struct ASTnode* n);
void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgloadint(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
void cgprintint(int r);
int cgloadglob(char* identifier);
// Store a register's value into a variable
int cgstorglob(int r, char* identifier);
//����ȫ�ַ���
void cgglobsym(char* sym);

void cgfuncpreamble(char* name);
void cgfuncpostamble();


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
void var_declaration(void);

//expr.c
//�� ���ʽ����ת��ΪAST��Ӧ����
int arithop(int tokentype);
// ��ȡ��������ȼ�
static int op_precedence(int tokentype);
// �����﷨�� ����rootΪ+ - * /��ast��  ����pΪ֮ǰ�����ȼ�
struct ASTnode* binexpr(int p);


//expr2.c
// ���س��Ա��ʽ
struct ASTnode* multiplicative_expr(void);
// ���Ա��ʽ
struct ASTnode* additive_expr(void);


// gen.c
void genpreamble();
void genpostamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(char* s);


//interp.c
// ����ASTֵ   �ɸ�Ϊ���ӿڴ�����gen.c
int interpretAST(struct ASTnode* n);


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
int addglob(char* name);




// gen.c�еľ�̬�����Ӧ���ǻ���е� Label:
static int label(void)
{
    static int id = 1;
    ++id;
    return id;
}

static struct ASTnode* primary();//���� token ���ж����Ӧ��ASTNode 
// static ÿ���ļ�
static struct ASTnode* primary()
{
    struct ASTnode* n = NULL;
    int id;
    // ��token����ΪT_INTLIT ��Ϊ ASTҶ�ӽڵ� �����쳣
    switch (Token.token)
    {
    case T_INTLIT:
        n = mkastleaf(A_INTLIT, Token.intvalue);
        scan(&Token);  // �ж�����
        return n;

    case T_IDENT:
        id = findglob(Text);
        if (id == -1)
            fatals("Unknown variable", Text);

        n = mkastleaf(A_IDENT, id);
        break;
    default:
        fatald("Syntax error, token", Token.token);
    }

    scan(&Token);//ɨ����һ�����Ʋ�����Ҷ�ӽڵ�
    return n;
}


// ���� if  if_else �Ӿ� 
static int genIFAST(struct ASTnode* n)
{
    int Lfalse, Lend;

    /*
                   A_IF
              /      |    \
      condition   ���֧  �ٷ�֧

    */

    /*
����������ǩ��һ�����ڼٸ�����䣬��һ������
����IF���Ľ�β����û��ELSE�Ӿ�ʱ��Lfalse�ǽ�����ǩ��
    */
    Lfalse = label();
    if (n->right)      // ���ٷ�֧ �������
        Lend = label();



    genAST(n->left, Lfalse, n->op);// Condition and jump to Lfalse
    genfreeregs();

    //  �������֧ 
    genAST(n->mid, NOREG, n->op);
    genfreeregs();


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //���ٴ��� ���ɼٷ�֧��� ����ת
    if (n->right)
    {
        genAST(n->right, NOREG, n->op);
        genfreeregs();
        cglabel(Lend);
    }

    return NOREG;
}

static int genWHILE(struct ASTnode* n)
{

    int Lstart, Lend;

    // ���ɱ�ǩ
    Lstart = label();
    Lend = label();
    // ���ջ���﷨���ɻ����ʽ��while
    cglabel(Lstart);


    genAST(n->left, Lend, n->op);
    genfreeregs();

    genAST(n->right, NOREG, n->op);
    genfreeregs();

    cgjump(Lstart);
    cglabel(Lend);  // ����while
    return NOREG;




}




static int genAST(struct ASTnode* n, int reg, int parentASTop);
// interpretAST�Ļ��ӿڰ汾
static int genAST(struct ASTnode* n, int reg, int parentASTop)  // regΪ���ʹ�üĴ�����Ӧ�±�
{
    int  leftreg;
    int  midreg;
    int  rightreg;



    // We now have specific AST node handling at the top
    switch (n->op)//�˴���д������� if fun ������
    {
    case A_IF:
        return genIFAST(n);
    case A_WHILE:
        return genWHILE(n);
    case A_GLUE:
        // Do each child statement, and free the
        // registers after each child
        genAST(n->left, NOREG, n->op);
        genfreeregs();
        genAST(n->right, NOREG, n->op);
        genfreeregs();
        return NOREG;

    case A_FUNCTION:
        cgfuncpreamble(Gsym[n->v.id].name);  // ����֮ǰ��cgpreamble���ɺ���ǰ����
        genAST(n->left, NOREG, n->op);
        cgfuncpostamble(); // ����֮ǰ��cgpostamble���ɺ���ǰ����

        return NOREG;
    }

    if (n->left)
        leftreg = genAST(n->left, NOREG, n->op);
    if (n->right)
        rightreg = genAST(n->right, leftreg, n->op);

    switch (n->op)
    {
    case A_ADD:
        return cgadd(leftreg, rightreg);
    case A_SUBTRACT:
        return cgsub(leftreg, rightreg);
    case A_MULTIPLY:
        return cgmul(leftreg, rightreg);
    case A_DIVIDE:
        return cgdiv(leftreg, rightreg);
    case A_INTLIT:
        return cgloadint(n->v.intvalue); // ���ط���ļĴ����±��
    case A_IDENT:
        return cgloadglob(Gsym[n->v.id].name);
    case A_LVIDENT:
       // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, Gsym[n->v.id].name);

        // �Ƚ��ж� �����ֵΪ���ҼĴ������
    case A_EQ:
        if (parentASTop == A_IF|| parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cgequal(leftreg, rightreg);
    case A_NE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cgnotequal(leftreg, rightreg);
    case A_LT:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cglessthan(leftreg, rightreg);
    case A_GT:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cggreaterthan(leftreg, rightreg);
    case A_LE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cglessequal(leftreg, rightreg);
    case A_GE:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
            return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
        else
            return cggreaterequal(leftreg, rightreg);


    case A_ASSIGN:
        // ��ǰ���ʽ�Ѿ�ִ����ɣ���Ҫ����
        return rightreg; // ���ؼĴ�������±�ֵ
    case A_PRINT:
        genprintint(leftreg); // ��ӡ��Ĵ�������ֵ
        /*
              A_PRINT
              /     \
            +        print
          /   \
         1     2
        */

        genfreeregs();
        return NOREG;

    default:
        fatald("Unknown AST operator", n->op);
    }
}