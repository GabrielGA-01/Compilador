
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgen.h"

Quad *head = NULL;
Quad *tail = NULL;

static int tempCount = 0;
static int labelCount = 0;

Address createVal(int val) {
    Address a;
    a.kind = INT_CONST;
    a.val = val;
    a.name = NULL;
    return a;
}

Address createVar(char *name) {
    Address a;
    a.kind = STRING_VAR;
    a.name = strdup(name);
    return a;
}

Address createTemp() {
    Address a;
    char buffer[20];
    sprintf(buffer, "t%d", ++tempCount);
    a.kind = TEMP_VAR;
    a.name = strdup(buffer);
    return a;
}

Address createLabel() {
    Address a;
    char buffer[20];
    sprintf(buffer, "L%d", ++labelCount);
    a.kind = LABEL_KIND;
    a.name = strdup(buffer);
    return a;
}

void emit(QuadOp op, Address a1, Address a2, Address a3) {
    Quad *q = (Quad *)malloc(sizeof(Quad));
    q->op = op;
    q->addr1 = a1;
    q->addr2 = a2;
    q->addr3 = a3;
    q->next = NULL;

    if (head == NULL) {
        head = tail = q;
    } else {
        tail->next = q;
        tail = q;
    }
}

const char* opToString(QuadOp op) {
    switch(op) {
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case ASSIGN: return "ASSIGN";
        case LT: return "LT";
        case EQ: return "EQ";
        case GT: return "GT";
        case IF_FALSE: return "IFFALSE";
        case GOTO: return "GOTO";
        case LABEL: return "LABEL";
        case PARAM: return "PARAM";
        case CALL: return "CALL";
        case RETURN: return "RETURN";
        case HALTS: return "HALT";
        default: return "UNKNOWN";
    }
}

void printAddr(Address a) {
    switch(a.kind) {
        case EMPTY: printf("_"); break;
        case INT_CONST: printf("%d", a.val); break;
        case STRING_VAR: printf("%s", a.name); break;
        case TEMP_VAR: printf("%s", a.name); break;
        case LABEL_KIND: printf("%s", a.name); break;
    }
}

void printCode() {
    Quad *current = head;
    while(current != NULL) {
        if (current->op == LABEL) {
            printAddr(current->addr1);
            printf(":\n");
        } else {
            printf("(%s, ", opToString(current->op));
            printAddr(current->addr1);
            printf(", ");
            printAddr(current->addr2);
            printf(", ");
            printAddr(current->addr3);
            printf(")\n");
        }
        current = current->next;
    }
}
