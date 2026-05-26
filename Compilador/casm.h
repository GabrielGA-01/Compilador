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

// Igual a tempControl
typedef struct regControl {
    Address reg;
    int sum;
    int safe;
    struct tempControl* next;
} regControl;

void generateAssembly(Quad* quadHead, FuncLabel* funHead, tempControl *tempControlHead);

#endif