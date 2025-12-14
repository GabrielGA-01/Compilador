/*******************************************************************************
 * Arquivo: cgen.h
 * 
 * INTERFACE DE GERAÇÃO DE CÓDIGO INTERMEDIÁRIO
 * 
 * Este header define as estruturas e funções para geração de código
 * intermediário no formato de quádruplas "Three-Address Code" (TAC).
 * 
 * O QUE É CÓDIGO INTERMEDIÁRIO?
 * Código intermediário é uma representação do programa entre o código
 * fonte (alto nível) e o código de máquina (baixo nível). Serve como
 * ponte que facilita otimizações e portabilidade para diferentes arquiteturas.
 * 
 * FORMATO DE QUÁDRUPLA:
 * (operador, operando1, operando2, resultado)
 * 
 * EXEMPLOS:
 *   x = a + b     →  (add, a, b, t1)  (asn, t1, x, _)
 *   if (x < y)    →  (lt, x, y, t1)   (if_f, t1, L1, _)
 *   func(x, y)    →  (param, x, _, _) (param, y, _, _) (call, func, 2, t1)
 * 
 * Esta fase é executada APENAS se não houver erros nas fases anteriores.
 ******************************************************************************/

#ifndef CGEN_H
#define CGEN_H

#include "ast.h"   /* Estruturas da AST */

/*******************************************************************************
 * ENUMERAÇÃO QuadOp
 * 
 * Define todos os operadores possíveis nas quádruplas.
 * Cada operador corresponde a uma operação no código intermediário.
 ******************************************************************************/
typedef enum {
    /* OPERAÇÕES ARITMÉTICAS */
    OP_ADD,      /* add: Adição (a + b) */
    OP_SUB,      /* sub: Subtração (a - b) */
    OP_MUL,      /* mul: Multiplicação (a * b) */
    OP_DIV,      /* div: Divisão (a / b) */
    OP_ASN,      /* asn: Atribuição (x = valor) */
    
    /* OPERAÇÕES RELACIONAIS (Comparação) */
    OP_LT,       /* lt: Menor que (a < b) */
    OP_LET,      /* let: Menor ou igual (a <= b) */
    OP_GT,       /* gt: Maior que (a > b) */
    OP_GET,      /* get: Maior ou igual (a >= b) */
    OP_EQ,       /* eq: Igual (a == b) */
    OP_DIF,      /* dif: Diferente (a != b) */
    
    /* CONTROLE DE FLUXO */
    OP_IF_F,     /* if_f: Desvio condicional se falso (if false goto L) */
    OP_GOTO,     /* goto: Desvio incondicional (goto L) */
    OP_LAB,      /* lab: Definição de label (L:) */
    
    /* ENTRADA/SAÍDA */
    OP_RD,       /* rd: Leitura de entrada (input) */
    OP_WRI,      /* wri: Escrita de saída (output) */
    
    /* CHAMADAS DE FUNÇÃO */
    OP_PARAM,    /* param: Passagem de parâmetro */
    OP_CALL,     /* call: Chamada de função */
    OP_RET,      /* ret: Retorno de função */
    
    /* CONTROLE DE PROGRAMA */
    OP_HALT,     /* halt: Fim do programa */
    
    /* OPERAÇÕES COM ARRAYS */
    OP_ARR_ACC,  /* arr_acc: Acesso a elemento de array (t = arr[i]) */
    OP_ARR_ASN   /* arr_asn: Atribuição a elemento de array (arr[i] = t) */
} QuadOp;

/*******************************************************************************
 * ENUMERAÇÃO OperandKind
 * 
 * Define os tipos de operandos que podem aparecer nas quádruplas.
 ******************************************************************************/
typedef enum {
    EMPTY,       /* Operando vazio (quando não necessário) */
    INT_CONST,   /* Constante inteira literal */
    STRING_VAR,  /* Variável de nome string (do código fonte) */
    TEMP_VAR,    /* Variável temporária gerada (t1, t2, ...) */
    LABEL_KIND   /* Label para controle de fluxo (L1, L2, ...) */
} OperandKind;

/*******************************************************************************
 * ESTRUTURA Address
 * 
 * Representa um operando (endereço) em uma quádrupla.
 * Pode ser uma constante, variável, temporário ou label.
 ******************************************************************************/
typedef struct {
    OperandKind kind;  /* Tipo do operando */
    int val;           /* Valor numérico (se INT_CONST) */
    char *name;        /* Nome (se variável, temporário ou label) */
} Address;

/*******************************************************************************
 * ESTRUTURA Quad
 * 
 * Representa uma quádrupla completa: (op, addr1, addr2, addr3)
 * As quádruplas são organizadas em uma lista encadeada.
 ******************************************************************************/
typedef struct Quad {
    QuadOp op;      /* Operador da quádrupla */
    Address addr1;  /* Primeiro operando */
    Address addr2;  /* Segundo operando */
    Address addr3;  /* Resultado/destino */
    struct Quad *next;  /* Próxima quádrupla na lista */
} Quad;

/*******************************************************************************
 * FUNÇÕES PARA CRIAR ENDEREÇOS (OPERANDOS)
 ******************************************************************************/

/**
 * emptyAddr: Cria um endereço vazio.
 * 
 * Usado quando um operando não é necessário na quádrupla.
 * EXEMPLO: (goto, L1, _, _) - os dois últimos são vazios
 * 
 * RETORNO: Address com kind=EMPTY
 */
Address emptyAddr();

/**
 * createVal: Cria um endereço para constante inteira.
 * 
 * PARÂMETROS:
 *   val - Valor da constante
 * 
 * RETORNO: Address com kind=INT_CONST e o valor especificado
 * 
 * EXEMPLO: createVal(42) para o literal "42"
 */
Address createVal(int val);

/**
 * createVar: Cria um endereço para variável do código fonte.
 * 
 * PARÂMETROS:
 *   name - Nome da variável
 * 
 * RETORNO: Address com kind=STRING_VAR
 * 
 * EXEMPLO: createVar("x") para a variável "x"
 */
Address createVar(char *name);

/**
 * createTemp: Cria um novo temporário único.
 * 
 * Temporários são variáveis artificiais criadas para armazenar
 * resultados intermediários de expressões.
 * 
 * RETORNO: Address com kind=TEMP_VAR e nome único (t1, t2, ...)
 * 
 * EXEMPLO: Para "a + b * c", gera:
 *   t1 = b * c    (t1 é temporário)
 *   t2 = a + t1   (t2 é temporário)
 */
Address createTemp();

/**
 * createLabel: Cria um novo label único.
 * 
 * Labels são usados para controle de fluxo (if, while, goto).
 * 
 * RETORNO: Address com kind=LABEL_KIND e nome único (L1, L2, ...)
 */
Address createLabel();

/*******************************************************************************
 * FUNÇÃO EMIT
 ******************************************************************************/

/**
 * emit: Emite (adiciona) uma nova quádrupla à lista de código.
 * 
 * Esta é a função principal usada durante a geração de código.
 * Cada chamada adiciona uma instrução ao programa intermediário.
 * 
 * PARÂMETROS:
 *   op - Operador da quádrupla
 *   a1 - Primeiro operando
 *   a2 - Segundo operando  
 *   a3 - Resultado/destino
 * 
 * EXEMPLO:
 *   emit(OP_ADD, createVar("a"), createVar("b"), createTemp())
 *   Gera: (add, a, b, t1)
 */
void emit(QuadOp op, Address a1, Address a2, Address a3);

/*******************************************************************************
 * FUNÇÕES DE GERAÇÃO DE CÓDIGO
 ******************************************************************************/

/**
 * generateCode: Gera código para um nó da AST.
 * 
 * Função recursiva que percorre a AST e gera quádruplas correspondentes.
 * Retorna o Address onde o resultado da expressão está armazenado.
 * 
 * PARÂMETROS:
 *   node - Nó da AST a processar
 * 
 * RETORNO:
 *   Address contendo o resultado (para expressões)
 *   emptyAddr() para comandos sem valor
 */
Address generateCode(ASTNode* node);

/**
 * generateProgram: Gera código para o programa completo.
 * 
 * Ponto de entrada para geração de código. Processa todas as declarações
 * no nível top do programa e adiciona HALT no final.
 * 
 * PARÂMETROS:
 *   tree - Raiz da AST (lista de declarações)
 */
void generateProgram(ASTNode* tree);

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO
 ******************************************************************************/

/**
 * printCode: Imprime o código intermediário no stdout.
 * 
 * Formato: (operador, op1, op2, resultado)
 */
void printCode();

/**
 * fprintCode: Imprime o código intermediário em um arquivo.
 * 
 * PARÂMETROS:
 *   out - Arquivo de saída (já aberto)
 */
void fprintCode(FILE* out);

#endif /* CGEN_H */
