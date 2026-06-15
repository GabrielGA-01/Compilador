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

