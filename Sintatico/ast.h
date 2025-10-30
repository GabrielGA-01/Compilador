#ifndef AST_H
#define AST_H

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
    NODE_FUN_CALL
    // ... other types of nodes you may need
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
        // ... other information, such as variable type, operator, etc.
    } data;
} ASTNode;

// Functions to create nodes (prototypes)
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right);
ASTNode* create_leaf_num(int value);
ASTNode* create_leaf_id(char* name);
void print_ast(ASTNode* node, int level);
// ... other helper functions

#endif
