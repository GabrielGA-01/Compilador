%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "analyze.h"

ASTNode *root = NULL;
void yyerror(const char *);
int yylex(void);
void abrirArq();
ASTNode* append_node(ASTNode* list, ASTNode* new_node);

/* Define YYSTYPE to be ASTNode pointer globally for the C file */
#define YYSTYPE struct ASTNode*

extern char *yytext;
%}

%code requires {
    /* Forward declaration to avoid include loop */
    struct ASTNode;
    #define YYSTYPE struct ASTNode*
}

%start programa
%left ADD SUB
%left MUL DIV
%nonassoc IFX
%nonassoc ELSE
%define parse.error verbose
%locations

%token ERRO
%token ADD SUB MUL DIV
%token LT LET GET GT EQ DIF
%token ASSIGN SEMICOLON COMMA
%token OPENPAR CLOSEPAR OPENCOL CLOSECOL OPENCHA CLOSECHA
%token IF ELSE INT RETURN VOID WHILE

%token NUM
%token ID

/* No types needed as everything is YYSTYPE (ASTNode*) */

%%

// 1 - Regra inicial 
programa: declaracao_lista
        {
            root = $1;
        }
        | declaracao_lista error
        {
            root = $1;
            yyerrok;
        }
        ;

// 2 - Lista de declarações
declaracao_lista: declaracao_lista declaracao
                { $$ = append_node($1, $2); }
                | declaracao
                { $$ = $1; }
                ;

// 3 - Uma única declaração
declaracao: var_declaracao
          | fun_declaracao
          ;

// 4 - Declaração de variável
var_declaracao: tipo_especificador ID SEMICOLON
              {
                  /* $2 agora é um nó ID vindo do scanner */
                  $$ = create_node(NODE_VAR_DECL, $1, $2);
                  $$->rightChild->lineno = @2.first_line;
                  $$->lineno = @2.first_line;
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  /* $2 é nó ID, $4 é nó NUM */
                  ASTNode* array_decl = create_node(NODE_ARRAY_DECL, $2, $4);
                  $$ = create_node(NODE_VAR_DECL, $1, array_decl);
              }
              | error SEMICOLON
              {
                  yyerrok;
                  $$ = NULL;
              }
              ;

// 5 - Especificador de tipo
tipo_especificador: INT
                  { $$ = create_leaf_type(INT); }
                  | VOID
                  { $$ = create_leaf_type(VOID); }
                  ;

// 6 - Declaração de função
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  // Nova estrutura: FUN_DECL -> tipo, nome, FUN_BODY -> params, corpo
                  ASTNode* func_node = create_node(NODE_FUN_DECL, $1, $2);
                  func_node->rightChild->lineno = @2.first_line;
                  func_node->lineno = @2.first_line;
                  ASTNode* body_node = create_node(NODE_FUN_BODY, $4, $6);
                  func_node->next = body_node;
                  $$ = func_node;
              }
              ;

// 7 - Parâmetros 
params: param_lista
      { $$ = $1; }
      | VOID
      { $$ = NULL; }
      | 
      { $$ = NULL; }
      ;

// 8 - Lista de parâmetros  
param_lista: param_lista COMMA param
           { $$ = append_node($1, $3); }
           | param
           { $$ = $1; }
           ;

// 9 - Um único parâmetro
param: tipo_especificador ID
     {
         $$ = create_node(NODE_PARAM, $1, $2);
         $$->rightChild->lineno = @2.first_line;
         $$->lineno = @2.first_line;
     }
     | tipo_especificador ID OPENCOL CLOSECOL
     {
         ASTNode* array_param = create_node(NODE_ARRAY_DECL, $2, NULL);
         $$ = create_node(NODE_PARAM, $1, array_param);
     }
     ;

// 10 - Bloco de código  
composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             { $$ = create_node(NODE_COMPOUND_STMT, $2, $3); }
             ;

// 11 - Declarações locais  
local_declaracoes: local_declaracoes var_declaracao
                 { $$ = append_node($1, $2); }
                 | 
                 { $$ = NULL; }
                 ;

// 12 - Lista de comandos  
statement_lista: statement_lista statement
               { $$ = append_node($1, $2); }
               | 
               { $$ = NULL; }
               ;

// 13 - Um único comando  
statement: expressao_decl
         | composto_decl
         | selecao_decl
         | iteracao_decl
         | retorno_decl
         | error
         {
             $$ = NULL;
         }
         ;

// 14 - Comando de expressão  
expressao_decl: expressao SEMICOLON
              { $$ = $1; }
              | SEMICOLON
              { $$ = NULL; }
              ;

// 15 - Comando de seleção
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                $$ = create_node(NODE_IF_STMT, $3, $5);
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                ASTNode* if_node = create_node(NODE_IF_STMT, $3, $5);
                if_node->next = $7;
                $$ = if_node;
            }
            ;

// 16 - Comando de iteração  
iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             { $$ = create_node(NODE_WHILE_STMT, $3, $5); }
             ;

// 17 - Comando de retorno  
retorno_decl: RETURN SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, NULL, NULL); }
            | RETURN expressao SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, $2, NULL); }
            ;

// 18 - Expressão  
expressao: var ASSIGN expressao
         { $$ = create_node(NODE_ASSIGN_EXPR, $1, $3); }
         | simples_expressao
         { $$ = $1; }
         ;

// 19 - Variável  
var: ID
   { 
       /* $1 já é um nó ID */
       $$ = $1; 
       $$->lineno = @1.first_line;
   }
   | ID OPENCOL expressao CLOSECOL
   { $$ = create_node(NODE_ARRAY_ACCESS, $1, $3); }
   ;

// 20 - Expressão simples
simples_expressao: soma_expressao relacional soma_expressao
                 {
                     ASTNode* op_node = create_leaf_operator($2->number);
                     op_node->leftChild = $1;
                     op_node->rightChild = $3;
                     $$ = op_node;
                 }
                 | soma_expressao
                 { $$ = $1; }
                 ;

// 21 - Operador relacional
relacional: LET { $$ = create_leaf_operator(LET); }
          | LT  { $$ = create_leaf_operator(LT); }
          | GT  { $$ = create_leaf_operator(GT); }
          | GET { $$ = create_leaf_operator(GET); }
          | EQ  { $$ = create_leaf_operator(EQ); }
          | DIF { $$ = create_leaf_operator(DIF); }
          ;

// 22 - Expressão de soma/subtração  
soma_expressao: soma_expressao ADD termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->number = ADD;
              }
              | soma_expressao SUB termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->number = SUB;
              }
              | termo
              { $$ = $1; }
              ;

// 23 - Termo  
termo: termo MUL fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->number = MUL;
     }
     | termo DIV fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->number = DIV;
     }
     | fator
     { $$ = $1; }
     ;

// 24 - Fator  
fator: OPENPAR expressao CLOSEPAR
     { $$ = $2; }
     | var
     { $$ = $1; }
     | ativacao
     { $$ = $1; }
     | NUM
     { $$ = $1; }
     ;

// 25 - Ativação  
ativacao: ID OPENPAR args CLOSEPAR
        { $$ = create_node(NODE_FUN_CALL, $1, $3); }
        ;

// 26 - Argumentos  
args: arg_lista
    { $$ = $1; }
    | 
    { $$ = NULL; }
    ;

// 27 - Lista de argumentos  
arg_lista: arg_lista COMMA expressao
         { $$ = append_node($1, $3); }
         | expressao
         { $$ = $1; }
         ;

%%

extern FILE *yyin;

int error_count = 0;

#include "symtab.h"

int main(int argc, char *argv[])
{
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
    if (!yyin) {
      fprintf(stderr, "Error opening file: %s\n", argv[1]);
      return 1;
    }
  }
  
  int aux = yyparse();
  
  if(aux == 0 || root != NULL) {
    /* Semantic analysis first to verify everything and print errors */
    buildSymtab(root);
    typeCheck(root);

    if (aux != 0 || error_count > 0 || Error) {
        printf("\nSyntax tree built partially.\n");
    } else {
        printf("\nSyntax tree built successfully.\n");
    }
    
    printf("\n----- Abstract Syntax Tree -----\n");
    print_ast(root, 0);
    
    printf("\n----- Symbol Table -----\n");
    printSymTab(stdout);
  }
  
  return aux;
}

void yyerror(const char *msg)
{
    extern int yylineno; 
    extern char *yytext;
    error_count++;
    
    /* Remove redundant 'syntax error' prefix if present */
    const char *prefix = "syntax error, ";
    if (strncmp(msg, prefix, strlen(prefix)) == 0) {
        msg += strlen(prefix);
    }
    
    fprintf(stderr, "[Syntax Error] Line %d: %s near '%s'\n", yylineno, msg, yytext);
}