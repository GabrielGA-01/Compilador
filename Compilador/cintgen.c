#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cintgen.h"

static int tempCount = 0;
static int labelCount = 0;

void generateProgram(ASTNode* tree){
    FILE* file = fopen("output/new_intermediate_code.txt", "w");
    ASTNode *current = tree; // Preserva a raiz

    while(current != NULL){
        switch(current->type) {
        
        }
        current = current->next;
    }

    printf("a");
    fclose(file);
}