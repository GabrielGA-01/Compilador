module Mux2 (E1, E2, S, Sinal);
input [31:0] E1;
input [31:0] E2;
input Sinal;
output reg [31:0] S;

always @(*)
	begin
		if(Sinal == 1'b1) S = E1;
		else S = E2;
	end

endmodule 