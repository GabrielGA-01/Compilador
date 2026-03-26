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
Address* search_temp(char* name, char* func_name){
    TempStorage* current = TSHead;
    while(current != NULL){
        if(strcmp(current->temp_addr->name, name) && strcmp(current->func_name, func_name)){
             return(current->temp_addr);
        }
        current = current->next;
    }

    Address* new_temp = createTemp();

    TempStorage* new_storage = (TempStorage *)malloc(sizeof(TempStorage));
    new_storage->func_name = func_name;
    new_storage->temp_addr = new_temp;
    new_storage->next = TSHead;
    TSHead = new_storage;

    return(new_temp);
}

// Já existe essa função em ast.c, porém precisa ser em letra minúscula o retorno
char* number_to_type(int num) {
    switch(num) {
        case INT: return "int";
        case VOID: return "void";
    }
}

// Forma novas quádruplas e salva usando os ponteiros head e tail
Quad* make_new_quad(QuadOp op, Address a1, Address a2, Address a3) {
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
        case OP_IFF:    return "iff";
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
        case OP_FUN: return "fun";
        case OP_ARG: return "arg";
        case OP_LOAD: return "load";
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

Address* determine_type(ASTNode* current){
    Address* d_type = (Address *)malloc(sizeof(Address));
    *d_type = emptyAddr();
    // Olha para esquerda para ver o tipo
    if(current->leftChild->type == NODE_TYPE){
        char* type = number_to_type(current->leftChild->number);
        *d_type = createVar(type);
    }
    else printf("Deu ruim - Tipo não encontrado");

    return d_type;
}

Address* determine_name(ASTNode* current){
    Address* d_name = (Address *)malloc(sizeof(Address));
    *d_name = emptyAddr();
    // Olha para a direita para ver o nome
    if(current->rightChild->type == NODE_VAR){
        char* name = current->rightChild->identifier;
        *d_name = createVar(name);
    }
    else printf("Deu ruim - Nome de função não econtrado");

    return d_name;
}

Quad* generateCode(ASTNode* tree){
    Quad *none = NULL;
    if(tree == NULL) return none;

    switch (tree->type){
    case NODE_FUN_DECL:

        Address* func_data_type = determine_type(tree);
        Address* func_data_name = determine_name(tree);

        make_new_quad(OP_FUN, *func_data_type, *func_data_name, emptyAddr());
        
        // Verifica as partes da função
        ASTNode *prox = tree->next;
        if(prox->type == NODE_FUN_BODY){
            // Verifica os parâmetros da função (se houver)
            if(prox->leftChild != NULL && prox->leftChild->type == NODE_PARAM){
                ASTNode *params = prox->leftChild; 
                Quad* param;
                while(params != NULL){
                    param = generateCode(params);
                    param->addr3 = *func_data_name;       // Define o escopo        
                    
                    Address* temp_name = search_temp(param->addr2.name, func_data_name->name);
                    make_new_quad(OP_LOAD, *temp_name, param->addr2, emptyAddr());
                    
                    params = params->next;
                }
            }
            else printf("Uma função sem o campo de parâmetros\n");

            // Verifica o corpo da função
            if(prox->rightChild != NULL && prox->rightChild->type == NODE_COMPOUND_STMT){
                ASTNode *func_body = prox->rightChild->rightChild;
                while(func_body != NULL){
                    Quad* func_part = generateCode(func_body);

                    // Se for alocação, define o escopo
                    if(func_part != NULL && func_part->op == OP_ALLOC){
                        func_part->addr2 = *func_data_name;
                    }
                    // Caso contrário, segue
                    func_body = func_body->next;
                }

            }
            else printf("Função sem corpo");
        }
        else printf("Deu ruim - Uma função está sem o corpo");
        break;

    case NODE_PARAM:
        // Identifica nome e tipo e retorna
        Address* param_data_type = determine_type(tree);
        Address* param_data_name = determine_name(tree);
        return(make_new_quad(OP_PARAM, *param_data_type, *param_data_name, emptyAddr()));
        break;

    case NODE_IF_STMT:
        
        break;
    default:
        break;
    }
        
    return none;
}

void generateProgram(ASTNode* tree){
    ASTNode *current = tree;
    
    // Percorre todos os nós irmãos da raíz
    while(current != NULL){
        generateCode(current);
        current = current->next;
    }
    
    FILE* file = fopen("output/new_intermediate_code.txt", "w");
    fprintCode(file);
    fclose(file);
}