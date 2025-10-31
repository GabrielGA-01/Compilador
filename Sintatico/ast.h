#ifndef AST_H
#define AST_H

#include "parser.tab.h"

// Enum atualizado com novos tipos de nós
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
    NODE_ARRAY_DECL,
    NODE_ARRAY_ACCESS,
    NODE_PARAM,
    NODE_PARAM_LIST,
    NODE_TYPE,           // NOVO: para tipos (INT, VOID)
    NODE_OPERATOR,       // NOVO: para operadores relacionais
    NODE_FUN_BODY        // NOVO: corpo de função
} NodeType;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode *leftChild;
    struct ASTNode *rightChild;
    struct ASTNode *next;
    union {
        int number;
        char* identifier;
    } data;
} ASTNode;

// Protótipos atualizados
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right);
ASTNode* create_leaf_num(int value);
ASTNode* create_leaf_id(char* name);
ASTNode* create_leaf_type(int type_token);     // NOVA
ASTNode* create_leaf_operator(int operator);   // NOVA
void print_ast(ASTNode* node, int level);
const char* token_to_string(int token);
ASTNode* append_node(ASTNode* list, ASTNode* new_node);

#endif