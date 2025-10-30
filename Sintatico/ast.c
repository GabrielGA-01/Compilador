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
    newNode->next = NULL;
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
    newNode->data.identifier = name;
    return newNode;
}

// Função para anexar um nó a uma lista (SOLUÇÃO DEFINITIVA PARA LISTAS)
ASTNode* append_node(ASTNode* list, ASTNode* new_node) {
    if (list == NULL) {
        return new_node;
    }
    
    ASTNode* current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
    return list;
}

// Função para traduzir tokens numéricos em strings legíveis
const char* token_to_string(int token) {
    switch(token) {
        case INT: return "INT";
        case VOID: return "VOID";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case LT: return "LT";
        case LET: return "LET";
        case GT: return "GT";
        case GET: return "GET";
        case EQ: return "EQ";
        case DIF: return "DIF";
        case ASSIGN: return "ASSIGN";
        default: {
            static char buffer[20];
            sprintf(buffer, "%d", token);
            return buffer;
        }
    }
}

// Function to print the AST (VERSÃO MELHORADA)
void print_ast(ASTNode* node, int level) {
    if (node == NULL) {
        return;
    }

    // Print indentation for the current level
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    // Print the node type with better representation
    switch (node->type) {
        case NODE_VAR_DECL: 
            printf("VAR_DECL\n");
            break;
        case NODE_FUN_DECL: 
            printf("FUN_DECL\n");
            break;
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
        case NODE_ASSIGN_EXPR: 
            printf("ASSIGN_EXPR\n");
            break;
        case NODE_BINARY_OP: 
            printf("BINARY_OP: %s\n", token_to_string(node->data.number));
            break;
        case NODE_NUM: 
            printf("NUM: %s\n", token_to_string(node->data.number));
            break;
        case NODE_VAR:
            printf("VAR: %s\n", node->data.identifier);
            break;
        case NODE_ARRAY_DECL:
            printf("ARRAY_DECL\n");
            break;
        case NODE_ARRAY_ACCESS:
            printf("ARRAY_ACCESS\n");
            break;
        case NODE_PARAM:
            printf("PARAM\n");
            break;
        case NODE_PARAM_LIST:
            printf("PARAM_LIST\n");
            break;
        case NODE_FUN_CALL:
            if (node->leftChild != NULL && node->leftChild->type == NODE_VAR) {
                printf("FUN_CALL: %s\n", node->leftChild->data.identifier);
            } else {
                printf("FUN_CALL\n");
            }
            break;
        default: 
            printf("UNKNOWN_NODE: %d\n", node->type);
            break;
    }

    // Recursively print children with better structure
    print_ast(node->leftChild, level + 1);
    print_ast(node->rightChild, level + 1);
    
    // Print next node (for lists)
    if (node->next != NULL) {
        print_ast(node->next, level);
    }
}