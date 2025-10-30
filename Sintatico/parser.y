%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

ASTNode *root = NULL;

void yyerror(const char *);
int yylex(void);
void abrirArq();

// Declarar função append_node
ASTNode* append_node(ASTNode* list, ASTNode* new_node);
%}

%union {
    int number;
    char* string;
    struct ASTNode *node;
}

%start programa
%left ADD SUB
%left MUL DIV
%nonassoc IFX
%nonassoc ELSE
%define parse.error verbose

%token ERRO
%token ADD SUB MUL DIV
%token LT LET GET GT EQ DIF
%token ASSIGN SEMICOLON COMMA
%token OPENPAR CLOSEPAR OPENCOL CLOSECOL OPENCHA CLOSECHA
%token IF ELSE INT RETURN VOID WHILE

%token <number> NUM
%token <string> ID

%type <node> programa declaracao_lista declaracao var_declaracao fun_declaracao
%type <node> composto_decl statement_lista statement expressao_decl selecao_decl
%type <node> iteracao_decl retorno_decl expressao simples_expressao soma_expressao
%type <node> termo fator var ativacao tipo_especificador params param_lista param args arg_lista relacional local_declaracoes

%%

/* 1. Regra inicial */
programa: declaracao_lista
        {
            root = $1;
            printf("Árvore sintática construída com sucesso.\n");
        }
        ;

/* 2. Lista de declarações - USANDO append_node */
declaracao_lista: declaracao_lista declaracao
                {
                    $$ = append_node($1, $2);
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

/* 4. Declaração de variável - CORRIGIDA para arrays */
var_declaracao: tipo_especificador ID SEMICOLON
              {
                  $$ = create_node(NODE_VAR_DECL, $1, create_leaf_id($2));
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  // CORREÇÃO: Usar NODE_ARRAY_DECL para arrays
                  ASTNode* array_decl = create_node(NODE_ARRAY_DECL, create_leaf_id($2), create_leaf_num($4));
                  $$ = create_node(NODE_VAR_DECL, $1, array_decl);
              }
              ;

/* 5. Especificador de tipo */
tipo_especificador: INT
                  { $$ = create_leaf_num(INT); }
                  | VOID
                  { $$ = create_leaf_num(VOID); }
                  ;

/* 6. Declaração de função - CORRIGIDA para parâmetros */
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  // CORREÇÃO: Estrutura melhorada para função
                  ASTNode* func_node = create_node(NODE_FUN_DECL, $1, create_leaf_id($2));
                  // Os parâmetros são armazenados como próximo do nó da função
                  func_node->next = create_node(NODE_PARAM_LIST, $4, $6);
                  $$ = func_node;
              }
              ;

/* 7. Parâmetros de uma função - CORRIGIDA */
params: param_lista
      { $$ = $1; }
      | VOID
      { $$ = NULL; }
      | 
      { $$ = NULL; }
      ;

/* 8. Lista de parâmetros - CORRIGIDA usando append_node */
param_lista: param_lista COMMA param
           {
               $$ = append_node($1, $3);
           }
           | param
           {
               $$ = $1;
           }
           ;

/* 9. Um único parâmetro - CORRIGIDA */
param: tipo_especificador ID
     {
         // CORREÇÃO: Criar nó PARAM para parâmetros simples
         $$ = create_node(NODE_PARAM, $1, create_leaf_id($2));
     }
     | tipo_especificador ID OPENCOL CLOSECOL
     {
         // CORREÇÃO: Criar nó PARAM para arrays
         ASTNode* array_param = create_node(NODE_ARRAY_DECL, create_leaf_id($2), NULL);
         $$ = create_node(NODE_PARAM, $1, array_param);
     }
     ;

/* 10. Bloco de código */
composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             {
                 $$ = create_node(NODE_COMPOUND_STMT, $2, $3);
             }
             ;

/* 11. Declarações locais - CORRIGIDA usando append_node */
local_declaracoes: local_declaracoes var_declaracao
                 {
                     $$ = append_node($1, $2);
                 }
                 | 
                 { $$ = NULL; }
                 ;

/* 12. Lista de comandos - CORRIGIDA usando append_node */
statement_lista: statement_lista statement
               {
                   $$ = append_node($1, $2);
               }
               | 
               { $$ = NULL; }
               ;

/* 13. Um único comando/instrução */
statement: expressao_decl
         | composto_decl
         | selecao_decl
         | iteracao_decl
         | retorno_decl
         ;

/* 14. Comando de expressão */
expressao_decl: expressao SEMICOLON
              { $$ = $1; }
              | SEMICOLON
              { $$ = NULL; }
              ;

/* 15. Comando de seleção - CORRIGIDA para else */
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                $$ = create_node(NODE_IF_STMT, $3, $5);
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                // CORREÇÃO: Estrutura melhorada para if-else
                ASTNode* if_node = create_node(NODE_IF_STMT, $3, $5);
                if_node->next = $7; // else como próximo nó
                $$ = if_node;
            }
            ;

/* 16. Comando de iteração */
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

/* 18. Expressão */
expressao: var ASSIGN expressao
         {
             $$ = create_node(NODE_ASSIGN_EXPR, $1, $3);
         }
         | simples_expressao
         {
             $$ = $1;
         }
         ;

/* 19. Variável - CORRIGIDA para acesso a arrays */
var: ID
   {
       $$ = create_leaf_id($1);
   }
   | ID OPENCOL expressao CLOSECOL
   {
       // CORREÇÃO: Usar NODE_ARRAY_ACCESS para acesso a arrays
       $$ = create_node(NODE_ARRAY_ACCESS, create_leaf_id($1), $3);
   }
   ;

/* 20. Expressão simples */
simples_expressao: soma_expressao relacional soma_expressao
                 {
                     $2->leftChild = $1;
                     $2->rightChild = $3;
                     $$ = $2;
                 }
                 | soma_expressao
                 { $$ = $1; }
                 ;

/* 21. Operador relacional - CORRIGIDA */
relacional: LET { $$ = create_leaf_num(LET); }
          | LT  { $$ = create_leaf_num(LT); }
          | GT  { $$ = create_leaf_num(GT); }
          | GET { $$ = create_leaf_num(GET); }
          | EQ  { $$ = create_leaf_num(EQ); }
          | DIF { $$ = create_leaf_num(DIF); }
          ;

/* 22. Expressão de soma/subtração */
soma_expressao: soma_expressao ADD termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->data.number = ADD;
              }
              | soma_expressao SUB termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->data.number = SUB;
              }
              | termo
              {
                  $$ = $1;
              }
              ;

/* 23. Termo */
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

/* 24. Fator */
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

/* 25. Ativação (chamada de função) */
ativacao: ID OPENPAR args CLOSEPAR
        {
            $$ = create_node(NODE_FUN_CALL, create_leaf_id($1), $3);
        }
        ;

/* 26. Argumentos de uma chamada de função */
args: arg_lista
    { $$ = $1; }
    | 
    { $$ = NULL; }
    ;

/* 27. Lista de argumentos - CORRIGIDA usando append_node */
arg_lista: arg_lista COMMA expressao
         {
             $$ = append_node($1, $3);
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
