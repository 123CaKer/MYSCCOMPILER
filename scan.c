#include "defs.h"
#include "data.h"
#include "decl.h"


// rejtoken 指针
static struct token* Rejtoken = NULL;

static int chrpos(char* s, int c)
// 返回c在s中的位置（第一个）
{
    char* p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}


static int next(void)
// 获取输入文件中的下一个字符
{
    int c;                     // 使用int类型而不是char是防止EOF失效
    if (Putback)              // 如果Putback中有的话就读取当前Putback值
    {
        c = Putback;
        Putback = 0;
        return c;
    }
    c = fgetc(Infile);		// 从输入文件中读取
    if ('\n' == c)
        Line++;			// 换行
    return c;
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

// Return the next character from a character
// or string literal
static int scanch(void)   // 主要用于识别 转义字符
{
    int c;

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
        default:
            fatalc("unknown escape sequence", c);
        }
    }
    return (c);			// Just an ordinary old character!
}



static int scanint(int c)
// 从输入文件中扫描并返回整形数据 
{
    int k, val = 0;

    // Convert each character into an int value
    while ((k = chrpos("0123456789", c)) >= 0)
    {
        val = val * 10 + k;
        c = next();
    }


    putback(c);// 不是的话就返回
    return val;
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
        else 
        {
            putback(c);
            t->token = T_PLUS;
        }
        break;
    case '-':
        if ((c = next()) == '-')
        {
            t->token = T_DEC;// 自减
        }
        else 
        {
            putback(c);
            t->token = T_MINUS;
        }
        break;
    case '*':
        t->token = T_STAR;
        break;
    case '/':
        t->token = T_SLASH;
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
            default:
                t->token = T_IDENT;
                break;
            }
        }
        else
            fatalc("Unrecognised character", c);

    }
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
    case 'r':
        if (!strcmp(s, "return"))
            return T_RETURN;
    case 'l':
        if (!strcmp(s, "long"))
            return T_LONG;
    case 's':
        if (!strcmp(s, "struct"))
            return (T_STRUCT);
    }
    return 0;
}