%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

ASTNode *root = NULL;
void yyerror(const char *);
int yylex(void);
void abrirArq();
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

/* 1. Regra inicial - mantém igual */
programa: declaracao_lista
        {
            root = $1;
            printf("Árvore sintática construída com sucesso.\n");
        }
        ;

/* 2. Lista de declarações - mantém igual */
declaracao_lista: declaracao_lista declaracao
                { $$ = append_node($1, $2); }
                | declaracao
                { $$ = $1; }
                ;

/* 3. Uma única declaração - mantém igual */
declaracao: var_declaracao
          | fun_declaracao
          ;

/* 4. Declaração de variável - CORRIGIDA para usar create_leaf_type */
var_declaracao: tipo_especificador ID SEMICOLON
              {
                  $$ = create_node(NODE_VAR_DECL, $1, create_leaf_id($2));
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  ASTNode* array_decl = create_node(NODE_ARRAY_DECL, create_leaf_id($2), create_leaf_num($4));
                  $$ = create_node(NODE_VAR_DECL, $1, array_decl);
              }
              ;

/* 5. Especificador de tipo - CORRIGIDA para usar create_leaf_type */
tipo_especificador: INT
                  { $$ = create_leaf_type(INT); }  // CORREÇÃO
                  | VOID
                  { $$ = create_leaf_type(VOID); } // CORREÇÃO
                  ;

/* 6. Declaração de função - COMPLETAMENTE REFEITA */
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  // Nova estrutura: FUN_DECL -> tipo, nome, FUN_BODY -> params, corpo
                  ASTNode* func_node = create_node(NODE_FUN_DECL, $1, create_leaf_id($2));
                  ASTNode* body_node = create_node(NODE_FUN_BODY, $4, $6);
                  func_node->next = body_node;
                  $$ = func_node;
              }
              ;

/* 7. Parâmetros - mantém igual */
params: param_lista
      { $$ = $1; }
      | VOID
      { $$ = NULL; }
      | 
      { $$ = NULL; }
      ;

/* 8. Lista de parâmetros - mantém igual */
param_lista: param_lista COMMA param
           { $$ = append_node($1, $3); }
           | param
           { $$ = $1; }
           ;

/* 9. Um único parâmetro - CORRIGIDA para usar create_leaf_type */
param: tipo_especificador ID
     {
         $$ = create_node(NODE_PARAM, $1, create_leaf_id($2));
     }
     | tipo_especificador ID OPENCOL CLOSECOL
     {
         ASTNode* array_param = create_node(NODE_ARRAY_DECL, create_leaf_id($2), NULL);
         $$ = create_node(NODE_PARAM, $1, array_param);
     }
     ;

/* 10. Bloco de código - mantém igual */
composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             { $$ = create_node(NODE_COMPOUND_STMT, $2, $3); }
             ;

/* 11. Declarações locais - mantém igual */
local_declaracoes: local_declaracoes var_declaracao
                 { $$ = append_node($1, $2); }
                 | 
                 { $$ = NULL; }
                 ;

/* 12. Lista de comandos - mantém igual */
statement_lista: statement_lista statement
               { $$ = append_node($1, $2); }
               | 
               { $$ = NULL; }
               ;

/* 13. Um único comando - mantém igual */
statement: expressao_decl
         | composto_decl
         | selecao_decl
         | iteracao_decl
         | retorno_decl
         ;

/* 14. Comando de expressão - mantém igual */
expressao_decl: expressao SEMICOLON
              { $$ = $1; }
              | SEMICOLON
              { $$ = NULL; }
              ;

/* 15. Comando de seleção - CORRIGIDA para estrutura adequada */
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                $$ = create_node(NODE_IF_STMT, $3, $5);
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                // CORREÇÃO: Criar nó if com else como próximo
                ASTNode* if_node = create_node(NODE_IF_STMT, $3, $5);
                if_node->next = $7;
                $$ = if_node;
            }
            ;

/* 16. Comando de iteração - mantém igual */
iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             { $$ = create_node(NODE_WHILE_STMT, $3, $5); }
             ;

/* 17. Comando de retorno - mantém igual */
retorno_decl: RETURN SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, NULL, NULL); }
            | RETURN expressao SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, $2, NULL); }
            ;

/* 18. Expressão - mantém igual */
expressao: var ASSIGN expressao
         { $$ = create_node(NODE_ASSIGN_EXPR, $1, $3); }
         | simples_expressao
         { $$ = $1; }
         ;

/* 19. Variável - mantém igual */
var: ID
   { $$ = create_leaf_id($1); }
   | ID OPENCOL expressao CLOSECOL
   { $$ = create_node(NODE_ARRAY_ACCESS, create_leaf_id($1), $3); }
   ;

/* 20. Expressão simples - CORRIGIDA para usar create_leaf_operator */
simples_expressao: soma_expressao relacional soma_expressao
                 {
                     // CORREÇÃO: Usar NODE_OPERATOR em vez de NUM para operadores
                     ASTNode* op_node = create_leaf_operator($2->data.number);
                     op_node->leftChild = $1;
                     op_node->rightChild = $3;
                     $$ = op_node;
                 }
                 | soma_expressao
                 { $$ = $1; }
                 ;

/* 21. Operador relacional - CORRIGIDA para usar create_leaf_operator */
relacional: LET { $$ = create_leaf_operator(LET); }
          | LT  { $$ = create_leaf_operator(LT); }
          | GT  { $$ = create_leaf_operator(GT); }
          | GET { $$ = create_leaf_operator(GET); }
          | EQ  { $$ = create_leaf_operator(EQ); }
          | DIF { $$ = create_leaf_operator(DIF); }
          ;

/* 22. Expressão de soma/subtração - mantém igual */
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
              { $$ = $1; }
              ;

/* 23. Termo - mantém igual */
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
     { $$ = $1; }
     ;

/* 24. Fator - mantém igual */
fator: OPENPAR expressao CLOSEPAR
     { $$ = $2; }
     | var
     { $$ = $1; }
     | ativacao
     { $$ = $1; }
     | NUM
     { $$ = create_leaf_num($1); }
     ;

/* 25. Ativação - mantém igual */
ativacao: ID OPENPAR args CLOSEPAR
        { $$ = create_node(NODE_FUN_CALL, create_leaf_id($1), $3); }
        ;

/* 26. Argumentos - mantém igual */
args: arg_lista
    { $$ = $1; }
    | 
    { $$ = NULL; }
    ;

/* 27. Lista de argumentos - mantém igual */
arg_lista: arg_lista COMMA expressao
         { $$ = append_node($1, $3); }
         | expressao
         { $$ = $1; }
         ;

%%

// main e yyerror mantêm iguais
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