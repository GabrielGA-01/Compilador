module Unidade_de_Controle(
input [5:0] Opcode,
output reg HLT,
output reg RegULA,
output reg SinalMemoria,
output reg [1:0] SinalMES,
output reg [1:0] SinalBanco,
output reg [1:0] SinalMuxE,
output reg [1:0] SinalMuxEE,
output reg [1:0] SinalMuxPC,
output reg [2:0] SinalGEN,
output reg [4:0] SinalULA
);

always @(*)
begin
	SinalMES = 2'b00;
			
	case(Opcode)
		// Formato 7 - hlt
		6'b000000:
		begin
			HLT = 1'b1;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b110; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 7 - nop
		6'b000001:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b110; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 7 - jmp
		6'b000010:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b01; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b110; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 7 - jal
		6'b000011:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b10; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b01; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b110; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 1 - add
		6'b000100:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b00000; 
		end
		// Formato 1 - sub
		6'b000101:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b00001; 
		end
		// Formato 1 - mul
		6'b000110:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b01100; 
		end
		// Formato 1 - div
		6'b000111:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b01101; 
		end
		// Formato 1 - and
		6'b001000:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b00010; 
		end
		// Formato 1 - or
		6'b001001:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b00011; 
		end
		// Formato 1 - xnor
		6'b001010:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b000; 		// Num Formato - 1
			SinalULA = 5'b00101; 
		end
		// Formato 2 - addi
		6'b001011:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00000; 
		end
		// Formato 2 - subi
		6'b001100:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00001; 
		end
		// Formato 2 - muli
		6'b001101:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b01100; 
		end
		// Formato 2 - divi
		6'b001110:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b01101; 
		end
		// Formato 2 - andi
		6'b001111:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00010; 
		end
		// Formato 2 - ori
		6'b010000:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00011; 
		end
		// Formato 2 - not
		6'b010001:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00100; 
		end
		// Formato 2 - sl
		6'b010010:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00110; 
		end
		// Formato 2 - sr
		6'b010011:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00111; 
		end
		// Formato 2 - mov
		6'b010100:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b01110; 
		end
		// Formato 2 - loadd
		6'b010101:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b10; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b00000; 
		end
		// Formato 2 - loadr
		6'b010110:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b10; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b001; 		// Num Formato - 1
			SinalULA = 5'b01110; 
		end
		// Formato 3 - beq
		6'b010111:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b01000; 
		end
		// Formato 3 - bnq
		6'b011000:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b01001; 
		end
		// Formato 3 - blt
		6'b011001:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b01010; 
		end
		// Formato 3 - bgt
		6'b011010:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b01011; 
		end
		// Formato 3 - stored
		6'b011011:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b1; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b00000; 
		end
		// Formato 3 - storer
		6'b011100:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b1; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b01110; 
		end
		// Formato 4 - movi
		6'b011101:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b01; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b011; 		// Num Formato - 1
			SinalULA = 5'b01111; 
		end
		// Formato 4 - loadi
		6'b011110:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b10; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b011; 		// Num Formato - 1
			SinalULA = 5'b01111; 
		end
		// Formato 4 - in
		6'b011111:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b01;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b01; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b011; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 4 - pop
		6'b100000:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b11; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b10; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b10;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b011; 		// Num Formato - 1
			SinalULA = 5'b01110; 
		end
		// Formato 5 - out
		6'b100001:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b10; 		// [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b100; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 5 - jr
		6'b100010:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b11; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b100; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
		// Formato 6 - storei
		6'b100011:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b1; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b101; 		// Num Formato - 1
			SinalULA = 5'b01111; 
		end
		// Formato 6 - push
		6'b100100:
		begin
			HLT = 1'b0;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b1; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b10; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b01;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b101; 		// Num Formato - 1
			SinalULA = 5'b01110; 
		end
		// Formato 3 - ble
		6'b100101:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b10001; 
		end
		// Formato 3 - bge
		6'b100110:
		begin
			HLT = 1'b0;
			RegULA = 1'b1; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b10; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b010; 		// Num Formato - 1
			SinalULA = 5'b10010; 
		end
		// Default - HLT
		default:
		begin
			HLT = 1'b1;
			RegULA = 1'b0; 			// Registrador para ULA
			SinalMemoria = 1'b0; 	// Controla escrita
			SinalMES = 2'b00;       // [10] = OUT, [01] = IN, [00] = default
			SinalBanco = 2'b00; 		// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos
			SinalMuxE = 2'b00; 		// [00] = Switches, [01] = ULA, [10] = Memoria
			SinalMuxEE = 2'b00;  	// [00] = RAL, [01] = Push, [10] = Pop
			SinalMuxPC = 2'b00; 		// [00] = PC, [01] = J, [10] = Desvio, [11] = JR
			SinalGEN = 3'b110; 		// Num Formato - 1
			SinalULA = 5'b11111; 
		end
	endcase
	
end

endmodule 