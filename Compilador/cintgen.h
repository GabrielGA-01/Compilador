#ifndef CINTGEN_H
#define CINTGEN_H

#include "ast.h"

typedef enum {
    OP_ADD,
    OP_ADDI,
    OP_ADDR,
    OP_SUB,
    OP_SUBI,
    OP_SUBR,
    OP_MUL,
    OP_MULI,
    OP_MULR,
    OP_DIV,
    OP_DIVI,
    OP_DIVR,
    OP_BLT,
    OP_BLE,
    OP_BGT,
    OP_BGE,
    OP_BEQ,
    OP_BNE,
    OP_IFF,
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
    OP_LOADDI,
    OP_LOADDR,
    OP_STORE,
    OP_STOREDI,
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
    int safeCall;
    int allocatedParam;
    Address* label;
    struct FuncLabel* next;
} FuncLabel;

typedef struct tempControl {
    Address temp;
    int sum;
    int safe;
    struct tempControl* next;
} tempControl;

int isArray(ASTNode* node);

Address determineVariableSize(ASTNode* node);
Address createEmptyAddr();
Address createNumericAddr(int val);
Address createStringAddr(char *name);
Address* createLabelAddr();
Address* createTempAddr();

char* numberToType(int num);
void update_or_insert(Address new_temp);

Quad* makeNewQuad(QuadOp op, Address a1, Address a2, Address a3);
Address generateCode(ASTNode* tree, char* escopo, int mode);

const char* opToString(QuadOp op);
void fprintAddr(FILE* out, Address a);
void fprintCode(FILE* out, Quad* head);
void generateProgram(ASTNode* tree);

#endif