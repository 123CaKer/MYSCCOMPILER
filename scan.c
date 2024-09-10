#include "defs.h"
#include "data.h"
#include "decl.h"


static int chrpos(char* s, int c)
// ����c��s�е�λ�ã���һ����
{
    char* p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}


static int next(void) 
// ��ȡ�����ļ��е���һ���ַ�
{
    int c;                     // ʹ��int���Ͷ�����char�Ƿ�ֹEOFʧЧ
    if (Putback)              // ���Putback���еĻ��Ͷ�ȡ��ǰPutbackֵ
    {		                 
        c = Putback;		
        Putback = 0;
        return c;
    }
    c = fgetc(Infile);		// �������ļ��ж�ȡ
    if ('\n' == c)
        Line++;			// ����
    return c;
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

// ɨ��token���ж����Ͳ�����
int scan(struct token* t) 
{
    int c = 0;
    //int tokentype=2 ;

    
    c = skip();  // ���Կո��з��� ����getc

    switch (c)
    {
    case EOF: // �����ļ��ѵ���ĩβ
        t->token = T_EOF;
        printf("�����ļ��Ѿ�ɨ�����\n");
        return 0;
    case '+':
        t->token = T_PLUS;
        break;
    case '-':
        t->token = T_MINUS;
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
             
             // logic  ???


            putback(c);
            fatalc("Unrecognised character", c);
        }
    case '>':
        if ((c = next()) == '=')
        {
            t->token = T_GE;
            break;
        }
        else
        {
            t->token = T_GT;
            break;
        }
    case '<':
        if ((c = next()) == '=')
        {
            t->token = T_LE;
            break;
        }
        else
        {
            t->token = T_LT;
            break;
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


    default:
        if (isdigit(c))
        {
            t->intvalue = scanint(c);//// �������ļ���ɨ�貢������������ 
            t->token = T_INTLIT;
            break;
        }
        else if (isalpha(c)||c=='_')
        {
            scanident(c, Text, TEXTLEN + 1);// ���뵽��������
            int judge= keyword(Text);
            switch (judge)
            {
            case T_PRINT:
                t->token = T_PRINT;
                break;
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
static int scanident(int c, char *buf, int bufferlen) 
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
    case 'p':
        if (!strcmp(s, "print"))
            return  T_PRINT;
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
    }
    return 0;
}