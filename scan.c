#include "defs.h"
#include "data.h"
#include "decl.h"


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

// 扫描token并判断类型并返回
int scan(struct token* t) 
{
    int c = 0;
    //int tokentype=2 ;

    
    c = skip();  // 忽略空格换行符等 类似getc

    switch (c)
    {
    case EOF: // 输入文件已到达末尾
        t->token = T_EOF;
        printf("输入文件已经扫描完成\n");
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
            t->intvalue = scanint(c);//// 从输入文件中扫描并返回整形数据 
            t->token = T_INTLIT;
            break;
        }
        else if (isalpha(c)||c=='_')
        {
            scanident(c, Text, TEXTLEN + 1);// 输入到缓冲区中
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

    
    return 1; //找到token
}

// 把c输入到buf缓冲中，lim为buf物理长度
static int scanident(int c, char *buf, int bufferlen) 
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
    case 'p':
        if (!strcmp(s, "print"))
            return  T_PRINT;
        break;
    }
    return 0;
}