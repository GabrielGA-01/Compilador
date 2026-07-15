module Mux_PC(
	input [1:0] SinalMuxPC, // [00] = PC, [01] = J, [10] = Desvio, [11] = JR
	input SD,					// Sinal da ULA
	input [31:0] PC,
	input [31:0] J,
	input [31:0] Desvio,
	input [31:0] JR,
	output reg [31:0] Novo_PC
);

always @(*)
begin
	case(SinalMuxPC)
		2'b00: Novo_PC = PC;
		2'b01: Novo_PC = J;
		2'b10: 
		begin
			if(SD == 1'b1) Novo_PC = Desvio;
			else Novo_PC = PC;
		end
		2'b11: Novo_PC = JR;
	endcase
end

endmodule 