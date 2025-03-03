
#include "defs.h"
#include "data.h"
#include "decl.h"

// Given a pointer to a symbol that may already exist
// return true if this symbol doesn't exist. We use
// this function to convert externs into globals

/*
*    extern int a （sym->class == C_GLOBAL global）
*     class    type= P_INT
*       int a ctype= P_INT
*/
int is_new_symbol(struct symtable* sym, int class, int type, struct symtable* ctype)
{

    // There is no existing symbol, thus is new
    // 如果当前是一个空的 说明是新的
    if (sym == NULL)
        return(1);

    // global versus extern: if they match that it's not new
    // and we can convert the class to global
    /*
    * 如果当前为全局并且extern的，那么就转换为全局符号
    */
    if ((sym->class == C_GLOBAL && class == C_EXTERN)
        || (sym->class == C_EXTERN && class == C_GLOBAL))
    {

        // If the types don't match, there's a problem
        if (type != sym->type)
            fatals("Type mismatch between global/extern", sym->name);

        // Struct/unions, also compare the ctype
        // 结构体和union必须匹配
        if (type >= P_STRUCT && ctype != sym->ctype)
            fatals("Type mismatch between global/extern", sym->name);

        // If we get to here, the types match, so mark the symbol
        // as global
        // 匹配好的是为全局变量
        sym->class = C_GLOBAL;
        // Return that symbol is not new
        return(0);
    }

    // It must be a duplicate symbol if we get here
    // 一定是一个重复声明的
    fatals("Duplicate global variable declaration", sym->name);
    return(-1);
}


// Parse the current token and return a primitive type enum value,
// a pointer to any composite type and possibly modify
// the class of the type.
// 解析当前类型
int parse_type(struct symtable** ctype, int* class)
{
    int type, exstatic = 1;

    // See if the class has been changed to extern and static
    while (exstatic)
    {
        switch (Token.token)
        {
        case T_EXTERN:
            if (*class == C_STATIC) //extern static int a; 为非法
                fatal("Illegal to have extern and static at the same time");
            *class = C_EXTERN;
            scan(&Token);
            break;
        case T_STATIC:
            if (*class == C_LOCAL)
                fatal("Compiler doesn't support static local declarations");
            if (*class == C_EXTERN)//extern static int a; 为非法
                fatal("Illegal to have extern and static at the same time");
            *class = C_STATIC;
            scan(&Token);
            break;
        default:
            exstatic = 0;// 不是exstatic 
        }
    }

    // 扫描token类型 并进行更改为对应的类型
    switch (Token.token)
    {
    case T_VOID:
        type = P_VOID;
        scan(&Token);
        break;
    case T_CHAR:
        type = P_CHAR;
        scan(&Token);
        break;
    case T_INT:
        type = P_INT;
        scan(&Token);
        break;
    case T_LONG:
        type = P_LONG;
        scan(&Token);
        break;

        // For the following, if we have a ';' after the
        // parsing then there is no type, so return -1.
        // Example: struct x {int y; int z};
    case T_STRUCT:
        type = P_STRUCT;
        *ctype = composite_declaration(P_STRUCT);
        if (Token.token == T_SEMI)
            type = -1;
        break;
    case T_UNION:
        type = P_UNION;
        *ctype = composite_declaration(P_UNION);
        if (Token.token == T_SEMI)
            type = -1;
        break;
    case T_ENUM:
        type = P_INT;		// Enums are really ints
        enum_declaration();
        if (Token.token == T_SEMI)
            type = -1;
        break;
    case T_TYPEDEF:
        type = typedef_declaration(ctype);
        if (Token.token == T_SEMI)
            type = -1;
        break;
    case T_IDENT:
        type = type_of_typedef(Text, ctype);
        break;
    default:
        fatals("Illegal type, token", Token.tokstr);
    }
    return (type);
}

// Given a type parsed by parse_type(), scan in any following
// '*' tokens and return the new type
/*
   int * p ,a;
   解析当前的指针为p 识别为指针


   */
int parse_stars(int type)
{

    while (1)
    {
        if (Token.token != T_STAR)// 识别为指针 ***********p
            break;
        type = pointer_to(type);
        scan(&Token);
    }
    return (type);
}

// Parse a type which appears inside a cast
// 解析强制转换
int parse_cast(void)
{
    int type, class;
    struct symtable* ctype;

    // Get the type inside the parentheses
    type = parse_stars(parse_type(&ctype, &class));

    // Do some error checking. I'm sure more can be done
    if (type == P_STRUCT || type == P_UNION || type == P_VOID)
        fatal("Cannot cast to a struct, union or void type");
    return(type);
}

// Given a type, parse an expression of literals and ensure
// that the type of this expression matches the given type.
// Parse any type cast that precedes the expression.
// If an integer literal, return this value.
// If a string literal, return the label number of the string.
// 解析是否为字符串或者字符常量 或者其他常量

int parse_literal(int type)
{
    struct ASTnode* tree;

    tree = optimise(binexpr(0));// 生成fold ast节点 其实也用于数组int a【 2+5】


     // If there's a cast, get the child and
  // mark it as having the type from the cast
    /*
                                    A_CAST
                                    /
                               A_INTLIT
                                  0

    */

    if (tree->op == A_CAST)
    {
        tree->left->type = tree->type;
        tree = tree->left;
    }


    if (tree->op != A_INTLIT && tree->op != A_STRLIT)
        fatal("Cannot initialise globals with a general expression");

    // If the type is char * and
    // 当前是char* 指针
    if (type == pointer_to(P_CHAR))
    {
        // We have a string literal, return the label number
        // 如果类型为"xsxasxas" bss代码段
        if (tree->op == A_STRLIT)
            return(tree->a_intvalue);// 返回字符串地址
         // 否则
        if (tree->op == A_INTLIT && tree->a_intvalue == 0)
            return(0);// 当前为 （xxx *）0 NULL
    }


    // 当前仅为变量
    // We only get here with an integer literal. The input type
    // is an integer type and is wide enough to hold the literal value

    /*
      long  x= 3;    // 3 可以被赋值到x
      char  y= 4000; // 4000 不可以 因为 y太小
      详见 chr 45
    */
    if (inttype(type) && typesize(type, NULL) >= typesize(tree->type, NULL))
        return(tree->a_intvalue);

    fatal("Type mismatch: literal vs. variable");
    return(0);	// 保证所有路径都有返回值
}



// Given the type, name and class of a scalar variable,
// parse any initialisation value and allocate storage for it.
// Return the variable's symbol table entry.
// 返回符号表的类型是extern 还是 local 还是C_MEMBER变量
struct symtable* scalar_declaration(char* varname, int type, struct symtable* ctype, int class, struct ASTnode** tree)
{
    struct symtable* sym = NULL;
    struct ASTnode* varnode,//变量值节点
        * exprnode;// 表达式节点
    int casttype;//强制类型转换的类型
    *tree = NULL;

    // Add this as a known scalar 仅为声明
    switch (class)
    {
    case C_STATIC:
    case C_EXTERN:
    case C_GLOBAL:
        // See if this variable is new or already exists
        sym = findglob(varname);
        if (is_new_symbol(sym, class, type, ctype))// 新的sym
            sym = addglob(varname, type, ctype, S_VARIABLE, class, 1, 0);
        break;
    case C_LOCAL:
        sym = addlocl(varname, type, ctype, S_VARIABLE, 1);
        break;
    case C_PARAM:
        sym = addparm(varname, type, ctype, S_VARIABLE);
        break;
    case C_MEMBER:
        sym = addmemb(varname, type, ctype, S_VARIABLE, 1);
        break;
    }

    // The variable is being initialised
    // 
    /*
        int a=4 char cc='1' 为赋值
    */
    if (Token.token == T_ASSIGN)
    {
        // 只可能是global 或者 local
        if (class != C_GLOBAL && class != C_LOCAL && class != C_STATIC)
            fatals("Variable can not be initialised", varname);
        scan(&Token);

        // Globals must be assigned a literal value
        // 在本c comp中目前必须实现全局初始化 static也是为global的一种但是要区分
        //if (class == C_GLOBAL || class == C_STATIC)为最新条件
        if (class == C_GLOBAL || class == C_STATIC)
        {

#if 0

            // If there is a cast
            if (Token.token == T_LPAREN)
            {


                // Get the type in the cast
                scan(&Token);
                casttype = parse_cast();
                rparen();

                // Check that the two types are compatible. Change
                // the new type so that the literal parse below works.
                // A 'void *' casstype can be assigned to any pointer type.
                // 第一种情况为释放掉强制类型转换 因为没必要 第二种为空指针强制类型转换
                //  2==比如说 char *str= (void *)0;
                /*

                                    A_ASSIGN
                                     /    \
                                 A_CAST  A_IDENT
                                    /      str
                               A_INTLIT
                                  0
                */
                if (casttype == type || (casttype == pointer_to(P_VOID) && ptrtype(type)))
                    type = P_NONE;// 释放掉强制类型转换 因为没必要
                else
                    fatal("Type mismatch");
            }

#endif // 0

            // Create one initial value for the variable and
            // parse this value
            sym->initlist = (int*)malloc(sizeof(int));
            sym->initlist[0] = parse_literal(type); // 在第chr 45 中parse_literal已经带有强制类型转换
         //   scan(&Token);
        }


        if (class == C_LOCAL)
        {
            // Make an A_IDENT AST node with the variable
            //生成标识符节点
            varnode = mkastleaf(A_IDENT, sym->type, sym->ctype, sym, 0);

            // Get the expression for the assignment, make into a rvalue
            // 右边值表达式
            exprnode = binexpr(0);
            exprnode->rvalue = 1;

            // Ensure the expression's type matches the variable
            exprnode = modify_type(exprnode, varnode->type, varnode->ctype, 0);
            if (exprnode == NULL)
                fatal("Incompatible expression in assignment");

            // Make an assignment AST tree
            *tree = mkastnode(A_ASSIGN, exprnode->type, exprnode->ctype, exprnode, NULL, varnode, NULL, 0);
        }
    }

    //生成全局空间
    if (class == C_GLOBAL || class == C_STATIC)
        genglobsym(sym);

    return (sym);
}

// Given the type, name and class of an variable, parse
// the size of the array, if any. Then parse any initialisation
// value and allocate storage for it.
// Return the variable's symbol table entry.
struct symtable* array_declaration(char* varname, int type, struct symtable* ctype, int class)
{

    struct symtable* sym = NULL;	// New symbol table entry
    int nelems = -1;	// Assume the number of elements won't be given 成员个数
    int maxelems;		// int c[]={......} 大括号中最多10个
    int* initlist;	// The list of initial elements {......} 的内容
    int i = 0, j;
    int casttype, //强制转换类型
        newtype; // 新类型

    // Skip past the '['
    scan(&Token);

    // See we have an array size 其实这里可以改为 一个binexpr 当前在chr 45 实现
    if (Token.token != T_RBRACKET)// 可以编译 extern int a[]; int a[11];
    {
        nelems = parse_literal(P_INT);// 经过变量折叠技术 将当前编译器值进行优化 
        if (nelems <= 0)
            fatald("Array size is illegal", nelems);
    }
    // 匹配 ']'
    match(T_RBRACKET, "]");

    // Add this as a known array. We treat the
    // array as a pointer to its elements' type
    // 队列添加
    switch (class)
    {
    case C_STATIC:
    case C_EXTERN:
    case C_GLOBAL:
        // See if this variable is new or already exists
        sym = findglob(varname);
        if (is_new_symbol(sym, class, pointer_to(type), ctype))
            sym = addglob(varname, pointer_to(type), ctype, S_ARRAY, class, 0, 0);
        break;
    case C_LOCAL:
        sym = addlocl(varname, pointer_to(type), ctype, S_ARRAY, 0);
        break;
    default:
        fatal("Declaration of array parameters is not implemented");// 不可以直接声明 int a[];
    }

    // 数组初始化 
    if (Token.token == T_ASSIGN)
    {
        if (class != C_GLOBAL && class != C_STATIC)// 在chr 56 时 阻止局部array赋值
            fatals("Variable can not be initialised", varname);
        scan(&Token);

        // Get the following left curly bracket
        match(T_LBRACE, "{");

#define TABLE_INCREMENT 10

        // If the array already has nelems, allocate that many elements
        // in the list. Otherwise, start with TABLE_INCREMENT.
        if (nelems != -1)
            maxelems = nelems;
        else
            maxelems = TABLE_INCREMENT;
        initlist = (int*)malloc(maxelems * sizeof(int));

        // 扫描{....}列表中的值 chr 56 以前
#if 0
        while (1)
        {

            // Get the original type
            newtype = type;
            // Check we can add the next value, then parse and add it
            if (nelems != -1 && i == maxelems)
                fatal("Too many values in initialisation list");


            if (Token.token == T_LPAREN)// 匹配大括号  错误定于此处 勿动
            {
                // Get the type in the cast
                scan(&Token);
                casttype = parse_cast();
                rparen();

                // Check that the two types are compatible. Change
                // the new type so that the literal parse below works.
                // A 'void *' casstype can be assigned to any pointer type.
                if (casttype == type || (casttype == pointer_to(P_VOID) && ptrtype(type)))
                    newtype = P_NONE;
                else
                    fatal("Type mismatch");
                newtype = P_NONE;
            }

            initlist[i++] = parse_literal(newtype);// 解析文本 传入的值为newtype
            scan(&Token);

            // Increase the list size if the original size was
            // not set and we have hit the end of the current list
            if (nelems == -1 && i == maxelems)
            {
                maxelems += TABLE_INCREMENT;
                initlist = (int*)realloc(initlist, maxelems * sizeof(int));
            }

            // Leave when we hit the right curly bracket
            if (Token.token == T_RBRACE)
            {
                scan(&Token);
                break;
            }

            // Next token must be a comma, then
            comma();
        }
#endif // 0

        //扫描{....}列表中的值
        while (1)
        {

            // Check we can add the next value, then parse and add it
            if (nelems != -1 && i == maxelems)
                fatal("Too many values in initialisation list");

            initlist[i++] = parse_literal(type);

            // Increase the list size if the original size was
            // not set and we have hit the end of the current list
               /*
           eg . int a[]={........} // 100 个数组
                    nelems==-1 【】未给出
             那么当前maxelems += TABLE_INCREMENT; 依照递增TABLE_INCREMENT来进行增加realloc分配空间

        */
            if (nelems == -1 && i == maxelems)
            {
                maxelems += TABLE_INCREMENT;
                initlist = (int*)realloc(initlist, maxelems * sizeof(int));
            }
            // Leave when we hit the right curly bracket
            if (Token.token == T_RBRACE)
            {
                scan(&Token);
                break;
            }

            comma();
        }


        // Zero any unused elements in the initlist.
        // Attach the list to the symbol table entry

        /*
             int a[10]={0,1,2,3.......}
             当前i为4 剩下的10-4=6个填充为0
        */
        for (j = i; j < sym->nelems; j++)
            initlist[j] = 0;
        if (i > nelems)
            nelems = i;
        sym->initlist = initlist;
    }

    // Set the size of the array and the number of elements  Only externs can have no elements.

    /*
    *    extern int a[];
    *     int a[23];
     fun()......     允许
    *
    */

    // 仅有extern的数组成员可以为空
    // extern int a[1];
    if (class != C_EXTERN && nelems <= 0)
        fatals("Array must have non-zero elements", sym->name);

    // Set the size of the array and the number of elements
    sym->nelems = nelems;//当前数组符号表的个数
    sym->size = sym->nelems * typesize(type, ctype);//当前数组符号表的大小
    // Generate any global and static space
    if (class == C_GLOBAL || class == C_STATIC)
        genglobsym(sym);
    return (sym);
}

// Given a pointer to the new function being declared and
// a possibly NULL pointer to the function's previous declaration,
// parse a list of parameters and cross-check them against the
// previous declaration. Return the count of parameters
// 函数参数列表fun(a,b,c......)
int param_declaration_list(struct symtable* oldfuncsym, struct symtable* newfuncsym)
{
    int type, paramcnt = 0;
    struct symtable* ctype;
    struct symtable* protoptr = NULL;
    struct ASTnode* unused;

    // Get the pointer to the first prototype parameter
    if (oldfuncsym != NULL)
        protoptr = oldfuncsym->member;

    // Loop getting any parameters
    while (Token.token != T_RPAREN)
    {

        // 第一个'void'
        if (Token.token == T_VOID)
        {
            // Peek at the next token. If a ')', the function
            // has no parameters, so leave the loop.
            scan(&Peektoken);// 在chr 46中peektoken的作用是为了维护(void)
            if (Peektoken.token == T_RPAREN)
            {
                // Move the Peektoken into the Token
                paramcnt = 0;
                scan(&Token); // 则扫描下一个及真正的token 其实就是Peektoken
                break;
            }
        }

        // Get the type of the next parameter
        type = declaration_list(&ctype, C_PARAM, T_COMMA, T_RPAREN, &unused);
        if (type == -1)
            fatal("Bad type in parameter list");

        // Ensure the type of this parameter matches the prototype
        // 确保参数与函数声明的类型匹配
        if (protoptr != NULL)
        {
            if (type != protoptr->type)
                fatald("Type doesn't match prototype for parameter", paramcnt + 1);
            protoptr = protoptr->next;
        }
        paramcnt++;

        // Stop when we hit the right parenthesis
        if (Token.token == T_RPAREN)
            break;
        // We need a comma as separator
        comma();
    }

    if (oldfuncsym != NULL && paramcnt != oldfuncsym->nelems)
        fatals("Parameter count mismatch for function", oldfuncsym->name);

    // Return the count of parameters
    return (paramcnt);
}


//
// function_declaration: type identifier '(' parameter_list ')' ;
//      | type identifier '(' parameter_list ')' compound_statement   ;
//
// Parse the declaration of function.

struct symtable* function_declaration(char* funcname, int type, struct symtable* ctype, int class)
{
    struct ASTnode* tree, * finalstmt;
    struct symtable* oldfuncsym, * newfuncsym = NULL;
    int endlabel = 0, paramcnt;

    // Text has the identifier's name. If this exists and is a
    // function, get the id. Otherwise, set oldfuncsym to NULL.
    if ((oldfuncsym = findsymbol(funcname)) != NULL)
        if (oldfuncsym->stype != S_FUNCTION)
            oldfuncsym = NULL;

    // If this is a new function declaration, get a
    // label-id for the end label, and add the function
    // to the symbol table,
    if (oldfuncsym == NULL) {
        endlabel = genlabel();
        // Assumtion: functions only return scalar types, so NULL below
        newfuncsym =
            addglob(funcname, type, NULL, S_FUNCTION, C_GLOBAL, 0, endlabel);
    }
    // Scan in the '(', any parameters and the ')'.
    // Pass in any existing function prototype pointer
    lparen();
    paramcnt = param_declaration_list(oldfuncsym, newfuncsym);
    rparen();

    // If this is a new function declaration, update the
    // function symbol entry with the number of parameters.
    // Also copy the parameter list into the function's node.
    if (newfuncsym) {
        newfuncsym->nelems = paramcnt;
        newfuncsym->member = Parmhead;
        oldfuncsym = newfuncsym;
    }
    // Clear out the parameter list
    Parmhead = Parmtail = NULL;

    // Declaration ends in a semicolon, only a prototype.
    if (Token.token == T_SEMI)
        return (oldfuncsym);

    // This is not just a prototype.
    // Set the Functionid global to the function's symbol pointer
    Functionid = oldfuncsym;

    // Get the AST tree for the compound statement and mark
    // that we have parsed no loops or switches yet
    Looplevel = 0;
    Switchlevel = 0;
    lbrace();
    tree = compound_statement(0);
    rbrace();

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
    // Build the A_FUNCTION node which has the function's symbol pointer
    // and the compound statement sub-tree
    tree = mkastunary(A_FUNCTION, type, ctype, tree, oldfuncsym, endlabel);


    // Do optimisations on the AST tree
    tree = optimise(tree);

#if 0

    // Generate the assembly code for it
    if (O_dumpAST)
    {
        dumpAST(tree, NOLABEL, 0);
        fprintf(stdout, "\n\n");
    }

#endif // 0


    genAST(tree, NOLABEL, NOLABEL, NOLABEL, 0);

    // Now free the symbols associated
    // with this function
    freeloclsyms();
    return (oldfuncsym);
}




// Parse composite type declarations: structs or unions.
// Either find an existing struct/union declaration, or build
// a struct/union symbol table entry and return its pointer.
 // struct 和 union 的声明
struct symtable* composite_declaration(int type)
{
    struct symtable* ctype = NULL;
    struct symtable* m;
    struct ASTnode* unused;
    int offset;
    int t;

    // Skip the struct/union keyword
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
        ctype = addstruct(Text);
    else
        ctype = addunion(Text);
    scan(&Token);

    // Scan in the list of members
    while (1) {
        // Get the next member. m is used as a dummy
        t = declaration_list(&m, C_MEMBER, T_SEMI, T_RBRACE, &unused);
        if (t == -1)
            fatal("Bad type in member list");
        if (Token.token == T_SEMI)
            scan(&Token);
        if (Token.token == T_RBRACE)
            break;
    }

    // Attach to the struct type's node
    rbrace();
    if (Membhead == NULL)
        fatals("No members in struct", ctype->name);
    ctype->member = Membhead;
    Membhead = Membtail = NULL;

    // Set the offset of the initial member
    // and find the first free byte after it
    m = ctype->member;
    m->st_posn = 0;
    offset = typesize(m->type, m->ctype);

    // Set the position of each successive member in the composite type
    // Unions are easy. For structs, align the member and find the next free byte
    for (m = m->next; m != NULL; m = m->next) {
        // Set the offset for this member
        if (type == P_STRUCT)
            m->st_posn = genalign(m->type, offset, 1);
        else
            m->st_posn = 0;

        // Get the offset of the next free byte after this member
        offset += typesize(m->type, m->ctype);
    }

    // Set the overall size of the composite type
    ctype->size = offset;
    return (ctype);
}

// Parse an enum declaration
void enum_declaration(void)
{
    struct symtable* etype = NULL;
    char* name = NULL;
    int intval = 0;

    // Skip the enum keyword.
    scan(&Token);

    // If there's a following enum type name, get a
    // pointer to any existing enum type node.
    if (Token.token == T_IDENT) {
        etype = findenumtype(Text);
        name = strdup(Text);	// As it gets tromped soon
        scan(&Token);
    }
    // If the next token isn't a LBRACE, check
    // that we have an enum type name, then return
    if (Token.token != T_LBRACE) {
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
    while (1) {
        // Ensure we have an identifier
        // Copy it in case there's an int literal coming up
        ident();
        name = strdup(Text);

        // Ensure this enum value hasn't been declared before
        etype = findenumval(name);
        if (etype != NULL)
            fatals("enum value redeclared:", Text);

        // If the next token is an '=', skip it and
        // get the following int literal
        if (Token.token == T_ASSIGN) {
            scan(&Token);
            if (Token.token != T_INTLIT)
                fatal("Expected int literal after '='");
            intval = Token.intvalue;
            scan(&Token);
        }
        // Build an enum value node for this identifier.
        // Increment the value for the next enum identifier.
        etype = addenum(name, C_ENUMVAL, intval++);

        // Bail out on a right curly bracket, else get a comma
        if (Token.token == T_RBRACE)
            break;
        comma();
    }
    scan(&Token);			// Skip over the right curly bracket
}

// Parse a typedef declaration and return the type
// and ctype that it represents
int typedef_declaration(struct symtable** ctype)
{
    int type, class = 0;

    // Skip the typedef keyword.
    scan(&Token);

    // Get the actual type following the keyword
    // 获取真实的类型
    type = parse_type(ctype, &class);
    if (class != 0)
        fatal("Can't have extern in a typedef declaration");

    // See if the typedef identifier already exists
    if (findtypedef(Text) != NULL)
        fatals("redefinition of typedef", Text);

    // 下面为需要添加新的typedef
    // Get any following '*' tokens
    // 解析是否有指针 typedef 
    // typedef int* aa;
    type = parse_stars(type);

    // It doesn't exist so add it to the typedef list
    addtypedef(Text, type, *ctype);
    scan(&Token);
    return (type);
}

// Given a typedef name, return the type it represents
 // 给出typedef声明代表的意思
 /*
 *  typedef int as;
 */
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

// Parse the declaration of a variable or function.
// The type and any following '*'s have been scanned, and we
// have the identifier in the Token variable.
// The class argument is the variable's class.
// Return a pointer to the symbol's entry in the symbol table
struct symtable* symbol_declaration(int type, struct symtable* ctype, int class, struct ASTnode** tree)
{
    struct symtable* sym = NULL;
    char* varname = strdup(Text);

    // Ensure that we have an identifier. 
    // We copied it above so we can scan more tokens in, e.g.
    // an assignment expression for a local variable.
    ident();

    // Deal with function declarations
    if (Token.token == T_LPAREN)
    {
        return (function_declaration(varname, type, ctype, class));
    }
    // See if this array or scalar variable has already been declared
    switch (class)
    {
    case C_EXTERN:
    case C_STATIC:
    case C_GLOBAL:
        /*  当前已更改 extern  故注释掉
        * if (findglob(varname) != NULL)
            fatals("Duplicate global variable declaration", varname);
        */
    case C_LOCAL:
    case C_PARAM:
        if (findlocl(varname) != NULL)
            fatals("Duplicate local variable declaration", varname);
    case C_MEMBER:
        if (findmember(varname) != NULL)
            fatals("Duplicate struct/union member declaration", varname);
    }

    // Add the array or scalar variable to the symbol table
    if (Token.token == T_LBRACKET)
    {
        sym = array_declaration(varname, type, ctype, class);
        *tree = NULL;	// Local arrays are not initialised
    }
    else
        sym = scalar_declaration(varname, type, ctype, class, tree);
    return (sym);
}



// Parse a list of symbols where there is an initial type.
// Return the type of the symbols. et1 and et2 are end tokens.
// 解析一下文本  et1 and et2 为结束符 class 为存储类型
/*
   int *a，*b， ccdd ，cs ，sds。。。。。
   */
int declaration_list(struct symtable** ctype, int class, int et1, int et2, struct ASTnode** gluetree)
{
    int inittype, type;
    struct symtable* sym;
    struct ASTnode* tree;
    *gluetree = NULL;// gluetree 为AST tree的分支 其头节点为A_GLUE

    // Get the initial type. If -1, it was
    // a composite type definition, return this
    if ((inittype = parse_type(ctype, &class)) == -1)
        return (inittype);

    // Now parse the list of symbols
    while (1)
    {
        // See if this symbol is a pointer
        type = parse_stars(inittype);

        // Parse this symbol
        sym = symbol_declaration(type, *ctype, class, &tree);

        // We parsed a function, there is no list so leave
        if (sym->stype == S_FUNCTION)
        {
            if (class != C_GLOBAL && class != C_STATIC)// 函数为全局
                fatal("Function definition not at global level");
            return (type);
        }


        // Glue any AST tree from a local declaration
       // to build a sequence of assignments to perform
        if (*gluetree == NULL)
            *gluetree = tree;
        else
            *gluetree = mkastnode(A_GLUE, P_NONE, NULL, *gluetree, NULL, tree, NULL, 0);


        // We are at the end of the list, leave
        if (Token.token == et1 || Token.token == et2)
            return (type);

        // Otherwise, we need a comma as separator
        comma();
    }
}

// Parse one or more global declarations, either
// variables, functions or structs
void global_declarations(void)
{
    struct symtable* ctype;
    struct ASTnode* unused;
    while (Token.token != T_EOF)
    {
        declaration_list(&ctype, C_GLOBAL, T_SEMI, T_EOF, &unused);// declaration_list的举例子
        /*
        * 因为一个声明链表为 ; 为最后
        */
        // Skip any semicolons and right curly brackets
        if (Token.token == T_SEMI)
            scan(&Token);
    }
}
