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
int cgloadglob(int id);
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
int addglob(char* name, int type, int stype, int endlabel);

// types.c
int type_compatible(int* left, int* right, int onlyright);

// gen.c�еľ�̬�����Ӧ���ǻ���е� Label:
//�±�ʶ
struct ASTnode* primary();//���� token ���ж����Ӧ��ASTNode �����壩
// static ÿ���ļ�
static struct ASTnode* primary()
{
    struct ASTnode* n = NULL;
    int id;
    // ��token����ΪT_INTLIT ��Ϊ ASTҶ�ӽڵ� �����쳣
    switch (Token.token)
    {
    case T_INTLIT: //ֵ

        if ((Token.intvalue) >= 0 && (Token.intvalue < 256))//char��
            n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
        else                                                // int��
            n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
        break;

    case T_IDENT:
        scan(&Token);

        // It's a '(', so a function call
        if (Token.token == T_LPAREN)
            return (funccall());

        // Not a function call, so reject the new token
        reject_token(&Token);

        // Check that the variable exists. XXX Add structural type test
        id = findglob(Text);
        if (id == -1)
            fatals("Unknown variable", Text);

        // Make a leaf AST node for it
        n = mkastleaf(A_IDENT, Gsym[id].type, id);
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
    Lfalse = genlabel();
    if (n->right)      // ���ٷ�֧ �������
        Lend = genlabel();



    genAST(n->left, Lfalse, n->op);// Condition and jump to Lfalse
    genfreeregs();

    //  �������֧ 
    genAST(n->mid, NOLABEL, n->op);
    genfreeregs();


    if (n->right)  /// Lfalse: lend
        cgjump(Lend);

    // Lfalse: label
    cglabel(Lfalse);

    //���ٴ��� ���ɼٷ�֧��� ����ת
    if (n->right)
    {
        genAST(n->right, NOLABEL, n->op);
        genfreeregs();
        cglabel(Lend);
    }

    return NOREG;
}

static int genWHILE(struct ASTnode* n)
{

    int Lstart, Lend;

    // ���ɱ�ǩ
    Lstart = genlabel();
    Lend = genlabel();
    // ���ջ���﷨���ɻ����ʽ��while
    cglabel(Lstart);


    genAST(n->left, Lend, n->op);
    genfreeregs();

    genAST(n->right, NOLABEL, n->op);
    genfreeregs();

    cgjump(Lstart);
    cglabel(Lend);  // ����while
    return NOREG;




}


// interpretAST�Ļ��ӿڰ汾  ����
static int genAST(struct ASTnode* n, int reg, int parentASTop)  // regΪ���ʹ�üĴ�����Ӧ�±�
{
    int  leftreg;
    //  int  midreg;
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
        genAST(n->left, NOLABEL, n->op);
        genfreeregs();
        genAST(n->right, NOLABEL, n->op);
        genfreeregs();
        return NOREG;


    case A_FUNCTION:
        cgfuncpreamble(n->v.id);  // ����֮ǰ��cgpreamble���ɺ���ǰ����
        genAST(n->left, NOLABEL, n->op);
        cgfuncpostamble(n->v.id); // ����֮ǰ��cgpostamble���ɺ���ǰ����
        return NOREG;

    }

    if (n->left)
        leftreg = genAST(n->left, NOLABEL, n->op);
    if (n->right)
        rightreg = genAST(n->right, NOLABEL, n->op);

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
        return (cgloadint(n->v.intvalue, n->type)); // ���ط���ļĴ����±��
    case A_IDENT:
        /*
               A_DEREF
               /   \        *p���Ѱַ
               p    *
        Ϊ��ֵ���߼��Ѱַ
        */
        if (n->rvalue || parentASTop == A_DEREF)
            return (cgloadglob(n->v.id));
        else
            return NOREG;

        /*
    case A_LVIDENT:
        // printf(" the reg is %d\n",reg);
        return cgstorglob(reg, n->v.id);
        */
        // �Ƚ��ж� �����ֵΪ���ҼĴ������
    case A_EQ:
        if (parentASTop == A_IF || parentASTop == A_WHILE)
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
        switch (n->right->op)
        {
        case A_IDENT:// ��ֵ����
            return cgstorglob(leftreg, n->right->v.id);
        case A_DEREF: // ���
            return cgstorderef(leftreg, rightreg, n->right->type);
        default:
            fatald("Can't A_ASSIGN in genAST(), op", n->op);
        }
#if 0
    case A_PRINT:
        genprintint(leftreg); // ��ӡ��Ĵ�������ֵ
        /*
              A_PRINT
              /     \
            +
          /   \
         1     2
        */

        genfreeregs();
        return NOREG;
#endif    
    case A_WIDEN:
        // Widen the child's type to the parent's type
        return (cgwiden(leftreg, n->left->type, n->type));

    case A_RETURN:
        cgreturn(leftreg, Functionid);
        return (NOREG);

    case A_FUNCCALL:
        return (cgcall(leftreg, n->v.id));

    case A_ADDR:
        return (cgaddress(n->v.id));
    case A_DEREF:

        //  return (cgderef(leftreg, n->left->type));

        if (n->rvalue)
            return cgderef(leftreg, n->left->type);
        else
            return (leftreg);

    case A_SCALE:
        // Small optimisation: use shift if the
        // scale value is a known power of two
        switch (n->v.size)
        {
        case 2:
            return(cgshlconst(leftreg, 1));
        case 4:
            return(cgshlconst(leftreg, 2));
        case 8:
            return(cgshlconst(leftreg, 3));
        default:
            // ��������int a[10]  int * 10
            rightreg = cgloadint(n->v.size, P_INT);
            return cgmul(leftreg, rightreg); //l = l * r;
        }
    default:
        fatald("Unknown AST operator", n->op);
    }
    return NOREG;
}
