#ifndef AST_H
#define AST_H

#include "parser.tab.h" // Inclui as definições de token do Bison

// Enum to identify the type of node
typedef enum {
    NODE_VAR_DECL,
    NODE_FUN_DECL,
    NODE_COMPOUND_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    NODE_ASSIGN_EXPR,
    NODE_BINARY_OP,
    NODE_NUM,
    NODE_VAR,
    NODE_FUN_CALL,
    NODE_ARRAY_DECL,      // Novo: declaração de array
    NODE_ARRAY_ACCESS,    // Novo: acesso a array
    NODE_PARAM,           // Novo: parâmetro de função
    NODE_PARAM_LIST       // Novo: lista de parâmetros
} NodeType;

// Generic structure for an AST node
typedef struct ASTNode {
    NodeType type;
    struct ASTNode *leftChild;
    struct ASTNode *rightChild;
    // Sibling for lists (e.g., list of statements, list of parameters)
    struct ASTNode *next;
    // Node-specific data
    union {
        int number;
        char* identifier;
    } data;
} ASTNode;

// Functions to create nodes (prototypes)
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right);
ASTNode* create_leaf_num(int value);
ASTNode* create_leaf_id(char* name);
void print_ast(ASTNode* node, int level);
const char* token_to_string(int token);  // Nova função para tokens legíveis
ASTNode* append_node(ASTNode* list, ASTNode* new_node);  // Nova função para listas

#endif
