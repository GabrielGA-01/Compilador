module Mux_Escrita (
input [1:0] SinalMuxE,		// Sinal de controle [00] = Switches, [01] = ULA, [10] = Memoria
input [31:0] Switches,		
input [31:0] ULA,
input [31:0] Memoria,
output reg [31:0] SE				// Dado para escrita
);

always @(*)
begin
	case(SinalMuxE)
		2'b00: SE = Switches;
		2'b01: SE = ULA;
		2'b10: SE = Memoria;
		default: SE = 32'b0;
	endcase
end

endmodule 
