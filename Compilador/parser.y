%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "analyze.h"
#include "symtab.h"
#include "cintgen.h"

extern FILE *yyin;
int error_count = 0;
extern int Error;

ASTNode *root = NULL;

void yyerror(const char *);
int yylex(void);

extern char *yytext;
%}

%code requires {
    struct ASTNode;
}

%union {
    int ival;
    char* sval;
    struct ASTNode* node;
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

%token <ival> NUM
%token <sval> ID

%type <node> programa declaracao_lista declaracao var_declaracao tipo_especificador fun_declaracao
%type <node> params param_lista param composto_decl local_declaracoes statement_lista statement
%type <node> expressao_decl selecao_decl iteracao_decl retorno_decl expressao var simples_expressao
%type <node> relacional soma_expressao termo fator ativacao args arg_lista

%%

programa: declaracao_lista
        { root = $1; }
        | declaracao_lista error
        { root = $1; yyerrok; }
        ;

declaracao_lista: declaracao_lista declaracao
                { $$ = append_node($1, $2); }
                | declaracao
                { $$ = $1; }
                ;

declaracao: var_declaracao
          | fun_declaracao
          ;

var_declaracao: tipo_especificador ID SEMICOLON
              {
                  ASTNode* id_node = create_leaf_id($2);
                  id_node->lineno = @2.first_line;
                  $$ = create_node(NODE_VAR_DECL, $1, id_node);
                  $$->lineno = @2.first_line;
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  ASTNode* id_node = create_leaf_id($2);
                  id_node->lineno = @2.first_line;
                  ASTNode* num_node = create_leaf_num($4);
                  num_node->lineno = @4.first_line;
                  ASTNode* array_decl = create_node(NODE_ARRAY_DECL, id_node, num_node);
                  $$ = create_node(NODE_VAR_DECL, $1, array_decl);
              }
              | error SEMICOLON
              { yyerrok; $$ = NULL; }
              ;

tipo_especificador: INT
                  { $$ = create_leaf_type(INT); }
                  | VOID
                  { $$ = create_leaf_type(VOID); }
                  ;

fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  ASTNode* id_node = create_leaf_id($2);
                  id_node->lineno = @2.first_line;
                  ASTNode* func_node = create_node(NODE_FUN_DECL, $1, id_node);
                  func_node->lineno = @2.first_line;
                  ASTNode* body_node = create_node(NODE_FUN_BODY, $4, $6);
                  func_node->next = body_node;
                  $$ = func_node;
              }
              ;

params: param_lista
      { $$ = $1; }
      | VOID
      { $$ = NULL; }
      | 
      { $$ = NULL; }
      ;

param_lista: param_lista COMMA param
           { $$ = append_node($1, $3); }
           | param
           { $$ = $1; }
           ;

param: tipo_especificador ID
     {
         ASTNode* id_node = create_leaf_id($2);
         id_node->lineno = @2.first_line;
         $$ = create_node(NODE_PARAM, $1, id_node);
         $$->lineno = @2.first_line;
     }
     | tipo_especificador ID OPENCOL CLOSECOL
     {
         ASTNode* id_node = create_leaf_id($2);
         id_node->lineno = @2.first_line;
         ASTNode* array_param = create_node(NODE_ARRAY_DECL, id_node, NULL);
         $$ = create_node(NODE_PARAM, $1, array_param);
     }
     ;

composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             { $$ = create_node(NODE_COMPOUND_STMT, $2, $3); }
             ;

local_declaracoes: local_declaracoes var_declaracao
                 { $$ = append_node($1, $2); }
                 | 
                 { $$ = NULL; }
                 ;

statement_lista: statement_lista statement
               { $$ = append_node($1, $2); }
               | 
               { $$ = NULL; }
               ;

statement: expressao_decl
         | composto_decl
         | selecao_decl
         | iteracao_decl
         | retorno_decl
         | error
         { $$ = NULL; }
         ;

expressao_decl: expressao SEMICOLON
              { $$ = $1; }
              | SEMICOLON
              { $$ = NULL; }
              ;

selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                $$ = create_node(NODE_IF_STMT, $3, $5);
                $$->number = 0;
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                ASTNode* if_node = create_node(NODE_IF_STMT, $3, $5);
                if_node->next = $7;
                if_node->number = 1;
                $$ = if_node;
            }
            ;

iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             { $$ = create_node(NODE_WHILE_STMT, $3, $5); }
             ;

retorno_decl: RETURN SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, NULL, NULL); }
            | RETURN expressao SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, $2, NULL); }
            ;

expressao: var ASSIGN expressao
         { $$ = create_node(NODE_ASSIGN_EXPR, $1, $3); }
         | simples_expressao
         { $$ = $1; }
         ;

var: ID
   { 
       $$ = create_leaf_id($1);
       $$->lineno = @1.first_line;
   }
   | ID OPENCOL expressao CLOSECOL
   { 
       ASTNode* id_node = create_leaf_id($1);
       id_node->lineno = @1.first_line;
       $$ = create_node(NODE_ARRAY_ACCESS, id_node, $3); 
   }
   ;

simples_expressao: soma_expressao relacional soma_expressao
                 {
                     $2->leftChild  = $1;
                     $2->rightChild = $3;
                     $$ = $2;
                 }
                 | soma_expressao
                 { $$ = $1; }
                 ;

relacional: LET { $$ = create_leaf_operator(LET); }
          | LT  { $$ = create_leaf_operator(LT); }
          | GT  { $$ = create_leaf_operator(GT); }
          | GET { $$ = create_leaf_operator(GET); }
          | EQ  { $$ = create_leaf_operator(EQ); }
          | DIF { $$ = create_leaf_operator(DIF); }
          ;

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

fator: OPENPAR expressao CLOSEPAR
     { $$ = $2; }
     | var
     { $$ = $1; }
     | ativacao
     { $$ = $1; }
     | NUM
     { 
         $$ = create_leaf_num($1); 
         $$->lineno = @1.first_line;
     }
     ;

ativacao: ID OPENPAR args CLOSEPAR
        { 
            ASTNode* id_node = create_leaf_id($1);
            id_node->lineno = @1.first_line;
            $$ = create_node(NODE_FUN_CALL, id_node, $3); 
        }
        ;

args: arg_lista
    { $$ = $1; }
    | 
    { $$ = NULL; }
    ;

arg_lista: arg_lista COMMA expressao
         { $$ = append_node($1, $3); }
         | expressao
         { $$ = $1; }
         ;

%%

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
    buildSymtab(root);
    typeCheck(root);

    system("mkdir -p output");

    FILE* ast_file = fopen("output/ast.txt", "w");
    if (ast_file) {
        fprint_ast(ast_file, root, 0);
        fclose(ast_file);
        printf("Abstract syntax tree generated in output/ast.txt\n");
    } else {
        fprintf(stderr, "Error: Could not create output/ast.txt\n");
    }

    FILE* symtab_file = fopen("output/symbol_table.txt", "w");
    if (symtab_file) {
        printSymTab(symtab_file);
        fclose(symtab_file);
        printf("Symbol table generated in output/symbol_table.txt\n");
    } else {
        fprintf(stderr, "Error: Could not create output/symbol_table.txt\n");
    }

    if (Error == 0 && error_count == 0) {
        printf("Tentando gerar código intermediário...\n");
        generateProgram(root);

        // FILE* code_file = fopen("output/intermediate_code.txt", "w");
        // if (code_file) {
        //     fprintCode(code_file);
        //     fclose(code_file);
        //     printf("Intermediate code generated in output/intermediate_code.txt\n");
        // } else {
        //     fprintf(stderr, "Error: Could not create output/intermediate_code.txt\n");
        // }
    } else {
        printf("Intermediate code generation skipped due to errors\n");
    }
  }
  
  return aux;
}

void yyerror(const char *msg)
{
    extern int yylineno;
    extern char *yytext;
    error_count++;
    
    const char *prefix = "syntax error, ";
    if (strncmp(msg, prefix, strlen(prefix)) == 0) {
        msg += strlen(prefix);
    }
    
    fprintf(stderr, "[Syntax Error] Line %d: %s near '%s'\n", yylineno, msg, yytext);
}