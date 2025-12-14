
#ifndef CGEN_H
#define CGEN_H

#include "ast.h"

/* Operadores das quádruplas - formato do slide */
typedef enum {
    OP_ADD,      /* add */
    OP_SUB,      /* sub */
    OP_MUL,      /* mul */
    OP_DIV,      /* div */
    OP_ASN,      /* asn (assign) */
    OP_LT,       /* lt (less than) */
    OP_LET,      /* let (less or equal) */
    OP_GT,       /* gt (greater than) */
    OP_GET,      /* get (greater or equal) */
    OP_EQ,       /* eq (equal) */
    OP_DIF,      /* dif (not equal) */
    OP_IF_F,     /* if_f (if false) */
    OP_GOTO,     /* goto */
    OP_LAB,      /* lab (label) */
    OP_RD,       /* rd (read/input) */
    OP_WRI,      /* wri (write/output) */
    OP_PARAM,    /* param */
    OP_CALL,     /* call */
    OP_RET,      /* ret (return) */
    OP_HALT,     /* halt */
    OP_ARR_ACC,  /* array access */
    OP_ARR_ASN   /* array assign */
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

/* Funções para criar endereços */
void emit(QuadOp op, Address a1, Address a2, Address a3);
Address createVal(int val);
Address createVar(char *name);
Address createTemp();
Address createLabel();
Address emptyAddr();

/* Geração de código intermediário */
Address generateCode(ASTNode* node);
void generateProgram(ASTNode* tree);

/* Impressão do código */
void printCode();
void fprintCode(FILE* out);

#endif

