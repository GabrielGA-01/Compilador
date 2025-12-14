/*******************************************************************************
 * Arquivo: cgen.c
 * 
 * IMPLEMENTAÇÃO DA GERAÇÃO DE CÓDIGO INTERMEDIÁRIO
 * 
 * Este arquivo implementa a geração de código intermediário no formato
 * de quádruplas (Three-Address Code). O código intermediário é gerado
 * a partir da AST, representando o programa de forma independente de
 * máquina específica.
 * 
 * ESTRUTURA DO CÓDIGO INTERMEDIÁRIO:
 * - Cada quádrupla representa uma operação simples
 * - Variáveis temporárias armazenam resultados parciais
 * - Labels controlam o fluxo de execução
 * - A lista de quádruplas é percorrida sequencialmente
 * 
 * EXEMPLO DE TRADUÇÃO:
 *   Código C-:        x = a + b * c;
 *   Quádruplas:       (mul, b, c, t1)
 *                     (add, a, t1, t2)
 *                     (asn, t2, x, _)
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgen.h"

/*******************************************************************************
 * VARIÁVEIS GLOBAIS
 ******************************************************************************/

Quad *head = NULL;    /* Início da lista de quádruplas */
Quad *tail = NULL;    /* Fim da lista (para inserção O(1)) */

/* Contadores para geração de nomes únicos */
static int tempCount = 0;   /* t1, t2, t3, ... */
static int labelCount = 0;  /* L1, L2, L3, ... */

/*******************************************************************************
 * FUNÇÕES DE CRIAÇÃO DE ENDEREÇOS
 ******************************************************************************/

/**
 * emptyAddr: Cria um operando vazio.
 * 
 * Usado quando o operando não é necessário em uma quádrupla.
 * EXEMPLO: (goto, L1, _, _) - os campos 2 e 3 são vazios
 * 
 * RETORNO: Address inicializado como EMPTY
 */
Address emptyAddr() {
    Address a;
    a.kind = EMPTY;
    a.val = 0;
    a.name = NULL;
    return a;
}

/**
 * createVal: Cria um operando para constante inteira.
 * 
 * PARÂMETROS:
 *   val - Valor numérico da constante
 * 
 * RETORNO: Address do tipo INT_CONST
 * 
 * EXEMPLO: createVal(10) para o literal "10"
 */
Address createVal(int val) {
    Address a;
    a.kind = INT_CONST;
    a.val = val;
    a.name = NULL;
    return a;
}

/**
 * createVar: Cria um operando para variável nomeada.
 * 
 * PARÂMETROS:
 *   name - Nome da variável (será duplicado)
 * 
 * RETORNO: Address do tipo STRING_VAR
 * 
 * OBSERVAÇÃO: Usa strdup para evitar problemas de vida útil da string
 */
Address createVar(char *name) {
    Address a;
    a.kind = STRING_VAR;
    a.name = strdup(name);  /* Faz cópia da string */
    return a;
}

/**
 * createTemp: Gera um novo temporário único.
 * 
 * Temporários são usados para armazenar resultados intermediários
 * de expressões complexas.
 * 
 * RETORNO: Address do tipo TEMP_VAR com nome único
 * 
 * SEQUÊNCIA: t1, t2, t3, ... (reinicia em cada programa)
 */
Address createTemp() {
    Address a;
    char buffer[20];
    sprintf(buffer, "t%d", ++tempCount);  /* Pré-incremento para começar em t1 */
    a.kind = TEMP_VAR;
    a.name = strdup(buffer);
    return a;
}

/**
 * createLabel: Gera um novo label único.
 * 
 * Labels são pontos de destino para desvios (goto, if_false).
 * 
 * RETORNO: Address do tipo LABEL_KIND com nome único
 * 
 * SEQUÊNCIA: L1, L2, L3, ...
 */
Address createLabel() {
    Address a;
    char buffer[20];
    sprintf(buffer, "L%d", ++labelCount);
    a.kind = LABEL_KIND;
    a.name = strdup(buffer);
    return a;
}

/*******************************************************************************
 * FUNÇÃO EMIT
 ******************************************************************************/

/**
 * emit: Adiciona uma quádrupla à lista de código.
 * 
 * Mantém a lista encadeada de quádruplas, adicionando sempre no final.
 * 
 * PARÂMETROS:
 *   op - Operador da quádrupla
 *   a1 - Primeiro operando (ou vazio)
 *   a2 - Segundo operando (ou vazio)
 *   a3 - Destino/resultado (ou vazio)
 * 
 * EXEMPLO DE USO:
 *   emit(OP_ADD, createVar("a"), createVar("b"), createTemp())
 *   Adiciona: (add, a, b, t1)
 */
void emit(QuadOp op, Address a1, Address a2, Address a3) {
    /* Aloca nova quádrupla */
    Quad *q = (Quad *)malloc(sizeof(Quad));
    q->op = op;
    q->addr1 = a1;
    q->addr2 = a2;
    q->addr3 = a3;
    q->next = NULL;

    /* Adiciona ao final da lista */
    if (head == NULL) {
        /* Lista vazia - primeiro elemento */
        head = tail = q;
    } else {
        /* Lista não vazia - adiciona após o último */
        tail->next = q;
        tail = q;
    }
}

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO
 ******************************************************************************/

/**
 * opToString: Converte enum QuadOp para string legível.
 * 
 * PARÂMETROS:
 *   op - Operador da quádrupla
 * 
 * RETORNO: String com nome do operador
 */
const char* opToString(QuadOp op) {
    switch(op) {
        case OP_ADD:     return "add";      /* Adição */
        case OP_SUB:     return "sub";      /* Subtração */
        case OP_MUL:     return "mul";      /* Multiplicação */
        case OP_DIV:     return "div";      /* Divisão */
        case OP_ASN:     return "asn";      /* Atribuição */
        case OP_LT:      return "lt";       /* Menor que */
        case OP_LET:     return "let";      /* Menor ou igual */
        case OP_GT:      return "gt";       /* Maior que */
        case OP_GET:     return "get";      /* Maior ou igual */
        case OP_EQ:      return "eq";       /* Igual */
        case OP_DIF:     return "dif";      /* Diferente */
        case OP_IF_F:    return "if_f";     /* Desvio se falso */
        case OP_GOTO:    return "goto";     /* Desvio incondicional */
        case OP_LAB:     return "lab";      /* Label */
        case OP_RD:      return "rd";       /* Leitura (input) */
        case OP_WRI:     return "wri";      /* Escrita (output) */
        case OP_PARAM:   return "param";    /* Parâmetro */
        case OP_CALL:    return "call";     /* Chamada de função */
        case OP_RET:     return "ret";      /* Retorno */
        case OP_HALT:    return "halt";     /* Fim do programa */
        case OP_ARR_ACC: return "arr_acc";  /* Acesso a array */
        case OP_ARR_ASN: return "arr_asn";  /* Atribuição a array */
        default:         return "unknown";
    }
}

/**
 * printAddr: Imprime um operando no stdout.
 * 
 * PARÂMETROS:
 *   a - Endereço (operando) a imprimir
 */
void printAddr(Address a) {
    switch(a.kind) {
        case EMPTY: printf("_"); break;           /* Operando vazio */
        case INT_CONST: printf("%d", a.val); break;  /* Constante */
        case STRING_VAR: printf("%s", a.name); break; /* Variável */
        case TEMP_VAR: printf("%s", a.name); break;   /* Temporário */
        case LABEL_KIND: printf("%s", a.name); break; /* Label */
    }
}

/**
 * fprintAddr: Imprime um operando em um arquivo.
 * 
 * PARÂMETROS:
 *   out - Arquivo de saída
 *   a   - Endereço a imprimir
 */
void fprintAddr(FILE* out, Address a) {
    switch(a.kind) {
        case EMPTY: fprintf(out, "_"); break;
        case INT_CONST: fprintf(out, "%d", a.val); break;
        case STRING_VAR: fprintf(out, "%s", a.name); break;
        case TEMP_VAR: fprintf(out, "%s", a.name); break;
        case LABEL_KIND: fprintf(out, "%s", a.name); break;
    }
}

/**
 * printCode: Imprime toda a lista de quádruplas no stdout.
 * 
 * Formato de saída: (operador, op1, op2, resultado)
 */
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

/**
 * fprintCode: Imprime toda a lista de quádruplas em um arquivo.
 * 
 * PARÂMETROS:
 *   out - Arquivo de saída (já aberto para escrita)
 */
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

/*******************************************************************************
 * FUNÇÃO PRINCIPAL DE GERAÇÃO DE CÓDIGO
 ******************************************************************************/

/**
 * generateCode: Gera código intermediário para um nó da AST.
 * 
 * Esta função recursiva visita cada nó da AST e emite as quádruplas
 * correspondentes. Para expressões, retorna o Address onde o resultado
 * foi armazenado.
 * 
 * ESTRATÉGIA POR TIPO DE NÓ:
 * - NUM: Retorna a constante diretamente
 * - VAR: Retorna o endereço da variável
 * - BINARY_OP: Gera operandos, emite operação, retorna temporário
 * - ASSIGN: Gera valor, emite atribuição
 * - IF: Gera condição, label, desvio condicional
 * - WHILE: Labels, condição, body, goto início
 * - FUN_CALL: Params, call, retorna resultado
 * 
 * PARÂMETROS:
 *   node - Nó da AST a processar
 * 
 * RETORNO:
 *   Address com o resultado (para expressões)
 *   emptyAddr() para comandos
 */
Address generateCode(ASTNode* node) {
    if (node == NULL) return emptyAddr();
    
    Address result;
    
    switch(node->type) {
        /*=====================================================================
         * DECLARAÇÃO DE VARIÁVEL
         * 
         * Declarações não geram código diretamente.
         * A alocação de memória seria feita em tempo de execução.
         *====================================================================*/
        case NODE_VAR_DECL:
            /* Não gera código - apenas registro na tabela de símbolos */
            break;
            
        /*=====================================================================
         * DECLARAÇÃO DE FUNÇÃO
         * 
         * Gera label com nome da função e processa o corpo.
         *====================================================================*/
        case NODE_FUN_DECL: {
            /* Emite label com nome da função */
            if (node->rightChild && node->rightChild->identifier) {
                Address label = createVar(node->rightChild->identifier);
                emit(OP_LAB, label, emptyAddr(), emptyAddr());
            }
            /* Processa corpo da função (compound_stmt) */
            if (node->next != NULL && node->next->type == NODE_FUN_BODY) {
                generateCode(node->next->rightChild);
            }
            break;
        }
        
        /*=====================================================================
         * CORPO DA FUNÇÃO
         * 
         * Processa os statements do corpo.
         *====================================================================*/
        case NODE_FUN_BODY:
            generateCode(node->rightChild);
            break;
            
        /*=====================================================================
         * BLOCO COMPOSTO (Compound Statement)
         * 
         * Processa declarações locais e lista de statements.
         *====================================================================*/
        case NODE_COMPOUND_STMT: {
            /* Processa declarações locais */
            ASTNode* decl = node->leftChild;
            while (decl != NULL) {
                generateCode(decl);
                decl = decl->next;
            }
            
            /* Processa statements */
            ASTNode* stmt = node->rightChild;
            while (stmt != NULL) {
                Address result = generateCode(stmt);
                /*-------------------------------------------------------------
                 * Tratamento especial: IF com ELSE
                 * 
                 * Quando NODE_IF_STMT processa seu else, ele retorna -1 como
                 * flag para indicar que o próximo nó (else) já foi consumido.
                 *------------------------------------------------------------*/
                if (result.kind == INT_CONST && result.val == -1) {
                    stmt = stmt->next; /* Pula o else já processado */
                }
                stmt = stmt->next;
            }
            break;
        }
            
        /*=====================================================================
         * COMANDO IF (com ou sem ELSE)
         * 
         * ESTRUTURA GERADA (sem else):
         *   <condição>
         *   (if_f, cond, L1, _)   ; se falso, pula para L1
         *   <then_stmt>
         *   (lab, L1, _, _)       ; label de saída
         * 
         * ESTRUTURA GERADA (com else):
         *   <condição>
         *   (if_f, cond, L1, _)   ; se falso, vai para else
         *   <then_stmt>
         *   (goto, L2, _, _)      ; pula o else
         *   (lab, L1, _, _)       ; início do else
         *   <else_stmt>
         *   (lab, L2, _, _)       ; saída
         *====================================================================*/
        case NODE_IF_STMT: {
            /* Gera código para a condição */
            Address cond = generateCode(node->leftChild);
            Address labelElse = createLabel();
            
            /* Desvio condicional: se falso, pula para labelElse */
            emit(OP_IF_F, cond, labelElse, emptyAddr());
            
            /* Gera código do bloco THEN */
            generateCode(node->rightChild);
            
            /* Verifica se tem ELSE (number=1 indica que tem) */
            if (node->number == 1 && node->next != NULL) {
                /* TEM ELSE */
                Address labelEnd = createLabel();
                emit(OP_GOTO, labelEnd, emptyAddr(), emptyAddr());  /* Pula o else */
                emit(OP_LAB, labelElse, emptyAddr(), emptyAddr());  /* Início do else */
                generateCode(node->next);                            /* Código do else */
                emit(OP_LAB, labelEnd, emptyAddr(), emptyAddr());   /* Saída */
                /* Retorna flag para indicar que consumiu o else */
                return createVal(-1);
            } else {
                /* NÃO TEM ELSE - apenas coloca o label de saída */
                emit(OP_LAB, labelElse, emptyAddr(), emptyAddr());
            }
            break;
        }
        
        /*=====================================================================
         * COMANDO WHILE (Loop)
         * 
         * ESTRUTURA GERADA:
         *   (lab, L1, _, _)       ; início do loop
         *   <condição>
         *   (if_f, cond, L2, _)   ; se falso, sai do loop
         *   <body>
         *   (goto, L1, _, _)      ; volta para o início
         *   (lab, L2, _, _)       ; saída do loop
         *====================================================================*/
        case NODE_WHILE_STMT: {
            Address labelStart = createLabel();
            Address labelEnd = createLabel();
            
            /* Label do início do loop */
            emit(OP_LAB, labelStart, emptyAddr(), emptyAddr());
            
            /* Gera e testa condição */
            Address cond = generateCode(node->leftChild);
            emit(OP_IF_F, cond, labelEnd, emptyAddr());
            
            /* Corpo do loop */
            generateCode(node->rightChild);
            
            /* Volta ao início */
            emit(OP_GOTO, labelStart, emptyAddr(), emptyAddr());
            
            /* Label de saída */
            emit(OP_LAB, labelEnd, emptyAddr(), emptyAddr());
            break;
        }
        
        /*=====================================================================
         * COMANDO RETURN
         * 
         * ESTRUTURA GERADA:
         *   <expressão de retorno> (se houver)
         *   (ret, valor, _, _)    ; ou (ret, _, _, _) se void
         *====================================================================*/
        case NODE_RETURN_STMT: {
            if (node->leftChild != NULL) {
                /* Return com valor */
                Address retVal = generateCode(node->leftChild);
                emit(OP_RET, retVal, emptyAddr(), emptyAddr());
            } else {
                /* Return sem valor (void) */
                emit(OP_RET, emptyAddr(), emptyAddr(), emptyAddr());
            }
            break;
        }
        
        /*=====================================================================
         * EXPRESSÃO DE ATRIBUIÇÃO
         * 
         * ESTRUTURA GERADA:
         *   <expressão lado direito>
         *   (asn, valor, destino, _)
         * 
         * Para arrays: (arr_asn, valor, arr, indice)
         *====================================================================*/
        case NODE_ASSIGN_EXPR: {
            /* Gera código para o lado direito */
            Address rhs = generateCode(node->rightChild);
            
            if (node->leftChild->type == NODE_ARRAY_ACCESS) {
                /* Atribuição a elemento de array: arr[i] = valor */
                Address arrName = createVar(node->leftChild->leftChild->identifier);
                Address index = generateCode(node->leftChild->rightChild);
                emit(OP_ARR_ASN, rhs, arrName, index);
            } else {
                /* Atribuição a variável simples: x = valor */
                Address lhs = createVar(node->leftChild->identifier);
                emit(OP_ASN, rhs, lhs, emptyAddr());
            }
            break;
        }
        
        /*=====================================================================
         * OPERAÇÃO BINÁRIA / OPERADOR RELACIONAL
         * 
         * ESTRUTURA GERADA:
         *   <operando esquerdo>
         *   <operando direito>
         *   (op, op1, op2, temp)
         * 
         * Retorna o temporário onde o resultado foi armazenado.
         *====================================================================*/
        case NODE_BINARY_OP:
        case NODE_OPERATOR: {
            /* Gera código para os operandos */
            Address left = generateCode(node->leftChild);
            Address right = generateCode(node->rightChild);
            Address temp = createTemp();
            
            /* Mapeia token do parser para QuadOp */
            int op = node->number;
            QuadOp quadOp;
            
            /*-----------------------------------------------------------------
             * MAPEAMENTO DE TOKENS PARA OPERADORES
             * 
             * Os valores numéricos (260, 261, etc.) são os tokens definidos
             * pelo Bison no arquivo parser.tab.h
             *----------------------------------------------------------------*/
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
        
        /*=====================================================================
         * LITERAL NUMÉRICO
         * 
         * Retorna diretamente o valor como constante.
         *====================================================================*/
        case NODE_NUM: {
            return createVal(node->number);
        }
        
        /*=====================================================================
         * REFERÊNCIA A VARIÁVEL
         * 
         * Retorna o endereço da variável.
         *====================================================================*/
        case NODE_VAR: {
            if (node->identifier) {
                return createVar(node->identifier);
            }
            break;
        }
        
        /*=====================================================================
         * ACESSO A ELEMENTO DE ARRAY
         * 
         * ESTRUTURA GERADA:
         *   <cálculo do índice>
         *   (arr_acc, arr, indice, temp)
         * 
         * Retorna o temporário com o valor do elemento.
         *====================================================================*/
        case NODE_ARRAY_ACCESS: {
            Address arrName = createVar(node->leftChild->identifier);
            Address index = generateCode(node->rightChild);
            Address temp = createTemp();
            emit(OP_ARR_ACC, arrName, index, temp);
            return temp;
        }
        
        /*=====================================================================
         * CHAMADA DE FUNÇÃO
         * 
         * TRATAMENTO ESPECIAL PARA BUILT-INS:
         * - input(): (rd, temp, _, _)
         * - output(x): (wri, x, _, _)
         * 
         * FUNÇÕES DO USUÁRIO:
         *   (param, arg1, _, _)
         *   (param, arg2, _, _)
         *   ...
         *   (call, funcName, numParams, temp)
         *====================================================================*/
        case NODE_FUN_CALL: {
            /* Obtém nome da função */
            char* funcName = NULL;
            if (node->leftChild && node->leftChild->identifier) {
                funcName = node->leftChild->identifier;
            }
            
            /* Tratamento de funções built-in */
            if (funcName && strcmp(funcName, "input") == 0) {
                /* input() - lê valor da entrada */
                Address temp = createTemp();
                emit(OP_RD, temp, emptyAddr(), emptyAddr());
                return temp;
            } else if (funcName && strcmp(funcName, "output") == 0) {
                /* output(expr) - escreve valor na saída */
                if (node->rightChild) {
                    Address arg = generateCode(node->rightChild);
                    emit(OP_WRI, arg, emptyAddr(), emptyAddr());
                }
                return emptyAddr();
            } else {
                /* Função definida pelo usuário */
                
                /* Emite parâmetros um por um */
                ASTNode* arg = node->rightChild;
                int paramCount = 0;
                while (arg != NULL) {
                    Address argAddr = generateCode(arg);
                    emit(OP_PARAM, argAddr, emptyAddr(), emptyAddr());
                    paramCount++;
                    arg = arg->next;
                }
                
                /* Emite chamada da função */
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
    
    /*-------------------------------------------------------------------------
     * NOTA IMPORTANTE:
     * 
     * A iteração por listas (via node->next) é feita explicitamente nos
     * contextos apropriados (generateProgram, NODE_COMPOUND_STMT), não
     * automaticamente aqui. Isso evita processamento duplicado.
     *------------------------------------------------------------------------*/
    
    return emptyAddr();
}

/*******************************************************************************
 * FUNÇÃO generateProgram - PONTO DE ENTRADA
 ******************************************************************************/

/**
 * generateProgram: Gera código para o programa completo.
 * 
 * Esta é a função principal de geração de código. Ela:
 * 1. Reseta os contadores de temporários e labels
 * 2. Limpa a lista de quádruplas
 * 3. Processa cada declaração top-level
 * 4. Adiciona HALT no final
 * 
 * PARÂMETROS:
 *   tree - Raiz da AST (primeira declaração)
 */
void generateProgram(ASTNode* tree) {
    /* Reseta estado para permitir múltiplas compilações */
    tempCount = 0;
    labelCount = 0;
    head = NULL;
    tail = NULL;
    
    /* Processa cada declaração no nível top */
    ASTNode* current = tree;
    while (current != NULL) {
        /*---------------------------------------------------------------------
         * Filtragem de nós
         * 
         * Alguns tipos de nós não devem gerar código diretamente:
         * - NODE_FUN_BODY: Processado internamente por NODE_FUN_DECL
         * - NODE_PARAM: São filhos do FUN_BODY
         *--------------------------------------------------------------------*/
        if (current->type != NODE_FUN_BODY && current->type != NODE_PARAM) {
            generateCode(current);
        }
        current = current->next;
    }
    
    /* Adiciona instrução HALT para indicar fim do programa */
    emit(OP_HALT, emptyAddr(), emptyAddr(), emptyAddr());
}
