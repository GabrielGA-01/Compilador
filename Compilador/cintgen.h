#ifndef CINTGEN_H
#define CINTGEN_H

#include "ast.h"

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_LET,
    OP_GT,
    OP_GET,
    OP_EQ,
    OP_DIF,
    OP_IFT,
    OP_JUMP,
    OP_JR,
    OP_LABEL,
    OP_IN,
    OP_OUT,
    OP_PARAM,
    OP_CALL,
    OP_RET,
    OP_HALT,
    OP_FUN,
    OP_END_FUN,
    OP_ARG,
    OP_LOAD,
    OP_LOADD,
    OP_LOADDR,
    OP_STORE,
    OP_STORED,
    OP_STOREDR,
    OP_MOV,
    OP_MOVI,
    OP_MOVR,
    OP_ALLOC,
    OP_FREE,
    NONE_OP
} QuadOp;

typedef enum {
    EMPTY,
    INT_CONST,
    STRING_VAR,
    TEMP_VAR,
    LABEL_KIND,
    REGISTER_KIND
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

typedef struct FuncLabel {
    char* name;
    Address* label;
    struct FuncLabel* next;
} FuncLabel;

int isArray(ASTNode* node);

Address determineVariableSize(ASTNode* node);
Address createEmptyAddr();
Address createNumericAddr(int val);
Address createStringAddr(char *name);
Address* createLabelAddr();
Address* createTempAddr();

char* numberToType(int num);

Quad* makeNewQuad(QuadOp op, Address a1, Address a2, Address a3);
Address generateCode(ASTNode* tree, char* escopo, int mode);

void fprintCode(FILE* out, Quad* head);
void generateProgram(ASTNode* tree);

#endif