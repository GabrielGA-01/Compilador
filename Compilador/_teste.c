#include <stdlib.h>
#include "cintgen.h"
#include "casm.h"
#include "binary.h"

labelControl* labelheadTest = NULL;
Quad* headTest = NULL;
Quad* tailTest = NULL;

Quad* makeNewQuadTest(QuadOp op, Address a1, Address a2, Address a3) {
    Quad *q = (Quad *)malloc(sizeof(Quad));
    q->op = op;
    q->addr1 = a1;
    q->addr2 = a2;
    q->addr3 = a3;

    if(a1.kind == TEMP_VAR) update_or_insert(a1);
    if(a2.kind == TEMP_VAR) update_or_insert(a2);
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

// Preenche com instruções HALT até atingir exatamente o número de linha desejado
void padWithHaltsToLine(int targetLine) {
    // Conta quantas instruções válidas (não-label) já existem na lista
    int currentLine = 0;
    Quad* current = headTest;
    while (current != NULL) {
        if (current->op != OP_LABEL) {
            currentLine++;
        }
        current = current->next;
    }

    // Adiciona HALTs enquanto a linha atual for menor que o alvo
    while (currentLine < targetLine) {
        makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
        currentLine++;
    }
}

void main(){
    // --- Inicialização (sem R0 e R1) ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(3), createNumericAddr(20), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(4), createNumericAddr(20), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(999), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());


    // --- Testes com Imediatos (Addi, Subi, Muli, Divi) com verificação via Beq ---
    // 1. Addi
    makeNewQuadTest(OP_ADDI, *createRegisterAddr(5), *createRegisterAddr(2), createNumericAddr(5));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(15), createEmptyAddr());
    Address l_addi = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_addi);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_addi, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 2. Subi
    makeNewQuadTest(OP_SUBI, *createRegisterAddr(5), *createRegisterAddr(3), createNumericAddr(5));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(15), createEmptyAddr());
    Address l_subi = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_subi);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_subi, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(2), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 3. Muli
    makeNewQuadTest(OP_MULI, *createRegisterAddr(5), *createRegisterAddr(2), createNumericAddr(3));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(30), createEmptyAddr());
    Address l_muli = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_muli);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_muli, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(3), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 4. Divi
    makeNewQuadTest(OP_DIVI, *createRegisterAddr(5), *createRegisterAddr(3), createNumericAddr(2));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(10), createEmptyAddr());
    Address l_divi = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_divi);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_divi, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());


    // --- Testes Aritméticos Registrador a Registrador (Addr, Subr, Mulr, Divr) ---
    // 5. Addr
    makeNewQuadTest(OP_ADDR, *createRegisterAddr(5), *createRegisterAddr(3), *createRegisterAddr(2));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(30), createEmptyAddr());
    Address l_addr = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_addr);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_addr, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 6. Subr
    makeNewQuadTest(OP_SUBR, *createRegisterAddr(5), *createRegisterAddr(3), *createRegisterAddr(2));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(10), createEmptyAddr());
    Address l_subr = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_subr);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_subr, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 7. Mulr
    makeNewQuadTest(OP_MULR, *createRegisterAddr(5), *createRegisterAddr(3), *createRegisterAddr(2));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(200), createEmptyAddr());
    Address l_mulr = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_mulr);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_mulr, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(7), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 8. Divr
    makeNewQuadTest(OP_DIVR, *createRegisterAddr(5), *createRegisterAddr(3), *createRegisterAddr(2));
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(6), createNumericAddr(2), createEmptyAddr());
    Address l_divr = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(5), *createRegisterAddr(6), l_divr);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l_divr, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());


    // --- Testes Lógicos e de Desvio (Beq) ---
    Address l0 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(3), *createRegisterAddr(4), l0);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l0, createEmptyAddr(), createEmptyAddr());

    Address l1 = *createLabelAddr();
    Address l2 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(2), *createRegisterAddr(3), l1);
    makeNewQuadTest(OP_JUMP, l2, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l1, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l2, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(9), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- BNE ---
    Address l3 = *createLabelAddr();
    makeNewQuadTest(OP_BNE, *createRegisterAddr(2), *createRegisterAddr(3), l3);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l3, createEmptyAddr(), createEmptyAddr());

    Address l4 = *createLabelAddr();
    Address l5 = *createLabelAddr();
    makeNewQuadTest(OP_BNE, *createRegisterAddr(3), *createRegisterAddr(4), l4);
    makeNewQuadTest(OP_JUMP, l5, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l4, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l5, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- BLT ---
    Address l6 = *createLabelAddr();
    makeNewQuadTest(OP_BLT, *createRegisterAddr(2), *createRegisterAddr(3), l6);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l6, createEmptyAddr(), createEmptyAddr());

    Address l7 = *createLabelAddr();
    Address l8 = *createLabelAddr();
    makeNewQuadTest(OP_BLT, *createRegisterAddr(3), *createRegisterAddr(2), l7);
    makeNewQuadTest(OP_JUMP, l8, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l7, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l8, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(11), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- BGT ---
    Address l9 = *createLabelAddr();
    makeNewQuadTest(OP_BGT, *createRegisterAddr(3), *createRegisterAddr(2), l9);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l9, createEmptyAddr(), createEmptyAddr());

    Address l10 = *createLabelAddr();
    Address l11 = *createLabelAddr();
    makeNewQuadTest(OP_BGT, *createRegisterAddr(2), *createRegisterAddr(3), l10);
    makeNewQuadTest(OP_JUMP, l11, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l10, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l11, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(12), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- BLE ---
    Address l12 = *createLabelAddr();
    makeNewQuadTest(OP_BLE, *createRegisterAddr(2), *createRegisterAddr(3), l12);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l12, createEmptyAddr(), createEmptyAddr());

    Address l13 = *createLabelAddr();
    Address l14 = *createLabelAddr();
    makeNewQuadTest(OP_BLE, *createRegisterAddr(3), *createRegisterAddr(2), l13);
    makeNewQuadTest(OP_JUMP, l14, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l13, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l14, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(13), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- BGE ---
    Address l15 = *createLabelAddr();
    makeNewQuadTest(OP_BGE, *createRegisterAddr(3), *createRegisterAddr(2), l15);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l15, createEmptyAddr(), createEmptyAddr());

    Address l16 = *createLabelAddr();
    Address l17 = *createLabelAddr();
    makeNewQuadTest(OP_BGE, *createRegisterAddr(2), *createRegisterAddr(3), l16);
    makeNewQuadTest(OP_JUMP, l17, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l16, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l17, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(14), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Salto via Registrador (JR) ---
    Address l18 = *createLabelAddr();
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(5), l18, createEmptyAddr());
    makeNewQuadTest(OP_JR, *createRegisterAddr(5), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l18, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(15), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Memória (Store/Load) ---
    Address l19 = *createLabelAddr();
    makeNewQuadTest(OP_MOVR, *createRegisterAddr(6), *createRegisterAddr(2), createEmptyAddr());
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(6), *createRegisterAddr(3), createNumericAddr(5));
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(7), *createRegisterAddr(6), createNumericAddr(5));
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(3), *createRegisterAddr(7), l19);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l19, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(16), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- E/S (In/Out) ---
    makeNewQuadTest(OP_IN, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVR, *createRegisterAddr(1), *createRegisterAddr(7), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(17), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Teste de Buffers (PC e Banco de Registradores) ---
    Address l20 = *createLabelAddr();
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(118), createEmptyAddr());
    makeNewQuadTest(OP_BUFBR, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BUFPC, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_APLBUF, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(10), *createRegisterAddr(2), l20);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l20, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(18), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Teste buffer ES ---
    Address L29 = *createLabelAddr();
    makeNewQuadTest(OP_IN, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LB, createEmptyAddr(), createEmptyAddr(), createEmptyAddr()); // Comando (LB, -, -, -)
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(8), *createRegisterAddr(1), L29);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, L29, createEmptyAddr(), createEmptyAddr());          // Rótulo (lab, L29, -, -)
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(19), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Teste completo: LBL e LB verificando o buffer resetado para -1 ---
    Address L30 = *createLabelAddr();
    Address L31 = *createLabelAddr();

    // 1. Bloco LBL
    makeNewQuadTest(OP_MOVR, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr()); // ou o equivalente à sua instrução movi R7, R8
    makeNewQuadTest(OP_MOVR, *createRegisterAddr(8), *createRegisterAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_LBL, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(8), *createRegisterAddr(7), L30);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_LABEL, L30, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(20), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 2. Bloco LB com addi para testar o -1 -> 0
    makeNewQuadTest(OP_LB, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(-1), createEmptyAddr());
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(8), *createRegisterAddr(7), L31);              
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_LABEL, L31, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(21), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // Teste das mensagens LCD 
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(10), createEmptyAddr());
    
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(2), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(3), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());
    
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(7), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(9), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(11), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(8), createEmptyAddr(), createEmptyAddr());

    // Teste dos números dos programas
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(2), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(3), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(7), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(9), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(4), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(11), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(5), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(12), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(8), createNumericAddr(6), createEmptyAddr());
    makeNewQuadTest(OP_DISP, *createRegisterAddr(7), *createRegisterAddr(8), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());

    // Teste de salto para endereço na troca de contexo - Pula para posição1 + 10 = 522
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_BUFBR, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BUFPC, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_APLBUF, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());

    // Linha 235
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(22), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Teste do uso do MADMP ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(18), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(3), createNumericAddr(128), createEmptyAddr());
    makeNewQuadTest(OP_BUFBR, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(2), createNumericAddr(2));
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(7), createNumericAddr(120));
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(3), createNumericAddr(203));
    makeNewQuadTest(OP_UPDMADMP, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(3), createNumericAddr(2));
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(2), createNumericAddr(120));
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(7), createNumericAddr(207));
    makeNewQuadTest(OP_BUFPC, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_APLBUF, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    
    //Linha 251
    // --- Configuração inicial dos registradores ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(1), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(18), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(3), createNumericAddr(128), createEmptyAddr());

    // --- Primeira verificação ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(26));
    Address l32 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(1), l32);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l32, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(27), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Segunda verificação ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(120));
    Address l33 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(2), l33);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l33, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(28), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Volta o MADMP para a posição 0 ---
    makeNewQuadTest(OP_UPDMADMP, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());

    // --- Terceira verificação (Endereço 2) ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(2));
    Address l34 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(2), l34);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l34, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(29), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Quarta verificação (Endereço 120) ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(120));
    Address l35 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(7), l35);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l35, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(30), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Quinta verificação (Endereço 203) ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(203));
    Address l36 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(3), l36);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l36, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(31), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Sexta verificação (Endereço 207) ---
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(207));
    Address l37 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(0), l37);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l37, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(32), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Fim do teste ---
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());

    // Completando halts até a linha 522
    padWithHaltsToLine(522);

    makeNewQuadTest(OP_OUT, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(235), createEmptyAddr());
    makeNewQuadTest(OP_BUFBR, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BUFPC, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_APLBUF, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());

    // 1. Verifica se o primeiro bloco foi gravado no endereço correto (buffer antigo)
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(2));
    Address l41 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(3), l41);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l41, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(23), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(120));
    Address l42 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(2), l42);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l42, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(24), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(203));
    Address l44 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(0), l44);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l44, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(25), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // 2. Verifica se o segundo bloco foi gravado no novo endereço traduzido pós-updMADMP
    makeNewQuadTest(OP_LOADDI, *createRegisterAddr(4), *createRegisterAddr(0), createNumericAddr(207));
    Address l43 = *createLabelAddr();
    makeNewQuadTest(OP_BEQ, *createRegisterAddr(4), *createRegisterAddr(7), l43);
    makeNewQuadTest(OP_HALT, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_LABEL, l43, createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(1), createNumericAddr(26), createEmptyAddr());
    makeNewQuadTest(OP_OUT, *createRegisterAddr(1), createEmptyAddr(), createEmptyAddr());

    // --- Escreve na Memória ---
    makeNewQuadTest(OP_STOREDI, *createRegisterAddr(0), *createRegisterAddr(1), createNumericAddr(26));

    // --- Retorno ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(0), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(251), createEmptyAddr());
    makeNewQuadTest(OP_BUFBR, *createRegisterAddr(2), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_BUFPC, *createRegisterAddr(7), createEmptyAddr(), createEmptyAddr());
    makeNewQuadTest(OP_APLBUF, createEmptyAddr(), createEmptyAddr(), createEmptyAddr());
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