/*******************************************************************************
 * Arquivo: ast.h
 * 
 * DEFINIÇÕES DA ÁRVORE SINTÁTICA ABSTRATA (AST)
 * 
 * Este header define as estruturas de dados fundamentais que representam o
 * código fonte de forma estruturada em árvore. A AST é construída pelo parser
 * e usada em todas as fases seguintes do compilador:
 * 
 * - Análise Semântica: verifica tipos, escopos e regras semânticas
 * - Geração de Código: transforma a AST em código intermediário
 * 
 * Cada nó da AST representa uma construção sintática da linguagem C-
 * (declarações, expressões, comandos, etc).
 ******************************************************************************/

#ifndef AST_H
#define AST_H

#include "parser.tab.h"  /* Tokens definidos pelo Bison */

/*******************************************************************************
 * ENUMERAÇÃO NodeType
 * 
 * Define todos os tipos de nós que podem existir na AST.
 * Cada tipo corresponde a uma construção sintática da linguagem C-.
 ******************************************************************************/
typedef enum {
    /* DECLARAÇÕES */
    NODE_VAR_DECL,      /* Declaração de variável: int x; ou int x[10]; */
    NODE_FUN_DECL,      /* Declaração de função: int foo(int x) { ... } */
    
    /* COMANDOS (Statements) */
    NODE_COMPOUND_STMT, /* Bloco de código: { declarações; comandos; } */
    NODE_IF_STMT,       /* Comando if: if (cond) stmt [else stmt] */
    NODE_WHILE_STMT,    /* Comando while: while (cond) stmt */
    NODE_RETURN_STMT,   /* Comando return: return; ou return expr; */
    
    /* EXPRESSÕES */
    NODE_ASSIGN_EXPR,   /* Expressão de atribuição: x = expr */
    NODE_BINARY_OP,     /* Operação binária: expr op expr (+, -, *, /) */
    NODE_NUM,           /* Literal numérico: 42 */
    NODE_VAR,           /* Referência a variável: x */
    NODE_FUN_CALL,      /* Chamada de função: foo(args) */
    NODE_ARRAY_DECL,    /* Declaração de array: [10] */
    NODE_ARRAY_ACCESS,  /* Acesso a elemento de array: x[i] */
    
    /* PARÂMETROS */
    NODE_PARAM,         /* Parâmetro de função: int x */
    NODE_PARAM_LIST,    /* Lista de parâmetros (não usado atualmente) */
    
    /* AUXILIARES */
    NODE_TYPE,          /* Especificador de tipo: int ou void */
    NODE_OPERATOR,      /* Operador relacional: <, >, ==, etc. */
    NODE_FUN_BODY       /* Corpo da função: parâmetros + bloco */
} NodeType;

/*******************************************************************************
 * ENUMERAÇÃO ExpType
 * 
 * Define os tipos de expressão usados na verificação de tipos.
 * Cada expressão na AST tem um expType indicando seu tipo de dado.
 ******************************************************************************/
typedef enum { 
    Void,      /* Tipo void - sem valor (usado para funções sem retorno) */
    Integer,   /* Tipo int - número inteiro */
    Boolean    /* Tipo boolean - resultado de comparações (interno) */
} ExpType;

/*******************************************************************************
 * ESTRUTURA ASTNode
 * 
 * Representa um nó da Árvore Sintática Abstrata.
 * 
 * A AST é uma árvore n-ária implementada como árvore binária com ponteiro
 * para irmãos (next). Isso permite que nós tenham número arbitrário de
 * filhos através da lista encadeada de irmãos.
 * 
 * EXEMPLO DE ESTRUTURA:
 * 
 *   Para: int gcd(int u, int v) { ... }
 * 
 *   FUN_DECL (node)
 *     ├─ leftChild: TYPE(int)
 *     ├─ rightChild: VAR(gcd)
 *     └─ next: FUN_BODY
 *                ├─ leftChild: PARAM(int u) → next: PARAM(int v) → NULL
 *                └─ rightChild: COMPOUND_STMT
 ******************************************************************************/
typedef struct ASTNode {
    /*-------------------------------------------------------------------------
     * Campos de Classificação
     *------------------------------------------------------------------------*/
    NodeType type;       /* Tipo do nó (NODE_VAR_DECL, NODE_IF_STMT, etc.) */
    ExpType expType;     /* Tipo da expressão (Integer, Void) - usado na 
                            análise semântica */
    
    /*-------------------------------------------------------------------------
     * Ponteiros para Navegação na Árvore
     *------------------------------------------------------------------------*/
    struct ASTNode *leftChild;   /* Filho esquerdo */
    struct ASTNode *rightChild;  /* Filho direito */
    struct ASTNode *next;        /* Próximo irmão (lista encadeada) */
    
    /*-------------------------------------------------------------------------
     * Dados Específicos do Nó
     *------------------------------------------------------------------------*/
    int number;          /* Valor numérico multiuso:
                            - NODE_NUM: valor do literal numérico
                            - NODE_TYPE: token do tipo (INT/VOID)
                            - NODE_BINARY_OP: token do operador (+,-,*,/)
                            - NODE_IF_STMT: flag (1=tem else, 0=não tem) */
    
    char* identifier;    /* Nome do identificador (para NODE_VAR) */
    
    int lineno;          /* Número da linha no código fonte 
                            (usado para mensagens de erro) */
} ASTNode;

/*******************************************************************************
 * PROTÓTIPOS DE FUNÇÕES
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 * Funções de Criação de Nós
 *----------------------------------------------------------------------------*/

/**
 * create_node: Cria um novo nó da AST com dois filhos.
 * 
 * @param type:  Tipo do nó (NODE_VAR_DECL, NODE_IF_STMT, etc.)
 * @param left:  Ponteiro para o filho esquerdo (pode ser NULL)
 * @param right: Ponteiro para o filho direito (pode ser NULL)
 * @return:      Ponteiro para o novo nó alocado
 */
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right);

/**
 * create_leaf_num: Cria um nó folha para um literal numérico.
 * 
 * @param value: Valor inteiro do número
 * @return:      Nó NODE_NUM com o valor armazenado em 'number'
 */
ASTNode* create_leaf_num(int value);

/**
 * create_leaf_id: Cria um nó folha para um identificador.
 * 
 * @param name: Nome do identificador (string será armazenada)
 * @return:     Nó NODE_VAR com o nome armazenado em 'identifier'
 */
ASTNode* create_leaf_id(char* name);

/**
 * create_leaf_type: Cria um nó folha para um especificador de tipo.
 * 
 * @param type_token: Token do tipo (INT ou VOID do parser)
 * @return:           Nó NODE_TYPE com o token em 'number'
 */
ASTNode* create_leaf_type(int type_token);

/**
 * create_leaf_operator: Cria um nó folha para um operador.
 * 
 * @param operator: Token do operador (LT, GT, EQ, etc.)
 * @return:         Nó NODE_OPERATOR com o token em 'number'
 */
ASTNode* create_leaf_operator(int operator);

/*-----------------------------------------------------------------------------
 * Funções de Manipulação de Listas
 *----------------------------------------------------------------------------*/

/**
 * append_node: Adiciona um nó ao final de uma lista encadeada.
 * 
 * Listas na AST são implementadas através do ponteiro 'next'.
 * Esta função percorre a lista até o final e adiciona o novo nó.
 * 
 * @param list:     Ponteiro para o início da lista (pode ser NULL)
 * @param new_node: Nó a ser adicionado
 * @return:         Ponteiro para o início da lista (pode ser new_node se 
 *                  list era NULL)
 */
ASTNode* append_node(ASTNode* list, ASTNode* new_node);

/*-----------------------------------------------------------------------------
 * Funções de Impressão/Debug
 *----------------------------------------------------------------------------*/

/**
 * print_ast: Imprime a AST de forma hierárquica no stdout.
 * 
 * @param node:  Nó raiz da subárvore a imprimir
 * @param level: Nível de indentação (0 = raiz)
 */
void print_ast(ASTNode* node, int level);

/**
 * fprint_ast: Imprime a AST de forma hierárquica em um arquivo.
 * 
 * @param out:   Ponteiro para o arquivo de saída
 * @param node:  Nó raiz da subárvore a imprimir
 * @param level: Nível de indentação (0 = raiz)
 */
void fprint_ast(FILE* out, ASTNode* node, int level);

/**
 * token_to_string: Converte um token numérico para string legível.
 * 
 * @param token: Código numérico do token
 * @return:      String representando o token (ex: "ADD", "INT", "EQ")
 */
const char* token_to_string(int token);

#endif /* AST_H */