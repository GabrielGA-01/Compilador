/*******************************************************************************
 * Arquivo: parser.y
 * 
 * ANALISADOR SINTÁTICO (Parser) para a linguagem C-
 * 
 * Este arquivo define a gramática da linguagem C- usando Bison. O parser é a
 * segunda fase do compilador, responsável por:
 * 1. Receber tokens do scanner (analisador léxico)
 * 2. Verificar se a sequência de tokens segue as regras da gramática
 * 3. Construir a Árvore Sintática Abstrata (AST)
 * 4. Reportar erros sintáticos quando a estrutura do código está incorreta
 * 
 * A gramática C- é baseada no livro "Compiler Construction: Principles and 
 * Practice" de Kenneth C. Louden.
 ******************************************************************************/

%{
/*-----------------------------------------------------------------------------
 * SEÇÃO DE CÓDIGO C - Includes e Declarações
 * 
 * Código C que será incluído no início do arquivo gerado pelo Bison.
 * Aqui declaramos as dependências e variáveis globais necessárias.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"       /* Estruturas e funções da AST */
#include "analyze.h"   /* Interface da análise semântica */

/*
 * root: Ponteiro para a raiz da AST.
 * Após o parsing, esta variável conterá toda a representação do programa
 * como uma árvore que será usada nas fases seguintes (análise semântica,
 * geração de código).
 */
ASTNode *root = NULL;

/* Declarações de funções */
void yyerror(const char *);          /* Função para reportar erros sintáticos */
int yylex(void);                      /* Função do scanner que retorna tokens */
void abrirArq();                      /* Função para abrir arquivo fonte */
ASTNode* append_node(ASTNode* list, ASTNode* new_node);  /* Anexar nó à lista */

/*
 * YYSTYPE: Define o tipo de dados para os valores semânticos.
 * Em nosso compilador, todos os valores são ponteiros para nós da AST.
 * Isso significa que cada símbolo da gramática (terminal ou não-terminal)
 * carrega um nó da árvore como seu valor.
 */
#define YYSTYPE struct ASTNode*

extern char *yytext;  /* Texto do token atual vindo do scanner */
%}

/*-----------------------------------------------------------------------------
 * DIRETIVA %code requires
 * 
 * Este bloco é incluído no arquivo header gerado (parser.tab.h).
 * Necessário para que outros arquivos que incluam o header saibam o
 * que é YYSTYPE sem criar dependências circulares.
 *----------------------------------------------------------------------------*/
%code requires {
    /* Declaração forward para evitar loop de includes */
    struct ASTNode;
    #define YYSTYPE struct ASTNode*
}

/*-----------------------------------------------------------------------------
 * CONFIGURAÇÕES DO BISON
 *----------------------------------------------------------------------------*/

%start programa           /* Símbolo inicial da gramática */

/*
 * Precedência e Associatividade de Operadores
 * 
 * Operadores listados primeiro têm MENOR precedência.
 * %left significa associatividade à esquerda: a + b + c = (a + b) + c
 * 
 * Isso resolve ambiguidades na gramática determinando qual operador
 * "vence" quando há conflito.
 */
%left ADD SUB             /* Menor precedência: + e - */
%left MUL DIV             /* Maior precedência: * e / */

/*
 * Resolução do conflito "dangling else"
 * 
 * Em C-, if (x) if (y) s else t é ambíguo:
 * - if (x) { if (y) s else t }  ou
 * - if (x) { if (y) s } else t
 * 
 * %prec IFX e %nonassoc resolvem isso fazendo o else se associar
 * ao if mais próximo (regra padrão em C/C++/Java).
 */
%nonassoc IFX             /* Precedência do IF sem ELSE */
%nonassoc ELSE            /* ELSE tem maior precedência que IFX */

%define parse.error verbose  /* Mensagens de erro detalhadas */
%locations                   /* Habilita rastreamento de linhas */

/*-----------------------------------------------------------------------------
 * DECLARAÇÃO DE TOKENS
 * 
 * Todos os tokens que podem vir do scanner. Estes são os "terminais" da
 * gramática - símbolos que aparecem diretamente no código fonte.
 *----------------------------------------------------------------------------*/

%token ERRO                              /* Token de erro léxico */
%token ADD SUB MUL DIV                   /* Operadores aritméticos */
%token LT LET GET GT EQ DIF              /* Operadores relacionais */
%token ASSIGN SEMICOLON COMMA            /* Atribuição e pontuação */
%token OPENPAR CLOSEPAR OPENCOL CLOSECOL OPENCHA CLOSECHA  /* Delimitadores */
%token IF ELSE INT RETURN VOID WHILE     /* Palavras reservadas */

%token NUM                               /* Número literal */
%token ID                                /* Identificador */

/* Não precisamos declarar tipos pois tudo é YYSTYPE (ASTNode*) */

%%

/*******************************************************************************
 * SEÇÃO DE REGRAS DA GRAMÁTICA
 * 
 * Cada regra define como um símbolo não-terminal pode ser formado a partir
 * de outros símbolos. O formato é:
 * 
 *   não-terminal: alternativa1 { ação1 }
 *               | alternativa2 { ação2 }
 *               ;
 * 
 * $$ representa o valor do lado esquerdo (não-terminal sendo definido)
 * $1, $2, ... representam os valores dos símbolos do lado direito
 * @1, @2, ... contêm as informações de localização (linha) dos símbolos
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 * Regra 1: PROGRAMA (Símbolo Inicial)
 * 
 * Um programa em C- é uma lista de declarações (variáveis e funções).
 * A árvore resultante é armazenada na variável global 'root'.
 *----------------------------------------------------------------------------*/
programa: declaracao_lista
        {
            root = $1;  /* A raiz da AST é a lista de declarações */
        }
        | declaracao_lista error
        {
            /* Recuperação de erro: tenta continuar mesmo com erro */
            root = $1;
            yyerrok;    /* Limpa o estado de erro do parser */
        }
        ;

/*-----------------------------------------------------------------------------
 * Regra 2: LISTA DE DECLARAÇÕES
 * 
 * Zero ou mais declarações formam uma lista encadeada através do campo 'next'.
 * A função append_node adiciona novos nós ao final da lista.
 *----------------------------------------------------------------------------*/
declaracao_lista: declaracao_lista declaracao
                { $$ = append_node($1, $2); }  /* Adiciona declaração à lista */
                | declaracao
                { $$ = $1; }                    /* Lista com um único elemento */
                ;

/*-----------------------------------------------------------------------------
 * Regra 3: DECLARAÇÃO
 * 
 * Uma declaração pode ser de variável ou de função.
 *----------------------------------------------------------------------------*/
declaracao: var_declaracao
          | fun_declaracao
          ;

/*-----------------------------------------------------------------------------
 * Regra 4: DECLARAÇÃO DE VARIÁVEL
 * 
 * Variáveis podem ser simples (int x;) ou arrays (int x[10];).
 * A estrutura da AST é: VAR_DECL -> tipo, identificador
 *----------------------------------------------------------------------------*/
var_declaracao: tipo_especificador ID SEMICOLON
              {
                  /* Declaração de variável simples: int x; */
                  /* $2 é um nó ID criado pelo scanner */
                  $$ = create_node(NODE_VAR_DECL, $1, $2);
                  $$->rightChild->lineno = @2.first_line;  /* Linha do ID */
                  $$->lineno = @2.first_line;
              }
              | tipo_especificador ID OPENCOL NUM CLOSECOL SEMICOLON
              {
                  /* Declaração de array: int x[10]; */
                  /* $2 é nó ID, $4 é nó NUM com o tamanho */
                  ASTNode* array_decl = create_node(NODE_ARRAY_DECL, $2, $4);
                  $$ = create_node(NODE_VAR_DECL, $1, array_decl);
              }
              | error SEMICOLON
              {
                  /* Recuperação de erro: pula até o próximo ';' */
                  yyerrok;
                  $$ = NULL;
              }
              ;

/*-----------------------------------------------------------------------------
 * Regra 5: ESPECIFICADOR DE TIPO
 * 
 * C- tem apenas dois tipos: int (inteiro) e void (sem valor).
 * Criamos um nó TYPE que armazena qual tipo foi especificado.
 *----------------------------------------------------------------------------*/
tipo_especificador: INT
                  { $$ = create_leaf_type(INT); }   /* Tipo inteiro */
                  | VOID
                  { $$ = create_leaf_type(VOID); }  /* Tipo void */
                  ;

/*-----------------------------------------------------------------------------
 * Regra 6: DECLARAÇÃO DE FUNÇÃO
 * 
 * Funções têm: tipo de retorno, nome, parâmetros e corpo.
 * Estrutura: FUN_DECL -> tipo, nome
 *            FUN_BODY -> parâmetros, corpo (conectado via ->next)
 *----------------------------------------------------------------------------*/
fun_declaracao: tipo_especificador ID OPENPAR params CLOSEPAR composto_decl
              {
                  /* Cria nó da função com tipo e nome */
                  ASTNode* func_node = create_node(NODE_FUN_DECL, $1, $2);
                  func_node->rightChild->lineno = @2.first_line;
                  func_node->lineno = @2.first_line;
                  
                  /* Cria nó do corpo com parâmetros e bloco de código */
                  ASTNode* body_node = create_node(NODE_FUN_BODY, $4, $6);
                  
                  /* Conecta corpo ao nó da função via ponteiro 'next' */
                  func_node->next = body_node;
                  $$ = func_node;
              }
              ;

/*-----------------------------------------------------------------------------
 * Regra 7: PARÂMETROS
 * 
 * A lista de parâmetros pode ser: lista de parâmetros, VOID, ou vazia.
 *----------------------------------------------------------------------------*/
params: param_lista
      { $$ = $1; }          /* Lista de parâmetros */
      | VOID
      { $$ = NULL; }        /* Explicitamente void */
      | 
      { $$ = NULL; }        /* Lista vazia */
      ;

/*-----------------------------------------------------------------------------
 * Regra 8: LISTA DE PARÂMETROS
 * 
 * Parâmetros separados por vírgula, formando uma lista encadeada.
 *----------------------------------------------------------------------------*/
param_lista: param_lista COMMA param
           { $$ = append_node($1, $3); }  /* Adiciona parâmetro à lista */
           | param
           { $$ = $1; }                    /* Lista com um parâmetro */
           ;

/*-----------------------------------------------------------------------------
 * Regra 9: PARÂMETRO
 * 
 * Um parâmetro tem tipo e nome. Pode ser array (tipo nome[]).
 *----------------------------------------------------------------------------*/
param: tipo_especificador ID
     {
         /* Parâmetro simples: int x */
         $$ = create_node(NODE_PARAM, $1, $2);
         $$->rightChild->lineno = @2.first_line;
         $$->lineno = @2.first_line;
     }
     | tipo_especificador ID OPENCOL CLOSECOL
     {
         /* Parâmetro array: int x[] */
         ASTNode* array_param = create_node(NODE_ARRAY_DECL, $2, NULL);
         $$ = create_node(NODE_PARAM, $1, array_param);
     }
     ;

/*-----------------------------------------------------------------------------
 * Regra 10: DECLARAÇÃO COMPOSTA (Bloco de Código)
 * 
 * Um bloco é delimitado por chaves e contém declarações locais e comandos.
 * Estrutura: COMPOUND_STMT -> declarações_locais, lista_de_comandos
 *----------------------------------------------------------------------------*/
composto_decl: OPENCHA local_declaracoes statement_lista CLOSECHA
             { $$ = create_node(NODE_COMPOUND_STMT, $2, $3); }
             ;

/*-----------------------------------------------------------------------------
 * Regra 11: DECLARAÇÕES LOCAIS
 * 
 * Zero ou mais declarações de variáveis dentro de um bloco.
 *----------------------------------------------------------------------------*/
local_declaracoes: local_declaracoes var_declaracao
                 { $$ = append_node($1, $2); }  /* Adiciona declaração */
                 | 
                 { $$ = NULL; }                  /* Lista vazia */
                 ;

/*-----------------------------------------------------------------------------
 * Regra 12: LISTA DE COMANDOS (Statements)
 * 
 * Zero ou mais comandos dentro de um bloco.
 *----------------------------------------------------------------------------*/
statement_lista: statement_lista statement
               { $$ = append_node($1, $2); }   /* Adiciona comando */
               | 
               { $$ = NULL; }                   /* Lista vazia */
               ;

/*-----------------------------------------------------------------------------
 * Regra 13: COMANDO (Statement)
 * 
 * Os tipos de comandos em C-: expressão, bloco, if, while, return.
 *----------------------------------------------------------------------------*/
statement: expressao_decl     /* Expressão como comando (ex: x = 5;) */
         | composto_decl      /* Bloco aninhado { ... } */
         | selecao_decl       /* Comando if-else */
         | iteracao_decl      /* Comando while */
         | retorno_decl       /* Comando return */
         | error              /* Recuperação de erro */
         {
             $$ = NULL;
         }
         ;

/*-----------------------------------------------------------------------------
 * Regra 14: COMANDO DE EXPRESSÃO
 * 
 * Uma expressão seguida de ponto-e-vírgula, ou apenas ponto-e-vírgula.
 *----------------------------------------------------------------------------*/
expressao_decl: expressao SEMICOLON
              { $$ = $1; }        /* Expressão como comando */
              | SEMICOLON
              { $$ = NULL; }      /* Comando vazio */
              ;

/*-----------------------------------------------------------------------------
 * Regra 15: COMANDO DE SELEÇÃO (if-else)
 * 
 * if (condição) comando [else comando]
 * O campo 'number' indica se há else (1 = tem, 0 = não tem).
 *----------------------------------------------------------------------------*/
selecao_decl: IF OPENPAR expressao CLOSEPAR statement %prec IFX
            {
                /* IF sem ELSE */
                $$ = create_node(NODE_IF_STMT, $3, $5);
                $$->number = 0;  /* Flag: NÃO tem else */
            }
            | IF OPENPAR expressao CLOSEPAR statement ELSE statement
            {
                /* IF com ELSE */
                ASTNode* if_node = create_node(NODE_IF_STMT, $3, $5);
                if_node->next = $7;    /* ELSE está no campo 'next' */
                if_node->number = 1;   /* Flag: TEM else */
                $$ = if_node;
            }
            ;

/*-----------------------------------------------------------------------------
 * Regra 16: COMANDO DE ITERAÇÃO (while)
 * 
 * while (condição) comando
 *----------------------------------------------------------------------------*/
iteracao_decl: WHILE OPENPAR expressao CLOSEPAR statement
             { $$ = create_node(NODE_WHILE_STMT, $3, $5); }
             ;

/*-----------------------------------------------------------------------------
 * Regra 17: COMANDO DE RETORNO
 * 
 * return; ou return expressão;
 *----------------------------------------------------------------------------*/
retorno_decl: RETURN SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, NULL, NULL); }  /* Sem valor */
            | RETURN expressao SEMICOLON
            { $$ = create_node(NODE_RETURN_STMT, $2, NULL); }    /* Com valor */
            ;

/*-----------------------------------------------------------------------------
 * Regra 18: EXPRESSÃO
 * 
 * Uma expressão pode ser uma atribuição ou uma expressão simples.
 *----------------------------------------------------------------------------*/
expressao: var ASSIGN expressao
         { $$ = create_node(NODE_ASSIGN_EXPR, $1, $3); }  /* Atribuição */
         | simples_expressao
         { $$ = $1; }                                      /* Sem atribuição */
         ;

/*-----------------------------------------------------------------------------
 * Regra 19: VARIÁVEL
 * 
 * Uma variável simples (x) ou acesso a array (x[i]).
 *----------------------------------------------------------------------------*/
var: ID
   { 
       /* Variável simples - $1 já é um nó ID do scanner */
       $$ = $1; 
       $$->lineno = @1.first_line;
   }
   | ID OPENCOL expressao CLOSECOL
   { $$ = create_node(NODE_ARRAY_ACCESS, $1, $3); }  /* Acesso a array */
   ;

/*-----------------------------------------------------------------------------
 * Regra 20: EXPRESSÃO SIMPLES
 * 
 * Comparação entre duas expressões ou apenas uma expressão de soma.
 *----------------------------------------------------------------------------*/
simples_expressao: soma_expressao relacional soma_expressao
                 {
                     /* Cria nó de operador com os dois operandos */
                     ASTNode* op_node = create_leaf_operator($2->number);
                     op_node->leftChild = $1;
                     op_node->rightChild = $3;
                     $$ = op_node;
                 }
                 | soma_expressao
                 { $$ = $1; }  /* Sem operador relacional */
                 ;

/*-----------------------------------------------------------------------------
 * Regra 21: OPERADOR RELACIONAL
 * 
 * Operadores de comparação retornam um nó com o tipo de operador.
 *----------------------------------------------------------------------------*/
relacional: LET { $$ = create_leaf_operator(LET); }  /* <= */
          | LT  { $$ = create_leaf_operator(LT); }   /* <  */
          | GT  { $$ = create_leaf_operator(GT); }   /* >  */
          | GET { $$ = create_leaf_operator(GET); }  /* >= */
          | EQ  { $$ = create_leaf_operator(EQ); }   /* == */
          | DIF { $$ = create_leaf_operator(DIF); }  /* != */
          ;

/*-----------------------------------------------------------------------------
 * Regra 22: EXPRESSÃO DE SOMA/SUBTRAÇÃO
 * 
 * Expressões aditivas com operadores + e -.
 * O campo 'number' armazena qual operador foi usado.
 *----------------------------------------------------------------------------*/
soma_expressao: soma_expressao ADD termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->number = ADD;  /* Marca como adição */
              }
              | soma_expressao SUB termo
              {
                  $$ = create_node(NODE_BINARY_OP, $1, $3);
                  $$->number = SUB;  /* Marca como subtração */
              }
              | termo
              { $$ = $1; }
              ;

/*-----------------------------------------------------------------------------
 * Regra 23: TERMO
 * 
 * Expressões multiplicativas com operadores * e /.
 * Maior precedência que + e -.
 *----------------------------------------------------------------------------*/
termo: termo MUL fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->number = MUL;  /* Marca como multiplicação */
     }
     | termo DIV fator
     {
         $$ = create_node(NODE_BINARY_OP, $1, $3);
         $$->number = DIV;  /* Marca como divisão */
     }
     | fator
     { $$ = $1; }
     ;

/*-----------------------------------------------------------------------------
 * Regra 24: FATOR
 * 
 * Unidade básica de uma expressão: expressão entre parênteses, variável,
 * chamada de função ou número literal.
 *----------------------------------------------------------------------------*/
fator: OPENPAR expressao CLOSEPAR
     { $$ = $2; }                /* Expressão entre parênteses */
     | var
     { $$ = $1; }                /* Variável */
     | ativacao
     { $$ = $1; }                /* Chamada de função */
     | NUM
     { $$ = $1; }                /* Número literal */
     ;

/*-----------------------------------------------------------------------------
 * Regra 25: ATIVAÇÃO (Chamada de Função)
 * 
 * Sintaxe: nome_funcao(argumentos)
 *----------------------------------------------------------------------------*/
ativacao: ID OPENPAR args CLOSEPAR
        { $$ = create_node(NODE_FUN_CALL, $1, $3); }
        ;

/*-----------------------------------------------------------------------------
 * Regra 26: ARGUMENTOS
 * 
 * Lista de argumentos passados a uma função.
 *----------------------------------------------------------------------------*/
args: arg_lista
    { $$ = $1; }        /* Lista de argumentos */
    | 
    { $$ = NULL; }      /* Sem argumentos */
    ;

/*-----------------------------------------------------------------------------
 * Regra 27: LISTA DE ARGUMENTOS
 * 
 * Argumentos separados por vírgula.
 *----------------------------------------------------------------------------*/
arg_lista: arg_lista COMMA expressao
         { $$ = append_node($1, $3); }  /* Adiciona argumento */
         | expressao
         { $$ = $1; }                    /* Um único argumento */
         ;

%%

/*******************************************************************************
 * SEÇÃO DE CÓDIGO C FINAL
 * 
 * Código C que aparece após as regras da gramática.
 * Contém a função main e funções auxiliares.
 ******************************************************************************/

extern FILE *yyin;        /* Arquivo de entrada do scanner */
int error_count = 0;      /* Contador de erros sintáticos */

#include "symtab.h"       /* Tabela de símbolos */
#include "cgen.h"         /* Geração de código */
extern int Error;         /* Flag de erro semântico */

/*-----------------------------------------------------------------------------
 * FUNÇÃO MAIN
 * 
 * Ponto de entrada do compilador. Executa todas as fases:
 * 1. Abre o arquivo fonte
 * 2. Parsing (análise sintática) - constrói a AST
 * 3. Análise semântica - verifica tipos e escopos
 * 4. Geração de código intermediário (se não houver erros)
 * 5. Salva os resultados em arquivos de saída
 *----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  /* Verifica se foi passado um arquivo como argumento */
  if (argc > 1) {
    yyin = fopen(argv[1], "r");
    if (!yyin) {
      fprintf(stderr, "Error opening file: %s\n", argv[1]);
      return 1;
    }
  }
  
  /* FASE 2: Análise Sintática (Parsing) */
  int aux = yyparse();  /* Chama o parser, que por sua vez chama o scanner */
  
  if(aux == 0 || root != NULL) {
    /*-------------------------------------------------------------------------
     * FASE 3: Análise Semântica
     * 
     * buildSymtab: Percorre a AST e constrói a tabela de símbolos,
     *              verificando declarações duplicadas e variáveis não declaradas.
     * typeCheck: Verifica compatibilidade de tipos nas operações.
     *------------------------------------------------------------------------*/
    buildSymtab(root);
    typeCheck(root);

    /* Cria diretório de saída se não existir */
    system("mkdir -p output");

    /* Salva a Árvore Sintática Abstrata em arquivo */
    FILE* ast_file = fopen("output/ast.txt", "w");
    if (ast_file) {
        fprint_ast(ast_file, root, 0);
        fclose(ast_file);
        printf("Abstract syntax tree generated in output/ast.txt\n");
    } else {
        fprintf(stderr, "Error: Could not create output/ast.txt\n");
    }

    /* Salva a Tabela de Símbolos em arquivo */
    FILE* symtab_file = fopen("output/symbol_table.txt", "w");
    if (symtab_file) {
        printSymTab(symtab_file);
        fclose(symtab_file);
        printf("Symbol table generated in output/symbol_table.txt\n");
    } else {
        fprintf(stderr, "Error: Could not create output/symbol_table.txt\n");
    }

    /*-------------------------------------------------------------------------
     * FASE 4: Geração de Código Intermediário
     * 
     * Só executa se não houver erros nas fases anteriores.
     * O código intermediário usa formato de quádruplas.
     *------------------------------------------------------------------------*/
    if (Error == 0 && error_count == 0) {
        generateProgram(root);

        /* Salva código intermediário em arquivo */
        FILE* code_file = fopen("output/intermediate_code.txt", "w");
        if (code_file) {
            fprintCode(code_file);
            fclose(code_file);
            printf("Intermediate code generated in output/intermediate_code.txt\n");
        } else {
            fprintf(stderr, "Error: Could not create output/intermediate_code.txt\n");
        }
    } else {
        printf("Intermediate code generation skipped due to errors\n");
    }
  }
  
  return aux;
}

/*-----------------------------------------------------------------------------
 * FUNÇÃO YYERROR
 * 
 * Chamada pelo Bison quando encontra um erro sintático.
 * Formata e imprime uma mensagem de erro amigável.
 *
 * @param msg: Mensagem de erro gerada pelo Bison
 *----------------------------------------------------------------------------*/
void yyerror(const char *msg)
{
    extern int yylineno;   /* Número da linha atual */
    extern char *yytext;   /* Texto do token problemático */
    error_count++;
    
    /* Remove prefixo redundante "syntax error, " se presente */
    const char *prefix = "syntax error, ";
    if (strncmp(msg, prefix, strlen(prefix)) == 0) {
        msg += strlen(prefix);  /* Avança o ponteiro para pular o prefixo */
    }
    
    fprintf(stderr, "[Syntax Error] Line %d: %s near '%s'\n", yylineno, msg, yytext);
}