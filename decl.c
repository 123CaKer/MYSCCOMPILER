#include "defs.h"
#include "data.h"
#include "decl.h"


// 声明变量 type 为变量类型 int char  long
// variable_declaration: type identifier ';'
//        | type identifier '[' INTLIT ']' ';'
//        ;
//
// Parse the declaration of a scalar variable or an array
// with a given size.
// The identifier has been scanned & we have the type.
// class 为变量类型
// Return the pointer to variable's entry in the symbol table
struct symtable* var_declaration(int type, struct symtable* ctype, int class)
{
	struct symtable* sym = NULL;

	// See if this has already been declared
	switch (class)
	{
	case C_GLOBAL:
		if (findglob(Text) != NULL)
			fatals("Duplicate global variable declaration", Text);
	case C_LOCAL:
	case C_PARAM:
		if (findlocl(Text) != NULL)
			fatals("Duplicate local variable declaration", Text);
	case C_MEMBER:
		if (findmember(Text) != NULL)
			fatals("Duplicate struct/union member declaration", Text);
	}

	// Text now has the identifier's name.
	// If the next token is a '['
	if (Token.token == T_LBRACKET)
	{
		// Skip past the '['
		scan(&Token);

		// Check we have an array size
		if (Token.token == T_INTLIT)
		{
			// Add this as a known array and generate its space in assembly.
			// We treat the array as a pointer to its elements' type
			switch (class)
			{
			case C_GLOBAL:
				sym = addglob(Text, pointer_to(type), ctype, S_ARRAY, Token.intvalue);
				break;
			case C_LOCAL:
			case C_PARAM:
			case C_MEMBER:
				fatal
				("For now, declaration of non-global arrays is not implemented");
			}
		}
		// Ensure we have a following ']'
		scan(&Token);
		match(T_RBRACKET, "]");
	}
	else {
		// Add this as a known scalar
		// and generate its space in assembly
		switch (class)
		{
		case C_GLOBAL:
			sym = addglob(Text, type, ctype, S_VARIABLE, 1);
			break;
		case C_LOCAL:
			sym = addlocl(Text, type, ctype, S_VARIABLE, 1);
			break;
		case C_PARAM:
			sym = addparm(Text, type, ctype, S_VARIABLE, 1);
			break;
		case C_MEMBER:
			sym = addmemb(Text, type, ctype, S_VARIABLE, 1);
			break;
		}
	}
	return (sym);
}

// var_declaration_list: <null>
//           | variable_declaration
//           | variable_declaration separate_token var_declaration_list ;
//
//funsym是fun或者struct union enum等 class 是C_  separate_token是区分符
//end_token是最后一个token 即为 } 或 ）
static int var_declaration_list(struct symtable* funcsym, int class, int separate_token, int end_token)
{
	int type;
	int paramcnt = 0;
	struct symtable* protoptr = NULL;
	struct symtable* ctype;

	// If there is a prototype, get the pointer
	// to the first prototype parameter
	if (funcsym != NULL)
		protoptr = funcsym->member;// 获取原型的成员

	// Loop until the final end token
	while (Token.token != end_token)
	{
		// Get the type and identifier
		type = parse_type(&ctype);
		ident();

		// Check that this type matches the prototype if there is one
		if (protoptr != NULL)
		{
			if (type != protoptr->type)
				fatald("Type doesn't match prototype for parameter", paramcnt + 1);
			protoptr = protoptr->next;
		}
		else
		{
			// Add a new parameter to the right symbol table list, based on the class
			var_declaration(type, ctype, class);
		}
		paramcnt++;

		// Must have a separate_token or ')' at this point
		if ((Token.token != separate_token) && (Token.token != end_token))
			fatald("Unexpected token in parameter list", Token.token);
		if (Token.token == separate_token)
			scan(&Token);
	}

	// Check that the number of parameters in this list matches
	// any existing prototype
	if ((funcsym != NULL) && (paramcnt != funcsym->nelems))
		fatals("Parameter count mismatch for function", funcsym->name);

	//返回成员个数
	return (paramcnt);
}


//
// function_declaration: type identifier '(' parameter_list ')' ;
//      | type identifier '(' parameter_list ')' compound_statement   ;
//
// Parse the declaration of function.
// The identifier has been scanned & we have the type.
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree,
		* finalstmt;// 最终语句
	struct symtable* oldfuncsym,// 函数体的函数
		* newfuncsym = NULL;// 函数声明的函数
	int endlabel, paramcnt;

	// Text has the identifier's name. If this exists and is a
	// function, get the id. Otherwise, set oldfuncsym to NULL.
	if ((oldfuncsym = findsymbol(Text)) != NULL)
		if (oldfuncsym->stype != S_FUNCTION)
			oldfuncsym = NULL;

	// If this is a new function declaration, get a
	// label-id for the end label, and add the function
	// to the symbol table,
	if (oldfuncsym == NULL)
	{
		endlabel = genlabel();
		// Assumtion: functions only return scalar types, so NULL below
		newfuncsym = addglob(Text, type, NULL, S_FUNCTION, endlabel);
	}
	// Scan in the '(', any parameters and the ')'.
	// Pass in any existing function prototype pointer
	lparen();
	paramcnt = var_declaration_list(oldfuncsym, C_PARAM, T_COMMA, T_RPAREN);
	rparen();

	// If this is a new function declaration, update the
	// function symbol entry with the number of parameters.
	// Also copy the parameter list into the function's node.
	if (newfuncsym)
	{
		newfuncsym->nelems = paramcnt; // 形参个数
		newfuncsym->member = Parmhead;// 获取成员参数头
		oldfuncsym = newfuncsym;
	}
	// Clear out the parameter list
	Parmhead = Parmtail = NULL;

	// Declaration ends in a semicolon, only a prototype.
	if (Token.token == T_SEMI)
	{
		scan(&Token);
		return (NULL);
	}
	// This is not just a prototype.
	// Set the Functionid global to the function's symbol pointer
	Functionid = oldfuncsym;

	// Get the AST tree for the compound statement
	tree = compound_statement();

	// If the function type isn't P_VOID ..
	if (type != P_VOID) {

		// Error if no statements in the function
		if (tree == NULL)
			fatal("No statements in function with non-void type");

		// Check that the last AST operation in the
		// compound statement was a return statement
		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
		if (finalstmt == NULL || finalstmt->op != A_RETURN)
			fatal("No return for function with non-void type");
	}
	// Return an A_FUNCTION node which has the function's symbol pointer
	// and the compound statement sub-tree
	return (mkastunary(A_FUNCTION, type, tree, oldfuncsym, endlabel));
}


// Parse struct declarations. Either find an existing
// struct declaration, or build a struct symbol table
// entry and return its pointer.
// 解析结构体声明，并且返回一个结构体声明或者返回一个结构体符号表开始指针
static struct symtable* struct_declaration(void)
{
	struct symtable* ctype = NULL;
	struct symtable* m;
	int offset;

	// Skip the struct keyword
	scan(&Token);

	// 如果当前是标识符 
	if (Token.token == T_IDENT)
	{
		// Find any matching composite type
		ctype = findstruct(Text);// 从符号表中找
		scan(&Token);
	}
	// If the next token isn't an LBRACE , this is
	// the usage of an existing struct type.
	// Return the pointer to the type.
	if (Token.token != T_LBRACE)
	{
		if (ctype == NULL)
			fatals("unknown struct type", Text);
		return (ctype);
	}
	// Ensure this struct type hasn't been
	// previously defined
	if (ctype)
		fatals("previously defined struct", Text);

	// Build the struct node and skip the left brace
	// 若当前符号表中没有则建立一个结构体节点并添加到符号表中，并且扫描下一个token
	ctype = addstruct(Text, P_STRUCT, NULL, 0, 0);
	scan(&Token);

	// 结构体成员链表
	var_declaration_list(NULL, C_MEMBER, T_SEMI, T_RBRACE);
	rbrace();
	ctype->member = Membhead;
	Membhead = Membtail = NULL;

	// Set the offset of the initial member
	// and find the first free byte after it
	m = ctype->member;
	m->posn = 0;
	offset = typesize(m->type, m->ctype);

	// Set the position of each successive member in the struct
	// 设置每一个成员在结构体中的位置
	/*
struct
{
  char x;               // At offset 0
  int y;                // At offset 4, not 1
}; 考虑到字符对其
*/
	for (m = m->next; m != NULL; m = m->next)
	{
		// Set the offset for this member
		m->posn = genalign(m->type, offset, 1);

		// Get the offset of the next free byte after this member
		offset += typesize(m->type, m->ctype);
	}

	// 设置最终的偏移量大小即为结构体大小
	ctype->size = offset;
	return (ctype);
}



// 声明函数 对应BNF参照笔记
#if 0
struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree, * finalstmt;
	int nameslot, endlabel;


	/*
	type = parse_type(Token.token);//获取当前函数声明类型 eg int add（）  为int
	//scan(&Token);
	ident();
	*/
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel);
	Functionid = nameslot;//当前函数下标
	lparen();
	rparen();
	tree = compound_statement();
	if (type != P_VOID)
	{
		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;

		/*
				 A_FUN
				 /   \
			 A_GLUE                    tree->op
			 /     \
	A_ASSIGN       A_PRINT
	 /   \           /   \
	2     j          j     print


		*/

		if (finalstmt == NULL || finalstmt->op != A_RETURN)
			fatal("No return for function with non-void type");
	}

	return mkastunary(A_FUNCTION, type, tree, nameslot);
}
#endif


// 解析变量声明 符号表
int parse_type(struct symtable** ctype)
{
	int typer = -1;
	switch (Token.token)
	{
	case T_CHAR:
		typer = P_CHAR;
		scan(&Token);
		break;
	case T_INT:
		typer = P_INT;
		scan(&Token);
		break;
	case T_VOID:
		typer = P_VOID;
		scan(&Token);
		break;
	case T_LONG:
		typer = P_LONG;
		scan(&Token);
		break;
	case T_STRUCT:
		typer = P_STRUCT;
		*ctype = struct_declaration();
		break;
	default:
		fatald("Illegal type, token", Token.token);
	}

	while (1) //  *****************p
	 {
			if (Token.token != T_STAR)
				break;
			else
			{
				typer = pointer_to(typer);
				scan(&Token);
			}
			
	}

	return typer;

}







// Parse one or more global declarations, either
// variables, functions or structs
void global_declarations(void)
{
	struct ASTnode* tree;
	struct symtable* ctype = NULL;
	int type;

	while (1)
	{
		// Stop when we have reached EOF
		if (Token.token == T_EOF)
			break;

		// Get the type
		type = parse_type(&ctype);

		// We might have just parsed a struct declaration
		// with no associated variable. The next token
		// might be a ';'. Loop back if it is. XXX. I'm
		// not happy with this as it allows "struct fred;"
		// as an accepted statement
		if (type == P_STRUCT && Token.token == T_SEMI)
		{
			scan(&Token);
			continue;
		}
		// We have to read past the identifier
		// to see either a '(' for a function declaration
		// or a ',' or ';' for a variable declaration.
		// Text is filled in by the ident() call.
		ident();
		if (Token.token == T_LPAREN)
		{

			// Parse the function declaration
			tree = function_declaration(type);

			// Only a function prototype, no code
			if (tree == NULL)
				continue;

#if 0
			// A real function, generate the assembly code for it
			if (O_dumpAST) {
				dumpAST(tree, NOLABEL, 0);
				fprintf(stdout, "\n\n");
			}
#endif // 0


			genAST(tree, NOLABEL, 0);

			// Now free the symbols associated
			// with this function
			freeloclsyms();
		}
		else {

			// Parse the global variable declaration
			// and skip past the trailing semicolon
			var_declaration(type, ctype, C_GLOBAL);
			semi();
		}
	}
}




