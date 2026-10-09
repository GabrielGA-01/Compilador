# Instale o flex, bison para o compilador (linux)
sudo apt update
sudo apt install flex bison 

# Instale o gdb para debugar
sudo apt install gdb

# Execução
Para executar o compilador, entre na pasta "Compilador":
```bash
cd Compilador
```
Em seguida, rode o script "compilar.sh":
```bash
./compilar.sh gdb --args ./compilador
```
Finalmente, execute o compilador:
```bash
gdb -q --args ./compilador ./testes/sort.txt
```

# Orientações para o futuro

Sobre escrever um código em C-
1 - Não declare variáveis dentro de while ou de if
2 - Tenha certeza que uma função do tipo inteiro irá retornar algum valor
3 - Cuidado ao escrever expressões e sem atribuir elas para alguma variável

Sobre o Quartus:
1 - Registrador R0 será preservado com o valor zero e não pode ser escrito nele (Quartus)
2 - Está sendo utilizado 16 registradores
    -> Se necessário, altere em "Banco_de_Registradores #(.ADDR_WIDTH(4))" em Processador_Completo.v
3 - Memória de instruções com 256 posições
    -> Se necessário, altere ADDR_WIDTH(8) em Memoria_Instrucoes.v
4 - Memória principal com 256 posições
    -> Se necessário, altere ADDR_WIDTH(8) em Memoria_Principal.v

Sobre a forma de onda:
0 - Waveform - Não tem saídas de debug
1 - Waveform - Contém as saídas de debug para o código 2 de AOC
2 - Waveform - Contém as saídas de debug para o GCD
3 - Waveform - Contém as saídas de debug para o SORT

Sobre o uso de registradores pelo compilador:
1 - Registradores usados pelo compilador R1 até R5 para as instruções 
    -> Se necessário, altere NUMBER_OF_REGISTERS(4) em casm.c
2 - Registradores das pilhas gerais e globais atualmente estão como registradores 12 e 13
    -> Se necessário, altere REG_PILHA_GLOBAL(13) e REG_PILHA_GERAL(12) em casm.c
3 - Está sendo considerada uma memória principal de 128 posições
    -> Se necessário, altere MEM_SIZE em casm.c

A partir de agora todas as operações terminam em R ou em I

# Próximos passos
Garantir que em operações de bench sejam usandos apenas registradores - OK
Fazer a atualização de operações aritméricas entre R e I - OK
Fazer o retorno das funções - OK
Fazer a chamada de função e preparação dos parâmetros - OK
Corrigir a lógica do número de parâmetros na pilha (param) para ser por escopo - OK. Solução: adiciona na lista de variáveis de verdade
Fazer a correção no acesso à variáveis globais - OK
Atualizar temporários em registradores - Problema de indentificação de temp na chamada de função - Ok - Resolvido
Fazer verificação de registrador sobrando em func call - OK

Quando atribuir valor as labels, lembre que branch vai para Label + 1

# Sobre SO

Registradores de uso específico
R15 - Pilha
R14 - JAL
R13 - Pilha global
R12 - Pilha geral
R11 - Número do programa
R10 - Posição do programa
R9 - Endereço do PC
R8 - I/O: processo <-> SO
R0 - Zero

Núcleo do SO:

definirProgramaParaExecutar:
- Escreve o número do programa em R11
- Escreve a posição do programa no buffer do banco de registradores
- saveRegs()
- loadRegs() {Não carrega R0, R10 e R11} || Não atualiza R9 de R10=0
- Move o conteúdo de R9 para o buffer do do PC  
- Atualiza o endereço de retorno do Módulo de Interrupção
- Aplica os valores contidos nos buffers do PC e do banco para o PC e R10

Instruções para o compilador:
- (alteração 1.0) definirProgramaParaExecutar([numero])
- (alteração 1.1) definirNumeroDoPrograma([numero]) - Escreve no registrador R11
- (alteração 1.1) definirPosicaoDoPrograma([numero]) - Escreve no buffer do BR
- (alteração 1.1) trocarDeContexto()
- definirQuantum([numero]) - Utiliza um registrador para passar o valor para GI
- saveRegs()
- loadRegs()
- (ES) lerBuffer() - Escreve buffer de ES {LB} em R8 e retorna R8
- (ES) lerBufferLimpa() - {LBL} + retorno R8
- (PR/ES) lerReg([constante]) - Obrigatoriamente é passada uma constante. Não pode ser variável.
- (ES) definirModoES([número]) - Utiliza um registrador para passar o valor para ES
- (Bonus) showLCD([número]) - Envia o registrador R11 junto com um registrador com o número da mensagem

(PR) = Processos
(ME) = Memória
(ES) = Entrada e saída

Instruções para o processador:
- (PR) bufBR (OP, REG) - Salva o valor no buffer do BR
- (PR) bufPC (OP, REG) - Salva o valor no buffer do PC
- (PR) aplBuf (OP, -) - Aplica o buffer do PC e do BR ao mesmo tempo
- (PR) retSO (OP, RD) - Define o endereço de retorno
- (PR) quant (OP, RD)
- (ES) LB (OP, -) - Faz a leitura do buffer de ES para o registrador R8
- (ES) LBL (OP, -) - Faz a leitura do buffer de ES para o registrador R8 e limpa o buffer
- (ES) DFMES (OP, RD) - Define o modo de ES
- disp (OP, RS, RT) - Passa um registrador com o número da mensagem e outro com o número do programa
[Adicionar depois]
- (ES) Novas instruções de IN e OUT que levam ao SO