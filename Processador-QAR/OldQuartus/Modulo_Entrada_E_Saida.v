module Modulo_Entrada_E_Saida(
input clock,

input [1:0] SinalMES,				// [10] = OUT, [01] = IN, [00] = default

input [31:0] Display_IN,

input [17:0] Switches_IN,

output [31:0] Switches_OUT,

input Destrava,
output reg TravaPC,

output [6:0] Display_0,
output [6:0] Display_1,
output [6:0] Display_2,
output [6:0] Display_3,
output [6:0] Display_4,
output [6:0] Display_5,
output [6:0] Display_6,
output [6:0] Display_7
);

// Módulo de entrada
reg Controle;
reg [21:0] Contador;

// Código debounce de pulso único
always @(posedge clock)
	begin
		if(Destrava == 1'b1) Contador <= 22'd0;							// Botão solto = 1
		else if(Contador < 22'd60000) Contador <= Contador + 1;
		
		if(Contador == 22'd50000) Controle <= 1'b1;
		else Controle <= 1'b0;
	end

// Controle do TravaPC
always @(SinalMES[0] or Controle) 
	begin
		if(~SinalMES[0] || Controle) TravaPC <= 1'b0;
		else TravaPC <= 1'b1;
	end

// Leitura e extensão dos Switches
assign Switches_OUT = {{14{Switches_IN[17]}}, Switches_IN[17:0]};


// Módulo de saída

reg [31:0] reg_out;
reg aux = 1'b1;

always @(negedge clock)
begin
	if(aux == 1'b0) 
		begin
			reg_out = 32'b0;
			aux = 1'b1;
		end
	else
		begin
			if(SinalMES[1]) reg_out = Display_IN;
			else reg_out = reg_out;
		end
end



// Variáveis auxiliares para ficarem com os caracteres
reg [3:0] d0;
reg [3:0] d1;
reg [3:0] d2;
reg [3:0] d3;
reg [3:0] d4;
reg [3:0] d5;
reg [3:0] d6;
reg [3:0] d7;

// Separa o valor de entrada em caracteres
always @(*) 
	begin
		d0 = reg_out % 10;
		d1 = (reg_out / 10) % 10;
		d2 = (reg_out / 100) % 10;
		d3 = (reg_out / 1000) % 10;
		d4 = (reg_out / 10000) % 10;
		d5 = (reg_out / 100000) % 10;
		d6 = (reg_out / 1000000) % 10;
		d7 = (reg_out / 10000000) % 10;
	end


	
// Usa o decodificador para escrever os caracteres no display7
Decodificador_BCD(.bcd(d0), .seg(Display_0));
Decodificador_BCD(.bcd(d1), .seg(Display_1));
Decodificador_BCD(.bcd(d2), .seg(Display_2));
Decodificador_BCD(.bcd(d3), .seg(Display_3));
Decodificador_BCD(.bcd(d4), .seg(Display_4));
Decodificador_BCD(.bcd(d5), .seg(Display_5));
Decodificador_BCD(.bcd(d6), .seg(Display_6));
Decodificador_BCD(.bcd(d7), .seg(Display_7));

endmodule



module Decodificador_BCD(
  input      [3:0] bcd,
  output reg [6:0] seg
);

always @(*) 
begin
	case(bcd)
		4'b0000   : seg = 7'b1000000;
		4'b0001   : seg = 7'b1111001;
		4'b0010   : seg = 7'b0100100;
		4'b0011   : seg = 7'b0110000;
		4'b0100   : seg = 7'b0011001;
		4'b0101   : seg = 7'b0010010;
		4'b0110   : seg = 7'b0000010;
		4'b0111   : seg = 7'b1111000;
		4'b1000   : seg = 7'b0000000;
		4'b1001   : seg = 7'b0011000;
		default   : seg = 7'bXXXXXXX;
	endcase
end
endmodule 