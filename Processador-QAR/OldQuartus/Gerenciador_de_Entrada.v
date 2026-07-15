module Gerenciador_de_Entrada(
input [2:0]SinalGEN,				// Sinal para determinar o formato [1:0] e o registrador especial [3:2]
input [25:0]Operandos,			// Campo com os operandos
output reg [5:0] L1,					// Registrador para leitura 1
output reg [5:0] L2,					// Registrador para leitura 2
output reg [5:0] E1,					// Registrador para escrita
output reg [5:0] E2,					// Registrador para escrita especial
output reg [31:0] Imediato			// Imediato extendido
);

always @(*)
begin
	// Para determinar o formato
	case(SinalGEN[2:0])		
		// Formato 1
		3'b000:
		begin
			E1 <= Operandos[25:20];										// RD
			E2 <= 6'b0;														// Resp - Não usado
			L1 <= Operandos[19:14];										// RS
			L2 <= Operandos[13:8];										// RT
			Imediato <= 32'b0;											// Ime - Não usado
		end
		// Formato 2
		3'b001:
		begin
			E1 <= Operandos[25:20];										// RD
			E2 <= 6'b0;										 				// Resp - Não usado
			L1 <= Operandos[19:14];										// RS
			L2 <= 6'b0;														// RT - Não usado
			Imediato <= {{18{Operandos[13]}}, Operandos[13:0]};// Ime
		end
		// Formatos 3
		3'b010:
		begin
			E1 <= 6'b0;														// RD - não usado
			E2 <= 6'b0;														// Resp - Não usado
			L1 <= Operandos[25:20];										// RS
			L2 <= Operandos[19:14];										// RT
			Imediato <= {{18{Operandos[13]}}, Operandos[13:0]};// Ime
		end
		// Formatos 4
		3'b011:
		begin
			E1 <= Operandos[25:20];										// RD
			E2 <= 6'b111111;												// Resp - Usado somente no pop como sendo RPI
			L1 <= 6'b111111;												// RS - Usado somente no pop como sendo RPI
			L2 <= 6'b0;														// RT
			Imediato <= {{12{Operandos[19]}}, Operandos[19:0]};// Ime
		end
		// Formato 5
		3'b100:
		begin
			E1 <= 6'b0;														// RD - Não usado
			E2 <= 6'b0;														// Resp - Não usado
			L1 <= Operandos[25:20];										// RS
			L2 <= 6'b0;														// RT - Não usado
			Imediato <= 32'b0;											// Ime - Não usado
		end
		// Formato 6
		3'b101:
		begin
			E1 <= 6'b0;														// RD - Não usado
			E2 <= 6'b111111;												// Resp - Usado somente no push como sendo RPI
			L1 <= 6'b111111;												// RS - Usado somente no push como sendo RPI
			L2 <= Operandos[25:20];										// RT
			Imediato <= {{12{Operandos[19]}}, Operandos[19:0]};// Ime
		end
		// Formato 7
		3'b110:
		begin
			E1 <= 6'b0;														// RD - Não usado
			E2 <= 6'b111110;												// Resp - Usado somente no JAL como sendo RAL
			L1 <= 6'b0;														// RS - Não usado
			L2 <= 6'b0;														// RT - Não usado
			Imediato <= {{6{Operandos[25]}}, Operandos[25:0]};	// Ime
		end
		default:
		begin
			E1 <= 6'b0;														// RD
			E2 <= 6'b0;														// Resp
			L1 <= 6'b0;														// RS
			L2 <= 6'b0;														// RT
			Imediato <= 32'b0;											// Ime
		end
	endcase
end

endmodule 