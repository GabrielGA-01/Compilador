#include <stdlib.h>
#include "cintgen.h"
#include "casm.h"
#include "binary.h"

labelControl* labelheadTest = NULL;
Quad* headTest = NULL;
Quad* tailTest = NULL;

// Forma novas quádruplas e salva usando os ponteiros headTest e tailTest
Quad* makeNewQuadTest(QuadOp op, Address a1, Address a2, Address a3) {
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

    if (headTest == NULL) {
        headTest = tailTest = q;
    } else {
        tailTest->next = q;
        tailTest = q;
    }

    return q;
}

void main(){
    // --- Inicialização ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(0), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(20), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(20), createEmptyAddr());

    // --- Aritmética ---
    makeNewQuadTest(OP_ADDR, *createRegisterAddr(3), *createRegisterAddr(1), *createRegisterAddr(0));
    makeNewQuadTest(OP_ADDI, *createRegisterAddr(4), *createRegisterAddr(1), createNumericAddr(10));
    makeNewQuadTest(OP_SUBR, *createRegisterAddr(5), *createRegisterAddr(1), *createRegisterAddr(0));
    makeNewQuadTest(OP_SUBI, *createRegisterAddr(6), *createRegisterAddr(1), createNumericAddr(10));
    makeNewQuadTest(OP_MULR, *createRegisterAddr(7), *createRegisterAddr(1), *createRegisterAddr(0));
    makeNewQuadTest(OP_MULI, *createRegisterAddr(8), *createRegisterAddr(1), createNumericAddr(10));
    makeNewQuadTest(OP_DIVR, *createRegisterAddr(9), *createRegisterAddr(1), *createRegisterAddr(0));
    makeNewQuadTest(OP_DIVI, *createRegisterAddr(10), *createRegisterAddr(1), createNumericAddr(10));

        // --- BEQ ---
    Address l0 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(1), *createRegisterAddr(2), l0);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l0, createEmptyAddr(), createEmptyAddr());

    Address l1 = *createLabelAddr();
    Address l2 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(0), *createRegisterAddr(1), l1);
    makeNewQuadTest(OP_JUMP, l2, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l1, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l2, createEmptyAddr(), createEmptyAddr());

    // --- BNE ---
    Address l3 = *createLabelAddr();
    makeNewQuadTest(OP_BNE, *createRegisterAddr(0), *createRegisterAddr(1), l3);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l3, createEmptyAddr(), createEmptyAddr());

    Address l4 = *createLabelAddr();
    Address l5 = *createLabelAddr();
    makeNewQuadTest(OP_BNE, *createRegisterAddr(1), *createRegisterAddr(2), l4);
    makeNewQuadTest(OP_JUMP, l5, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l4, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l5, createEmptyAddr(), createEmptyAddr());

    // --- BLT ---
    Address l6 = *createLabelAddr();
    makeNewQuadTest(OP_BLT, *createRegisterAddr(0), *createRegisterAddr(1), l6);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l6, createEmptyAddr(), createEmptyAddr());

    Address l7 = *createLabelAddr();
    Address l8 = *createLabelAddr();
    makeNewQuadTest(OP_BLT, *createRegisterAddr(1), *createRegisterAddr(0), l7);
    makeNewQuadTest(OP_JUMP, l8, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l7, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l8, createEmptyAddr(), createEmptyAddr());

    // --- BGT ---
    Address l9 = *createLabelAddr();
    makeNewQuadTest(OP_BGT, *createRegisterAddr(1), *createRegisterAddr(0), l9);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l9, createEmptyAddr(), createEmptyAddr());

    Address l10 = *createLabelAddr();
    Address l11 = *createLabelAddr();
    makeNewQuadTest(OP_BGT, *createRegisterAddr(0), *createRegisterAddr(1), l10);
    makeNewQuadTest(OP_JUMP, l11, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l10, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l11, createEmptyAddr(), createEmptyAddr());

    // --- BLE ---
    Address l12 = *createLabelAddr();
    makeNewQuadTest(OP_BLE, *createRegisterAddr(0), *createRegisterAddr(1), l12);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l12, createEmptyAddr(), createEmptyAddr());

    Address l13 = *createLabelAddr();
    Address l14 = *createLabelAddr();
    makeNewQuadTest(OP_BLE, *createRegisterAddr(1), *createRegisterAddr(0), l13);
    makeNewQuadTest(OP_JUMP, l14, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l13, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l14, createEmptyAddr(), createEmptyAddr());

    // --- BGE ---
    Address l15 = *createLabelAddr();
    makeNewQuadTest(OP_BGE, *createRegisterAddr(1), *createRegisterAddr(0), l15);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l15, createEmptyAddr(), createEmptyAddr());

    Address l16 = *createLabelAddr();
    Address l17 = *createLabelAddr();
    makeNewQuadTest(OP_BGE, *createRegisterAddr(0), *createRegisterAddr(1), l16);
    makeNewQuadTest(OP_JUMP, l17, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l16, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l17, createEmptyAddr(), createEmptyAddr());

    // --- Salto via Registrador ---
    Address l18 = *createLabelAddr();
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(3), l18, createEmptyAddr());
    makeNewQuadTest(OP_JR, *createRegisterAddr(3), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l18, createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVR, *createRegisterAddr(4), *createRegisterAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(4), *createRegisterAddr(1), createNumericAddr(5));
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(5), *createRegisterAddr(4), createNumericAddr(5));
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());

    Quad* before = NULL;
    Quad* current = headTest;

    int lineNumber = 0;
    while(current != NULL){
        switch (current->op){    
        case OP_LABEL:
            insertLabel(&labelheadTest, current->addr1.name, lineNumber);
            current->addr1.val = lineNumber;
            break;
        default:
            break;
        }

        if(current != NULL && current != before && current->op != OP_LABEL) lineNumber++;

        // Lógica de percorrimento
        if(current == NULL && before != NULL) current = before->next;
        else{
            before = current;
            current = current->next;
        }
    }

    // Quadruplas
    FILE* quadruplesFile = fopen("output/testes/quadruples_assembly_code.txt", "w");
    fprintCode(quadruplesFile, headTest);
    fclose(quadruplesFile);
    
    // Assembly
    FILE* assemblyFile = fopen("output/testes/assembly_code.txt", "w");
    fprintRealAssemblyCode(assemblyFile, headTest);
    fclose(assemblyFile);

    generateBinary(headTest, labelheadTest);
}