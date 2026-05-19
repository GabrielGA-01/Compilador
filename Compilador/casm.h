#ifndef CASM_H
#define CASM_H

#include "cintgen.h"

typedef struct variables {
    char* name;
    int offset;
    int size;
    struct variables* next;
} variables;

typedef struct variablesAtStack {
    char* scope;
    variables* var;
    struct variablesAtStack* next; 
} variablesAtStack;


void generateAssembly(Quad* quadHead, FuncLabel* funHead);

#endif