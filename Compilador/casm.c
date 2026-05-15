#include <stdlib.h>
#include "casm.h"
#include "cintgen.h"
#include "string.h"

#define RPI_Virtual 61
#define MEM_SIZE 63

Address* searchFuncLabel(char* name, FuncLabel* funHead){
    FuncLabel* current = funHead;
    while(current != NULL){
        if(strcmp(name, current->name) == 0) return(current->label);
        current = current->next;
    }
    return NULL;
}

Quad* insertQuadAfter(Quad* target, QuadOp op, Address a1, Address a2, Address a3) {
    if (target == NULL) {
        return NULL; 
    }

    Quad *q = (Quad *)malloc(sizeof(Quad));
    q->op = op;
    q->addr1 = a1;
    q->addr2 = a2;
    q->addr3 = a3;

    q->next = target->next;
    target->next = q;

    return q;
}

// Para criar um novo registrador
Address* createRegisterAddr(int number) {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    
    sprintf(buffer, "R%d", number);
    
    a->kind = REGISTER_KIND;
    a->name = strdup(buffer);
    a->val = number;
    return a;
}

void generateAssembly(Quad* quadHead, FuncLabel* funHead){
    // Prepara o registrador com pilha virtual
    Address* RPI = createRegisterAddr(RPI_Virtual);
    Quad* start = (Quad *)malloc(sizeof(Quad));
    start->op = OP_MOVI;
    start->addr1 = *RPI;
    start->addr2 = createNumericAddr(MEM_SIZE);
    start->addr3 = createEmptyAddr();
    start->next = quadHead;
    quadHead = start;

    // Jump para main
    insertQuadAfter(start, OP_JUMP, *searchFuncLabel("main", funHead), createEmptyAddr(), createEmptyAddr());


    Quad* current = start;
    Quad* before = NULL;
    
    char *scope;
    int isAlloc = 0;
    while(current != NULL){
        if(current->op != OP_ARG) isAlloc = 0;

        switch (current->op)
        {
        case OP_FUN:
            scope = current->addr2.name;
            isAlloc = 1;

            // if(before != NULL) before->next = current->next;
            // current = NULL;
            break;
        case OP_ARG:
            

        default:
            break;
        }

        if(current == NULL && before != NULL) current = before->next;
        else{
            before = current;
            current = current->next;
        }
    }

    FILE* file = fopen("output/assembly_code.txt", "w");
    fprintCode(file, quadHead);
    fclose(file);
}