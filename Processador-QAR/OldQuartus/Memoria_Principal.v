// Quartus Prime Verilog Template

module Memoria_Principal
#(parameter DATA_WIDTH=32, parameter ADDR_WIDTH=9, parameter DEPTH=258) // 32 bits e 258 posicoes
(
	input clock,										// Sinal de clock
	input [(ADDR_WIDTH-1):0] END,	  				// Endereço de acesso
	input [(DATA_WIDTH-1):0] dado_escrita,		// Dado para escrever
	input SinalMemoria, 								// Sinal de escrita
	output reg [(DATA_WIDTH-1):0] saida			// Dado de saída				
);
	
	// Criação da memória
	reg [DATA_WIDTH-1:0] ram[DEPTH-1:0];
	
	// Leitura na borda de descida do clock
	always @ (negedge clock)
	begin
		saida <= ram[END];
		if (SinalMemoria) ram[END] <= dado_escrita;
	end
	
endmodule
