#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cintgen.h"

static int tempCount = 0;
static int labelCount = -1;

Quad *head = NULL;
Quad *tail = NULL;
TempStorage* TSHead = NULL;

// Para indicar um operando sem uso
Address createEmptyAddr() {
    Address a;
    a.kind = EMPTY;
    a.val = 0;
    a.name = NULL;
    return a;
}

// Para criar um operando numérico
Address createNumericAddr(int val) {
    Address a;
    a.kind = INT_CONST;
    a.val = val;
    a.name = NULL;
    return a;
}

// Para criar um operando string
Address createStringAddr(char *name) {
    Address a;
    a.kind = STRING_VAR;
    a.name = strdup(name);
    return a;
}

// Para criar um novo label
Address createLabelAddr() {
    Address a;
    char buffer[20];
    sprintf(buffer, "L%d", labelCount++);
    a.kind = LABEL_KIND;
    a.name = strdup(buffer);
    return a;
}

// Para criar uma nova variável
Address* createTempAddr() {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    sprintf(buffer, "t%d", tempCount++);
    a->kind = TEMP_VAR;
    a->name = strdup(buffer);
    
    return a;
}

// Tenta buscar se o temp já existe
Address* searchCreateTempAddr(char* name, char* scope_name, int array_size){
    TempStorage* current = TSHead;
    while(current != NULL){
        // Verifica nome e escopo
        if(strcmp(current->var_name, name) == 0 && strcmp(current->scope_name, scope_name) == 0){
            return(current->temp_addr);
        }
        // Verifica se é global
        else if(strcmp(current->scope_name, "Global") == 0) return(current->temp_addr);
        current = current->next;
    }

    Address* new_temp = createTempAddr();

    TempStorage* new_storage = (TempStorage *)malloc(sizeof(TempStorage));
    new_storage->var_name = name;
    new_storage->scope_name = scope_name;
    new_storage->temp_addr = new_temp;
    new_storage->array_size = array_size;
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
        case OP_ASSIGN:     return "assign";
        case OP_LT:      return "lt";
        case OP_LET:     return "let";
        case OP_GT:      return "gt";
        case OP_GET:     return "get";
        case OP_EQ:      return "eq";
        case OP_DIF:     return "dif";
        case OP_IFF:     return "iff";
        case OP_JUMP:    return "jmp";
        case OP_LABEL:     return "lab";
        case OP_IN:      return "in";
        case OP_OUT:     return "out";
        case OP_PARAM:   return "param";
        case OP_CALL:    return "call";
        case OP_RET:     return "ret";
        case OP_HALT:    return "halt";
        case OP_ARRAY_ACCESS: return "arrces";
        case OP_ARRAY_ASSIGN: return "arrsgn";
        case OP_FUN:     return "fun";
        case OP_END_FUN: return "end";
        case OP_ARG:     return "arg";
        case OP_ARG_ARRAY: return "arrarg";
        case OP_LOAD:    return "load";
        case OP_STORE:   return "store";
        case OP_ALLOC:   return "alloc";
        default:         return "unknown";
    }
}

// Extrai corretamente o tipo do operando e escreve ele
void fprintAddr(FILE* out, Address a) {
    switch(a.kind) {
        case EMPTY: fprintf(out, "-"); break;
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

Address generateCode(ASTNode* node, char* scope){
    if(node == NULL) return createEmptyAddr();

    switch (node->type){
    // Caso seja uma função
    case NODE_FUN_DECL:
        char* func_data_type = numberToType(node->leftChild->number);
        char* func_name = node->rightChild->identifier;

        Address addr_func_data_type = createStringAddr(func_data_type);
        Address addr_func_name = createStringAddr(func_name);
        makeNewQuad(OP_FUN, addr_func_data_type, addr_func_name, createEmptyAddr());
        
        // Verifica as partes da função
        
        // Verifica os parâmetros da função (se houver)
        ASTNode* fun_body = node->next;

        // Estrutura a função
        if(fun_body != NULL && fun_body->type == NODE_FUN_BODY) generateCode(fun_body, func_name);

        // Fim da funço
        makeNewQuad(OP_END_FUN, addr_func_name, createEmptyAddr(), createEmptyAddr());

        break;
    
    case NODE_FUN_BODY:
        // Verificação dos parâmetros
        ASTNode* param = node->leftChild;
        while(param != NULL && param->type == NODE_PARAM){
            generateCode(param, scope);
            param = param->next;
        }
        // Verificação dos componetes da função
        if(node->rightChild != NULL && node->rightChild->type == NODE_COMPOUND_STMT) generateCode(node->rightChild, scope);
        break;

    case NODE_PARAM:
        // Declaração de um parâmetro
        Address addr_param_data_type = generateCode(node->leftChild, scope);
        Address addr_param_name = createStringAddr(node->rightChild->identifier);
        
        QuadOp op_param;
        int array_size_param;

        // Chuta que não é array e depois verifica
        if(node->rightChild != NULL){
            if(node->rightChild->type == NODE_VAR){
                op_param = OP_ARG;
                array_size_param = 0;
            }
            else if(node->rightChild->type == NODE_ARRAY_DECL){
                op_param = OP_ARG_ARRAY;
                ASTNode* a_size = node->rightChild->rightChild;
                if(a_size == NULL) array_size_param = 1;
                else array_size_param = a_size->number;
            }
        }

        makeNewQuad(op_param, addr_param_data_type, addr_param_name, createEmptyAddr());
    
        break;

    case NODE_VAR_DECL:
        // Declaração de um parâmetro
        Address addr_var_decl_data_type = generateCode(node->leftChild, scope);
        Address addr_var_decl_name = createStringAddr(node->rightChild->identifier);
        
        QuadOp op_var_decl;
        int array_size_var_decl;

        // Chuta que não é array e depois verifica
        if(node->rightChild != NULL){
            if(node->type == NODE_VAR_DECL){
                if(node->rightChild->type == NODE_VAR){
                    op_var_decl = OP_ALLOC;
                    array_size_var_decl = 0;
                }
                // Fazer caso do array
            }
        }

        makeNewQuad(op_var_decl, addr_var_decl_data_type, addr_var_decl_name, createStringAddr(scope));
    
        break;

    case NODE_COMPOUND_STMT:
        // Verifica as variáveis que são declaradas na função
        ASTNode* declaracao = node->leftChild;
        while(declaracao != NULL){
            generateCode(declaracao, scope);
            declaracao = declaracao->next;
        }

        // Percorre todas instruções dentro do corpo da função
        // Há um caso especial para quando há um IF
        ASTNode* instruction = node->rightChild;
        while(instruction != NULL){
            generateCode(instruction, scope);
            if(instruction->type == NODE_IF_STMT && instruction->next != NULL) instruction = instruction->next->next;
            else instruction = instruction->next;
        }
        break;

    case NODE_IF_STMT:
        Address compare = generateCode(node->leftChild, scope);
        Address if_false_label = createLabelAddr();
        makeNewQuad(OP_IFF, compare, if_false_label, createEmptyAddr());

        // Inicio do caso para verdade
        generateCode(node->rightChild, scope);
        // Salto para o fim do IF
        Address if_true_label = createLabelAddr();
        makeNewQuad(OP_JUMP, if_true_label, createEmptyAddr(), createEmptyAddr());

        // Label do início do caso para falso
        makeNewQuad(OP_LABEL, if_false_label, createEmptyAddr(), createEmptyAddr());
        generateCode(node->next, scope);

        // Label indicando o fim do IF
        makeNewQuad(OP_LABEL, if_true_label, createEmptyAddr(), createEmptyAddr());
        break;

    case NODE_RETURN_STMT:
        Address ret = generateCode(node->leftChild, scope);

        makeNewQuad(OP_RET, ret, createEmptyAddr(), createEmptyAddr());
        return ret;
        break;

    case NODE_FUN_CALL:
        int param_number = 0;
        Address func_call_name = createStringAddr(node->leftChild->identifier);
        ASTNode* parameter= node->rightChild;
        while(parameter != NULL){
            Address current_parameter = generateCode(parameter, scope);
            makeNewQuad(OP_PARAM, current_parameter, createEmptyAddr(), createEmptyAddr());
            param_number++;
            parameter = parameter->next;
        }

        Address fun_call_temp = *createTempAddr();
        makeNewQuad(OP_CALL, fun_call_temp, func_call_name, createNumericAddr(param_number));

        return(fun_call_temp);
        break;

    case NODE_BINARY_OP:
    case NODE_OPERATOR:
        // Identifica o tipo de operação
        QuadOp operation;
        // Decodifica a operação
        switch(node->number) {
            case 260: operation = OP_ADD; break;
            case 261: operation = OP_SUB; break;
            case 262: operation = OP_MUL; break;
            case 263: operation = OP_DIV; break;
            case 264: operation = OP_LT;  break;
            case 265: operation = OP_LET; break;
            case 266: operation = OP_GET; break;
            case 267: operation = OP_GT;  break;
            case 268: operation = OP_EQ;  break;
            case 269: operation = OP_DIF; break;
            default:  operation = OP_ADD; break;
        }

        createLabelAddr();
        Address left_operator = generateCode(node->leftChild, scope);
        Address right_operator = generateCode(node->rightChild, scope);

        Address* temp_operator = createTempAddr();
        makeNewQuad(operation, *temp_operator, left_operator, right_operator);

        return(*temp_operator);
        break;

    case NODE_ASSIGN_EXPR:
        Address value = generateCode(node->rightChild, scope);
    
        // Não funciona para array
        if(node->leftChild != NULL){
            if(node->leftChild->type == NODE_VAR){
                Address variable = createStringAddr(node->leftChild->identifier);
                makeNewQuad(OP_STORE, variable, value, createEmptyAddr());
            }
            else if(node->leftChild->type == NODE_ARRAY_ACCESS){
                Address variable = generateCode(node->leftChild, scope);
                makeNewQuad(OP_ASSIGN, variable, value, createEmptyAddr());
            }
        }

        break;

    // Retorna o Address com o tipo
    case NODE_TYPE:
        char* node_data_type = numberToType(node->number);
        Address addr_node_data_type = createStringAddr(node_data_type); 
        return(addr_node_data_type);
        break;

    // Retorn um temp com o valor
    case NODE_VAR:
        char* node_name = node->identifier;      
        Address addr_node_name = createStringAddr(node_name);

        Address var_temp_addr = *createTempAddr(&node_name, scope, 1);
        makeNewQuad(OP_LOAD, var_temp_addr, addr_node_name, createEmptyAddr());

        return(var_temp_addr);
        break;

    case NODE_NUM:
        Address addr_number = createNumericAddr(node->number);
        return(addr_number);
        break;

    default:
        break;
    }
        
    return createEmptyAddr();
}

void generateProgram(ASTNode* tree){
    ASTNode *current = tree;
    
    // Percorre todos os nós irmãos da raíz
    while(current != NULL){
        if(current->type != NODE_FUN_BODY) generateCode(current, "Global");
        current = current->next;
    }
    makeNewQuad(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    
    FILE* file = fopen("output/new_intermediate_code.txt", "w");
    fprintCode(file);
    fclose(file);
}