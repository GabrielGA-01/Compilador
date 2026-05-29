#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cintgen.h"
#include "symtab.h" // Para buscar por vetor na tabela de símbolos

static int tempCount = 0;
static int labelCount = 0;

Quad *head = NULL;
Quad *tail = NULL;

FuncLabel *functionsHead = NULL;
FuncLabel *functionsTail = NULL;

tempControl *tempControlHead = NULL;

char *globalScope;

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
Address* createLabelAddr() {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    
    labelCount; // Incrementa primeiro
    sprintf(buffer, "L%d", labelCount); // Usa depois
    
    labelCount++;

    a->kind = LABEL_KIND;
    a->name = strdup(buffer);
    a->val = labelCount;
    return a;
}

// Para criar uma nova variável
Address* createTempAddr() {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    sprintf(buffer, "t%d", tempCount);
    a->kind = TEMP_VAR;
    a->val = tempCount;
    a->name = strdup(buffer);
    tempCount++;
    
    return a;
}

void createFuncLabel(char* name, Address* label){
    FuncLabel* a = (FuncLabel*)malloc(sizeof(FuncLabel));
    a->name = strdup(name);
    a->label = label;
    a->next = NULL;
    a->safeCall = 1;
    a->allocatedParam = 0;

    if(functionsHead == NULL){
        functionsHead = a;
        functionsTail = a;
    }
    else{
        functionsTail->next = a;
        functionsTail = a;
    }
}

// Procura e soma ou insere
void update_or_insert(Address new_temp) {
    tempControl* current_node = tempControlHead;
    tempControl* previous_node = NULL;
    int found = 0;

    // Procura na estrutura
    while (current_node != NULL) {
        if (current_node->temp.name != NULL && new_temp.name != NULL) {
            if (strcmp(current_node->temp.name, new_temp.name) == 0) {
                found = 1;
                
                // Se tentar acessar novamente após um fun call
                if (current_node->safe == 1) {
                    current_node->safe = -1;
                    printf("Inseguro para %s", new_temp.name);
                    
                    // A mudança de 1 para -1 ocorreu! 
                    // Faz a busca na estrutura FuncLabel buscando pelo name == scope
                    FuncLabel* current_func = functionsHead; // Usando a cabeça global da lista de funções
                    while (current_func != NULL) {
                        if (current_func->name != NULL && globalScope != NULL) {
                            if (strcmp(current_func->name, globalScope) == 0) {
                                current_func->safeCall = -1;
                                break; // Encontrou o escopo correspondente, pode parar a busca interna
                            }
                        }
                        current_func = current_func->next;
                    }
                }
                
                // Contagem
                current_node->sum += 1;
                break;
            }
        }
        previous_node = current_node;
        current_node = current_node->next;
    }

    // Se não houver, insere ao final
    if (!found) {
        tempControl* new_node = (tempControl*)malloc(sizeof(tempControl));
        if (new_node == NULL) {
            printf("Memory allocation error.\n");
            return;
        }

        new_node->temp.kind = new_temp.kind;
        new_node->temp.val = new_temp.val;
        if (new_temp.name != NULL) {
            new_node->temp.name = strdup(new_temp.name); 
        } else {
            new_node->temp.name = NULL;
        }

        new_node->sum = 1;
        new_node->safe = 0;
        new_node->next = NULL;

        // Se o tempControlHead global for nulo, apenas atribui
        if (tempControlHead == NULL) {
            tempControlHead = new_node; 
        } else {
            previous_node->next = new_node;
        }
    }
}

// Faz o controle de segurança
void enable_safe_conditional() {
    tempControl* current_node = tempControlHead;
    
    while (current_node != NULL) {
        if (current_node->safe == 0) {
            current_node->safe = 1;
        }
        current_node = current_node->next;
    }
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

    // Adiciona na lista de controle
    if(a1.kind == TEMP_VAR) update_or_insert(a1);
    if(a2.kind == TEMP_VAR) update_or_insert(a2); // Provavelmente nem precisa
    if(a3.kind == TEMP_VAR) update_or_insert(a3);

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
        case OP_ADDI:    return "addi";
        case OP_ADDR:    return "addr";
        case OP_SUB:     return "sub";
        case OP_SUBI:    return "subi";
        case OP_SUBR:    return "subr";
        case OP_MUL:     return "mul";
        case OP_MULI:    return "muli";
        case OP_MULR:    return "mulr";
        case OP_DIV:     return "div";
        case OP_DIVI:    return "divi";
        case OP_DIVR:    return "divr";
        case OP_BLT:     return "blt";
        case OP_BLE:     return "ble";
        case OP_BGT:     return "bgt";
        case OP_BGE:     return "bge";
        case OP_BEQ:     return "beq";
        case OP_BNE:     return "bne";
        case OP_IFT:     return "ift";
        case OP_JUMP:    return "jmp";
        case OP_JR:      return "jr";
        case OP_LABEL:   return "lab";
        case OP_IN:      return "in";
        case OP_OUT:     return "out";
        case OP_PARAM:   return "param";
        case OP_CALL:    return "call";
        case OP_RET:     return "ret";
        case OP_HALT:    return "halt";
        case OP_FUN:     return "fun";
        case OP_END_FUN: return "end";
        case OP_ARG:     return "arg";
        case OP_LOAD:    return "load";
        case OP_LOADDI:   return "loaddi";
        case OP_LOADDR:  return "loaddr";
        case OP_STORE:   return "store";
        case OP_STOREDI:  return "storedi";
        case OP_STOREDR: return "storedr";
        case OP_MOV:    return "mov";
        case OP_MOVI:    return "movi";
        case OP_MOVR:    return "movr";
        case OP_ALLOC:   return "alloc";
        case OP_FREE:    return "free";
        default:         return "unknown";
    }
}

// Extrai corretamente o tipo do operando e escreve ele
void fprintAddr(FILE* out, Address a) {
    switch(a.kind) {
        case EMPTY: fprintf(out, "-"); break;
        case INT_CONST: fprintf(out, "%d", a.val); break;
        case STRING_VAR: fprintf(out, "%s", a.name); break;
        case REGISTER_KIND: fprintf(out, "%s", a.name); break;
        case TEMP_VAR: fprintf(out, "%s", a.name); break;
        case LABEL_KIND: fprintf(out, "%s", a.name); break;
    }
}

// Organiza a escrita das quádruplas no arquivo
void fprintCode(FILE* out, Quad* head) {
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

// Verifica o tamanho do array/determina se é variável
Address determineVariableSize(ASTNode* node){
    Address size;
    if(node->type == NODE_VAR){
        size = createNumericAddr(1);
    }
    else if(node->type == NODE_ARRAY_DECL || node->type == NODE_ARRAY_ACCESS){
        ASTNode* a_size = node->rightChild;
        if(a_size == NULL) size = createNumericAddr(1); // Caso seja um argumento array
        else size = generateCode(a_size, " ", 1);    // Identifica e retornar um addr de número ou temp (com load feito). Não usa scope.
    }
    return size;
}

int isArray(ASTNode* node){
    if(node != NULL){
        if(node->type == NODE_VAR) return 0;
        else if(node->type == NODE_ARRAY_DECL) return 1;
    }
    return -1;
}

Address generateCode(ASTNode* node, char* scope, int mode){
    globalScope = scope;
    if(node == NULL) return createEmptyAddr();

    switch (node->type){
    // Caso seja uma função
    case NODE_FUN_DECL:
        char* func_data_type = numberToType(node->leftChild->number);
        char* func_name = node->rightChild->identifier; 
    
        Address* fun_decl_label = createLabelAddr();
        makeNewQuad(OP_LABEL, *fun_decl_label, createEmptyAddr(), createEmptyAddr());
        createFuncLabel(func_name, fun_decl_label);
    
        Address addr_func_data_type = createStringAddr(func_data_type);
        Address addr_func_name = createStringAddr(func_name);
        makeNewQuad(OP_FUN, addr_func_data_type, addr_func_name, createEmptyAddr());
        
        // Verifica as partes da função
        
        // Verifica os parâmetros da função (se houver)
        ASTNode* fun_body = node->next;

        // Estrutura a função
        if(fun_body != NULL && fun_body->type == NODE_FUN_BODY) generateCode(fun_body, func_name, 1);

        // Fim da funço
        makeNewQuad(OP_END_FUN, addr_func_name, createEmptyAddr(), createEmptyAddr());

        break;
    
    case NODE_FUN_BODY:
        // Verificação dos parâmetros
        ASTNode* param = node->leftChild;
        while(param != NULL && param->type == NODE_PARAM){
            generateCode(param, scope, 1);
            param = param->next;
        }
        // Verificação dos componetes da função
        if(node->rightChild != NULL && node->rightChild->type == NODE_COMPOUND_STMT) generateCode(node->rightChild, scope, 1);
        break;

    // Declaração de um parâmetro de uma função
    case NODE_PARAM:
        // Address addr_param_data_type = generateCode(node->leftChild, scope); // Não precisa do tipo
        Address addr_param_name = generateCode(node->rightChild, scope, 0);
        Address array_size_param = determineVariableSize(node->rightChild);
        
        QuadOp op_param = OP_ARG;

        makeNewQuad(op_param, addr_param_name, array_size_param, createStringAddr(scope));
    
        break;

    case NODE_VAR_DECL:
        // Declaração de um parâmetro
        // Address addr_var_decl_data_type = generateCode(node->leftChild, scope); // Não precisa do tipo
        Address addr_var_decl_name = generateCode(node->rightChild, scope, 0);
        
        Address array_size_var_decl = determineVariableSize(node->rightChild);

        QuadOp op_var_decl = OP_ALLOC;
        
        makeNewQuad(op_var_decl, addr_var_decl_name, array_size_var_decl, createStringAddr(scope));
    
        break;

    case NODE_COMPOUND_STMT:
        // Verifica as variáveis que são declaradas na função se mode != 2
        if(mode != 2){
            ASTNode* declaracao = node->leftChild;
            while(declaracao != NULL){
                generateCode(declaracao, scope, 1);
                declaracao = declaracao->next;
            }
        }

        // Percorre todas instruções dentro do corpo da função
        // Há um caso especial para quando há um IF
        ASTNode* instruction = node->rightChild;
        while(instruction != NULL){
            generateCode(instruction, scope, 1);
            if(instruction->type == NODE_IF_STMT && instruction->number == 1 && instruction->number == 1 && instruction->next != NULL) instruction = instruction->next->next;
            else instruction = instruction->next;
        }
        break;

    case NODE_IF_STMT:
        Address if_true_label = generateCode(node->leftChild, scope, 1);
        Address* if_end_label = createLabelAddr();
        
        // Início do caso para falso se tiver else
        if(node->number == 1){
            generateCode(node->next, scope, 1);
        }
        makeNewQuad(OP_JUMP, *if_end_label, createEmptyAddr(), createEmptyAddr());

        // Início do caso para verdadeiro
        makeNewQuad(OP_LABEL, if_true_label, createEmptyAddr(), createEmptyAddr());
        generateCode(node->rightChild, scope, 1);

        // Label indicando o fim do IF
        makeNewQuad(OP_LABEL, *if_end_label, createEmptyAddr(), createEmptyAddr());

        break;

    case NODE_WHILE_STMT:
        Address* while_initial_label = createLabelAddr();
        Address* while_final_label = createLabelAddr();
        
        // Declara as variáveis do while antes de entrar nele
        ASTNode* variable_decl = node->rightChild->leftChild; //->NODE_COMPOUND_STMT->Variables declarations
        
        int while_variables = 0;
        while(variable_decl != NULL){
            generateCode(variable_decl, scope, 1);
            variable_decl = variable_decl->next;
            while_variables++;
        }
        
        // Cria label no início do while
        makeNewQuad(OP_LABEL, *while_initial_label, createEmptyAddr(), createEmptyAddr());
        
        Address while_content = generateCode(node->leftChild, scope, 1);
        makeNewQuad(OP_JUMP, *while_final_label, createEmptyAddr(), createEmptyAddr());  // Caso falso
        makeNewQuad(OP_LABEL, while_content, createEmptyAddr(), createEmptyAddr());     // Caso verdade

        // Conteúdo do while
        generateCode(node->rightChild, scope, 2);   // Não declara as variáveis de novo
        makeNewQuad(OP_JUMP, *while_initial_label, createEmptyAddr(), createEmptyAddr());


        // Cria label no final do while
        makeNewQuad(OP_LABEL, *while_final_label, createEmptyAddr(), createEmptyAddr());

        // Desaloca as variáveos do while
        if(while_variables != 0){
            makeNewQuad(OP_FREE, createNumericAddr(while_variables), createEmptyAddr(), createEmptyAddr());
        }

        break;

    case NODE_ARRAY_DECL:
        return(generateCode(node->leftChild, scope, 0));

    case NODE_ARRAY_ACCESS:
        Address array_access_name = generateCode(node->leftChild, scope, 0);
        
        if(mode == 0) return(array_access_name);
        else{
            
            Address second_parameter_array_scope;
            // Caso seja um ponteiro, faz um novo acesso à memória
            if(st_lookup_is_array_scope(array_access_name.name, scope) == 2){
                Address array_access_deslocamento_indireto = createNumericAddr(0);
                second_parameter_array_scope = *createTempAddr();
                makeNewQuad(OP_LOAD, second_parameter_array_scope, array_access_name, array_access_deslocamento_indireto);
            }
            else second_parameter_array_scope = array_access_name;

            Address *array_access_temp_loaded = createTempAddr();
            Address array_access_deslocamento = determineVariableSize(node);

            makeNewQuad(OP_LOAD, *array_access_temp_loaded, second_parameter_array_scope, array_access_deslocamento);
            return *array_access_temp_loaded;
        }
        break;

    case NODE_RETURN_STMT:
        Address ret = generateCode(node->leftChild, scope, 1);

        // Se for um número, passa para um registrador antes
        Address true_ret = ret;
        if(ret.kind == INT_CONST){
            true_ret = *createTempAddr();
            makeNewQuad(OP_MOV, true_ret, ret, createEmptyAddr());
        } 

        makeNewQuad(OP_RET, true_ret, createEmptyAddr(), createEmptyAddr());
        return ret;
        break;

    case NODE_FUN_CALL:
        int param_number = 0;
        Address func_call_name = generateCode(node->leftChild, scope, 0);
        ASTNode* parameter= node->rightChild;
        while(parameter != NULL){
            Address current_parameter = generateCode(parameter, scope, 1);

            // Garante que o parâmetro está em um registrador
            Address real_current_patameter = current_parameter;
            if(current_parameter.kind == INT_CONST){
                real_current_patameter = *createTempAddr();
                makeNewQuad(OP_MOV, real_current_patameter, current_parameter, createEmptyAddr());
            }

            makeNewQuad(OP_PARAM, real_current_patameter, createEmptyAddr(), createEmptyAddr());
            param_number++;
            parameter = parameter->next;
        }

        // Se não for tipo void retorna o valor
        if(st_lookup_type(func_call_name.name) != Void){
            enable_safe_conditional();  // Guarda os temporários seguros
            Address fun_call_temp = *createTempAddr();
            makeNewQuad(OP_CALL, fun_call_temp, func_call_name, createNumericAddr(param_number));
            return(fun_call_temp);
        }
        // Caso contrário, apenas chama a função
        else{
            makeNewQuad(OP_CALL, createEmptyAddr(), func_call_name, createNumericAddr(param_number));
            enable_safe_conditional();  // Guarda os temporários seguros
        }
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
            case 264: operation = OP_BLT; break;
            case 265: operation = OP_BLE; break;
            case 266: operation = OP_BGE; break;
            case 267: operation = OP_BGT; break;
            case 268: operation = OP_BEQ; break;
            case 269: operation = OP_BNE; break;
            default:  operation = OP_ADD; break;
        }

        createLabelAddr();
        Address left_operator = generateCode(node->leftChild, scope, 1);
        Address right_operator = generateCode(node->rightChild, scope, 1);

        // Todas essas operações exigem que o primeiro operando seja um registrador
        // Se for um número, move para um registrador antes de utilizar
        Address true_left_operator = left_operator;
        if(left_operator.kind == INT_CONST){
            true_left_operator = *createTempAddr();
            makeNewQuad(OP_MOV, true_left_operator,  left_operator, createEmptyAddr());
        }
        
        Address* retorno = NULL;
        // Caso aritmético
        if(node->number < 264){
            // Retorno é um temporário
            retorno = createTempAddr();
            makeNewQuad(operation, *retorno, true_left_operator, right_operator);
        }
        // Caso lógico
        else{
            // Retorno é uma label
            retorno = createLabelAddr();
            Address true_left_operator = left_operator;
            Address true_right_operator = right_operator;

            // Para operações de branch é esperado que o segundo valor também seja um registrador
            if(right_operator.kind == INT_CONST){
                true_right_operator = *createTempAddr();
                makeNewQuad(OP_MOV, true_right_operator,  right_operator, createEmptyAddr());
            }  

            makeNewQuad(operation, true_left_operator, true_right_operator, *retorno);
        }
        return(*retorno);
        break;

    case NODE_ASSIGN_EXPR:
        Address node_assign_name = generateCode(node->leftChild, scope, 0);
        Address value_to_store = generateCode(node->rightChild, scope, 1);
    
        Address value_to_store_temp = value_to_store;
        if(value_to_store.kind == INT_CONST){
            value_to_store_temp = *createTempAddr();
            makeNewQuad(OP_MOV, value_to_store_temp, value_to_store, createEmptyAddr());
        }

        Address assign_expr_deslocamento = determineVariableSize(node->leftChild);
        if(isArray(node->leftChild) == 0) assign_expr_deslocamento.val--; // Se for uma variável, lê a "posição zero"

        Address first_parameter_assign_name;
        // Caso seja um ponteiro, faz um novo acesso à memória
        if(st_lookup_is_array_scope(node_assign_name.name, scope) == 2){
            Address assign_expr_deslocamento_indireto = createNumericAddr(0);
            first_parameter_assign_name = *createTempAddr();
            makeNewQuad(OP_LOAD, first_parameter_assign_name, node_assign_name, assign_expr_deslocamento_indireto);
        }
        else first_parameter_assign_name = node_assign_name;

        makeNewQuad(OP_STORE, first_parameter_assign_name, value_to_store_temp, assign_expr_deslocamento);

        break;

    // Retorna o Address com o tipo
    case NODE_TYPE:
        char* node_data_type = numberToType(node->number);
        Address addr_node_data_type = createStringAddr(node_data_type); 
        return(addr_node_data_type);
        break;

    // Retorn o nome (mode==0) ou temp (mode==1) com o valor
    case NODE_VAR:
        char* node_name = node->identifier;      
        Address node_var_name = createStringAddr(node_name);

        if(mode == 0) return(node_var_name);

        else{
            Address var_temp_addr = *createTempAddr();
            // Caso queira o endereço de um array
            if(st_lookup_is_array_scope(node_name, scope) == 1){
                makeNewQuad(OP_MOV, var_temp_addr, node_var_name, createNumericAddr(0));
            }
            // Caso queira o valor da variável
            else{
                Address node_var_deslocamento = determineVariableSize(node);
                if(isArray(node) == 0) node_var_deslocamento.val--; // Se for uma variável, lê a "posição zero"

                // Acessa com um deslocamento
                makeNewQuad(OP_LOAD, var_temp_addr, node_var_name, node_var_deslocamento);
            }
            return(var_temp_addr);
        }

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
    tempControlHead = (tempControl*)malloc(sizeof(tempControl));
    tempControlHead->temp.name = NULL;
    tempControlHead->next = NULL;

    ASTNode *current = tree;

    // Percorre todos os nós irmãos da raíz
    while(current != NULL){
        if(current->type != NODE_FUN_BODY) generateCode(current, "global", 1);
        if(current->type == NODE_IF_STMT && current->next != NULL) current = current->next->next;
        else current = current->next;
    }
    enable_safe_conditional();
    
    FILE* file = fopen("output/intermediate_code.txt", "w");
    fprintCode(file, head);
    fclose(file);
}