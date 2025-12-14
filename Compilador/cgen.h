#ifndef CGEN_H
#define CGEN_H

#include "ast.h"

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_ASN,
    OP_LT,
    OP_LET,
    OP_GT,
    OP_GET,
    OP_EQ,
    OP_DIF,
    OP_IF_F,
    OP_GOTO,
    OP_LAB,
    OP_RD,
    OP_WRI,
    OP_PARAM,
    OP_CALL,
    OP_RET,
    OP_HALT,
    OP_ARR_ACC,
    OP_ARR_ASN
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
    int val;
    char *name;
} Address;

typedef struct Quad {
    QuadOp op;
    Address addr1;
    Address addr2;
    Address addr3;
    struct Quad *next;
} Quad;

Address emptyAddr();
Address createVal(int val);
Address createVar(char *name);
Address createTemp();
Address createLabel();

void emit(QuadOp op, Address a1, Address a2, Address a3);
Address generateCode(ASTNode* node);
void generateProgram(ASTNode* tree);
void printCode();
void fprintCode(FILE* out);

#endif
