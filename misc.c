#include "defs.h"
#include "data.h"
#include "decl.h"
 
// 匹配关键字 在匹配的时候会获取下一个字符并进行全局token赋值
void match(int t, char* what) 
{
    if (Token.token == t) 
        scan(&Token);
    else
    {
        printf("%s expected on line %d\n", what, Line);
        exit(1);
    }
}
// 匹配分号
void semi(void)
{
    match(T_SEMI, ";");
}

// 匹配关键字为标识符
void ident(void)
{
    match(T_IDENT, "identifier"); 
}




void lbrace(void) {
    match(T_LBRACE, "{");
}

void rbrace(void) {
    match(T_RBRACE, "}");
}

void lparen(void) {
    match(T_LPAREN, "(");
}

// Match a right parenthesis and fetch the next token
void rparen(void) {
    match(T_RPAREN, ")");
}


// 错误信息
void fatal(char* s)
{
    fprintf(stderr, "%s on line %d\n", s, Line); 
    exit(1);
}

void fatals(char* s1, char* s2)
{
    fprintf(stderr, "%s:%s on line %d\n", s1, s2, Line); 
    exit(1);
}

void fatald(char* s, int d)
{
    fprintf(stderr, "%s:%d on line %d\n", s, d, Line);
    exit(1);
}

void fatalc(char* s, int c)
{
    fprintf(stderr, "%s:%c on line %d\n", s, c, Line); 
    exit(1);
}