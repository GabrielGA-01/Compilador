#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cintgen.h"

static int tempCount = 0;
static int labelCount = 0;

Quad *head = NULL;
Quad *tail = NULL;
TempStorage* TSHead = NULL;

// Para indicar um operando sem uso
Address emptyAddr() {
    Address a;
    a.kind = EMPTY;
    a.val = 0;
    a.name = NULL;
    return a;
}

// Para criar um operando numérico
Address createVal(int val) {
    Address a;
    a.kind = INT_CONST;
    a.val = val;
    a.name = NULL;
    return a;
}

// Para criar um operando string
Address createVar(char *name) {
    Address a;
    a.kind = STRING_VAR;
    a.name = strdup(name);
    return a;
}

// Para criar um novo label
Address createLabel() {
    Address a;
    char buffer[20];
    sprintf(buffer, "L%d", labelCount++);
    a.kind = LABEL_KIND;
    a.name = strdup(buffer);
    return a;
}

// Para criar uma nova variável
Address* createTemp() {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    sprintf(buffer, "t%d", tempCount++);
    a->kind = TEMP_VAR;
    a->name = strdup(buffer);
    
    return a;
}

// Tenta buscar se o temp já existe
Address* searchTemp(char* name, char* scope_name){
    TempStorage* current = TSHead;
    while(current != NULL){
        if(strcmp(current->var_name, name) == 0 && strcmp(current->scope_name, scope_name) == 0){
            return(current->temp_addr);
        }
        current = current->next;
    }

    Address* new_temp = createTemp();

    TempStorage* new_storage = (TempStorage *)malloc(sizeof(TempStorage));
    new_storage->var_name = name;
    new_storage->scope_name = scope_name;
    new_storage->temp_addr = new_temp;
    new_storage->next = TSHead;
    TSHead = new_storage;

    return(new_temp);
}

// Já existe essa função em ast.c, porém precisa ser em letra minúscula o retorno
char* numberToType(int num) {
    switch(num) {
        case INT: return "int";
        case VOID: return "void";
    }
}

// Forma novas quádruplas e salva usando os ponteiros head e tail
Quad* makeNewQuad(QuadOp op, Address a1, Address a2, Address a3) {
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

    return q;
}

// Traduz o código da operação para uma string
const char* opToString(QuadOp op) {
    switch(op) {
        case OP_ADD:     return "add";
        case OP_SUB:     return "sub";
        case OP_MUL:     return "mul";
        case OP_DIV:     return "div";
        case OP_ASN:     return "asn";
        case OP_LT:      return "lt";
        case OP_LET:     return "let";
        case OP_GT:      return "gt";
        case OP_GET:     return "get";
        case OP_EQ:      return "eq";
        case OP_DIF:     return "dif";
        case OP_IFF:     return "iff";
        case OP_GOTO:    return "goto";
        case OP_LAB:     return "lab";
        case OP_IN:      return "in";
        case OP_OUT:     return "out";
        case OP_PARAM:   return "param";
        case OP_CALL:    return "call";
        case OP_RET:     return "ret";
        case OP_HALT:    return "halt";
        case OP_ARRAY_ACCESS: return "araccess";
        case OP_ARRAY_ASSIGN: return "arassign";
        case OP_FUN:     return "fun";
        case OP_ARG:     return "arg";
        case OP_LOAD:    return "load";
        default:         return "unknown";
    }
}

// Extrai corretamente o tipo do operando e escreve ele
void fprintAddr(FILE* out, Address a) {
    switch(a.kind) {
        case EMPTY: fprintf(out, "_"); break;
        case INT_CONST: fprintf(out, "%d", a.val); break;
        case STRING_VAR: fprintf(out, "%s", a.name); break;
        case TEMP_VAR: fprintf(out, "%s", a.name); break;
        case LABEL_KIND: fprintf(out, "%s", a.name); break;
    }
}

// Organiza a escrita das quádruplas no arquivo
void fprintCode(FILE* out) {
    Quad *current = head;
    while(current != NULL) {
        fprintf(out, "(%s, ", opToString(current->op));
        fprintAddr(out, current->addr1);
        fprintf(out, ", ");
        fprintAddr(out, current->addr2);
        fprintf(out, ", ");
        fprintAddr(out, current->addr3);
        fprintf(out, ")\n");
        current = current->next;
    }
}

Data* generateCode(ASTNode* node, char* scope){
    Data* none = NULL;
    if(node == NULL) return none;

    switch (node->type){
    // Caso seja uma função
    case NODE_FUN_DECL:
        char* func_data_type = numberToType(node->leftChild->number);
        char* func_name = node->rightChild->identifier;

        Address addr_func_data_type = createVar(func_data_type);
        Address addr_func_name = createVar(func_name);
        makeNewQuad(OP_FUN, addr_func_data_type, addr_func_name, emptyAddr());
        
        // Verifica as partes da função
        
        // Verifica os parâmetros da função (se houver)
        ASTNode* fun_body = node->next;
        if(fun_body != NULL && fun_body->type == NODE_FUN_BODY) generateCode(fun_body, func_name);
        break;
    
    case NODE_FUN_BODY:
        // Verificação dos parâmetros
        if(node->leftChild != NULL && node->leftChild->type == NODE_PARAM) generateCode(node->leftChild, scope);
        // Verificação dos componetes da função
        if(node->rightChild != NULL && node->rightChild->type == NODE_COMPOUND_STMT) generateCode(node->rightChild, scope);
        break;

    case NODE_PARAM:
        char* param_data_type = numberToType(node->leftChild->number);
        char* param_name = node->rightChild->identifier;      
        
        // Faz a declaração
        Address addr_param_name = createVar(param_name);
        Address addr_param_data_type = createVar(param_data_type);
        makeNewQuad(OP_ARG, addr_param_data_type, addr_param_name, emptyAddr());

        // Carrega para um registrador
        Address* temp_name = searchTemp(param_name, scope);
        makeNewQuad(OP_LOAD, *temp_name, addr_param_name, emptyAddr());
        
        // Vai para os irmãos
        if(node->next != NULL) generateCode(node->next, scope);
        break;

    // case NODE_IF_STMT:
    //     break;
    // case NODE_OPERATOR:
    //     // Identifica o tipo de operação
    //     QuadOp operation;

        

    //     // Acessa o filho da esquerda e obtém o valor
    //     int num = node->number;

    //     // Decodifica a operação
    //     switch(num) {
    //         case 260: operation = OP_ADD; break;
    //         case 261: operation = OP_SUB; break;
    //         case 262: operation = OP_MUL; break;
    //         case 263: operation = OP_DIV; break;
    //         case 264: operation = OP_LT;  break;
    //         case 265: operation = OP_LET; break;
    //         case 266: operation = OP_GET; break;
    //         case 267: operation = OP_GT;  break;
    //         case 268: operation = OP_EQ;  break;
    //         case 269: operation = OP_DIF; break;
    //         default:  operation = OP_ADD; break;
    //     }

    default:
        break;
    }
        
    return none;
}

void generateProgram(ASTNode* tree){
    ASTNode *current = tree;
    
    // Percorre todos os nós irmãos da raíz
    while(current != NULL){
        if(current->type != NODE_FUN_BODY) generateCode(current, "Global");
        current = current->next;
    }
    
    FILE* file = fopen("output/new_intermediate_code.txt", "w");
    fprintCode(file);
    fclose(file);
}