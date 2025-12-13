
#include "cgen.h"
#include "ast.h"

/* 
 * Função recursiva que percorre a AST e gera código.
 * Retorna o endereço (Address) onde o resultado da sub-árvore está armazenado.
 */
Address cgen(ASTNode *node) {
    Address result = {EMPTY};
    
    if (node == NULL) return result;

    switch (node->type) {
        
        case NODE_NUM:
            /* Se é um número, retorna o valor direto */
            return createVal(node->number); // Ajustar se number não for o campo certo

        case NODE_VAR:
            /* Se é uma variável, retorna seu nome */
            return createVar(node->identifier);

        case NODE_ASSIGN_EXPR:
            /* 
             * Atribuição: x = expr
             * 1. Gera código para a expressão (lado direito)
             * 2. Emite (ASSIGN, resultado_expr, _, x)
             */
            {
                Address right = cgen(node->rightChild);
                Address left = cgen(node->leftChild); /* Normalmente é só uma var */
                
                Address empty;
                empty.kind = EMPTY;
                
                emit(ASSIGN, right, empty, left);
                return left;
            }
            break;

        case NODE_BINARY_OP:
            /*
             * Operação Binária: expr1 + expr2
             * 1. Gera código para expr1 -> t1
             * 2. Gera código para expr2 -> t2
             * 3. Cria t3
             * 4. Emite (ADD, t1, t2, t3)
             * 5. Retorna t3
             */
            {
                Address op1 = cgen(node->leftChild);
                Address op2 = cgen(node->rightChild);
                Address temp = createTemp();
                
                QuadOp opCode;
                // Mapear operation token para QuadOp
                // Assumindo que node->number guarda o token do operador
                // Isso precisaria de um switch case real aqui
                opCode = ADD; // Simplificacao para exemplo

                emit(opCode, op1, op2, temp);
                return temp;
            }
            break;

        case NODE_IF_STMT:
            /*
             * If (cond) stmt
             * 1. Gera cond -> t1
             * 2. Cria Label L_False
             * 3. Emite (IF_FALSE, t1, _, L_False)
             * 4. Gera stmt
             * 5. Emite (LABEL, L_False, _, _)
             */
            {
                Address cond = cgen(node->leftChild);
                Address labelFalse = createLabel();
                
                Address empty;
                empty.kind = EMPTY;
                
                emit(IF_FALSE, cond, empty, labelFalse);
                
                cgen(node->rightChild); // Then block
                
                // Se tivesse Else, precisaria de mais labels e GOTO
                
                emit(LABEL, labelFalse, empty, empty);
                return result;
            }
            break;
            
        case NODE_COMPOUND_STMT:
            {
                ASTNode *stmt = node->rightChild; // Lista de statements
                while (stmt != NULL) {
                    cgen(stmt);
                    stmt = stmt->next; // Em listas encadeadas, next aponta pro proximo
                }
            }
            break;

        default:
            // Percorre filhos por padrão se não souber o que fazer
            cgen(node->leftChild);
            cgen(node->rightChild);
            break;
    }
    return result;
}
