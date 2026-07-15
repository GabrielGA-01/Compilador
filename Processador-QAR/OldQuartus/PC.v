module PC(clock, Novo_Endereco, HLT, TravaPC, PC);

input clock;						// Sinal de clock
input [8:0]Novo_Endereco;		// Novo valor do PC
input HLT;							// Sinal de encerrar
input TravaPC;						// Sinal para segurar para ler switches
output reg [8:0]PC;				// Saída do PC

reg aux = 1'b0;

always @(negedge clock)
begin
	if(aux == 1'b0)
	begin
		PC = 9'd0;
		aux = 1'b1;
	end

	if(!HLT & !TravaPC)
	begin
		PC = Novo_Endereco;
	end
end

endmodule 
