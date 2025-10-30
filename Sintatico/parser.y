%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define YYDEBUG 1

ASTNode *root = NULL; // Variável global para a raiz da AST

void yyerror(const char *);
int yylex(void);
void abrirArq();
%}

%union {
    int number;
    char* string;
    struct ASTNode *node; // Ponteiro para um nó da AST
}

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
%token IF ELSE INT RETURN VOID WHILE

%token <number> NUM
%token <string> ID

// Associando não-terminais que representarão sub-árvores a um ponteiro de nó
%type <node> programa declaracao_lista declaracao var_declaracao fun_declaracao
%type <node> composto_decl statement_lista statement expressao_decl selecao_decl
%type <node> iteracao_decl retorno_decl expressao simples_expressao soma_expressao
%type <node> termo fator var ativacao tipo_especificador params param_lista param args arg_lista relacional local_declaracoes

%printer { fprintf (yyoutput, "'%d'", $$); } NUM

%%

/* 1. Regra inicial */
programa: declaracao_lista
        {
            root = $1; // Salva a árvore completa na variável global
            printf("Árvore sintática construída com sucesso.\n");
        }
        ;

/* 2. Lista de declarações (variáveis ou funções) */
declaracao_lista: declaracao_lista declaracao
                {
                    // Adiciona a nova declaração ao final da lista
                    ASTNode* current = $1;
                    while (current->next != NULL) {
                        current = current->next;
                    }
                    current->next = $2;
                    $$ = $1;
                }
                | declaracao
                {
                    $$ = $1;
                }
                ;

/* 3. Uma única declaração */
declaracao: var_declaracao
          | fun_declaracao
          ;

/* 4. Declaração de variável (simples ou array) */
var_declaracao: tipo_especificador ID SEMICOLON
              {
                  $$ = create_node(NODE_VAR_DECL, $1, create_leaf_id($2));
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  $$ = create_node(NODE_VAR_DECL, $1, create_node(NODE_VAR, create_leaf_id($2), create_leaf_num($4)));
              }
              ;

/* 5. Especificador de tipo */
tipo_especificador: INT
                  { $$ = create_leaf_num(INT); }
                  | VOID
                  { $$ = create_leaf_num(VOID); }
                  ;

/* 6. Declaração de função */
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  $$ = create_node(NODE_FUN_DECL, $1, create_node(NODE_FUN_CALL, create_leaf_id($2), $4));
                  $$->rightChild->next = $6;
              }
              ;

/* 7. Parâmetros de uma função */
params: param_lista
      { $$ = $1; }
      | VOID /* Cuidado: aqui VOID é usado como tipo e como ausência de params. O scanner deve diferenciá-los se necessário. */
      { $$ = NULL; }
      | /* vazio para funções sem parâmetros, ex: int main() */
      { $$ = NULL; }
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
             {
                 $$ = create_node(NODE_COMPOUND_STMT, $2, $3);
             }
             ;

/* 11. Declarações locais dentro de um bloco */
local_declaracoes: local_declaracoes var_declaracao
                 {
                    ASTNode* current = $1;
                    if (current == NULL) {
                        $$ = $2;
                    } else {
                        while (current->next != NULL) {
                            current = current->next;
                        }
                        current->next = $2;
                        $$ = $1;
                    }
                 }
                 | /* vazio */
                 { $$ = NULL; }
                 ;

/* 12. Lista de comandos/instruções */
statement_lista: statement_lista statement
               {
                    ASTNode* current = $1;
                    if (current == NULL) {
                        $$ = $2;
                    } else {
                        while (current->next != NULL) {
                            current = current->next;
                        }
                        current->next = $2;
                        $$ = $1;
                    }
               }
               | /* vazio */
               { $$ = NULL; }
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
              { $$ = $1; }
              | SEMICOLON /* Comando vazio */
              { $$ = NULL; }
              ;

/* 15. Comando de seleção (if/else) */
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                $$ = create_node(NODE_IF_STMT, $3, $5);
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                $$ = create_node(NODE_IF_STMT, $3, create_node(NODE_IF_STMT, $5, $7));
            }
            ;

/* 16. Comando de iteração (while) */
iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             {
                 $$ = create_node(NODE_WHILE_STMT, $3, $5);
             }
             ;

/* 17. Comando de retorno */
retorno_decl: RETURN SEMICOLON
            {
                $$ = create_node(NODE_RETURN_STMT, NULL, NULL);
            }
            | RETURN expressao SEMICOLON
            {
                $$ = create_node(NODE_RETURN_STMT, $2, NULL);
            }
            ;

/* 18. Expressão (atribuição ou expressão simples) */
expressao: var ASSIGN expressao
         {
             $$ = create_node(NODE_ASSIGN_EXPR, $1, $3);
         }
         | simples_expressao
         {
             $$ = $1;
         }
         ;

/* 19. Variável (identificador ou acesso a array) */
var: ID
   {
       $$ = create_leaf_id($1);
   }
   | ID OPENCOL expressao CLOSECOL
   {
       $$ = create_node(NODE_VAR, create_leaf_id($1), $3);
   }
   ;

/* 20. Expressão simples (com ou sem operador relacional) */
simples_expressao: soma_expressao relacional soma_expressao
                 {
                     $2->leftChild = $1;
                     $2->rightChild = $3;
                     $$ = $2;
                 }
                 | soma_expressao
                 { $$ = $1; }
                 ;

/* 21. Operador relacional */
relacional: LET { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = LET; }
          | LT  { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = LT; }
          | GT  { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = GT; }
          | GET { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = GET; }
          | EQ  { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = EQ; }
          | DIF { $$ = create_node(NODE_BINARY_OP, NULL, NULL); $$->data.number = DIF; }
          ;

/* 22. Expressão de soma/subtração */
soma_expressao: soma_expressao ADD termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->data.number = ADD; // Store operator type
              }
              | soma_expressao SUB termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->data.number = SUB; // Store operator type
              }
              | termo
              {
                  $$ = $1;
              }
              ;

/* 23. Operador de soma (regra foi absorvida na 22 para precedência) */
/* soma: ADD | SUB; */

/* 24. Termo (expressão de multiplicação/divisão) */
termo: termo MUL fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->data.number = MUL;
     }
     | termo DIV fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->data.number = DIV;
     }
     | fator
     {
         $$ = $1;
     }
     ;

/* 25. Operador de multiplicação (regra foi absorvida na 24 para precedência) */
/* mult: MUL | DIV; */

/* 26. Fator (unidade básica de uma expressão) */
fator: OPENPAR expressao CLOSEPAR
     {
         $$ = $2;
     }
     | var
     {
         $$ = $1;
     }
     | ativacao
     {
         $$ = $1;
     }
     | NUM
     {
         $$ = create_leaf_num($1);
     }
     ;

/* 27. Ativação (chamada de função) */
ativacao: ID OPENPAR args CLOSEPAR
        {
            $$ = create_node(NODE_FUN_CALL, create_leaf_id($1), $3);
        }
        ;

/* 28. Argumentos de uma chamada de função */
args: arg_lista
    { $$ = $1; }
    | /* vazio */
    { $$ = NULL; }
    ;

/* 29. Lista de argumentos */
arg_lista: arg_lista COMMA expressao
         {
            ASTNode* current = $1;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = $3;
            $$ = $1;
         }
         | expressao
         { $$ = $1; }
         ;

%%

int main()
{
  printf("\nParser em execução...\n");
  abrirArq();
  int aux = yyparse();
  if(!aux) {
    printf("Sucesso demais\n");
    printf("\n--- Árvore Sintática Abstrata ---\n");
    print_ast(root, 0);
    printf("---------------------------------\n");
  }
  return aux;
}

void yyerror(const char *msg)
{
    extern int yylineno; 
    extern char *yytext; 

    fprintf(stderr, "Erro na linha %d no lexema '%s': %s\n", yylineno, yytext, msg);
}
