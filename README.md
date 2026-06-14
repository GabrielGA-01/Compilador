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

Sobre o uso de registradores pelo compilador:
1 - Registrador R0 será preservado com o valor zero e não pode ser escrito nele (Quartus)
2 - Registradores usados pelo compilador R1 até R5 para as instruções 
    -> Se necessário altere NUMBER_OF_REGISTERS(4) em casm.c
3 - Registradores das pilhas gerais e globais atualmente estão como registradores 12 e 13
    -> Se necessário altere REG_PILHA_GLOBAL(13) e REG_PILHA_GERAL(12) em casm.c
4 - Está sendo considerada uma memória de 124 posições
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

