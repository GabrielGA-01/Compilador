
#ifndef CGEN_H
#define CGEN_H

typedef enum {
    ADD, SUB, MUL, DIV,
    ASSIGN,
    LT, EQ, GT, IF_FALSE, GOTO,
    LABEL,
    PARAM, CALL, RETURN,
    HALTS
} QuadOp;

typedef enum {
    EMPTY,
    INT_CONST,
    STRING_VAR,
    TEMP_VAR,
    LABEL_KIND
} OperandKind;

typedef struct {
    OperandKind kind;
    int val;      /* Value if INT_CONST */
    char *name;   /* Name if VAR, TEMP, or LABEL */
} Address;

typedef struct Quad {
    QuadOp op;
    Address addr1;
    Address addr2;
    Address addr3;
    struct Quad *next;
} Quad;

void emit(QuadOp op, Address a1, Address a2, Address a3);
Address createVal(int val);
Address createVar(char *name);
Address createTemp(); /* creates a new temp and returns its address */
Address createLabel(); /* creates a new label and returns its address */

void printCode(); /* prints the list of quads */

#endif
