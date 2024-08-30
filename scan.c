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
            if (keyword(Text)== T_PRINT)
            {
                t->token = T_PRINT;
                break;
            }
            else
            {
                printf("Unrecognised symbol <%s> on line <%d>\n", Text, Line);
                exit(1);
            }
          
        }

        
       
        printf("Unexpected token <%c> on line <%d> \n", c, Line);
        exit(1);
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
        break;
    }
    return 0;
}