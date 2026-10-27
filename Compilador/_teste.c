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

void main(){
    // --- Inicialização (sem R0 e R1) ---
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(2), createNumericAddr(10), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(3), createNumericAddr(20), createEmptyAddr());
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(4), createNumericAddr(20), createEmptyAddr());

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
    makeNewQuadTest(OP_MOVI, *createRegisterAddr(7), createNumericAddr(115), createEmptyAddr());
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