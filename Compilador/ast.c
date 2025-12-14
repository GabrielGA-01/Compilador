/*******************************************************************************
 * Arquivo: ast.c
 * 
 * IMPLEMENTAÇÃO DA ÁRVORE SINTÁTICA ABSTRATA (AST)
 * 
 * Este arquivo contém as implementações das funções para criar, manipular e
 * imprimir a AST. A AST é a estrutura de dados central do compilador que
 * representa o programa fonte de forma estruturada.
 * 
 * FLUXO DE DADOS:
 * Scanner → Parser → [AST] → Análise Semântica → Geração de Código
 * 
 * O parser chama as funções create_* durante a análise sintática para
 * construir a árvore. As funções print_* são usadas para debug e visualização.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/*******************************************************************************
 * FUNÇÕES DE CRIAÇÃO DE NÓS
 * 
 * Estas funções alocam memória para novos nós da AST e inicializam seus
 * campos apropriadamente.
 ******************************************************************************/

/**
 * create_node
 * 
 * Cria um novo nó genérico da AST com dois filhos especificados.
 * Esta é a função base usada por outras funções de criação.
 * 
 * PARÂMETROS:
 *   type  - Tipo do nó (define o significado semântico)
 *   left  - Ponteiro para o filho esquerdo (pode ser NULL)
 *   right - Ponteiro para o filho direito (pode ser NULL)
 * 
 * RETORNO:
 *   Ponteiro para o nó recém-criado
 * 
 * OBSERVAÇÕES:
 *   - A função termina o programa se não conseguir alocar memória
 *   - O número da linha é obtido automaticamente da variável global yylineno
 *   - O tipo de expressão é inicializado como Void (será definido na análise)
 */
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right) {
    /* Aloca memória para o novo nó */
    ASTNode* newNode = malloc(sizeof(ASTNode));
    
    /* Inicializa os campos do nó */
    newNode->type = type;           /* Tipo do nó */
    newNode->expType = Void;        /* Tipo de expressão (será preenchido depois) */
    newNode->leftChild = left;      /* Filho esquerdo */
    newNode->rightChild = right;    /* Filho direito */
    newNode->next = NULL;           /* Próximo irmão (lista) */
    
    /* Obtém o número da linha atual do scanner */
    extern int yylineno;
    newNode->lineno = yylineno;
    
    return newNode;
}

/**
 * create_leaf_num
 * 
 * Cria um nó folha para representar um literal numérico.
 * 
 * PARÂMETROS:
 *   value - Valor inteiro do número literal
 * 
 * RETORNO:
 *   Nó do tipo NODE_NUM com o valor armazenado
 * 
 * EXEMPLO:
 *   Para "42" no código fonte, cria NODE_NUM com number=42
 */
ASTNode* create_leaf_num(int value) {
    /* Cria nó sem filhos */
    ASTNode* newNode = create_node(NODE_NUM, NULL, NULL);
    /* Armazena o valor numérico */
    newNode->number = value;
    return newNode;
}

/**
 * create_leaf_id
 * 
 * Cria um nó folha para representar um identificador (nome de variável/função).
 * 
 * PARÂMETROS:
 *   name - String com o nome do identificador (já deve ser uma cópia - strdup)
 * 
 * RETORNO:
 *   Nó do tipo NODE_VAR com o nome armazenado
 * 
 * OBSERVAÇÃO:
 *   O caller é responsável por fazer strdup do nome antes de passar.
 *   Isso porque yytext (buffer do scanner) é sobrescrito a cada token.
 */
ASTNode* create_leaf_id(char* name) {
    /* Cria nó sem filhos */
    ASTNode* newNode = create_node(NODE_VAR, NULL, NULL);
    /* Armazena o nome do identificador */
    newNode->identifier = name;
    return newNode;
}

/**
 * create_leaf_type
 * 
 * Cria um nó folha para representar um especificador de tipo.
 * 
 * PARÂMETROS:
 *   type_token - Token do tipo (INT=274 ou VOID=276, valores do parser)
 * 
 * RETORNO:
 *   Nó do tipo NODE_TYPE com o token armazenado
 * 
 * EXEMPLO:
 *   Para "int" cria NODE_TYPE com number=INT
 *   Para "void" cria NODE_TYPE com number=VOID
 */
ASTNode* create_leaf_type(int type_token) {
    ASTNode* newNode = create_node(NODE_TYPE, NULL, NULL);
    newNode->number = type_token;
    return newNode;
}

/**
 * create_leaf_operator
 * 
 * Cria um nó folha para representar um operador relacional.
 * 
 * PARÂMETROS:
 *   operator - Token do operador (LT, GT, EQ, DIF, LET, GET)
 * 
 * RETORNO:
 *   Nó do tipo NODE_OPERATOR com o token armazenado
 * 
 * USO:
 *   Usado em expressões relacionais como "x < y", "a == b"
 *   O operador é posteriormente movido para ser a raiz da subárvore
 */
ASTNode* create_leaf_operator(int operator) {
    ASTNode* newNode = create_node(NODE_OPERATOR, NULL, NULL);
    newNode->number = operator;
    return newNode;
}

/*******************************************************************************
 * FUNÇÕES DE MANIPULAÇÃO DE LISTAS
 ******************************************************************************/

/**
 * append_node
 * 
 * Adiciona um nó ao final de uma lista encadeada de irmãos.
 * 
 * Na AST, listas (como lista de declarações, lista de parâmetros) são
 * implementadas através do campo 'next' de cada nó.
 * 
 * PARÂMETROS:
 *   list     - Início da lista existente (pode ser NULL)
 *   new_node - Nó a ser adicionado ao final
 * 
 * RETORNO:
 *   Ponteiro para o início da lista (pode ser new_node se list era NULL)
 * 
 * EXEMPLO:
 *   Dada a lista:  [A] -> [B] -> NULL
 *   Chamando:      append_node(A, C)
 *   Resultado:     [A] -> [B] -> [C] -> NULL
 * 
 * COMPLEXIDADE:
 *   O(n) onde n é o tamanho da lista (precisa percorrer até o final)
 */
ASTNode* append_node(ASTNode* list, ASTNode* new_node) {
    /* Caso base: lista vazia */
    if (list == NULL) {
        return new_node;  /* O novo nó é o início da lista */
    }
    
    /* Percorre até o último nó da lista */
    ASTNode* current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    
    /* Adiciona o novo nó ao final */
    current->next = new_node;
    
    return list;  /* Retorna o início inalterado */
}

/*******************************************************************************
 * FUNÇÕES AUXILIARES DE IMPRESSÃO
 ******************************************************************************/

/**
 * token_to_string
 * 
 * Converte um código de token numérico para sua representação em string.
 * Usado para tornar a saída de debug mais legível.
 * 
 * PARÂMETROS:
 *   token - Código numérico do token (constantes do parser)
 * 
 * RETORNO:
 *   String representando o token de forma legível
 * 
 * OBSERVAÇÃO:
 *   Usa um buffer estático para tokens desconhecidos - NÃO é thread-safe
 */
const char* token_to_string(int token) {
    switch(token) {
        /* Tipos de dados */
        case INT: return "INT";      /* Tipo inteiro */
        case VOID: return "VOID";    /* Tipo void */
        
        /* Operadores aritméticos */
        case ADD: return "ADD";      /* + (adição) */
        case SUB: return "SUB";      /* - (subtração) */
        case MUL: return "MUL";      /* * (multiplicação) */
        case DIV: return "DIV";      /* / (divisão) */
        
        /* Operadores relacionais */
        case LT: return "LT";        /* < (menor que) */
        case LET: return "LET";      /* <= (menor ou igual) */
        case GT: return "GT";        /* > (maior que) */
        case GET: return "GET";      /* >= (maior ou igual) */
        case EQ: return "EQ";        /* == (igual) */
        case DIF: return "DIF";      /* != (diferente) */
        
        /* Outros */
        case ASSIGN: return "ASSIGN"; /* = (atribuição) */
        
        /* Token desconhecido - retorna valor numérico */
        default: {
            static char buffer[20];  /* Buffer estático - cuidado! */
            sprintf(buffer, "%d", token);
            return buffer;
        }
    }
}

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO DA AST (STDOUT)
 ******************************************************************************/

/**
 * print_ast
 * 
 * Imprime a AST de forma hierárquica e indentada no stdout.
 * Cada nível da árvore é indentado com 2 espaços adicionais.
 * 
 * PARÂMETROS:
 *   node  - Nó raiz da subárvore a imprimir
 *   level - Nível atual de indentação (0 para a raiz)
 * 
 * ESTRATÉGIA DE TRAVESSIA:
 *   1. Pre-order: Imprime o nó atual
 *   2. Recursão: Visita filho esquerdo, depois direito
 *   3. Continua: Visita próximo irmão (lista)
 * 
 * EXEMPLO DE SAÍDA:
 *   FUN_DECL
 *     TYPE: INT
 *     VAR: main
 *     FUN_BODY
 *       COMPOUND_STMT
 *         ...
 */
void print_ast(ASTNode* node, int level) {
    /* Caso base: nó nulo */
    if (node == NULL) {
        return;
    }

    /* Imprime indentação (2 espaços por nível) */
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    /* Imprime o nó de acordo com seu tipo */
    switch (node->type) {
        /* Declarações */
        case NODE_VAR_DECL: 
            printf("VAR_DECL\n");
            break;
        case NODE_FUN_DECL: 
            printf("FUN_DECL\n");
            break;
            
        /* Comandos */
        case NODE_COMPOUND_STMT: 
            printf("COMPOUND_STMT\n");
            break;
        case NODE_IF_STMT: 
            printf("IF_STMT\n");
            break;
        case NODE_WHILE_STMT: 
            printf("WHILE_STMT\n");
            break;
        case NODE_RETURN_STMT: 
            printf("RETURN_STMT\n");
            break;
            
        /* Expressões */
        case NODE_ASSIGN_EXPR: 
            printf("ASSIGN_EXPR\n");
            break;
        case NODE_BINARY_OP: 
            printf("BINARY_OP: %s\n", token_to_string(node->number));
            break;
        case NODE_NUM: 
            printf("NUM: %d\n", node->number);
            break;
        case NODE_VAR:
            printf("VAR: %s\n", node->identifier);
            break;
            
        /* Auxiliares */
        case NODE_TYPE:
            printf("TYPE: %s\n", token_to_string(node->number));
            break;
        case NODE_OPERATOR:
            printf("OP: %s\n", token_to_string(node->number));
            break;
            
        /* Arrays */
        case NODE_ARRAY_DECL:
            /* Imprime nome e tamanho do array se disponíveis */
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("ARRAY_DECL: %s", node->leftChild->identifier);
                if (node->rightChild && node->rightChild->type == NODE_NUM) {
                    printf("[%d]", node->rightChild->number);
                }
            } else {
                printf("ARRAY_DECL");
            }
            printf("\n");
            break;
        case NODE_ARRAY_ACCESS:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("ARRAY_ACCESS: %s\n", node->leftChild->identifier);
            } else {
                printf("ARRAY_ACCESS\n");
            }
            break;
            
        /* Parâmetros */
        case NODE_PARAM: 
            printf("PARAM\n");
            break;
        case NODE_PARAM_LIST: 
            printf("PARAM_LIST\n");
            break;
            
        /* Funções */
        case NODE_FUN_CALL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("FUN_CALL: %s\n", node->leftChild->identifier);
            } else {
                printf("FUN_CALL\n");
            }
            break;
        case NODE_FUN_BODY: 
            printf("FUN_BODY\n");
            break;
            
        default: 
            printf("UNKNOWN(%d)\n", node->type);
            break;
    }

    /* Recursão para os filhos (próximo nível de indentação) */
    if (node->leftChild != NULL) {
        print_ast(node->leftChild, level + 1);
    }
    
    if (node->rightChild != NULL) {
        print_ast(node->rightChild, level + 1);
    }
    
    /* Continua para o próximo irmão (mesmo nível de indentação) */
    if (node->next != NULL) {
        print_ast(node->next, level);
    }
}

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO DA AST (ARQUIVO)
 ******************************************************************************/

/**
 * fprint_ast
 * 
 * Versão da print_ast que escreve em um arquivo ao invés do stdout.
 * Idêntica em funcionalidade, mas usa fprintf ao invés de printf.
 * 
 * PARÂMETROS:
 *   out   - Ponteiro para o arquivo de saída (já aberto)
 *   node  - Nó raiz da subárvore a imprimir
 *   level - Nível atual de indentação (0 para a raiz)
 * 
 * USO TÍPICO:
 *   FILE* f = fopen("output/ast.txt", "w");
 *   fprint_ast(f, root, 0);
 *   fclose(f);
 */
void fprint_ast(FILE* out, ASTNode* node, int level) {
    if (node == NULL) {
        return;
    }

    /* Indentação */
    for (int i = 0; i < level; i++) {
        fprintf(out, "  ");
    }

    /* Imprime o nó de acordo com seu tipo */
    switch (node->type) {
        case NODE_VAR_DECL: 
            fprintf(out, "VAR_DECL\n");
            break;
        case NODE_FUN_DECL: 
            fprintf(out, "FUN_DECL\n");
            break;
        case NODE_COMPOUND_STMT: 
            fprintf(out, "COMPOUND_STMT\n");
            break;
        case NODE_IF_STMT: 
            fprintf(out, "IF_STMT\n");
            break;
        case NODE_WHILE_STMT: 
            fprintf(out, "WHILE_STMT\n");
            break;
        case NODE_RETURN_STMT: 
            fprintf(out, "RETURN_STMT\n");
            break;
        case NODE_ASSIGN_EXPR: 
            fprintf(out, "ASSIGN_EXPR\n");
            break;
        case NODE_BINARY_OP: 
            fprintf(out, "BINARY_OP: %s\n", token_to_string(node->number));
            break;
        case NODE_NUM: 
            fprintf(out, "NUM: %d\n", node->number);
            break;
        case NODE_VAR:
            fprintf(out, "VAR: %s\n", node->identifier);
            break;
        case NODE_TYPE:
            fprintf(out, "TYPE: %s\n", token_to_string(node->number));
            break;
        case NODE_OPERATOR:
            fprintf(out, "OP: %s\n", token_to_string(node->number));
            break;
        case NODE_ARRAY_DECL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                fprintf(out, "ARRAY_DECL: %s", node->leftChild->identifier);
                if (node->rightChild && node->rightChild->type == NODE_NUM) {
                    fprintf(out, "[%d]", node->rightChild->number);
                }
            } else {
                fprintf(out, "ARRAY_DECL");
            }
            fprintf(out, "\n");
            break;
        case NODE_ARRAY_ACCESS:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                fprintf(out, "ARRAY_ACCESS: %s\n", node->leftChild->identifier);
            } else {
                fprintf(out, "ARRAY_ACCESS\n");
            }
            break;
        case NODE_PARAM: 
            fprintf(out, "PARAM\n");
            break;
        case NODE_PARAM_LIST: 
            fprintf(out, "PARAM_LIST\n");
            break;
        case NODE_FUN_CALL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                fprintf(out, "FUN_CALL: %s\n", node->leftChild->identifier);
            } else {
                fprintf(out, "FUN_CALL\n");
            }
            break;
        case NODE_FUN_BODY: 
            fprintf(out, "FUN_BODY\n");
            break;
        default: 
            fprintf(out, "UNKNOWN(%d)\n", node->type);
            break;
    }

    /* Recursão para filhos */
    if (node->leftChild != NULL) {
        fprint_ast(out, node->leftChild, level + 1);
    }
    
    if (node->rightChild != NULL) {
        fprint_ast(out, node->rightChild, level + 1);
    }
    
    /* Próximo irmão */
    if (node->next != NULL) {
        fprint_ast(out, node->next, level);
    }
}