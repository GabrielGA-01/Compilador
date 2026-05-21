#include <stdlib.h>
#include "casm.h"
#include "cintgen.h"
#include "string.h"

#define MEM_SIZE 63
#define PILHA_GLOBAL MEM_SIZE

variablesAtStack* scopesHead = NULL;

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

void addVariableToStack(char* scope, char *varName, int numPositions) {
    if (numPositions <= 0) return;

    // Verifica se há espaço
    if (availableMem - numPositions < -1) {
        printf("Erro: Stack Overflow! Limite posições atingido.\n");
        return;
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

int getFirstVariableShift(char* scope) {
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

    return currentScope->var->offset;
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
            return;
        }
    }
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

// Lógica de eliminar uma quádrupla
Quad* removeQuad(Quad* before, Quad* current){
    if (current == NULL) return NULL;

    Quad* nextNode = current->next;
    if (before != NULL) {
        before->next = nextNode;
    }

    return NULL;
}

void generateAssembly(Quad* quadHead, FuncLabel* funHead){
    Address* PilhaGlobal = createRegisterAddr(61);
    
    // Move o valor inicial para a pilha global
    Quad* start = (Quad *)malloc(sizeof(Quad));
    start->op = OP_MOVI;
    start->addr1 = *PilhaGlobal;
    start->addr2 = createNumericAddr(PILHA_GLOBAL);
    start->addr3 = createEmptyAddr();
    start->next = quadHead;
    quadHead = start;


    Quad* current = start->next;
    Quad* before = start; 

    // Percorre as variável globais alocando espaço
    while(current != NULL && current->op == OP_ALLOC){
        // Verifica tamanho e reduz a pilha global
        int alloc_size = current->addr2.val;
        addVariableToStack("global", current->addr1.name, alloc_size);
        insertQuadAfter(current, OP_SUBI, *PilhaGlobal, *PilhaGlobal, createNumericAddr(alloc_size));
        current = removeQuad(before, current);

        before = before->next;
        current = before->next;
    }

    // Cria a pilha geral no regitrador 60
    Address* PilhaGeral = createRegisterAddr(60);

    // Inializa o valor da pilha geral com o espaço que sobrou e faz jump para main (aparece na ordem inversa abaixo)
    insertQuadAfter(before, OP_JUMP, *searchFuncLabel("main", funHead), createEmptyAddr(), createEmptyAddr());
    insertQuadAfter(before, OP_MOVR, *PilhaGeral, *PilhaGlobal, createEmptyAddr());
    
    char *scope = NULL;
    int isAlloc = 1;
    int numParam = 0;
    while(current != NULL){
        // A primeira instrução que não for argumento após uma função é um retorno (exceção da main)
        if(current->op != OP_ARG && isAlloc == 1 && scope != NULL && strcmp("main", scope) != 0){
            addVariableToStack(scope, "&ret", 1);
            isAlloc = 0;
        }

        switch (current->op)
        {
        case OP_FUN:
            scope = current->addr2.name;
            isAlloc = 1;
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
            break;
        case OP_LOAD:
            // Atualiza a operação de load
            if(current->addr3.kind == INT_CONST) current->op = OP_LOADD;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_LOADDR;
            break;
        case OP_STORE:
            // Atualiza a operação de store
            if(current->addr3.kind == INT_CONST) current->op = OP_STORED;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_STOREDR;
            break;      
        case OP_MOV:
            if(current->addr2.kind == STRING_VAR){
                int varShift = verifyVariableShift(scope, current->addr2.name) + numParam;  // Desconta a posição de parâmetros adicionados à pilha
                current->op = OP_LOADD;
                current->addr2 = *PilhaGeral;
                current->addr3 = createNumericAddr(varShift);
            }
            else if(current->addr2.kind == INT_CONST) current->op = OP_MOVI;
            break;
        case OP_ADD:
            if(current->addr3.kind == INT_CONST) current->op = OP_ADDI;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_ADDR;
            break;
        case OP_SUB:
            if(current->addr3.kind == INT_CONST) current->op = OP_SUBI;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_SUBR;
            break;
        case OP_DIV:
            if(current->addr3.kind == INT_CONST) current->op = OP_DIVI;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_DIVR;
            break;
        case OP_MUL:
            if(current->addr3.kind == INT_CONST) current->op = OP_MULI;
            else if(current->addr3.kind == TEMP_VAR) current->op = OP_MULR;
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
                insertQuadAfter(current, OP_STORE, *PilhaGeral, current->addr1, createNumericAddr(1));
                insertQuadAfter(current, OP_SUB, *PilhaGeral, *PilhaGeral, createNumericAddr(1));

                // Incrementa o número de pametros na pilha
                numParam += 1;
            }
            
            current = removeQuad(before, current);
            break;
        case OP_CALL:
            // Adiciona o endereço de retorno à pilha e recebe o retorno da função
            Address* retAddrs = createLabelAddr();

            char* funcName = current->addr2.name;
            // Caso seja input, apenas faz um input
            if(strcmp(funcName, "input") == 0){
                insertQuadAfter(current, OP_IN, current->addr1, createEmptyAddr(), createEmptyAddr()); 
                
                // Não possui parâmetros para estarem na pilha
            }
            // Caso seja output
            else if(strcmp(funcName, "output") == 0){
                insertQuadAfter(current, OP_OUT, current->addr1, createEmptyAddr(), createEmptyAddr());

                // O parâmetro está escrito no primeiro endereço da quádrupla e não na pilha
            }
            // Caso geral de chamada de função
            else{
                Address* tempWithRetAddr = createTempAddr();

                // 6 - Faz a leitura do valor retornado se houver
                if(current->addr1.kind == TEMP_VAR){
                    // 2 - Libera o espaço da pilha
                    insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, createNumericAddr(1));
                    // 1 - Carrega o último valor da pilha (o retorno de uma função)
                    insertQuadAfter(current, OP_LOAD, current->addr1,*PilhaGeral, createNumericAddr(1));

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

                // Atualiza o número de parâmetros na pilha
                numParam -= current->addr3.val;
            }

            current = removeQuad(before, current);
            break;
        case OP_RET:
        case OP_END_FUN:
            // Ao chegar ao fim, libera todo o espaço alocado na pilha para ela...
            // Incluindo o dos parâmetros que recebeu
            int stackUp = getFirstVariableShift(scope);
            Address stackUpAddr = createNumericAddr(stackUp);

            // Função main
            if(strcmp(scope, "main") == 0){
                insertQuadAfter(current, OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
                insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, stackUpAddr);
                current = removeQuad(before, current);
            }

            // Todas as demais
            else{
                int retShift = verifyVariableShift(scope, "&ret") + numParam;  // Desconta a posição de parâmetros adicionados à pilha;
                Address* retAddr = createTempAddr();

                // Caso haja um retorno de algum valor
                if(current->op == OP_RET && current->addr1.kind != EMPTY ){
                    // Carrega o endereço de retorno || Libera a pilha (n-1) || Adiciona o retorno na pilha || Faz o salto
                    insertQuadAfter(current, OP_JR, *retAddr, createEmptyAddr(), createEmptyAddr());
                    stackUpAddr.val -= 1;
                    insertQuadAfter(current, OP_STORE, *PilhaGeral, current->addr1, createNumericAddr(1));
                    insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, stackUpAddr);
                    insertQuadAfter(current, OP_LOAD, *retAddr, *PilhaGeral, createNumericAddr(retShift));
                }
                // Caso não haja retorno de valor
                else{
                    // Carrega o endereço de retorno || Libera a pilha || Faz o salto
                    insertQuadAfter(current, OP_JR, *retAddr, createEmptyAddr(), createEmptyAddr());
                    insertQuadAfter(current, OP_ADD, *PilhaGeral, *PilhaGeral, stackUpAddr);
                    insertQuadAfter(current, OP_LOAD, *retAddr, *PilhaGeral, createNumericAddr(retShift));
                }
                
                current = removeQuad(before, current);
            }
            break;
        default:
            break;
        }


        if(current == NULL && before != NULL) current = before->next;
        else{
            before = current;
            current = current->next;
        }
    }
    printStack();
    FILE* file = fopen("output/assembly_code.txt", "w");
    fprintCode(file, quadHead);
    fclose(file);
}