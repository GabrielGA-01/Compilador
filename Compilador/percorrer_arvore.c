/*******************************************************************************
 * Arquivo: percorrer_arvore.c
 * 
 * VERSÃO ALTERNATIVA DE GERAÇÃO DE CÓDIGO
 * 
 * Este arquivo contém uma implementação alternativa/simplificada de geração
 * de código intermediário. Serve como exemplo didático de como percorrer
 * a AST e gerar código de três endereços.
 * 
 * NOTA: Esta implementação NÃO está completa e serve principalmente como
 * referência ou prototipação. A implementação principal está em cgen.c.
 * 
 * CONCEITOS DEMONSTRADOS:
 * 1. Travessia recursiva da AST
 * 2. Geração de temporários para subexpressões
 * 3. Geração de labels para controle de fluxo
 * 4. Emisão de quádruplas para diferentes tipos de nós
 ******************************************************************************/

#include "cgen.h"
#include "ast.h"

/*******************************************************************************
 * FUNÇÃO PRINCIPAL DE GERAÇÃO
 ******************************************************************************/

/**
 * cgen: Gera código intermediário para um nó da AST.
 * 
 * Esta é uma função recursiva que percorre a AST em profundidade (depth-first)
 * e gera as quádruplas correspondentes a cada tipo de nó.
 * 
 * PARÂMETROS:
 *   node - Nó da AST a ser processado
 * 
 * RETORNO:
 *   Address contendo o resultado da expressão (onde o valor está armazenado)
 *   - Para números: retorna o valor diretamente
 *   - Para variáveis: retorna o nome da variável
 *   - Para operações: retorna o temporário onde o resultado foi armazenado
 *   - Para comandos: retorna endereço vazio
 * 
 * ESTRATÉGIA DE TRAVESSIA:
 *   A função usa um switch sobre o tipo do nó para decidir como processar.
 *   Para cada tipo, implementa a lógica específica de geração de código.
 */
Address cgen(ASTNode *node) {
    /* Endereço vazio como valor padrão de retorno */
    Address result = {EMPTY};
    
    /* Caso base: nó nulo não gera código */
    if (node == NULL) return result;

    switch (node->type) {
        
        /*=====================================================================
         * LITERAL NUMÉRICO
         * 
         * Um número não precisa de código - é um valor imediato.
         * Simplesmente retornamos o valor para que o nó pai possa usar.
         * 
         * EXEMPLO:
         *   42 -> não gera código, retorna Address{kind=INT_CONST, val=42}
         *====================================================================*/
        case NODE_NUM:
            return createVal(node->number);

        /*=====================================================================
         * REFERÊNCIA A VARIÁVEL
         * 
         * Similar a números - apenas retorna o nome da variável.
         * O nó pai decide se vai ler ou escrever nessa variável.
         * 
         * EXEMPLO:
         *   x -> não gera código, retorna Address{kind=STRING_VAR, name="x"}
         *====================================================================*/
        case NODE_VAR:
            return createVar(node->identifier);

        /*=====================================================================
         * EXPRESSÃO DE ATRIBUIÇÃO
         * 
         * x = expr
         * 
         * PASSOS:
         * 1. Gera código para a expressão (lado direito)
         * 2. Gera código para o destino (lado esquerdo - normalmente uma var)
         * 3. Emite quádrupla de atribuição
         * 
         * QUÁDRUPLA GERADA:
         *   (ASSIGN, <valor>, _, <destino>)
         * 
         * EXEMPLO:
         *   x = 5 + y
         *   -> (ADD, 5, y, t1)
         *   -> (ASSIGN, t1, _, x)
         *====================================================================*/
        case NODE_ASSIGN_EXPR:
            {
                /* Gera código para a expressão do lado direito */
                Address right = cgen(node->rightChild);
                
                /* Obtém o destino (normalmente é apenas uma variável) */
                Address left = cgen(node->leftChild);
                
                /* Cria endereço vazio para operando não utilizado */
                Address empty;
                empty.kind = EMPTY;
                
                /* Emite a atribuição: destino = valor */
                emit(ASSIGN, right, empty, left);
                
                /* Retorna o destino (para encadeamento de atribuições) */
                return left;
            }
            break;

        /*=====================================================================
         * OPERAÇÃO BINÁRIA (Aritmética ou Relacional)
         * 
         * expr1 op expr2
         * 
         * PASSOS:
         * 1. Gera código para operando esquerdo -> resultado em t1
         * 2. Gera código para operando direito -> resultado em t2
         * 3. Cria novo temporário t3
         * 4. Emite operação: (op, t1, t2, t3)
         * 5. Retorna t3
         * 
         * EXEMPLO:
         *   a + b * c
         *   -> (MUL, b, c, t1)    ; primeiro b*c (maior precedência)
         *   -> (ADD, a, t1, t2)   ; depois a+t1
         *   -> retorna t2
         * 
         * NOTA: Esta implementação simplificada assume que 'op' é ADD.
         * Uma implementação completa deveria mapear node->number para QuadOp.
         *====================================================================*/
        case NODE_BINARY_OP:
            {
                /* Gera código para o operando esquerdo */
                Address op1 = cgen(node->leftChild);
                
                /* Gera código para o operando direito */
                Address op2 = cgen(node->rightChild);
                
                /* Cria temporário para o resultado */
                Address temp = createTemp();
                
                /*-------------------------------------------------------------
                 * Mapeamento de Operador
                 * 
                 * NOTA: Esta versão simplifica assumindo ADD.
                 * Uma versão completa usaria switch sobre node->number:
                 * 
                 * switch(node->number) {
                 *     case ADD: opCode = OP_ADD; break;
                 *     case SUB: opCode = OP_SUB; break;
                 *     case MUL: opCode = OP_MUL; break;
                 *     case DIV: opCode = OP_DIV; break;
                 *     ...
                 * }
                 *------------------------------------------------------------*/
                QuadOp opCode;
                opCode = ADD; /* Simplificação para exemplo */

                /* Emite a operação */
                emit(opCode, op1, op2, temp);
                
                /* Retorna o temporário com o resultado */
                return temp;
            }
            break;

        /*=====================================================================
         * COMANDO IF (Condicional)
         * 
         * if (cond) stmt
         * 
         * ESTRUTURA DO CÓDIGO GERADO:
         * 
         *   <código para avaliar condição> -> resultado em t1
         *   (IF_FALSE, t1, _, L_False)     ; se t1 é falso, pula para L_False
         *   <código do bloco then>
         *   (LABEL, L_False, _, _)         ; ponto de chegada se condição falsa
         * 
         * NOTA SOBRE IF-ELSE:
         * Para if-else, seria necessário adicionar:
         *   (GOTO, L_End, _, _)    ; antes do label L_False
         *   (LABEL, L_False)       ; início do else
         *   <código do else>
         *   (LABEL, L_End)         ; fim do if-else
         *====================================================================*/
        case NODE_IF_STMT:
            {
                /* Gera código para a condição */
                Address cond = cgen(node->leftChild);
                
                /* Cria label para onde pular se condição for falsa */
                Address labelFalse = createLabel();
                
                Address empty;
                empty.kind = EMPTY;
                
                /* Emite desvio condicional */
                emit(IF_FALSE, cond, empty, labelFalse);
                
                /* Gera código do bloco then */
                cgen(node->rightChild);
                
                /* Se tivesse else, precisaria:
                 * 1. Criar labelEnd
                 * 2. Emitir GOTO labelEnd antes do labelFalse
                 * 3. Emitir código do else
                 * 4. Emitir LABEL labelEnd no final
                 */
                
                /* Emite label para o ponto após o if */
                emit(LABEL, labelFalse, empty, empty);
                
                return result;
            }
            break;
            
        /*=====================================================================
         * BLOCO COMPOSTO (Compound Statement)
         * 
         * { stmt1; stmt2; ... }
         * 
         * ESTRATÉGIA:
         * Itera pela lista de statements (conectados via ->next) e
         * gera código para cada um sequencialmente.
         *====================================================================*/
        case NODE_COMPOUND_STMT:
            {
                /* A lista de statements está no filho direito */
                ASTNode *stmt = node->rightChild;
                
                /* Processa cada statement da lista */
                while (stmt != NULL) {
                    cgen(stmt);
                    stmt = stmt->next;  /* Vai para o próximo statement */
                }
            }
            break;

        /*=====================================================================
         * CASO DEFAULT
         * 
         * Para nós não reconhecidos, tenta processar os filhos.
         * Isso garante que a travessia continue mesmo para tipos
         * não implementados.
         *====================================================================*/
        default:
            /* Tenta processar filhos recursivamente */
            cgen(node->leftChild);
            cgen(node->rightChild);
            break;
    }
    
    return result;
}
