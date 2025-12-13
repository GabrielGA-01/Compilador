%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>

extern int yylineno;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

%}

digito        [0-9]
letra         [a-zA-Z]
numero        {digito}+
identificador {letra}({letra}|{digito})*
whitespace    [ \t]+
quebralinha   [\r\n]+

%%

"/*"          { char c1 = 0;
                char c2 = 0;
                do
                { c1 = c2;
                  c2 = input();
                  if (c2 == EOF) break;
                  if (c2 == '\n') yylineno++;
                } while (c1 != '*' || c2 != '/');
              }

"+"           { return ADD; }
"-"           { return SUB; }
"*"           { return MUL; }
"/"           { return DIV; }
"<="          { return LET; }
"<"           { return LT;  }
">="          { return GET; }
">"           { return GT;  }
"=="          { return EQ;  }
"!="          { return DIF; }
"="           { return ASSIGN; }
";"           { return SEMICOLON; }
","           { return COMMA; }
"("           { return OPENPAR; }
")"           { return CLOSEPAR; }
"["           { return OPENCOL; }
"]"           { return CLOSECOL; }
"{"           { return OPENCHA; }
"}"           { return CLOSECHA; }

"if"          { return IF; }
"else"        { return ELSE; }
"int"         { return INT; }
"return"      { return RETURN; }
"void"        { return VOID; }
"while"       { return WHILE; }
    
{numero}      { yylval = create_leaf_num(atoi(yytext)); return NUM; }
{identificador} { yylval = create_leaf_id(strdup(yytext)); return ID; }
{whitespace}  { }
\n            { yylineno++; }
.             { printf("[Lexical Error] Line %d: Unrecognized character '%s'\n", yylineno, yytext); return ERRO; }

%%