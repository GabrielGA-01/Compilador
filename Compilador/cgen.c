
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgen.h"

Quad *head = NULL;
Quad *tail = NULL;

static int tempCount = 0;
static int labelCount = 0;

Address emptyAddr() {
    Address a;
    a.kind = EMPTY;
    a.val = 0;
    a.name = NULL;
    return a;
}

Address createVal(int val) {
    Address a;
    a.kind = INT_CONST;
    a.val = val;
    a.name = NULL;
    return a;
}

Address createVar(char *name) {
    Address a;
    a.kind = STRING_VAR;
    a.name = strdup(name);
    return a;
}

Address createTemp() {
    Address a;
    char buffer[20];
    sprintf(buffer, "t%d", ++tempCount);
    a.kind = TEMP_VAR;
    a.name = strdup(buffer);
    return a;
}

Address createLabel() {
    Address a;
    char buffer[20];
    sprintf(buffer, "L%d", ++labelCount);
    a.kind = LABEL_KIND;
    a.name = strdup(buffer);
    return a;
}

void emit(QuadOp op, Address a1, Address a2, Address a3) {
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
}

/* Converte operador para string no formato do slide */
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
        case OP_IF_F:    return "if_f";
        case OP_GOTO:    return "goto";
        case OP_LAB:     return "lab";
        case OP_RD:      return "rd";
        case OP_WRI:     return "wri";
        case OP_PARAM:   return "param";
        case OP_CALL:    return "call";
        case OP_RET:     return "ret";
        case OP_HALT:    return "halt";
        case OP_ARR_ACC: return "arr_acc";
        case OP_ARR_ASN: return "arr_asn";
        default:         return "unknown";
    }
}

void printAddr(Address a) {
    switch(a.kind) {
        case EMPTY: printf("_"); break;
        case INT_CONST: printf("%d", a.val); break;
        case STRING_VAR: printf("%s", a.name); break;
        case TEMP_VAR: printf("%s", a.name); break;
        case LABEL_KIND: printf("%s", a.name); break;
    }
}

void fprintAddr(FILE* out, Address a) {
    switch(a.kind) {
        case EMPTY: fprintf(out, "_"); break;
        case INT_CONST: fprintf(out, "%d", a.val); break;
        case STRING_VAR: fprintf(out, "%s", a.name); break;
        case TEMP_VAR: fprintf(out, "%s", a.name); break;
        case LABEL_KIND: fprintf(out, "%s", a.name); break;
    }
}

void printCode() {
    Quad *current = head;
    while(current != NULL) {
        printf("(%s, ", opToString(current->op));
        printAddr(current->addr1);
        printf(", ");
        printAddr(current->addr2);
        printf(", ");
        printAddr(current->addr3);
        printf(")\n");
        current = current->next;
    }
}

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

Address generateCode(ASTNode* node) {
    if (node == NULL) return emptyAddr();
    
    Address result;
    
    switch(node->type) {
        case NODE_VAR_DECL:
            /* Declaração de variável - não gera código */
            break;
            
        case NODE_FUN_DECL: {
            /* Gera label para a função */
            if (node->rightChild && node->rightChild->identifier) {
                Address label = createVar(node->rightChild->identifier);
                emit(OP_LAB, label, emptyAddr(), emptyAddr());
            }
            /* Processa corpo da função */
            if (node->next != NULL && node->next->type == NODE_FUN_BODY) {
                generateCode(node->next->rightChild); /* compound_stmt */
            }
            break;
        }
        
        case NODE_FUN_BODY:
            /* Corpo da função - processa statements */
            generateCode(node->rightChild);
            break;
            
        case NODE_COMPOUND_STMT: {
            /* Bloco de código - processa declarações locais e statements */
            /* Itera explicitamente pela lista de declarações locais */
            ASTNode* decl = node->leftChild;
            while (decl != NULL) {
                generateCode(decl);
                decl = decl->next;
            }
            /* Itera explicitamente pela lista de statements */
            ASTNode* stmt = node->rightChild;
            while (stmt != NULL) {
                Address result = generateCode(stmt);
                /* Se retornou o flag especial (-1), significa que NODE_IF_STMT
                   consumiu node->next como else, então pula */
                if (result.kind == INT_CONST && result.val == -1) {
                    stmt = stmt->next; /* Pula o else que já foi processado */
                }
                stmt = stmt->next;
            }
            break;
        }
            
        case NODE_IF_STMT: {
            /* if (condição) then_stmt [else else_stmt] 
               node->number == 1 significa que TEM else
               node->number == 0 significa que NÃO tem else */
            Address cond = generateCode(node->leftChild);
            Address labelElse = createLabel();
            
            emit(OP_IF_F, cond, labelElse, emptyAddr());
            
            /* Then branch */
            generateCode(node->rightChild);
            
            if (node->number == 1 && node->next != NULL) {
                /* TEM else - precisa do goto e processar else */
                Address labelEnd = createLabel();
                emit(OP_GOTO, labelEnd, emptyAddr(), emptyAddr());
                emit(OP_LAB, labelElse, emptyAddr(), emptyAddr());
                generateCode(node->next);
                emit(OP_LAB, labelEnd, emptyAddr(), emptyAddr());
                /* Retorna flag especial para indicar que consumiu o else */
                return createVal(-1);
            } else {
                /* NÃO tem else - apenas emite o label de saída */
                emit(OP_LAB, labelElse, emptyAddr(), emptyAddr());
            }
            break;
        }
        
        case NODE_WHILE_STMT: {
            /* while (condição) statement */
            Address labelStart = createLabel();
            Address labelEnd = createLabel();
            
            emit(OP_LAB, labelStart, emptyAddr(), emptyAddr());
            
            Address cond = generateCode(node->leftChild);
            emit(OP_IF_F, cond, labelEnd, emptyAddr());
            
            generateCode(node->rightChild);
            
            emit(OP_GOTO, labelStart, emptyAddr(), emptyAddr());
            emit(OP_LAB, labelEnd, emptyAddr(), emptyAddr());
            break;
        }
        
        case NODE_RETURN_STMT: {
            if (node->leftChild != NULL) {
                Address retVal = generateCode(node->leftChild);
                emit(OP_RET, retVal, emptyAddr(), emptyAddr());
            } else {
                emit(OP_RET, emptyAddr(), emptyAddr(), emptyAddr());
            }
            break;
        }
        
        case NODE_ASSIGN_EXPR: {
            Address rhs = generateCode(node->rightChild);
            
            if (node->leftChild->type == NODE_ARRAY_ACCESS) {
                /* arr[i] = valor */
                Address arrName = createVar(node->leftChild->leftChild->identifier);
                Address index = generateCode(node->leftChild->rightChild);
                emit(OP_ARR_ASN, rhs, arrName, index);
            } else {
                /* var = valor */
                Address lhs = createVar(node->leftChild->identifier);
                emit(OP_ASN, rhs, lhs, emptyAddr());
            }
            break;
        }
        
        case NODE_BINARY_OP:
        case NODE_OPERATOR: {
            Address left = generateCode(node->leftChild);
            Address right = generateCode(node->rightChild);
            Address temp = createTemp();
            
            int op = node->number;
            QuadOp quadOp;
            
            /* Mapeia operador do parser para QuadOp */
            switch(op) {
                case 260: quadOp = OP_ADD; break; /* ADD */
                case 261: quadOp = OP_SUB; break; /* SUB */
                case 262: quadOp = OP_MUL; break; /* MUL */
                case 263: quadOp = OP_DIV; break; /* DIV */
                case 264: quadOp = OP_LT;  break; /* LT */
                case 265: quadOp = OP_LET; break; /* LET */
                case 266: quadOp = OP_GET; break; /* GET */
                case 267: quadOp = OP_GT;  break; /* GT */
                case 268: quadOp = OP_EQ;  break; /* EQ */
                case 269: quadOp = OP_DIF; break; /* DIF */
                default:  quadOp = OP_ADD; break;
            }
            
            emit(quadOp, left, right, temp);
            return temp;
        }
        
        case NODE_NUM: {
            return createVal(node->number);
        }
        
        case NODE_VAR: {
            if (node->identifier) {
                return createVar(node->identifier);
            }
            break;
        }
        
        case NODE_ARRAY_ACCESS: {
            /* arr[i] - retorna temporário com valor */
            Address arrName = createVar(node->leftChild->identifier);
            Address index = generateCode(node->rightChild);
            Address temp = createTemp();
            emit(OP_ARR_ACC, arrName, index, temp);
            return temp;
        }
        
        case NODE_FUN_CALL: {
            /* Chamada de função */
            char* funcName = NULL;
            if (node->leftChild && node->leftChild->identifier) {
                funcName = node->leftChild->identifier;
            }
            
            /* Verifica se é input() ou output() */
            if (funcName && strcmp(funcName, "input") == 0) {
                Address temp = createTemp();
                emit(OP_RD, temp, emptyAddr(), emptyAddr());
                return temp;
            } else if (funcName && strcmp(funcName, "output") == 0) {
                /* output(expr) */
                if (node->rightChild) {
                    Address arg = generateCode(node->rightChild);
                    emit(OP_WRI, arg, emptyAddr(), emptyAddr());
                }
                return emptyAddr();
            } else {
                /* Função definida pelo usuário */
                /* Emite parâmetros */
                ASTNode* arg = node->rightChild;
                int paramCount = 0;
                while (arg != NULL) {
                    Address argAddr = generateCode(arg);
                    emit(OP_PARAM, argAddr, emptyAddr(), emptyAddr());
                    paramCount++;
                    arg = arg->next;
                }
                
                /* Emite chamada */
                Address temp = createTemp();
                Address func = createVar(funcName ? funcName : "unknown");
                Address count = createVal(paramCount);
                emit(OP_CALL, func, count, temp);
                return temp;
            }
        }
        
        default:
            break;
    }
    
    /* REMOVIDA a propagação automática via node->next
       Agora a iteração por listas é feita explicitamente pelo caller
       (generateProgram ou generateStmtList) */
    
    return emptyAddr();
}

/* Gera código para o programa completo */
void generateProgram(ASTNode* tree) {
    /* Reseta contadores */
    tempCount = 0;
    labelCount = 0;
    head = NULL;
    tail = NULL;
    
    /* Gera código para cada declaração no nível top */
    ASTNode* current = tree;
    while (current != NULL) {
        
        /* Pula tipos que não são declarações top-level válidas:
           - NODE_FUN_BODY: processado pelo NODE_FUN_DECL via node->next
           - NODE_PARAM: são filhos do FUN_BODY, não devem gerar código próprio */
        if (current->type != NODE_FUN_BODY && current->type != NODE_PARAM) {
            generateCode(current);
        }
        current = current->next;
    }
    
    /* Adiciona HALT no final */
    emit(OP_HALT, emptyAddr(), emptyAddr(), emptyAddr());
}

