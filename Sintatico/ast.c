#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

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

ASTNode* create_leaf_num(int value) {
    ASTNode* newNode = create_node(NODE_NUM, NULL, NULL);
    newNode->data.number = value;
    return newNode;
}

ASTNode* create_leaf_id(char* name) {
    ASTNode* newNode = create_node(NODE_VAR, NULL, NULL);
    newNode->data.identifier = name;
    return newNode;
}

ASTNode* create_leaf_type(int type_token) {
    ASTNode* newNode = create_node(NODE_TYPE, NULL, NULL);
    newNode->data.number = type_token;
    return newNode;
}

ASTNode* create_leaf_operator(int operator) {
    ASTNode* newNode = create_node(NODE_OPERATOR, NULL, NULL);
    newNode->data.number = operator;
    return newNode;
}

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

void print_ast(ASTNode* node, int level) {
    if (node == NULL) {
        return;
    }

    // Indentação mais compacta
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    // Impressão compacta do nó atual
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
            printf("NUM: %d\n", node->data.number);
            break;
        case NODE_VAR:
            printf("VAR: %s\n", node->data.identifier);
            break;
        case NODE_TYPE:
            printf("TYPE: %s\n", token_to_string(node->data.number));
            break;
        case NODE_OPERATOR:
            printf("OP: %s\n", token_to_string(node->data.number));
            break;
        case NODE_ARRAY_DECL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("ARRAY_DECL: %s", node->leftChild->data.identifier);
                if (node->rightChild && node->rightChild->type == NODE_NUM) {
                    printf("[%d]", node->rightChild->data.number);
                }
            } else {
                printf("ARRAY_DECL");
            }
            printf("\n");
            break;
        case NODE_ARRAY_ACCESS:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("ARRAY_ACCESS: %s\n", node->leftChild->data.identifier);
            } else {
                printf("ARRAY_ACCESS\n");
            }
            break;
        case NODE_PARAM: 
            printf("PARAM\n");
            break;
        case NODE_PARAM_LIST: 
            printf("PARAM_LIST\n");
            break;
        case NODE_FUN_CALL:
            if (node->leftChild && node->leftChild->type == NODE_VAR) {
                printf("FUN_CALL: %s\n", node->leftChild->data.identifier);
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

    // Recursão para filhos - mais compacta
    if (node->leftChild != NULL) {
        print_ast(node->leftChild, level + 1);
    }
    
    if (node->rightChild != NULL) {
        print_ast(node->rightChild, level + 1);
    }
    
    // Próximo nó
    if (node->next != NULL) {
        print_ast(node->next, level);
    }
}