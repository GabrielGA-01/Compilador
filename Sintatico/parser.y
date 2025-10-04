%{
#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 0

void yyerror(char *);
int yylex(void);
void abrirArq();
%}

%start entrada
%token ERRO FIMLIN 
%token ADD SUB MUL DIV
%token LT LET GET GT EQ DIF
%token ASSIGN SEMICOLON COLON
%token OPENPAR CLOSEPAR OPENCOL CLOSECOL OPENCHA CLOSECHA
%token NUM ID ERROR
%token IF ELSE INT RETURN VOID WHILE

%printer { fprintf (yyoutput, "'%d'", $$); } NUM

%%

entrada : /* entrada vazia */
  |   entrada result;

result  : FIMLIN
  | exp FIMLIN    { printf("\nResposta: %d\n", $1); }
  | error FIMLIN  { yyerrok; }
  ;

exp : exp ADD termo   {$$ = $1 + $3;}
  | exp SUB termo   {$$ = $1 - $3;}
  | termo           {$$ = $1;}
  ;

termo : termo MUL fator {$$ = $1 * $3;}
  | fator             {$$ = $1;}
  ;

fator : OPENPAR exp CLOSEPAR   {$$ = $2;}
  | NUM                        {$$ = $1;}
  ;

%%

int main()
{
  printf("\nParser em execução...\n");
  abrirArq();
  return yyparse();
}

void yyerror(char * msg)
{
  extern char* yytext;
  extern int yylval;
  printf("\n%s : %s %d\n", msg, yytext, yylval);
}
