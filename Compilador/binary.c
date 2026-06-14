#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cintgen.h"
#include "casm.h"

char* getOpcodeBinary(QuadOp op) {
    switch(op) {
        case OP_HALT:    return "000000";
        case OP_JUMP:    return "000010";
        case OP_ADDR:    return "000100";
        case OP_SUBR:    return "000101";
        case OP_MULR:    return "000110";
        case OP_DIVR:    return "000111";
        case OP_ADDI:    return "001011";
        case OP_SUBI:    return "001100";
        case OP_MULI:    return "001101";
        case OP_DIVI:    return "001110";
        case OP_MOVR:    return "010100";
        case OP_LOADDI:  return "010101";
        case OP_BEQ:     return "010111";
        case OP_BNE:     return "011000";
        case OP_BLT:     return "011001";
        case OP_BGT:     return "011010";
        case OP_STOREDI: return "011011";
        case OP_MOVI:    return "011101";
        case OP_IN:      return "011111";
        case OP_OUT:     return "100001";
        case OP_JR:      return "100010";
        case OP_BLE:     return "100101";
        case OP_BGE:     return "100110";
        default:         return "000001"; // NOP ou erro
    }
}

#include <stdlib.h>

char* intToBinaryString(int value, int numBits) {
    // Allocate memory for the string + null terminator
    char* binary = (char*)malloc(numBits + 1);
    
    if (binary == NULL) return NULL;

    // Set the null terminator at the end
    binary[numBits] = '\0';

    // Fill the string from right to left
    for (int i = numBits - 1; i >= 0; i--) {
        // Check if the current bit is 1 or 0
        binary[i] = (value & 1) ? '1' : '0';
        // Shift value to the right to process the next bit
        value >>= 1;
    }

    return binary;
}

#include <stdbool.h>

// Função auxiliar para verificar se é um branch
bool isBranch(QuadOp op) {
    return (op == OP_BLT || op == OP_BLE || op == OP_BGT || 
            op == OP_BGE || op == OP_BEQ || op == OP_BNE);
}

void generateBinary(Quad* quadHead, labelControl* labelHead) {
    FILE* file = fopen("output/binary.txt", "w");
    if (!file) return;

    Quad* current = quadHead;

    int lineNumber = 0;
    while (current != NULL) {
        if (current->op != OP_LABEL) {
            char* opCode = getOpcodeBinary(current->op);
            char *op1 = NULL, *op2 = NULL, *op3 = NULL, *immed = NULL;

            // Formato 1: R R R 2
            if (current->addr1.kind == REGISTER_KIND && 
                current->addr2.kind == REGISTER_KIND && 
                current->addr3.kind == REGISTER_KIND) {
                
                op1 = intToBinaryString(current->addr1.val, 6);
                op2 = intToBinaryString(current->addr2.val, 6);
                op3 = intToBinaryString(current->addr3.val, 6);
                immed = intToBinaryString(0, 8);
            }
            // Formatos 2 e 3 (inclui Branches): R R I14
            else if (current->addr1.kind == REGISTER_KIND && current->addr2.kind == REGISTER_KIND) {
                op1 = intToBinaryString(current->addr1.val, 6);
                op2 = intToBinaryString(current->addr2.val, 6);
                
                int val = 0;
                if (current->addr3.kind == LABEL_KIND) {
                    val = getLineByLabel(labelHead, current->addr3.name);
                    // Regra específica para Branch: (label - (PC + 1))
                    if (isBranch(current->op)) {
                        val -= (lineNumber + 1);
                    }
                } else {
                    val = current->addr3.val;
                }
                immed = intToBinaryString(val, 14);
            }
            // Formatos 4, 5 e 6: R I20
            else if (current->addr1.kind == REGISTER_KIND) {
                op1 = intToBinaryString(current->addr1.val, 6);
                int val = (current->addr2.kind == LABEL_KIND) ? getLineByLabel(labelHead, current->addr2.name) : current->addr2.val;
                immed = intToBinaryString(val, 20);
            }
            // Formato 7: I26
            else {
                int val = (current->addr1.kind == LABEL_KIND) ? getLineByLabel(labelHead, current->addr1.name) : current->addr1.val;
                immed = intToBinaryString(val, 26);
            }

            // Escrita unificada
            fprintf(file, "%s", opCode);
            if (op1)   fprintf(file, "%s", op1);
            if (op2)   fprintf(file, "%s", op2);
            if (op3)   fprintf(file, "%s", op3);
            if (immed) fprintf(file, "%s", immed);
            fprintf(file, "\n");

            // Limpeza segura
            free(op1); free(op2); free(op3); free(immed);

            // Incremente a linha
            lineNumber++;
        }
        current = current->next;
    }
    fclose(file);
}