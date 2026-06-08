#include <errno.h>
#include <stdlib.h>
#include "casm.h"
#include "cintgen.h"
#include "string.h"

#define MEM_SIZE 123
#define PILHA_GLOBAL MEM_SIZE
#define NUMBER_OF_REGISTERS 4

variablesAtStack* scopesHead = NULL;
labelControl* labelHead = NULL;

int availableMem = MEM_SIZE; 

Address* searchFuncLabel(char* name, FuncLabel* funHead){
    FuncLabel* current = funHead;
    while(current != NULL){
        if(strcmp(name, current->name) == 0) return(current->label);
        current = current->next;
    }
    return NULL;
}

Quad* insertQuadAfter(Quad* target, QuadOp op, Address a1, Address a2, Address a3) {
    if (target == NULL) {
        return NULL; 
    }

    Quad *q = (Quad *)malloc(sizeof(Quad));
    q->op = op;
    q->addr1 = a1;
    q->addr2 = a2;
    q->addr3 = a3;

    q->next = target->next;
    target->next = q;

    // Adiciona na lista de controle
    if(a1.kind == TEMP_VAR) update_or_insert(a1);
    if(a2.kind == TEMP_VAR) update_or_insert(a2); // Provavelmente nem precisa
    if(a3.kind == TEMP_VAR) update_or_insert(a3);

    return q;
}

// Para criar um novo registrador
Address* createRegisterAddr(int number) {
    Address* a = (Address*)malloc(sizeof(Address));
    char buffer[20];
    
    sprintf(buffer, "R%d", number);
    
    a->kind = REGISTER_KIND;
    a->name = strdup(buffer);
    a->val = number;
    return a;
}

regControl* populateRegisters(int n) {
    if (n <= 0) return NULL;

    regControl* regVector = (regControl*)malloc(n * sizeof(regControl));
    if (regVector == NULL) {
        printf("Memory allocation error for register vector.\n");
        return NULL;
    }

    char buffer[20];

    for (int i = 0; i < n; i++) {
        sprintf(buffer, "R%d", i);

        // Inicializa o Address interno
        regVector[i].reg.kind = REGISTER_KIND;
        regVector[i].reg.name = strdup(buffer);
        regVector[i].reg.val = i;

        // Nova inicialização: sem nome
        regVector[i].labelName = NULL;

        // Inicializações anteriores mantidas
        regVector[i].sum = 0;
        regVector[i].safe = 1;
    }

    return regVector;
}

// Função que gerencia o mapeamento de temporárias para registradores
Address* allocate_register(char* temp_name, tempControl* tempControlHead, regControl* regVector) {
    if (temp_name == NULL || regVector == NULL) {
        perror("Error: Invalid arguments passed to allocate_register");
        return NULL;
    }

    // 1. Procurar se em algum dos registradores o labelName é o mesmo do temp
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (regVector[i].labelName != NULL) {
            if (strcmp(regVector[i].labelName, temp_name) == 0) {
                // 1.1. Caso seja, reduz em 1 o valor da soma
                regVector[i].sum -= 1;
                
                // Retorna o endereço do registrador encontrado
                return &(regVector[i].reg);
            }
        }
    }

    // 2. Caso não tenham o labelName correspondente...
    // Primeiro, precisamos buscar os dados da temporária em tempControlHead
    tempControl* current_temp = tempControlHead;
    int temp_sum = 1;  // Valores padrão de contingência
    int temp_safe = 0;

    while (current_temp != NULL) {
        if (current_temp->temp.name != NULL && strcmp(current_temp->temp.name, temp_name) == 0) {
            temp_sum = current_temp->sum;
            temp_safe = current_temp->safe;
            break;
        }
        current_temp = current_temp->next;
    }

    // 2.1. Procurar o primeiro registrador cuja soma é zero
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (regVector[i].sum == 0) {
            // Se o labelName antigo existia (está sendo substituído), limpa a memória dele
            if (regVector[i].labelName != NULL) {
                free(regVector[i].labelName);
            }

            // Atualiza o labelName e o safe
            regVector[i].labelName = strdup(temp_name);
            regVector[i].safe = temp_safe;

            // MODIFICAÇÃO AQUI: Inicializa a nova soma como sum(label) - 1
            regVector[i].sum = temp_sum - 1;

            // Retorna o endereço do registrador escolhido
            return &(regVector[i].reg);
        }
    }

    // 2.2. Caso não tenha nenhum registrador disponível (nenhum sum == 0)
    // Imprime o estado de uso dos registradores antes do perror
    fprintf(stderr, "\n--- Active Registers in Use ---\n");
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        fprintf(stderr, "Register %s is holding: %s (Sum: %d)\n", 
                regVector[i].reg.name ? regVector[i].reg.name : "??",
                regVector[i].labelName ? regVector[i].labelName : "(free but blocked)", 
                regVector[i].sum);
    }
    fprintf(stderr, "-------------------------------\n");

    fprintf(stderr, "Without space for: %s\n", temp_name);
    perror("Error: No physical registers available (Spill required)");
    return NULL;
}

void check_register_leaks(regControl* regVector) {
    if (regVector == NULL) return;

    int leaks_found = 0;

    // Primeiro passamos verificando se há erros
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        if (regVector[i].sum != 0) {
            leaks_found = 1;
            break;
        }
    }

    // Se houver algum vazamento/erro, detalha o cenário antes de travar
    if (leaks_found) {
        fprintf(stderr, "\n❌ FATAL COMPILER ERROR: Register allocation leak detected!\n");
        fprintf(stderr, "Some temporaries were not properly released (sum != 0).\n");
        fprintf(stderr, "-------------------------------------------------------\n");

        for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
            if (regVector[i].sum != 0) {
                fprintf(stderr, "-> Register %s is deadlocked holding: %s | Remaining Sum: %d\n",
                        regVector[i].reg.name ? regVector[i].reg.name : "??",
                        regVector[i].labelName ? regVector[i].labelName : "(unknown)",
                        regVector[i].sum);
            }
        }
        fprintf(stderr, "-------------------------------------------------------\n");
        
        // Finaliza a execução do compilador apontando erro crítico
        exit(EXIT_FAILURE); 
    }
}

// Retorna o status de safeCall da função
int is_function_safe(char* scope_name, FuncLabel* funcLabelHead) {
    if (scope_name == NULL || funcLabelHead == NULL) {
        return -99; 
    }

    FuncLabel* current = funcLabelHead;

    // Percorre a lista de rótulos de função
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, scope_name) == 0) {
            return current->safeCall; // Encontrou, retorna o status
        }
        current = current->next;
    }

    // Caso saia do laço sem encontrar o escopo
    return -99; 
}

// Adiciona um valor ao allocatedParam do escopo e retorna o novo total
int update_and_get_allocated_param(char* scope_name, int number, FuncLabel* funcLabelHead) {
    if (scope_name == NULL || funcLabelHead == NULL) {
        return -99;
    }

    FuncLabel* current = funcLabelHead;

    // Percorre a lista procurando o escopo correspondente
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, scope_name) == 0) {
            current->allocatedParam += number; // Adiciona o número recebido
            return current->allocatedParam;    // Retorna o valor atualizado
        }
        current = current->next;
    }

    // Escopo não encontrado
    fprintf(stderr, "Warning: Scope '%s' not found to update allocatedParam.\n", scope_name);
    return -99;
}

// Retorna o endereço da estrutura reg ou NULL se não encontrar
Address* find_unsafe_active_register(regControl* regVector) {
    if (regVector == NULL) {
        return NULL;
    }

    // Percorre o banco de registradores do compilador
    for (int i = 0; i < NUMBER_OF_REGISTERS; i++) {
        // Critério: Inseguro (-1) E ativo/com uso pendente (sum != 0)
        if (regVector[i].safe == -1 && regVector[i].sum != 0) {
            
            // Retorna o endereço da estrutura reg do registrador encontrado
            return &regVector[i].reg;
        }
    }

    // Retorna NULL caso todos os registradores ativos estejam seguros (safe == 1)
    // ou se todos os inseguros já estiverem livres (sum == 0)
    return NULL;
}

void addVariableToStack(char* scope, char *varName, int numPositions) {
    if (numPositions <= 0) return;

    // Verifica se há espaço
    if (availableMem - numPositions < -1) {
        printf("Erro: Stack Overflow! Limite posições atingido.\n");
        exit(1);
    }

    // Atualiza o topo da pilha
    availableMem -= numPositions;

    variablesAtStack* currentScope = scopesHead;

    // Busca o escopo
    while (currentScope != NULL) {
        if (strcmp(currentScope->scope, scope) == 0) {
            break;
        }
        currentScope = currentScope->next;
    }
    // Se o escopo não existir, cria e empilha
    if (currentScope == NULL) {
        currentScope = (variablesAtStack*)malloc(sizeof(variablesAtStack));
        currentScope->scope = strdup(scope);
        currentScope->var = NULL;
        currentScope->next = scopesHead;
        scopesHead = currentScope;
    }

    variables* newVar = (variables*)malloc(sizeof(variables));
    newVar->name = strdup(varName);
    newVar->size = numPositions;
    newVar->next = NULL;
    newVar->offset = 1;

    // Insere no final da lista do escopo
    if (currentScope->var == NULL) {
        currentScope->var = newVar;
    } else {
        variables* currentVar = currentScope->var;
        while (currentVar != NULL) {
            currentVar->offset += numPositions;
            if (currentVar->next == NULL) {
                break;
            }
            currentVar = currentVar->next;
        }
        currentVar->next = newVar;
    }
}

int verifyVariableShift(char* scope, char* varName) {
    variablesAtStack* currentScope = scopesHead;

    // Busca o escopo
    while (currentScope != NULL) {
        if (strcmp(currentScope->scope, scope) == 0) {
            break;
        }
        currentScope = currentScope->next;
    }

    if (currentScope == NULL) return -1;

    // Busca a variável
    variables* currentVar = currentScope->var;
    while (currentVar != NULL) {
        if (strcmp(currentVar->name, varName) == 0) {
            return (currentVar->offset);
        }
        currentVar = currentVar->next;
    }

    return -1;
}

int getScopeVariableShift(char* scope) {
    variablesAtStack* currentScope = scopesHead;

    while (currentScope != NULL) {
        if (strcmp(currentScope->scope, scope) == 0) {
            break;
        }
        currentScope = currentScope->next;
    }

    // Se o escopo não foi encontrado, ou se ele não possui nenhuma variável
    if (currentScope == NULL || currentScope->var == NULL) {
        return -1;
    }

    return currentScope->var->offset + currentScope->var->size - 1; // Posição da última variável na pilha + seu tamanho - 1
}

void removeVariableFromStack(char *scope) {
    variablesAtStack* currentScope = scopesHead;

    while (currentScope != NULL) {
        if (strcmp(currentScope->scope, scope) == 0) {
            break;
        }
        currentScope = currentScope->next;
    }

    if (currentScope == NULL || currentScope->var == NULL) return;

    // Remove a última variável do escopo
    variables* currentVar = currentScope->var;
    
    int offsetRemoved = 0;
    if (currentVar->next == NULL) {
        offsetRemoved = currentVar->size;
        free(currentVar->name);
        free(currentVar);
        currentScope->var = NULL;
    } else {
        while (currentVar->next->next != NULL) {
            currentVar = currentVar->next;
        }
        variables* lastVar = currentVar->next;
        currentVar->next = NULL;

        offsetRemoved = lastVar->size;
        free(lastVar->name);
        free(lastVar);
    }

    // Atualiza os offsets
    if(offsetRemoved != 0){
        currentVar = currentScope->var;
        if (currentVar != NULL) {
            if (currentVar->next == NULL) {
                currentVar->offset -= offsetRemoved;
            } else {
                while (currentVar != NULL) {
                    currentVar->offset -= offsetRemoved;
                    currentVar = currentVar->next;
                }
            }
        }

        availableMem += offsetRemoved;
        if (availableMem > MEM_SIZE) {
            printf("Erro: Stack Underflow! Posição abaixo da mínima.\n");
            exit(1);
        }
    }
}

/* 1. Função para inserir no final da lista */
void insertLabel(labelControl** head, char* name, int line) {
    labelControl* newNode = (labelControl*)malloc(sizeof(labelControl));
    newNode->labelName = strdup(name); // Cria uma cópia da string
    newNode->lineNumber = line;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    labelControl* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

/* 2. Função para buscar o número da linha pelo nome */
int getLineByLabel(labelControl* head, char* name) {
    labelControl* current = head;
    while (current != NULL) {
        if (strcmp(current->labelName, name) == 0) {
            return current->lineNumber;
        }
        current = current->next;
    }
    return -1; // Não encontrado
}

/* 3. Função para printar a estrutura de labels */
void printLabels(labelControl* head) {
    printf("\n======================= TABELA DE LABELS =======================\n");
    
    if (head == NULL) {
        printf("A lista de labels está vazia.\n");
        printf("================================================================\n\n");
        return;
    }

    labelControl* current = head;
    while (current != NULL) {
        printf(" ├── Label: %-15s | Linha de Destino: %-5d\n", 
               current->labelName, 
               current->lineNumber);
        current = current->next;
    }
    
    printf("================================================================\n\n");
}

void printStack() {
    variablesAtStack* currentScope = scopesHead;

    printf("\n=================== ESPELHO DA PILHA VIRTUAL ===================\n");
    
    if (currentScope == NULL) {
        printf("A pilha de escopos está totalmente vazia.\n");
        printf("================================================================\n\n");
        return;
    }

    // Percorre a lista encadeada de escopos
    while (currentScope != NULL) {
        printf("Escopo: [%s]\n", currentScope->scope);
        
        variables* currentVar = currentScope->var;
        
        if (currentVar == NULL) {
            printf("  └─ (Nenhuma variável alocada neste escopo)\n");
        } else {
            // Percorre a lista encadeada de variáveis dentro deste escopo
            while (currentVar != NULL) {
                printf("  ├── Variável: %-10s | Tamanho (Size): %-3d | Deslocamento (Offset): %-3d\n", 
                       currentVar->name, 
                       currentVar->size, 
                       currentVar->offset);
                currentVar = currentVar->next;
            }
        }
        printf("----------------------------------------------------------------\n");
        currentScope = currentScope->next;
    }
    printf("================================================================\n\n");
}

void printTempControl(tempControl* functionsHead) {
    tempControl* current_node = functionsHead;

    printf("\n=================== ESPELHO DO CONTROLE TEMPORÁRIO ===================\n");
    
    if (current_node == NULL) {
        printf("A lista de controle temporário está totalmente vazia.\n");
        printf("=======================================================================\n\n");
        return;
    }

    // Percorre a lista encadeada de controle temporário
    while (current_node != NULL) {
        // Exibe o nome da variável temporária como o identificador do bloco
        printf("Temporária: [%s]\n", current_node->temp.name ? current_node->temp.name : "NULL");
        
        // Exibe os atributos alinhados para manter a legibilidade
        printf("  ├── Valor (Val): %-5d | Tipo (Kind): %-3d\n", 
               current_node->temp.val, 
               current_node->temp.kind);
        printf("  └── Soma (Sum): %-6d | Seguro (Safe): %-3d\n", 
               current_node->sum, 
               current_node->safe);
               
        printf("-----------------------------------------------------------------------\n");
        current_node = current_node->next;
    }
    printf("=======================================================================\n\n");
}

void printFuncLabel(FuncLabel *current_func) {

    printf("\n==================== ESPELHO DOS RÓTULOS DE FUNÇÃO ====================\n");
    
    if (current_func == NULL) {
        printf("A lista de rótulos de função está totalmente vazia.\n");
        printf("=======================================================================\n\n");
        return;
    }

    // Percorre a lista encadeada de funções/rótulos
    while (current_func != NULL) {
        printf("Função/Escopo: [%s]\n", current_func->name ? current_func->name : "NULL");
        printf("  ├── Chamada Segura (SafeCall): %-3d\n", current_func->safeCall);
        
        // Verifica se o ponteiro interno Address existe antes de acessar seus dados
        if (current_func->label == NULL) {
            printf("  └── Rótulo (Label): (Nenhum endereço/rótulo associado)\n");
        } else {
            printf("  └── Rótulo (Label) -> Nome: %-10s | Valor: %-3d | Tipo: %-3d\n", 
                   current_func->label->name ? current_func->label->name : "NULL", 
                   current_func->label->val, 
                   current_func->label->kind);
        }
        
        printf("-----------------------------------------------------------------------\n");
        current_func = current_func->next;
    }
    printf("=======================================================================\n\n");
}

void printRegControl(regControl* regVector) {
    int n = NUMBER_OF_REGISTERS;
    printf("\n=================== ESPELHO DO CONTROLE DE REGISTRADORES ===================\n");
    
    if (regVector == NULL || n <= 0) {
        printf("O vetor de registradores está vazio ou não foi inicializado.\n");
        printf("============================================================================\n\n");
        return;
    }

    // Percorre o vetor usando índices numéricos
    for (int i = 0; i < n; i++) {
        printf("Registrador: [%s]\n", regVector[i].reg.name ? regVector[i].reg.name : "NULL");
        
        // Exibe o labelName (mostra "Nenhum" caso seja NULL)
        printf("  ├── Rótulo Associado (LabelName): %s\n", 
               regVector[i].labelName ? regVector[i].labelName : "(Nenhum)");
               
        printf("  └── Valor (Val): %-4d | Soma (Sum): %-4d | Seguro (Safe): %-3d\n", 
               regVector[i].reg.val, 
               regVector[i].sum, 
               regVector[i].safe);
               
        printf("----------------------------------------------------------------------------\n");
    }
    printf("============================================================================\n\n");
}

// Lógica de eliminar uma quádrupla
Quad* removeQuad(Quad* before, Quad* current){
    if (current == NULL) return NULL;

    Quad* nextNode = current->next;
    if (before != NULL) {
        before->next = nextNode;
    }

    return NULL;
}

// Gera o arquivo Assembly no formato final (canônico)
void fprintRealAssemblyCode(FILE* out, Quad* head) {
    Quad *current = head;
    
    while(current != NULL) {
        // Tratamento exclusivo para Labels: "L1:\n"
        if(current->op == OP_LABEL) {
            fprintAddr(out, current->addr1);
            fprintf(out, ":\n");
            current = current->next;
            continue;
        }

        // Para instruções normais, inicia com um TAB e o mnemônico
        fprintf(out, "\t%s", opToString(current->op));

        // Flag para controlar se precisamos imprimir uma vírgula
        int has_prev_arg = 0;

        // Processa o primeiro operando
        if(current->addr1.kind != EMPTY) {
            fprintf(out, " ");
            fprintAddr(out, current->addr1);
            has_prev_arg = 1;
        }
        
        // Processa o segundo operando
        if(current->addr2.kind != EMPTY) {
            if(has_prev_arg) fprintf(out, ", ");
            else fprintf(out, " ");
            
            fprintAddr(out, current->addr2);
            has_prev_arg = 1;
        }

        // Processa o terceiro operando
        if(current->addr3.kind != EMPTY) {
            if(has_prev_arg) fprintf(out, ", ");
            else fprintf(out, " ");
            
            fprintAddr(out, current->addr3);
        }

        fprintf(out, "\n");
        current = current->next;
    }
}

Quad* generateAssembly(Quad* quadHead, FuncLabel* funHead, tempControl *tempControlHead){
    regControl *regVector = populateRegisters(NUMBER_OF_REGISTERS);

    // Debug
    printFuncLabel(funHead);
    printTempControl(tempControlHead);
    printRegControl(regVector);

    Address* PilhaGlobal = createRegisterAddr(61);
    int lineNumber = 0;
    
    // Move o valor inicial para a pilha global
    Quad* start = (Quad *)malloc(sizeof(Quad));
    start->op = OP_MOVI;
    start->addr1 = *PilhaGlobal;
    start->addr2 = createNumericAddr(PILHA_GLOBAL);
    start->addr3 = createEmptyAddr();
    start->next = quadHead;
    quadHead = start;
    lineNumber++;

    Quad* current = start->next;
    Quad* before = start; 
    // Percorre as variável globais alocando espaço
    while(current != NULL && current->op == OP_ALLOC){
        // Verifica tamanho e reduz a pilha global
        int alloc_size = current->addr2.val;
        addVariableToStack("global", current->addr1.name, alloc_size);
        insertQuadAfter(current, OP_SUBI, *PilhaGlobal, *PilhaGlobal, createNumericAddr(alloc_size));
        lineNumber++;
        current = removeQuad(before, current);

        before = before->next;
        current = before->next;
    }

    // Cria a pilha geral no regitrador 60
    Address* PilhaGeral = createRegisterAddr(60);

    // Inializa o valor da pilha geral com o espaço que sobrou e faz jump para main (aparece na ordem inversa abaixo)
    insertQuadAfter(before, OP_JUMP, *searchFuncLabel("main", funHead), createEmptyAddr(), createEmptyAddr());
    insertQuadAfter(before, OP_MOVR, *PilhaGeral, *PilhaGlobal, createEmptyAddr());
    lineNumber += 2;

    char *scope = NULL;
    int isAlloc = 1;
    int safetyControl = 0;
    while(current != NULL){
        // Verificações
        if(scope != NULL){
            // Caso seja uma função insegura, aloca um espaço para o registrador inseguro
            if(safetyControl == 0 && is_function_safe(scope, funHead) == -1){
                // Aloca um espaço para guardar o valor do registrador inseguro
                insertQuadAfter(before, OP_SUB, *PilhaGeral, *PilhaGeral, createNumericAddr(1));
                // Adiciona a variável usada para o registrador inseguro na pilha
                addVariableToStack(scope, "&safeReg", 1);

                current = before;   // Força o atual a estar olhando para a anterior
                                    // Na próxima execução, o atual será a instrução inserida após before
            }
            safetyControl = 1;

            // A primeira instrução que não for argumento após uma função é um retorno (exceção da main)
            if(current->op != OP_ARG && isAlloc == 1 && strcmp("main", scope) != 0){
                addVariableToStack(scope, "&ret", 1);
                isAlloc = 0;
            }
        }

        switch (current->op)
        {
        case OP_FUN:
            scope = current->addr2.name;
            isAlloc = 1;
            safetyControl = 0;
            current = removeQuad(before, current);
            break;
        case OP_ARG:
            addVariableToStack(scope, current->addr1.name, current->addr2.val);
            current = removeQuad(before, current);
            break;
        case OP_ALLOC:
            int alloc_size = current->addr2.val;
            addVariableToStack(scope, current->addr1.name, alloc_size);
            insertQuadAfter(current, OP_SUB, *PilhaGeral, *PilhaGeral, createNumericAddr(alloc_size));
            current = removeQuad(before, current);
            break;
        case OP_FREE:
            // Libera alocações após while
            int i = current->addr1.val;
            while(i != 0){
                i -= 1;
                removeVariableFromStack(scope);
            }
            insertQuadAfter(current, OP_ADDI, *PilhaGeral, *PilhaGeral, createNumericAddr(current->addr1.val));
            current = removeQuad(before, current);
            break;
        case OP_LOAD:
            // Atualiza a operação de load
            if(current->addr3.kind == INT_CONST){
                current->op = OP_LOADDI;

                if(current->addr2.kind == STRING_VAR){
                    char* varNameLoad = current->addr2.name;
                    int varShiftLoad = verifyVariableShift(scope, varNameLoad);
                    // Caso seja uma variável local
                    if(varShiftLoad != -1){
                        current->addr2 = *PilhaGeral;   // Utiliza a pilha geral
                        current->addr3.val += varShiftLoad; // Soma a posição da pilha com o índice desejado
                    }
                    // Caso contrário, busca no global
                    else{
                        varShiftLoad = verifyVariableShift("global", varNameLoad);
                        if(varShiftLoad != -1){
                            current->addr2 = *PilhaGlobal;  // Utiliza a pilha global
                            current->addr3.val += varShiftLoad; // Soma a posição da pilha com o índice desejado
                        }
                        else{
                            printf("Deu ruim variável inexistente! %s\n", varNameLoad);
                        }
                    }
                }
            }
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND){
                current->op = OP_LOADDI;

                // Transforma o terceiro endereço (com o deslocamento) em registrador
                Address loadThirdAddrs = current->addr3;
                if(loadThirdAddrs.kind = TEMP_VAR){
                    loadThirdAddrs = *allocate_register(loadThirdAddrs.name, tempControlHead, regVector);
                }

                if(current->addr2.kind == STRING_VAR){
                    Address pilhaload;

                    char* varNameload = current->addr2.name;
                    int varShiftload = verifyVariableShift(scope, varNameload);
                    if(varShiftload != -1){
                        pilhaload = *PilhaGeral;   // Utiliza a pilha geral
                    }
                    else{
                        varShiftload = verifyVariableShift("global", varNameload);
                        if(varShiftload != -1){
                            pilhaload = *PilhaGlobal;  // Utiliza a pilha global
                        }
                        else{
                            printf("Deu ruim variável inexistente! %s\n", varNameload);
                            exit(1);
                        }
                    }
                    insertQuadAfter(before, OP_ADD, loadThirdAddrs, loadThirdAddrs, pilhaload);
                    current->addr2 = loadThirdAddrs;
                    // current->addr2 = loadSecondAddrs;
                    current->addr3 = createNumericAddr(varShiftload);
                    current = before;
                }
                else if(current->addr2.kind == TEMP_VAR){
                    Address loadFirstAddrs = *allocate_register(current->addr2.name, tempControlHead, regVector);
                    insertQuadAfter(before, OP_ADD, loadThirdAddrs, loadThirdAddrs, loadFirstAddrs);
                    current->addr2 = loadThirdAddrs;
                    // current->addr2 = loadSecondAddrs;
                    current->addr3 = createNumericAddr(0);
                    current = before;
                }
            }   
            break;
        case OP_STORE:
            // Atualiza a operação de store
            if(current->addr3.kind == INT_CONST){
                current->op = OP_STOREDI;
                
                if(current->addr1.kind == STRING_VAR){
                    char* varNameStore = current->addr1.name;
                    int varShiftStore = verifyVariableShift(scope, varNameStore);
                    // Caso seja uma variável local
                    if(varShiftStore != -1){
                        current->addr1 = *PilhaGeral;   // Utiliza a pilha geral
                        current->addr3.val += varShiftStore; // Soma a posição da pilha com o índice desejado
                    }
                    // Caso contrário, busca no global
                    else{
                        varShiftStore = verifyVariableShift("global", varNameStore);
                        if(varShiftStore != -1){
                            current->addr1 = *PilhaGlobal;  // Utiliza a pilha global
                            current->addr3.val += varShiftStore; // Soma a posição da pilha com o índice desejado
                        }
                        else{
                            printf("Deu ruim variável inexistente! %s\n", varNameStore);
                        }
                    }
                }   
            }
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND){
                current->op = OP_STOREDI;

                // Transforma o terceiro endereço (com o deslocamento) em registrador
                Address storeThirdAddrs = current->addr3;
                if(storeThirdAddrs.kind = TEMP_VAR){
                    storeThirdAddrs = *allocate_register(storeThirdAddrs.name, tempControlHead, regVector);
                }

                if(current->addr1.kind == STRING_VAR){
                    Address pilhaStore;

                    char* varNameStore = current->addr1.name;
                    int varShiftStore = verifyVariableShift(scope, varNameStore);
                    if(varShiftStore != -1){
                        pilhaStore = *PilhaGeral;   // Utiliza a pilha geral
                    }
                    else{
                        varShiftStore = verifyVariableShift("global", varNameStore);
                        if(varShiftStore != -1){
                            pilhaStore = *PilhaGlobal;  // Utiliza a pilha global
                        }
                        else{
                            printf("Deu ruim variável inexistente! %s\n", varNameStore);
                            exit(1);
                        }
                    }
                    insertQuadAfter(before, OP_ADD, storeThirdAddrs, storeThirdAddrs, pilhaStore);
                    current->addr1 = storeThirdAddrs;
                    // current->addr2 = storeSecondAddrs;
                    current->addr3 = createNumericAddr(varShiftStore);
                    current = before;
                }
                else if(current->addr1.kind == TEMP_VAR){
                    Address storeFirstAddrs = *allocate_register(current->addr1.name, tempControlHead, regVector);
                    insertQuadAfter(before, OP_ADD, storeThirdAddrs, storeThirdAddrs, storeFirstAddrs);
                    current->addr1 = storeThirdAddrs;
                    // current->addr2 = storeSecondAddrs;
                    current->addr3 = createNumericAddr(0);
                    current = before;
                }
            }   
            break;      
        case OP_MOV:
            if(current->addr2.kind == STRING_VAR){
                char* varNameStore = current->addr2.name;
                int varShiftStore = verifyVariableShift(scope, varNameStore);
                // Caso seja uma variável local
                if(varShiftStore != -1){
                    current->addr2 = *PilhaGeral;   // Utiliza a pilha geral
                    current->addr3.val += varShiftStore; // Soma a posição da pilha com o índice desejado
                }
                // Caso contrário, busca no global
                else{
                    varShiftStore = verifyVariableShift("global", varNameStore);
                    if(varShiftStore != -1){
                        current->addr2 = *PilhaGlobal;  // Utiliza a pilha global
                        current->addr3.val += varShiftStore; // Soma a posição da pilha com o índice desejado
                    }
                    else{
                        printf("Deu ruim variável inexistente! %s Antes esse erro do que outro:", varNameStore);
                    }
                }
                current->op = OP_ADDI;
            }

            else if(current->addr2.kind == TEMP_VAR || current->addr2.kind == REGISTER_KIND){
                current->op = OP_MOVR;
            }
            else if(current->addr2.kind == INT_CONST || current->addr2.kind == LABEL_KIND) current->op = OP_MOVI;
            break;
        case OP_ADD:
            if(current->addr3.kind == INT_CONST) current->op = OP_ADDI;
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND) current->op = OP_ADDR;
            break;
        case OP_SUB:
            if(current->addr3.kind == INT_CONST) current->op = OP_SUBI;
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND) current->op = OP_SUBR;
            break;
        case OP_DIV:
            if(current->addr3.kind == INT_CONST) current->op = OP_DIVI;
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND) current->op = OP_DIVR;
            break;
        case OP_MUL:
            if(current->addr3.kind == INT_CONST) current->op = OP_MULI;
            else if(current->addr3.kind == TEMP_VAR || current->addr3.kind == REGISTER_KIND) current->op = OP_MULR;
            break;
        case OP_PARAM:
            // Apenas faz a alocação do espaço, a liberação é feita na função que for chamada

            // Caso a próxima instrução for um call output
            // Apenas adiciona o registrador a ser escrito nela
            if(current->next != NULL && current->next->op == OP_CALL &&
               strcmp(current->next->addr2.name, "output") == 0){
                current->next->addr1 = current->addr1;
            }
            // Caso geral de parâmetro
            else{
                // Aloca um espaço na pilha || Adiciona o parâmetro
                Address* paramReg = allocate_register(current->addr1.name, tempControlHead, regVector);

                insertQuadAfter(current, OP_STORE, *PilhaGeral, *paramReg, createNumericAddr(1));
                insertQuadAfter(current, OP_SUB, *PilhaGeral, *PilhaGeral, createNumericAddr(1));

                // Incrementa o número de pametros na pilha
                addVariableToStack(scope, "&param", 1);

                // Acompanha o número de parâmetros alocados por escopo
                update_and_get_allocated_param(scope, 1, funHead);
            }
            
            current = removeQuad(before, current);
            break;
        case OP_LABEL:
            insertLabel(&labelHead, current->addr1.name, lineNumber);
            current->addr1.val = lineNumber;
            break;
        case OP_CALL:
            // Adiciona o endereço de retorno à pilha e recebe o retorno da função
            Address* retAddrs = createLabelAddr();

            // Acompanha o número de parâmetros liberados após a chamada
            update_and_get_allocated_param(scope, -current->addr3.val, funHead);

            char* funcName = current->addr2.name;
            // Caso seja input, apenas faz um input
            if(strcmp(funcName, "input") == 0){
                Address inputReg = *allocate_register(current->addr1.name, tempControlHead, regVector);
                insertQuadAfter(current, OP_IN, inputReg, createEmptyAddr(), createEmptyAddr()); 
                // Não possui parâmetros para estarem na pilha
            }
            // Caso seja output
            else if(strcmp(funcName, "output") == 0){
                Address outputReg = *allocate_register(current->addr1.name, tempControlHead, regVector);
                insertQuadAfter(current, OP_OUT, outputReg, createEmptyAddr(), createEmptyAddr());

                // O parâmetro está escrito no primeiro endereço da quádrupla e não na pilha
            }
            // Caso geral de chamada de função
            else{
                Address* tempWithRetAddr = createTempAddr();

                // Identifica, se houver, um registrador inseguro
                Address* unsafeReg = find_unsafe_active_register(regVector);

                
                // 7 - Recupera o valor do registrador inseguro, se houver
                if(is_function_safe(scope, funHead) == -1 && unsafeReg != NULL){
                    int auxOffset = verifyVariableShift(scope, "&safeReg") - current->addr3.val; // Menos o que vai ser liberado no final

                    // 1 - Carrega o valor para o registrador inseguro
                    insertQuadAfter(current, OP_LOAD, *unsafeReg, *PilhaGeral, createNumericAddr(auxOffset));
                }
                // 6 - Faz a leitura do valor retornado se houver
                if(current->addr1.kind == TEMP_VAR){
                    // 2 - Libera o espaço da pilha
                    insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, createNumericAddr(1));
                    // 1 - Carrega o último valor da pilha (o retorno de uma função)
                    // Não pode ser o próprio registrador inseguro, pois ele ainda tem mais uso
                    Address funretReg = *allocate_register(current->addr1.name, tempControlHead, regVector);
                    insertQuadAfter(current, OP_LOAD, funretReg,*PilhaGeral, createNumericAddr(1));

                }
                // 5 - Cria label de retorno
                insertQuadAfter(current, OP_LABEL, *retAddrs, createEmptyAddr(), createEmptyAddr());
                // 4 - Faz a chamada da função
                insertQuadAfter(current, OP_JUMP, *searchFuncLabel(funcName, funHead), createEmptyAddr(), createEmptyAddr());
                // 3 - Adiciona o registrador com o endereço de retorno à pilha
                insertQuadAfter(current, OP_STORE, *PilhaGeral, *tempWithRetAddr, createNumericAddr(1));
                // 2 - Aloca espaço na pilha
                insertQuadAfter(current, OP_SUB, *PilhaGeral, *PilhaGeral, createNumericAddr(1));
                // 1 - Move o endereço de retorno para um registrador
                insertQuadAfter(current, OP_MOV, *tempWithRetAddr, *retAddrs, createEmptyAddr());

                // 0 - Guarda o valor do registrador inseguro, se houver
                if(is_function_safe(scope, funHead) == -1 && unsafeReg != NULL){                  
                    int auxOffset = verifyVariableShift(scope, "&safeReg");
                    Address* auxTemp = createTempAddr();

                    // 1 - Guarda o valor do registrador inseguro
                    insertQuadAfter(current, OP_STORE, *PilhaGeral, *unsafeReg, createNumericAddr(auxOffset));
                }

                // Remove os parâmetros na pilha de variáveis da função
                for(int i = 0; i < current->addr3.val; i++){
                    removeVariableFromStack(scope);
                }
            }

            current = removeQuad(before, current);
            break;
        case OP_RET:
        case OP_END_FUN:
            // Ao chegar ao fim, libera todo o espaço alocado na pilha para ela...
            // Incluindo o dos parâmetros que recebeu
            int stackUp = getScopeVariableShift(scope);
            Address stackUpAddr = createNumericAddr(stackUp);

            // Função main
            if(strcmp(scope, "main") == 0){
                insertQuadAfter(current, OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
                insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, stackUpAddr);
                current = removeQuad(before, current);
            }

            // Todas as demais
            else{
                Address pilha;
                int retShift = verifyVariableShift(scope, "&ret");
                // Caso não esteja no escopo local, procura no global
                if(retShift != -1){
                    pilha = *PilhaGeral;
                }
                else{
                    retShift = verifyVariableShift("global", "&ret");
                    if(retShift != -1) pilha = *PilhaGlobal;
                    else{
                        printf("Deu ruim variável inexistente! %s Antes esse erro do que outro:", "&ret");
                    }
                }

                Address* retAddr =  allocate_register(createTempAddr()->name, tempControlHead, regVector);

                // Caso haja um retorno de algum valor
                if(current->op == OP_RET && current->addr1.kind != EMPTY ){
                    Address* regAddr = allocate_register(current->addr1.name, tempControlHead, regVector);

                    // Carrega o endereço de retorno || Libera a pilha (n-1) || Adiciona o retorno na pilha || Faz o salto
                    insertQuadAfter(current, OP_JR, *retAddr, createEmptyAddr(), createEmptyAddr());
                    stackUpAddr.val -= 1;
                    insertQuadAfter(current, OP_STORE, pilha, *regAddr, createNumericAddr(1));
                    insertQuadAfter(current, OP_ADD, pilha, pilha, stackUpAddr);
                    insertQuadAfter(current, OP_LOAD, *retAddr,pilha, createNumericAddr(retShift));
                }
                // Caso não haja retorno de valor
                else{
                    // Carrega o endereço de retorno || Libera a pilha || Faz o salto
                    insertQuadAfter(current, OP_JR, *retAddr, createEmptyAddr(), createEmptyAddr());
                    insertQuadAfter(current, OP_ADD, pilha, pilha, stackUpAddr);
                    insertQuadAfter(current, OP_LOAD, *retAddr, pilha, createNumericAddr(retShift));
                }
                
                current = removeQuad(before, current);
            }
            break;
        default:
            break;
        }

        if(current != NULL){
            if(current->addr1.kind == TEMP_VAR) current->addr1 = *allocate_register(current->addr1.name, tempControlHead, regVector);
            if(current->addr2.kind == TEMP_VAR) current->addr2 = *allocate_register(current->addr2.name, tempControlHead, regVector);
            if(current->addr3.kind == TEMP_VAR) current->addr3 = *allocate_register(current->addr3.name, tempControlHead, regVector);
        }

        if(current != NULL && current != before && current->op != OP_LABEL) lineNumber++;

        // Lógica de percorrimento
        if(current == NULL && before != NULL) current = before->next;
        else{
            before = current;
            current = current->next;
        }
    }
    // Debug
    printStack();
    printRegControl(regVector);
    printLabels(labelHead);
    
    check_register_leaks(regVector);    // Todos registradores devem ter soma 0 ao final
    printf("Total number of lines of assembly code: %d\n", lineNumber - 1); // Sempre aponta para a próxima linha
    
    // Quadruplas
    FILE* quadruplesFile = fopen("output/quadruples_assembly_code.txt", "w");
    fprintCode(quadruplesFile, quadHead);
    fclose(quadruplesFile);
    
    // Assembly
    FILE* assemblyFile = fopen("output/assembly_code.txt", "w");
    fprintRealAssemblyCode(assemblyFile, quadHead);
    fclose(assemblyFile);
    
    return quadHead;
}