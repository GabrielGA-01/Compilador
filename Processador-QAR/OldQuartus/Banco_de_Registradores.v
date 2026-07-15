module Banco_de_Registradores #(parameter N = 4) (clock, reg_leitura1, reg_leitura2, reg_escrita, reg_especial, dado_escrita, dado_especial, SinalBanco, saida1, saida2);

input clock;								// Clock do processador
input [5:0] reg_leitura1;				// Endereço do registrador 1
input [5:0] reg_leitura2;				// Endereço do registrador 2
input [5:0] reg_escrita;				// Endereço de escrita
input [5:0] reg_especial;				// Endereço de escrita especial
input [31:0] dado_escrita;				// Dado para escrever
input [31:0] dado_especial;			// Dado para escrever especial

input [1:0] SinalBanco;					// [00] = sem escrita, [01] = escrita padrao, [10] = escrita especial, [11] = ambos

output [31:0] saida1;					// Dado de saída 1
output [31:0] saida2;					// Dado de saída 2

reg [31:0] banco [2**N-1:0];				// Criação do banco de registradores

reg aux = 1'b1;

always @(posedge clock)
	begin
		if(aux == 1'b0)
		begin
			banco[0] = 32'b0;
			aux = 1'b1;
		end
		
		if(SinalBanco[0] == 1'b1) banco[reg_escrita] <= dado_escrita;
		if(SinalBanco[1] == 1'b1) banco[reg_especial] <= dado_especial;
	end

assign saida1 = banco[reg_leitura1];
assign saida2 = banco[reg_leitura2];
	
endmodule 