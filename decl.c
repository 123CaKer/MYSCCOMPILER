#include "defs.h"
#include "data.h"
#include "decl.h"



// �������� type Ϊ�������� int char  long
// variable_declaration: type identifier ';'
//        | type identifier '[' INTLIT ']' ';'
//        ;
//
// Parse the declaration of a scalar variable or an array
// with a given size.
// The identifier has been scanned & we have the type.
// class Ϊ��������
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
//funsym��fun����struct union enum�� class ��C_  separate_token�����ַ�
//end_token�����һ��token ��Ϊ } �� ��
static int var_declaration_list(struct symtable* funcsym, int class, int separate_token, int end_token)
{
	int type;
	int paramcnt = 0;
	struct symtable* protoptr = NULL;
	struct symtable* ctype;

	// If there is a prototype, get the pointer
	// to the first prototype parameter
	if (funcsym != NULL)
		protoptr = funcsym->member;// ��ȡԭ�͵ĳ�Ա

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

	//���س�Ա����
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
		* finalstmt;// �������
	struct symtable* oldfuncsym,// ������ĺ���
		* newfuncsym = NULL;// ���������ĺ���
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
		newfuncsym->nelems = paramcnt; // �βθ���
		newfuncsym->member = Parmhead;// ��ȡ��Ա����ͷ
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

#if 0
// Parse struct declarations. Either find an existing
// struct declaration, or build a struct symbol table
// entry and return its pointer.
// �����ṹ�����������ҷ���һ���ṹ���������߷���һ���ṹ����ű�ʼָ��
static struct symtable* struct_declaration(void)
{
	struct symtable* ctype = NULL;
	struct symtable* m;
	int offset;

	// Skip the struct keyword
	scan(&Token);

	// �����ǰ�Ǳ�ʶ�� 
	if (Token.token == T_IDENT)
	{
		// Find any matching composite type
		ctype = findstruct(Text);// �ӷ��ű�����
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
	// ����ǰ���ű���û������һ���ṹ��ڵ㲢��ӵ����ű��У�����ɨ����һ��token
	ctype = addstruct(Text, P_STRUCT, NULL, 0, 0);
	scan(&Token);

	// �ṹ���Ա����
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
	// ����ÿһ����Ա�ڽṹ���е�λ��
	/*
struct
{
  char x;               // At offset 0
  int y;                // At offset 4, not 1
}; ���ǵ��ַ�����
*/
	for (m = m->next; m != NULL; m = m->next)
	{
		// Set the offset for this member
		m->posn = genalign(m->type, offset, 1);

		// Get the offset of the next free byte after this member
		offset += typesize(m->type, m->ctype);
	}

	// �������յ�ƫ������С��Ϊ�ṹ���С
	ctype->size = offset;
	return (ctype);
}



// �������� ��ӦBNF���ձʼ�

struct ASTnode* function_declaration(int type)
{
	struct ASTnode* tree, * finalstmt;
	int nameslot, endlabel;


	/*
	type = parse_type(Token.token);//��ȡ��ǰ������������ eg int add����  Ϊint
	//scan(&Token);
	ident();
	*/
	endlabel = genlabel();
	nameslot = addglob(Text, type, S_FUNCTION, endlabel);
	Functionid = nameslot;//��ǰ�����±�
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

//����enum����
void enum_declaration(void)
{
	struct symtable* etype = NULL;
	char* name = NULL;
	int intval = 0;

	// Skip the enum keyword.
	scan(&Token);

	// If there's a following enum type name, get a
	// pointer to any existing enum type node.
  // ����ǰΪ��ʶ�� enum aaa{}�е�aaa
	if (Token.token == T_IDENT)
	{
		etype = findenumtype(Text);
		name = strdup(Text);        // As it gets tromped soon ����Text�ַ���
		/*We only have one global variable, Text,
		to hold a scanned-in word, and we
		have to be able to parse enum foo var1.
		If we scan in the token after the foo,
		we will lose the foo string.
		So we need to strdup() this.
		strdup() is a dupliacting function*/
		//scan(&Token);
	}
	// We do have an LBRACE. Skip it
	scan(&Token);

	// If the next token isn't a LBRACE, check
 // that we have an enum type name, then return
	if (Token.token != T_LBRACE)
	{
		if (etype == NULL)
			fatals("undeclared enum type:", name);
		return;
	}

	// We do have an LBRACE. Skip it
	scan(&Token);

	// If we have an enum type name, ensure that it
	// hasn't been declared before.
	if (etype != NULL)
		fatals("enum type redeclared:", etype->name);
	else
		// Build an enum type node for this identifier
		etype = addenum(name, C_ENUMTYPE, 0);

	// Loop to get all the enum values
	while (1)
	{
		// Ensure we have an identifier
		// Copy it in case there's an int literal coming up
		ident();// ѭ��ƥ���ʶ��
		name = strdup(Text);// �ӱ�ʶ�����Ƶ�name

		// Ensure this enum value hasn't been declared before
		etype = findenumval(name);
		if (etype != NULL)
			fatals("enum value redeclared:", Text);

		// If the next token is an '=', skip it and
		// get the following int literal
		if (Token.token == T_ASSIGN)
		{
			scan(&Token);
			if (Token.token != T_INTLIT)
				fatal("Expected int literal after '='");
			intval = Token.intvalue;// ��ǰΪT_INTLIT��intvalueΪ��Ӧ������
			scan(&Token);
		}
		// Build an enum value node for this identifier.
		// Increment the value for the next enum identifier.
		etype = addenum(name, C_ENUMVAL, intval++);

		// Bail out on a right curly bracket, else get a comma
		if (Token.token == T_RBRACE)
			break;
		else
			comma(); // ƥ�䶺��
	}
	scan(&Token);			// Skip over the right curly bracket
}


// ����typedef
// Parse a typedef declaration and return the type
// and ctype that it represents
int typedef_declaration(struct symtable** ctype)
{
	int type;

	// Skip the typedef keyword.
	scan(&Token);

	// Get the actual type following the keyword
	type = parse_type(ctype);

	// See if the typedef identifier already exists
	if (findtypedef(Text) != NULL)
		fatals("redefinition of typedef", Text);

	// It doesn't exist so add it to the typedef list
	addtypedef(Text, type, *ctype, 0, 0);
	scan(&Token);
	return (type);
}

// Given a typedef name, return the type it represents
int type_of_typedef(char* name, struct symtable** ctype) 
{
	struct symtable* t;

	// Look up the typedef in the list
	t = findtypedef(name);
	if (t == NULL)
		fatals("unknown type", name);
	scan(&Token);
	*ctype = t->ctype;
	return (t->type);
}


// ������������ ���ű�
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
		*ctype = composite_declaration(P_STRUCT);
		break;
	case T_UNION:
		typer = P_UNION;
		*ctype = composite_declaration(P_UNION);
		break;
	case T_ENUM:
		typer = P_INT;             // ö��ͨ��Ϊint����
		enum_declaration();       // ö����ɺ󣬱�Ȼɨ�赽��
		if (Token.token == T_SEMI)// ɨ�赽�ֺ� ˵������β 
			typer = -1;          
		break;
	case T_TYPEDEF: // �䴦���֮ǰ��ö������
		typer = typedef_declaration(ctype);
		if (Token.token == T_SEMI)
			typer = -1;
		break;
	case T_IDENT:
		typer = type_of_typedef(Text, ctype);
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

// Parse composite type declarations: structs or unions.
// Either find an existing struct/union declaration, or build
// a struct/union symbol table entry and return its pointer.
struct symtable* composite_declaration(int type) 
{
	struct symtable* ctype = NULL;// ���ű����� C_***
	struct symtable* m;
	int offset;

	// Skip the struct/union keyword
	// ɨ����һ��
	scan(&Token);

	// See if there is a following struct/union name
	if (Token.token == T_IDENT)
	{
		// Find any matching composite type
		if (type == P_STRUCT)
			ctype = findstruct(Text);
		else
			ctype = findunion(Text);
		scan(&Token);
	}
	// If the next token isn't an LBRACE , this is
	// the usage of an existing struct/union type.
	// Return the pointer to the type.
	if (Token.token != T_LBRACE) {
		if (ctype == NULL)
			fatals("unknown struct/union type", Text);
		return (ctype);
	}
	// Ensure this struct/union type hasn't been
	// previously defined
	if (ctype)
		fatals("previously defined struct/union", Text);

	// Build the composite type and skip the left brace
	if (type == P_STRUCT)
		ctype = addstruct(Text, P_STRUCT, NULL, 0, 0);
	else
		ctype = addunion(Text, P_UNION, NULL, 0, 0);
	scan(&Token);

	// Scan in the list of members and attach
	// to the struct type's node
	var_declaration_list(NULL, C_MEMBER, T_SEMI, T_RBRACE);
	rbrace();
	ctype->member = Membhead;
	Membhead = Membtail = NULL;

	// Set the offset of the initial member
	// and find the first free byte after it
	m = ctype->member;
	m->posn = 0;
	offset = typesize(m->type, m->ctype);

	// Set the position of each successive member in the composite type
	// Unions are easy. For structs, align the member and find the next free byte
	for (m = m->next; m != NULL; m = m->next)
	{
		// Set the offset for this member
		if (type == P_STRUCT)
			m->posn = genalign(m->type, offset, 1);
		else
			m->posn = 0;

		// Get the offset of the next free byte after this member
		offset += typesize(m->type, m->ctype);
	}
	// Set the overall size of the composite type
	ctype->size = offset;
	return (ctype);// ���ص�ǰ���ű�ڵ�
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

		

		// We might have just parsed a struct, union or enum
		// declaration with no associated variable.
		// The next token might be a ';'. Loop back if it is.
		// XXX: I'm not happy with this as it allows
		// "struct fred;" as an accepted statement
			/*
				Ҫô������ ��Ҫô��ö�����
			*/
		// ������Ϊ��Ҫ�޸ĵĵط�
		if (type == -1)// ö�ٷ����
		{
			semi();
			continue;
		}
		else if ((type == P_STRUCT || type == P_UNION) && Token.token == T_SEMI)// struct ���� union
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









