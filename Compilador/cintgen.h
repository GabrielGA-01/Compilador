#ifndef CINTGEN_H
#define CINTGEN_H

#include "ast.h"

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_ASSIGN,
    OP_LT,
    OP_LET,
    OP_GT,
    OP_GET,
    OP_EQ,
    OP_DIF,
    OP_IFT,
    OP_JUMP,
    OP_LABEL,
    OP_IN,
    OP_OUT,
    OP_PARAM,
    OP_CALL,
    OP_RET,
    OP_HALT,
    OP_ARRAY_ACCESS,
    OP_ARRAY_ASSIGN,
    OP_FUN,
    OP_END_FUN,
    OP_ARG,
    OP_ARRAY_ARG,
    OP_LOAD,
    OP_STORE,
    OP_MOVI,
    OP_ALLOC,
    OP_ARRAY_ALLOC,
    NONE_OP
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

typedef struct TempStorage {
    struct TempStorage* next;
    Address* temp_addr;
    char* var_name;
    char* scope_name;
    int array_size;
} TempStorage;

int isArray(ASTNode* node);

Address determineVariableSize(ASTNode* node);
Address createEmptyAddr();
Address createNumericAddr(int val);
Address createStringAddr(char *name);
Address createLabelAddr();
Address* createTempAddr();
Address* searchCreateTemp(char* name, char* func_name, int array_size);

Address* determineType(ASTNode* current);
Address* determineName(ASTNode* current);

char* numberToType(int num);

Quad* makeNewQuad(QuadOp op, Address a1, Address a2, Address a3);
Address generateCode(ASTNode* tree, char* escopo, int mode);

void fprintCode(FILE* out);
void generateProgram(ASTNode* tree);

#endif