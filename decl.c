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
// class is the variable's class
void var_declaration(int type, int class)
{

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
      if (class == C_LOCAL)
	  {
		fatal("For now, declaration of local arrays is not implemented");
      }
	  else 
	  {
		addglob(Text, pointer_to(type), S_ARRAY, class, 0, Token.intvalue);
      }
    }
    // Ensure we have a following ']'
    scan(&Token);
    match(T_RBRACKET, "]");
  } else {
    // Add this as a known scalar
    // and generate its space in assembly
    if (class == C_LOCAL)
	{// 添加 局部变量
      if (addlocl(Text, type, S_VARIABLE, class, 1) == -1)
		fatals("Duplicate local variable declaration", Text);
    } 
	else 
	{// 添加 全局变量
      addglob(Text, type, S_VARIABLE, class, 0, 1);
    }
  }
}

// param_declaration: <null>
//           | variable_declaration
//           | variable_declaration ',' param_declaration
//
// Parse the parameters in parentheses after the function name.
// Add them as symbols to the symbol table and return the number
// of parameters. If id is not -1, there is an existing function
// prototype, and the function has this symbol slot number.
static int param_declaration(int id)
{
	int type, param_id;
	int orig_paramcnt;// 函数原型的参数个数
	int paramcnt = 0;// 函数体实现时的参数个数

	// Add 1 to id so that it's either zero (no prototype), or
	// it's the position of the zeroth existing parameter in
	// the symbol table
	param_id = id + 1;// 参数序号从一开始好计算

	// Get any existing prototype parameter count
	if (param_id)
		orig_paramcnt = Gsym[id].nelems;

	// Loop until the final right parentheses
	while (Token.token != T_RPAREN)
	{
		// Get the type and identifier
		// and add it to the symbol table
		type = parse_type();
		ident();

		// We have an existing prototype.
		// Check that this type matches the prototype.
		if (param_id) 
		{
			if (type != Gsym[id].type)
				fatald("Type doesn't match prototype for parameter", paramcnt + 1);
			param_id++;// 参数个数++
		}
		else {
			// 使用Var——declation 声明函数
			var_declaration(type, C_PARAM);
		}
		paramcnt++;

		// Must have a ',' or ')' at this point
		switch (Token.token)
		{
		case T_COMMA:
			scan(&Token);
			break;
		case T_RPAREN:
			break;
		default:
			fatald("Unexpected token in parameter list", Token.token);
		}
	}

	// Check that the number of parameters in this list matches
	// any existing prototype
	if ((id != -1) && (paramcnt != orig_paramcnt)) // 这里建议类型匹配
		fatals("Parameter count mismatch for function", Gsym[id].name);

	// Return the count of parameters
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
	struct ASTnode* tree, * finalstmt;
	int id; // 函数下标识在符号表中
	int nameslot, endlabel, paramcnt;

	// Text has the identifier's name. If this exists and is a
	// function, get the id. Otherwise, set id to -1
	if ((id = findsymbol(Text)) != -1)
		if (Gsym[id].stype != S_FUNCTION)
			id = -1;

	// If this is a new function declaration, get a
	// label-id for the end label, and add the function
	// to the symbol table,
	if (id == -1)
	{
		endlabel = genlabel();
		nameslot = addglob(Text, type, S_FUNCTION, C_GLOBAL, endlabel, 0);
	}
	// Scan in the '(', any parameters and the ')'.
	// Pass in any existing function prototype symbol slot number
	lparen();
	paramcnt = param_declaration(id);
	rparen();

	// If this is a new function declaration, update the
	// function symbol entry with the number of parameters
	if (id == -1)
		Gsym[nameslot].nelems = paramcnt;

	// 函数声明
	if (Token.token == T_SEMI)
	{
		scan(&Token);
		return (NULL);
	}

	// 下面不是函数声明，为函数实现
	// This is not just a prototype.
	// Copy the global parameters to be local parameters
	if (id == -1)
		id = nameslot;
	copyfuncparams(id);

	// Set the Functionid global to the function's symbol-id
	Functionid = id;

	// Get the AST tree for the compound statement
	tree = compound_statement();

	// If the function type isn't P_VOID ..
	if (type != P_VOID)
	{

		// Error if no statements in the function
		if (tree == NULL)
			fatal("No statements in function with non-void type");

		// Check that the last AST operation in the
		// compound statement was a return statement
		finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
		if (finalstmt == NULL || finalstmt->op != A_RETURN)
			fatal("No return for function with non-void type");
	}
	// Return an A_FUNCTION node which has the function's id
	// and the compound statement sub-tree
	return (mkastunary(A_FUNCTION, type, tree, id));
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
int parse_type()
{
	int typer = -1;
	switch (Token.token)
	{
	case T_CHAR:
		typer = P_CHAR;
		break;
	case T_INT:
		typer = P_INT;
		break;
	case T_VOID:
		typer = P_VOID;
		break;
	case T_LONG:
		typer = P_LONG;
		break;
	default:
		fatald("Illegal type, token", Token.token);
	}

	while (1) //  *****************p
	{
		scan(&Token);
		if (Token.token == T_STAR)
			typer = pointer_to(typer);
		else
			break;
	}
	return typer;

}


void global_declarations(void)
{
	struct ASTnode* tree;
	int type;

	while (1)
	{
		type = parse_type();
		ident();
		if (Token.token == T_LPAREN)
		{

			// Parse the function declaration and
			// generate the assembly code for it
			tree = function_declaration(type);

			// 仅为函数原型
			if (tree == NULL)
				continue;

			if (O_dumpAST)
			{
				dumpAST(tree, NOLABEL, 0);
				fprintf(stdout, "\n\n");
			}
			genAST(tree, NOLABEL, 0);

			// Now free the symbols associated  with this function
			freeloclsyms();
		}
		else
		{

			// 解析全局变量声明 并且匹配分号
			var_declaration(type, 0, 0);
			semi();
		}
		if (Token.token == T_EOF)
			break;
	}
}

