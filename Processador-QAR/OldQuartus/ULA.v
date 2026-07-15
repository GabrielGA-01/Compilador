module ULA (EA, EB, SinalULA, SL, SD);

input wire [31:0] EA;		// Entrada A
input wire [31:0] EB;		// Entrada B
input wire [4:0] SinalULA;	// Código de operação
output reg [31:0] SL;		// Saída padrão
output reg SD;					// Saída para desvio

always @(*)
begin
	SD = 1'b0;
	SL = 32'b0;

	case(SinalULA)
		5'b00000: SL = EA + EB;				// add
		5'b00001: SL = EA - EB;				// sub
		5'b00010: SL = EA & EB;				// and
		5'b00011: SL = EA | EB;				// or
		5'b00100: SL = ~EA;					// not EA
		5'b00101: SL = EA ~^ EB;			// xnor
		5'b00110: SL = EA << EB[4:0];		// sl
		5'b00111: SL = EA >> EB[4:0];		// sr
		5'b01000: SD = (EA == EB);			// beq
		5'b01001: SD = (EA != EB);			// bne
		5'b01010: SD = (EA < EB);			// blt
		5'b01011: SD = (EA > EB);			// bgt
		5'b01100: SL = EA * EB;				// mul
		5'b01101: SL = EA / EB;				// div
		5'b01110: SL = EA;					// Soma zero em EA
		5'b01111: SL = EB;					// Soma zero em EB
		5'b10000: SL = EA + 1;				// Soma um em EA
		5'b10001: SD = (EA <= EB);		// ble
		5'b10010: SD = (EA >= EB);		// bge
		
		default: 
			begin
				SD = 1'b0;
				SL = 32'b0;
			end
			
	endcase
end

endmodule 