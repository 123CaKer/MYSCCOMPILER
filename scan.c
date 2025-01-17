#include "defs.h"
#include "data.h"
#include "decl.h"


// rejtoken ָ��
static struct token* Rejtoken = NULL;

static int chrpos(char* s, int c)
// ����c��s�е�λ�ã���һ����
{
    char* p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}


 int next(void)
// ��ȡ�����ļ��е���һ���ַ�
{
    int c, //��ȡ���ַ�
        l; // �к�

    if (Putback) 
    {			//�ַ�����
        c = Putback;			// ��Ҫ���ص�
        Putback = 0;
        return (c);
    }

    c = fgetc(Infile);			// Read from input file

    while (c == '#')
    {			// We've hit a pre-processor statement
        scan(&Token);			// ��ȡ�к�
        if (Token.token != T_INTLIT)
            fatals("Expecting pre-processor line number, got:", Text);
        l = Token.intvalue;

        scan(&Token);			// Get the filename in Text ��ȡ�ļ�����
        if (Token.token != T_STRLIT)
            fatals("Expecting pre-processor file name, got:", Text);

        if (Text[0] != '<')// ˵����ǰΪ�������ļ�����
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



static void putback(int c)  // ������Ҫ��ֵ�Ż�Putback�� 
{
    Putback = c;
}


static int skip(void)
//���ϵĻ�ȡ�ַ� �����Һ��Ե��Ǳ�Ҫ�Ķ��� Ʃ��whileѭ������ʾ  �� a c = 0; ���ʽ�пո����
{
    int c;

    c = next();
    while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c)
        c = next();
    return c;
}

// Return the next character from a character
// or string literal
static int scanch(void)   // ��Ҫ����ʶ�� ת���ַ�
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
            return '\a'; //������
        case 'b':
            return '\b'; // �˸�� ɾ����ǰ�ַ�
        case 'f':
            return '\f'; // 
        case 'n':
            return '\n';
        case 'r':
            return '\r'; // �س��� ������ƶ�����ǰ�еĿ�ͷ
        case 't':
            return '\t';
        case 'v':
            return '\v'; // ��ֱ�Ʊ��
        case '\\':
            return '\\'; // /
        case '"':
            return '"';// ˫����
        case '\'':
            return '\'';
        default:
            fatalc("unknown escape sequence", c);
        }
    }
    return (c);			// Just an ordinary old character!
}



static int scanint(int c)
// �������ļ���ɨ�貢������������ 
{
    int k, val = 0;

    // Convert each character into an int value
    while ((k = chrpos("0123456789", c)) >= 0)
    {
        val = val * 10 + k;
        c = next();
    }


    putback(c);// ���ǵĻ��ͷ���
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



// ɨ��token���ж����Ͳ�����
int scan(struct token* t)
{
    int c = 0;
    int tokentype;



    // �鿴�Ƿ���֮ǰ�ܾ���token ���еĻ���ȫ��token ����ΪRejtoken������
    if (Rejtoken != NULL)
    {
        t = Rejtoken;
        Rejtoken = NULL;
        return 1;
    }

    c = skip();  // ���Կո��з��� ����getc

    switch (c)
    {
    case EOF: // �����ļ��ѵ���ĩβ
        t->token = T_EOF;
        printf("�����ļ��Ѿ�ɨ�����\n");
        return 0;
    case '+':
        if ((c = next()) == '+') 
        {
            t->token = T_INC;  // ����
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
            t->token = T_DEC;// �Լ�
        }
        else if (c == '>')
        {
            t->token = T_ARROW;// ->
        }
        else 
        {
            putback(c);
            t->token = T_MINUS;
        }
        break;
    case '.':
        t->token = T_DOT;// .��Ա
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
        t->token = T_COMMA; // �� ,  , ��
        break;
        
    case'=':
        if ((c = next()) == '=') // ���õ��ĵ�ǰΪ�� ��Ϊ���ջ��� putback���Ȼ�ȡ
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
    case '!':  // ������
        if ((c = next()) == '=')
        {
            t->token = T_NE;
            break;
        }
        else
        {
            putback(c);
            t->token = T_LOGNOT;  // �߼���
            break;
            
        }
    case '>':
        if ((c = next()) == '=')
        {
            t->token = T_GE;
        }
        else if (c == '>')
        {
            t->token = T_RSHIFT; // ����
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
            t->token = T_LSHIFT; // ����
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
            putback(c);//����
            t->token = T_AMPER; //&p
        }
        break;

    case '|':
        if ((c = next()) == '|') 
        {
            t->token = T_LOGOR; // ��
        }
        else 
        {
            putback(c);
            t->token = T_OR; // ��λ��
        }
        break;

    case ':':
        t->token = T_COLON;
        break;
    case '~': // ��λȡ��
        t->token = T_INVERT;
        break;
    case '^': //  ���
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
            t->intvalue = scanint(c);//// �������ļ���ɨ�貢������������ 
            t->token = T_INTLIT;
            break;
        }
        else if (isalpha(c) || '_' == c)
        {
            scanident(c, Text, TEXTLEN + 1);// ���뵽��������
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
            default:
                t->token = T_IDENT;
                break;
            }
        }
        else
            fatalc("Unrecognised character", c);

    }
    return 1; //�ҵ�token
}

// ��c���뵽buf�����У�limΪbuf������
static int scanident(int c, char* buf, int bufferlen)
{
    int i = 0;

    // ��ĸ�����»���
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

    putback(c); // �������Ϸ�������
    buf[i] = '\0';// �������һ��Ϊ����
    return  i;
}

static int keyword(char* s)  //��ȡ�ؼ��ֵ�����ֵ
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