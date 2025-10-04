%{
#include "parser.tab.h"
void abrirArq();
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
","           { return COLON; }
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
    
{numero}      { yylval=atoi(yytext); return NUM; }
{identificador} { return ID;  }
{whitespace}  { }
{quebralinha} { ECHO; }
.             { return ERRO; }

%%

void abrirArq()
{
  yyin = fopen("entrada.txt", "r");
  if (!yyin) {
    fprintf(stderr, "Erro ao abrir entrada.txt\n");
    exit(1);
  }
}
