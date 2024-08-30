#include "defs.h"
#include "data.h"
#include "decl.h"

// Æ¥Åä¹Ø¼ü×Ö
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

// Æ¥Åä·ÖºÅ
void semi(void)
{
    match(T_SEMI, ";");
}