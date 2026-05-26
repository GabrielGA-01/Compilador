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

typedef struct regControl {
    Address reg;
    char* labelName;
    int sum;
    int safe;
} regControl;

void generateAssembly(Quad* quadHead, FuncLabel* funHead, tempControl *tempControlHead);

#endif