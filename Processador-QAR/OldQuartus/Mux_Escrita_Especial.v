module Mux_Escrita_Especial(
	input [1:0] SinalMuxEE,		// [00] = RAL, [01] = Push, [10] = Pop
	input [31:0] RAL,
	input [31:0] Push,
	input [31:0] Pop,
	output reg [31:0] SEE
);

// Extende o sinal de

always @(*)
begin
	case(SinalMuxEE)
		2'b00: SEE = RAL;
		2'b01: SEE = Push;
		2'b10: SEE = Pop;
		default: SEE = 32'b0;
	endcase
end

endmodule 