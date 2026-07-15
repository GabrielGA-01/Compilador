// Quartus Prime Verilog Template
// Single Port ROM

module Memoria_Instrucoes
#(parameter DATA_WIDTH=32, parameter ADDR_WIDTH=9, parameter DEPTH=258) // 258 posicoes, 9 bits de endereco
(
	input clock,							// Sinal de clock
	input [(ADDR_WIDTH-1):0] PC,
	output reg [5:0] Opcode,
	output reg [25:0] Operandos
);

	// Declare the ROM variable
	reg [DATA_WIDTH-1:0] rom[DEPTH-1:0];

	// Initialize the ROM with $readmemb.  Put the memory contents
	// in the file single_port_rom_init.txt.  Without this file,
	// this design will not compile.

	// See Verilog LRM 1364-2001 Section 17.2.8 for details on the
	// format of this file, or see the "Using $readmemb and $readmemh"
	// template later in this section.

	initial 
	begin
		$readmemb("single_port_rom_init.txt", rom);
		Opcode = 6'd1;       // Força Opcode inicial como 000001 (nop)
		Operandos = 26'd0;   // Apenas para atribuir um valor
	end


	// Leitura da memória de instrução na borda de subida do clock
	always @ (posedge clock)
	begin
		Opcode <= rom[PC][31:26];
		Operandos <= rom[PC][25:0];
	end

endmodule
