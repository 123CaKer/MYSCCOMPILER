#include "defs.h"
#include "data.h"
#include "decl.h"


// List of token strings, for debugging purposes
char* Tstring[] =
{
  "EOF", "=", "+=", "-=", "*=", "/=",
  "?", "||", "&&", "|", "^", "&",
  "==", "!=", ",", ">", "<=", ">=", "<<", ">>",
  "+", "-", "*", "/", "++", "--", "~", "!",
  "void", "char", "int", "long",
  "if", "else", "while", "for", "return",
  "struct", "union", "enum", "typedef",
  "extern", "break", "continue", "switch",
  "case", "default", "sizeof", "static",
  "intlit", "strlit", ";", "identifier",
  "{", "}", "(", ")", "[", "]", ",", ".",
  "->", ":"
};


// rejtoken 指针
static struct token* Rejtoken = NULL;

static int chrpos(char* s, int c)
// 返回c在s中的位置（第一个）
{
    char* p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}


int next(void)
// 获取输入文件中的下一个字符
{
    int c, //获取的字符
        l; // 行号

    if (Putback)
    {			//字符输入
        c = Putback;			// 需要返回的
        Putback = 0;
        return (c);
    }

    c = fgetc(Infile);			// Read from input file

    while (c == '#')
    {			// We've hit a pre-processor statement
        scan(&Token);			// 获取行号
        if (Token.token != T_INTLIT)
            fatals("Expecting pre-processor line number, got:", Text);
        l = Token.intvalue;

        scan(&Token);			// Get the filename in Text 获取文件名字
        if (Token.token != T_STRLIT)
            fatals("Expecting pre-processor file name, got:", Text);

        if (Text[0] != '<')// 说明当前为真正的文件名字
        {		// If this is a real filename
            if (strcmp(Text, Infilename))	// and not the one we have now
                Infilename = strdup(Text);	// save it. Then update the line num
            Line = l;
        }

        while ((c = fgetc(Infile)) != '\n'); // Skip to the end of the line
        c = fgetc(Infile);			// and get the next character
    }

    if ('\n' == c)
        Line++;				// Increment line count
    return (c);
}



static void putback(int c)  // 将不需要的值放回Putback中 
{
    Putback = c;
}


static int skip(void)
//不断的获取字符 ，并且忽略掉非必要的东西 譬如while循环中显示  即 a c = 0; 表达式中空格忽略
{
    int c;

    c = next();
    while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c)
        c = next();
    return c;
}


//读取16进制常数
int hexchar(void)
{
    int c, h, n = 0,
        f = 0;//f为1时为16进制数字

    // Loop getting characters
    while (isxdigit(c = next()))
    {
        // Convert from char to int value
        h = chrpos("0123456789abcdef", tolower(c));
        //16 转换为10进制值
        n = n * 16 + h;
        f = 1;
    }
    // We hit a non-hex character, put it back
    putback(c);// 最后while循环会判断调用c = next()
    // Flag tells us we never saw any hex characters
    if (!f)
        fatal("missing digits after '\\x'");
    if (n > 255)// 16进制数真值小于等于255
        fatal("value out of range after '\\x'");
    return n;
}


// Return the next character from a character
// or string literal
int scanch(void)   // 主要用于识别 转义字符
{
    int i, c, c2;

    // Get the next input character and interpret
    // metacharacters that start with a backslash
    c = next();
    if (c == '\\')
    {
        switch (c = next())
        {
        case 'a':
            return '\a'; //蜂鸣器
        case 'b':
            return '\b'; // 退格符 删除当前字符
        case 'f':
            return '\f'; // 
        case 'n':
            return '\n';
        case 'r':
            return '\r'; // 回车符 将光标移动到当前行的开头
        case 't':
            return '\t';
        case 'v':
            return '\v'; // 垂直制表符
        case '\\':
            return '\\'; // /
        case '"':
            return '"';// 双引号
        case '\'':
            return '\'';
            // Deal with octal constants by reading in
            // characters until we hit a non-octal digit.
            // Build up the octal value in c2 and count
            // # digits in i. Permit only 3 octal digits.
                    // 下面处理八进制值
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            for (i = c2 = 0; isdigit(c) && c < '8'; c = next())
            {
                if (++i > 3)// subc8进制仅能4位数
                    break;
                c2 = c2 * 8 + (c - '0');// 计算八进制值
            }
            putback(c);		// 同16进制回撤
            return (c2);
        case 'x':
            return hexchar();

        default:
            fatalc("unknown escape sequence", c);
        }
    }
    return (c);			// Just an ordinary old character!
}



static int scanint(int c)
// 从输入文件中扫描并返回整形数据 
{
    int k, val = 0, radix = 10;// radix是进制


    if (c == '0') //如果第一个字符是0
    {
        // 下一个是x
        if ((c = next()) == 'x')
        {
            radix = 16;
            c = next();
        }
        else
            radix = 8;

    }
    // Convert each character into an int value
    while ((k = chrpos("0123456789abcdef", tolower(c))) >= 0)
    {
        if (k >= radix)
            fatalc("invalid digit in integer literal", c);
        val = val * radix + k;//0xval val*16 + k
        /*
            举例 56
              第一遍 v=0*10+5 =5
              第二遍 v=5*10+6=56
              非常重点的一种数据转换方法
        */
        c = next();
    }


    putback(c);// 不是的话就返回
    return (val);
}




// Scan in a string literal from the input file,
// and store it in buf[]. Return the length of
// the string. 
static int scanstr(char* buf)
{
    int i, c;

    // Loop while we have enough buffer space
    for (i = 0; i < TEXTLEN - 1; i++)
    {
        // Get the next char and append to buf
        // Return when we hit the ending double quote
        if ((c = scanch()) == '"')
        {
            buf[i] = 0;
            return(i);
        }
        buf[i] = c;
    }
    // Ran out of buf[] space
    fatal("String literal too long");
    return(0);
}

//  Rejtoken = t;
void reject_token(struct token* t)
{
    if (Rejtoken != NULL)
        fatal("Can't reject token twice");
    Rejtoken = t;
}



// 扫描token并判断类型并返回
int scan(struct token* t)
{
    int c = 0;
    int tokentype;


    // If we have a lookahead token, return this token 为了 dangling else
    if (Peektoken.token != 0)
    {
        t->token = Peektoken.token;
        t->tokstr = Peektoken.tokstr;
        t->intvalue = Peektoken.intvalue;
        Peektoken.token = 0;
        return (1);
    }
    // 然后之后下一次去扫描他变为真正的token

    // 查看是否有之前拒绝的token 若有的话将全局token 设置为Rejtoken并返回
    if (Rejtoken != NULL)
    {
        t = Rejtoken;
        Rejtoken = NULL;
        return 1;
    }

    c = skip();  // 忽略空格换行符等 类似getc

    switch (c)
    {
    case EOF: // 输入文件已到达末尾
        t->token = T_EOF;
        printf("输入文件已经扫描完成\n");
        return 0;
    case '+':
        if ((c = next()) == '+')
        {
            t->token = T_INC;  // 自增
        }
        else if (c == '=')
        {
            t->token = T_ASPLUS;// +=
        }
        else
        {
            putback(c);
            t->token = T_PLUS;
        }
        break;
    case '-':
        if ((c = next()) == '-')  // --
        {
            t->token = T_DEC;
        }
        else if (c == '>')  // ->
        {
            t->token = T_ARROW;
        }
        else if (c == '=')
        {
            t->token = T_ASMINUS; // -=
        }
        else if (isdigit(c))
        {		// Negative int literal 负数
            t->intvalue = -scanint(c);
            t->token = T_INTLIT;
        }
        else
        {
            putback(c);
            t->token = T_MINUS;
        }
        break;
    case '.':
        t->token = T_DOT;// .成员
        break;
    case '*':
        if ((c = next()) == '=')
        {
            t->token = T_ASSTAR; // *=
        }
        else
        {
            putback(c);
            t->token = T_STAR;
        }
        break;
    case '/':
        if ((c = next()) == '=')
        {
            t->token = T_ASSLASH;
        }
        else
        {
            putback(c);
            t->token = T_SLASH;
        }
        break;
    case ';':
        t->token = T_SEMI;
        break;
    case ',':
        t->token = T_COMMA; // （ ,  , ）
        break;

    case'=':
        if ((c = next()) == '=') // 不用担心当前为真 因为最终回在 putback中先获取
        {
            t->token = T_EQ;
            break;
        }
        else
        {
            putback(c);
            t->token = T_ASSIGN;
            break;
        }
    case '!':  // 不等于
        if ((c = next()) == '=')
        {
            t->token = T_NE;
            break;
        }
        else
        {
            putback(c);
            t->token = T_LOGNOT;  // 逻辑非
            break;

        }
    case '?':
        t->token = T_QUESTION; // 三元运算符 ?
        break;
    case '>':
        if ((c = next()) == '=')
        {
            t->token = T_GE;
        }
        else if (c == '>')
        {
            t->token = T_RSHIFT; // 右移
        }
        else
        {
            putback(c);
            t->token = T_GT;
        }
        break;
    case '<':
        if ((c = next()) == '=')
        {
            t->token = T_LE;
        }
        else if (c == '<')
        {
            t->token = T_LSHIFT; // 左移
        }
        else {
            putback(c);
            t->token = T_LT;
        }
        break;
    case '{':
        t->token = T_LBRACE;
        break;
    case '}':
        t->token = T_RBRACE;
        break;
    case '(':
        t->token = T_LPAREN;
        break;
    case ')':
        t->token = T_RPAREN;
        break;
    case '&':
        if ((c = next()) == '&')
        {
            t->token = T_LOGAND; //&&
        }
        else
        {
            putback(c);//回退
            t->token = T_AMPER; //&p
        }
        break;

    case '|':
        if ((c = next()) == '|')
        {
            t->token = T_LOGOR; // 与
        }
        else
        {
            putback(c);
            t->token = T_OR; // 按位与
        }
        break;

    case ':':
        t->token = T_COLON;
        break;
    case '~': // 按位取反
        t->token = T_INVERT;
        break;
    case '^': //  异或
        t->token = T_XOR;
        break;

    case '[':
        t->token = T_LBRACKET;
        break;
    case ']':
        t->token = T_RBRACKET;
        break;

    case '\'':
        // If it's a quote, scan in the
        // literal character value and
        // the trailing quote
        t->intvalue = scanch();
        t->token = T_INTLIT;
        if (next() != '\'')
            fatal("Expected '\\'' at end of char literal");
        break;

    case '"':
        // Scan in a literal string
        scanstr(Text);
        t->token = T_STRLIT;
        break;

    default:
        if (isdigit(c))
        {
            t->intvalue = scanint(c);//// 从输入文件中扫描并返回整形数据 
            t->token = T_INTLIT;
            break;
        }
        else if (isalpha(c) || '_' == c)
        {
            scanident(c, Text, TEXTLEN + 1);// 输入到缓冲区中
            int judge = keyword(Text);
            switch (judge)
            {
                /*
            case T_PRINT:
                t->token = T_PRINT;
                break;
                */
            case T_INT:
                t->token = T_INT;
                break;
            case T_IF:
                t->token = T_IF;
                break;
            case T_ELSE:
                t->token = T_ELSE;
                break;
            case T_WHILE:
                t->token = T_WHILE;
                break;
            case T_FOR:
                t->token = T_FOR;
                break;
            case T_VOID:
                t->token = T_VOID;
                break;
            case T_CHAR:
                t->token = T_CHAR;
                break;
            case T_LONG:
                t->token = T_LONG;
                break;
            case T_RETURN:
                t->token = T_RETURN;
                break;
            case T_STRUCT:
                t->token = T_STRUCT;
                break;
            case T_UNION:
                t->token = T_UNION;
                break;
            case T_TYPEDEF:
                t->token = T_TYPEDEF;
                break;
            case T_ENUM:
                t->token = T_ENUM;
                break;
            case T_EXTERN:
                t->token = T_EXTERN;
                break;
            case T_BREAK:
                t->token = T_BREAK;
                break;
            case T_CONTINUE:
                t->token = T_CONTINUE;
                break;
            case T_DEFAULT:
                t->token = T_DEFAULT;
                break;
            case T_CASE:
                t->token = T_CASE;
                break;
            case T_SWITCH:
                t->token = T_SWITCH;
                break;
            case T_SIZEOF:
                t->token = T_SIZEOF;
                break;
            case T_STATIC:
                t->token = T_STATIC;
                break;
            default:
                t->token = T_IDENT;
                break;
            }
        }
        else
            fatalc("Unrecognised character", c);

    }
    // We found a token
    t->tokstr = Tstring[t->token];
    return 1; //找到token
}

// 把c输入到buf缓冲中，lim为buf物理长度
static int scanident(int c, char* buf, int bufferlen)
{
    int i = 0;

    // 字母数字下划线
    while (isalpha(c) || isdigit(c) || '_' == c)
    {

        if (bufferlen - 1 == i)
        {
            printf("identifier too long on line %d\n", Line);
            exit(1);
        }
        else if (i < bufferlen - 1)
            buf[i++] = c;
        c = next();
    }

    putback(c); // 遇见不合法，返回
    buf[i] = '\0';// 设置最后一个为结束
    return  i;
}

static int keyword(char* s)  //获取关键字的类型值
{
    switch (*s)
    {
        /*
        case 'p':
            if (!strcmp(s, "print"))
                return  T_PRINT;
                */
    case 'i':
        if (!strcmp(s, "int"))
            return  T_INT;
        if (!strcmp(s, "if"))
            return  T_IF;
    case 'e':
        if (!strcmp(s, "else"))
            return T_ELSE;
        else if (!strcmp(s, "enum"))
            return (T_ENUM);
        else if (!strcmp(s, "extern"))
            return (T_EXTERN);
    case 'w':
        if (!strcmp(s, "while"))
            return T_WHILE;
    case 'f':
        if (!strcmp(s, "for"))
            return T_FOR;
    case 'v':
        if (!strcmp(s, "void"))
            return T_VOID;
    case 'c':
        if (!strcmp(s, "char"))
            return T_CHAR;
        else if (!strcmp(s, "continue"))
            return T_CONTINUE;
        else if (!strcmp(s, "case"))
            return T_CASE;
    case 'r':
        if (!strcmp(s, "return"))
            return T_RETURN;
    case 'l':
        if (!strcmp(s, "long"))
            return T_LONG;
    case 's':
        if (!strcmp(s, "struct"))
            return (T_STRUCT);
        else if (!strcmp(s, "switch"))
            return (T_SWITCH);
        else if (!strcmp(s, "sizeof"))
            return (T_SIZEOF);
        else if (!strcmp(s, "static"))
            return (T_STATIC);
    case 'u':
        if (!strcmp(s, "union"))
            return (T_UNION);
    case 't':
        if (!strcmp(s, "typedef"))
            return (T_TYPEDEF);
    case 'b':
        if (!strcmp(s, "break"))
            return (T_BREAK);
    case 'd':
        if (!strcmp(s, "default"))
            return (T_DEFAULT);
    }
    return 0;
}