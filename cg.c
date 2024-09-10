#include "defs.h"
#include "data.h"
#include "decl.h"

// ���� x86-64 ������
static int freereg[4] = {0}; // ��Ӧ��4���Ĵ�����ʹ��״��
static char* reglist[4] = { "%r8", "%r9", "%r10", "%r11" };// ��
static char* breglist[4] = { "%r8b", "%r9b", "%r10b", "%r11b" }; // b���� r8�Ĵ�����8λ

// ����Ϊ1 �ͷ����мĴ���
void freeall_registers(void)
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}


// �Ĵ�������
static int alloc_register(void)
{
	for (int i = 0; i < 4; i++)
	{
		if (freereg[i])
		{
			freereg[i] = 0;
			return i;
		}
	}
	fprintf(stderr, "Out of registers!\n");
	exit(1);
}

// �ͷżĴ������Ϊ reg
static void free_register(int reg)
{
	if (freereg[reg] != 0) 
	{
		fprintf(stderr, "Error trying to free register %d\n", reg);
		exit(1);
	}
	freereg[reg] = 1;
}

// ��� assembly preamble
void cgpreamble()
{
	freeall_registers();
	fputs(
		"\t.text\n"
		".LC0:\n"
		"\t.string\t\"%d\\n\"\n"
		"printint:\n"
		"\tpushq\t%rbp\n"
		"\tmovq\t%rsp, %rbp\n"
		"\tsubq\t$16, %rsp\n"
		"\tmovl\t%edi, -4(%rbp)\n"
		"\tmovl\t-4(%rbp), %eax\n"
		"\tmovl\t%eax, %esi\n"
		"\tleaq	.LC0(%rip), %rdi\n"
		"\tmovl	$0, %eax\n"
		"\tcall	printf@PLT\n"
		"\tnop\n"
		"\tleave\n"
		"\tret\n"
		"\n"
		"\t.globl\tmain\n"
		"\t.type\tmain, @function\n"
		"main:\n"
		"\tpushq\t%rbp\n"
		"\tmovq	%rsp, %rbp\n",
		Outfile);
}

// ��� assembly postamble
void cgpostamble()
{
	fputs(
		"\tmovl	$0, %eax\n"
		"\tpopq	%rbp\n"
		"\tret\n",
		Outfile);
}

// ���ͱ������ص��Ĵ�����
int cgloadint(int value)
{

	
	int r = alloc_register();

	// Print out the code to initialise it
	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
	return(r);
}

// �Ӽ��˳�
int cgadd(int r1, int r2)
{
	fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
	free_register(r1);
	return(r2);
}


int cgsub(int r1, int r2)
{
	fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
	free_register(r2);
	return(r1);
}


int cgmul(int r1, int r2)
{
	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]); // ������˷�
	free_register(r1); // �ͷ�r1
	return(r2);// ����r2
}


int cgdiv(int r1, int r2)
{
	fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
	fprintf(Outfile, "\tcqo\n");
	fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
	fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
	free_register(r2);
	return(r1);
}


// �Ƚ��ж�

static int cgcompare(int r1, int r2, char* how) 
{
	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]); //  r1=r1-r2
	fprintf(Outfile, "\t%s\t%s\n", how, breglist[r2]);
	fprintf(Outfile, "\tandq\t$255,%s\n", reglist[r2]); //  ����andq ��ȡ�ò�ֵ
	free_register(r1);
	return r2;
}


int cgequal(int r1, int r2) 
{
	return(cgcompare(r1, r2, "sete"));
}
int cgnotequal(int r1, int r2)
{
	return(cgcompare(r1, r2, "setne"));
}
int cglessthan(int r1, int r2)
{
	return(cgcompare(r1, r2, "setl"));
}
int cggreaterthan(int r1, int r2) 
{
	return(cgcompare(r1, r2, "setg"));
}
int cglessequal(int r1, int r2)
{
	return(cgcompare(r1, r2, "setle"));
}
int cggreaterequal(int r1, int r2) 
{
	return(cgcompare(r1, r2, "setge")); 
}



//���Ĵ����е�ֵ��ӡ��
void cgprintint(int r) 
{
	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
	fprintf(Outfile, "\tcall\tprintint\n");
	free_register(r);
}

int cgloadglob(char* identifier) 
{
	int r = alloc_register();

	// Print out the code to initialise it
	fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", identifier, reglist[r]);
	return (r);
}

// Store a register's value into a variable
int cgstorglob(int r, char* identifier) 
{
	fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], identifier);
	return (r);
}

//����ȫ�ַ���
void cgglobsym(char* sym)
{
	fprintf(Outfile, "\t.comm\t%s,8,8\n", sym);
}

// List of comparison instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char* cmplist[] ={ "sete", "setne", "setl", "setg", "setle", "setge" };

// Compare two registers and set if true.
int cgcompare_and_set(int ASTop, int r1, int r2)
{

	// Check the range of the AST operation
	if (ASTop < A_EQ || ASTop > A_GE)
		fatal("Bad ASTop in cgcompare_and_set()");

	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
	fprintf(Outfile, "\t%s\t%s\n", cmplist[ASTop - A_EQ], breglist[r2]);
	fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);
	free_register(r1);
	return (r2);
}

// ���ɱ�ǩ
void cglabel(int l) {
	fprintf(Outfile, "L%d:\n", l);
}

// Generate a jump to a label
void cgjump(int l)
{
	fprintf(Outfile, "\tjmp\tL%d\n", l);
}

// List of inverted jump instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char* invcmplist[] = { "jne", "je", "jge", "jle", "jg", "jl" };

// Compare two registers and jump if false.
int cgcompare_and_jump(int ASTop, int r1, int r2, int label)
{

	// Check the range of the AST operation
	if (ASTop < A_EQ || ASTop > A_GE)
		fatal("Bad ASTop in cgcompare_and_set()");

	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
	fprintf(Outfile, "\t%s\tL%d\n", invcmplist[ASTop - A_EQ], label);
	freeall_registers();
	return (NOREG);
}