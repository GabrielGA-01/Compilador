#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Function to create a generic node
ASTNode* create_node(NodeType type, ASTNode* left, ASTNode* right) {
    ASTNode* newNode = (ASTNode*) malloc(sizeof(ASTNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    newNode->type = type;
    newNode->leftChild = left;
    newNode->rightChild = right;
    newNode->next = NULL; // Initialize next pointer to NULL
    return newNode;
}

// Function to create a leaf node for a number
ASTNode* create_leaf_num(int value) {
    ASTNode* newNode = create_node(NODE_NUM, NULL, NULL);
    newNode->data.number = value;
    return newNode;
}

// Function to create a leaf node for an identifier
ASTNode* create_leaf_id(char* name) {
    ASTNode* newNode = create_node(NODE_VAR, NULL, NULL);
    // Note: The name is already allocated by strdup in the scanner
    newNode->data.identifier = name;
    return newNode;
}

// Function to print the AST
void print_ast(ASTNode* node, int level) {
    if (node == NULL) {
        return;
    }

    // Print indentation for the current level
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    // Print the node type
    switch (node->type) {
        case NODE_VAR_DECL: printf("VAR_DECL\n"); break;
        case NODE_FUN_DECL: printf("FUN_DECL\n"); break;
        case NODE_COMPOUND_STMT: printf("COMPOUND_STMT\n"); break;
        case NODE_IF_STMT: printf("IF_STMT\n"); break;
        case NODE_WHILE_STMT: printf("WHILE_STMT\n"); break;
        case NODE_RETURN_STMT: printf("RETURN_STMT\n"); break;
        case NODE_ASSIGN_EXPR: printf("ASSIGN_EXPR\n"); break;
        case NODE_BINARY_OP: printf("BINARY_OP: %d\n", node->data.number); break;
        case NODE_NUM: printf("NUM: %d\n", node->data.number); break;
        case NODE_VAR:
            if (node->data.identifier != NULL) {
                printf("VAR: %s\n", node->data.identifier);
            } else {
                printf("VAR (Array)\n");
            }
            break;
        case NODE_FUN_CALL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("FUN_CALL: %s\n", node->leftChild->data.identifier);
            } else {
                printf("FUN_CALL\n");
            }
            break;
        default: printf("UNKNOWN_NODE\n"); break;
    }

    // Recursively print children and siblings
    print_ast(node->leftChild, level + 1);
    print_ast(node->rightChild, level + 1);
    print_ast(node->next, level);
}
