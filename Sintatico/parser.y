%{
#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 1

void yyerror(const char *);
int yylex(void);
void abrirArq();
%}

// Símbolo não terminal inicial
%start programa

// Para resolver ambiguidades
%left ADD SUB
%left MUL DIV

%nonassoc IFX
%nonassoc ELSE
%define parse.error verbose

// Tokens utilizados
%token ERRO
%token ADD SUB MUL DIV
%token LT LET GET GT EQ DIF
%token ASSIGN SEMICOLON COMMA
%token OPENPAR CLOSEPAR OPENCOL CLOSECOL OPENCHA CLOSECHA
%token NUM ID
%token IF ELSE INT RETURN VOID WHILE

%printer { fprintf (yyoutput, "'%d'", $$); } NUM

%%

/* 1. Regra inicial */
programa: declaracao_lista
        ;

/* 2. Lista de declarações (variáveis ou funções) */
declaracao_lista: declaracao_lista declaracao
                | declaracao
                ;

/* 3. Uma única declaração */
declaracao: var_declaracao
          | fun_declaracao
          ;

/* 4. Declaração de variável (simples ou array) */
var_declaracao: tipo_especificador ID SEMICOLON
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              ;

/* 5. Especificador de tipo */
tipo_especificador: INT
                  | VOID
                  ;

/* 6. Declaração de função */
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              ;

/* 7. Parâmetros de uma função */
params: param_lista
      | VOID /* Cuidado: aqui VOID é usado como tipo e como ausência de params. O scanner deve diferenciá-los se necessário. */
      | /* vazio para funções sem parâmetros, ex: int main() */
      ;

/* 8. Lista de parâmetros */
param_lista: param_lista COMMA param
           | param
           ;

/* 9. Um único parâmetro */
param: tipo_especificador ID
     | tipo_especificador ID OPENCOL CLOSECOL
     ;

/* 10. Bloco de código (corpo de função, if, while, etc.) */
composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             ;

/* 11. Declarações locais dentro de um bloco */
local_declaracoes: local_declaracoes var_declaracao
                 | /* vazio */
                 ;

/* 12. Lista de comandos/instruções */
statement_lista: statement_lista statement
               | /* vazio */
               ;

/* 13. Um único comando/instrução */
statement: expressao_decl
         | composto_decl
         | selecao_decl
         | iteracao_decl
         | retorno_decl
         ;

/* 14. Comando de expressão (ex: x=1; ou func();) */
expressao_decl: expressao SEMICOLON
              | SEMICOLON /* Comando vazio */
              ;

/* 15. Comando de seleção (if/else) */
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            ;

/* 16. Comando de iteração (while) */
iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             ;

/* 17. Comando de retorno */
retorno_decl: RETURN SEMICOLON
            | RETURN expressao SEMICOLON
            ;

/* 18. Expressão (atribuição ou expressão simples) */
expressao: var ASSIGN expressao
         | simples_expressao
         ;

/* 19. Variável (identificador ou acesso a array) */
var: ID
   | ID OPENCOL expressao CLOSECOL
   ;

/* 20. Expressão simples (com ou sem operador relacional) */
simples_expressao: soma_expressao relacional soma_expressao
                 | soma_expressao
                 ;

/* 21. Operador relacional */
relacional: LET | LT | GT | GET | EQ | DIF
          ;

/* 22. Expressão de soma/subtração */
soma_expressao: soma_expressao ADD termo
              | soma_expressao SUB termo
              | termo
              ;

/* 23. Operador de soma (regra foi absorvida na 22 para precedência) */
/* soma: ADD | SUB; */

/* 24. Termo (expressão de multiplicação/divisão) */
termo: termo MUL fator
     | termo DIV fator
     | fator
     ;

/* 25. Operador de multiplicação (regra foi absorvida na 24 para precedência) */
/* mult: MUL | DIV; */

/* 26. Fator (unidade básica de uma expressão) */
fator: OPENPAR expressao CLOSEPAR
     | var
     | ativacao
     | NUM
     ;

/* 27. Ativação (chamada de função) */
ativacao: ID OPENPAR args CLOSEPAR
        ;

/* 28. Argumentos de uma chamada de função */
args: arg_lista
    | /* vazio */
    ;

/* 29. Lista de argumentos */
arg_lista: arg_lista COMMA expressao
         | expressao
         ;

%%

int main()
{
  printf("\nParser em execução...\n");
  abrirArq();
  int aux = yyparse();
  if(!aux) printf("Sucesso demais\n");
  return aux;
}

void yyerror(const char *msg)
{
    extern int yylineno; 
    extern char *yytext; 

    fprintf(stderr, "Erro na linha %d no lexema '%s': %s\n", yylineno, yytext, msg);
}