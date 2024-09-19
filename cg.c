#include "defs.h"
#include "data.h"
#include "decl.h"
// 生成 x86-64 汇编代码
static int freereg[4] = {0}; // 对应的4个寄存器的使用状况
static char* reglist[4] = { "%r8", "%r9", "%r10", "%r11" };// 名
static char* breglist[4] = { "%r8b", "%r9b", "%r10b", "%r11b" }; // b代表 32位 r8寄存器低8位
static char* dreglist[4] = { "%r8d", "%r9d", "%r10d", "%r11d" };// d代表 64位 r8寄存器低8位

// 0 means no size. P_NONE, P_VOID, P_CHAR, P_INT, P_LONG
static int psize[] = { 0,       0,      1,     4,     8 };

// 空闲为1 释放所有寄存器
void freeall_registers(void)
{
	freereg[0] = freereg[1] = freereg[2] = freereg[3] = 1;
}


// 寄存器分配
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

// 释放寄存器编号为 reg
static void free_register(int reg)
{
	if (freereg[reg] != 0) 
	{
		fprintf(stderr, "Error trying to free register %d\n", reg);
		exit(1);
	}
	freereg[reg] = 1;
}

// 输出 assembly preamble
void cgpreamble()
{
	freeall_registers(); // 释放寄存器
	/*
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
		*/
	/*fputs("\t.text\n"
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
		"\tcall	printf@PLT\n" "\tnop\n" "\tleave\n" "\tret\n" "\n", Outfile);
		*/
	fputs("\t.text\n", Outfile);

	/*
	    使用外部库fputs("\t.text\n", Outfile);
		不用外部库时使用第二段
	
	*/
}

// 输出 assembly postamble
void cgpostamble()
{
	fputs(
		"\tmovl	$0, %eax\n"
		"\tpopq	%rbp\n"
		"\tret\n",
		Outfile);
}

// 整型变量加载到寄存器中
int cgloadint(int value)
{
	int r = alloc_register();

	// Print out the code to initialise it
	fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
	return(r);
}

// 加减乘除
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
	fprintf(Outfile, "\timulq\t%s, %s\n", reglist[r1], reglist[r2]); // 输出汇编乘法
	free_register(r1); // 释放r1
	return(r2);// 返回r2
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


// 比较判断

static int cgcompare(int r1, int r2, char* how) 
{
	fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]); //  r1=r1-r2
	fprintf(Outfile, "\t%s\t%s\n", how, breglist[r2]);
	fprintf(Outfile, "\tandq\t$255,%s\n", reglist[r2]); //  减法andq 仅取得差值
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



//将寄存器中的值打印出
void cgprintint(int r) 
{
	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
	fprintf(Outfile, "\tcall\tprintint\n");
	free_register(r);
}
// 从符号表中取得符号并实现mov到 rip寄存器
int cgloadglob(int id) 
{
	int r = alloc_register();

	// Print out the code to initialise it
	switch (Gsym[id].type)
	{
	case P_CHAR:
		fprintf(Outfile, "\tmovzbq\t%s(\%%rip), %s\n", Gsym[id].name,
			reglist[r]);
		break;
	case P_INT:
		fprintf(Outfile, "\tmovzbl\t%s(\%%rip), %s\n", Gsym[id].name,
			reglist[r]);
		break;
	case P_LONG:
		fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", Gsym[id].name, reglist[r]);
		break;
	case P_CHARPTR:
	case P_INTPTR:
	case P_LONGPTR:
		fprintf(Outfile, "\tmovq\t%s(\%%rip), %s\n", Gsym[id].name, reglist[r]);
		break;
	default:
		fatald("Bad type in cgloadglob:", Gsym[id].type);
	}
	return r;
}

// Store a register's value into a variable
int cgstorglob(int r, int id) 
{
	switch (Gsym[id].type)
	{
	case P_CHAR:
		fprintf(Outfile, "\tmovb\t%s, %s(\%%rip)\n", breglist[r],Gsym[id].name);
		break;
	case P_INT:
		fprintf(Outfile, "\tmovl\t%s, %s(\%%rip)\n", dreglist[r],Gsym[id].name);
		break;
	case P_LONG:
		fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], Gsym[id].name);
		break;
	case P_CHARPTR:
	case P_INTPTR:
	case P_LONGPTR:
		fprintf(Outfile, "\tmovq\t%s, %s(\%%rip)\n", reglist[r], Gsym[id].name);
		break;

	default:
		fatald("Bad type in cgloadglob:", Gsym[id].type);
		
	}
	return r;
}

//生成全局符号
void cgglobsym(int id)
{
		int typesize;
		typesize = cgprimsize(Gsym[id].type); // 获取大小
		fprintf(Outfile, "\t.comm\t%s,%d,%d\n", Gsym[id].name, typesize, typesize);
	

	
}
// List of comparison instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static char* cmplist[] = { "sete", "setne", "setl", "setg", "setle", "setge" };

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

// 生成标签
void cglabel(int l) 
{
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



// Print out a function preamble
void cgfuncpreamble(int id)
{
	char* name = Gsym[id].name;
	fprintf(Outfile,
		"\t.text\n"
		"\t.globl\t%s\n"
		"\t.type\t%s, @function\n"
		"%s:\n" "\tpushq\t%%rbp\n"
		"\tmovq\t%%rsp, %%rbp\n", name, name, name);
}

// 打印函数后缀码 postamble
void cgfuncpostamble(int id)
{
	cglabel(Gsym[id].endlabel);
	fputs("\tpopq %rbp\n" "\tret\n", Outfile);
}


// Widen the value in the register from the old
// to the new type, and return a register with
// this new value
int cgwiden(int r, int oldtype, int newtype)
{
	// Nothing to do
	return (r);
}

// 获取类型大小 依比特为准
int cgprimsize(int type)
{
	// Check the type is valid
	if (type < P_NONE || type > P_LONGPTR)
		fatal("Bad type in cgprimsize()");
	return (psize[type]);
}

int cgcall(int r, int id) 
{
	// Get a new register
	int outr = alloc_register();
	fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
	fprintf(Outfile, "\tcall\t%s\n", Gsym[id].name);
	fprintf(Outfile, "\tmovq\t%%rax, %s\n", reglist[outr]);
	free_register(r);
	return (outr);
}

// 函数返回值
void cgreturn(int reg, int id)
{
	// Generate code depending on the function's type
	switch (Gsym[id].type)
	{
	case P_CHAR:
		fprintf(Outfile, "\tmovzbl\t%s, %%eax\n", breglist[reg]);
		break;
	case P_INT:
		fprintf(Outfile, "\tmovl\t%s, %%eax\n", dreglist[reg]);
		break;
	case P_LONG:
		fprintf(Outfile, "\tmovq\t%s, %%rax\n", reglist[reg]);
		break;
	default:
		fatald("Bad function type in cgreturn:", Gsym[id].type);
	}
	cgjump(Gsym[id].endlabel);
}

//生成地址 汇编
int cgaddress(int id)
{
	int r = alloc_register();

	fprintf(Outfile, "\tleaq\t%s(%%rip), %s\n", Gsym[id].name, reglist[r]);
	return r;
}

// 间接寻址 汇编
int cgderef(int r, int type)
{
	switch (type)
	{
	case P_CHARPTR:
		fprintf(Outfile, "\tmovzbq\t(%s), %s\n", reglist[r], reglist[r]);
		break;
	case P_INTPTR:
	case P_LONGPTR:
		fprintf(Outfile, "\tmovq\t(%s), %s\n", reglist[r], reglist[r]);
		break;
	}
	return r;
}