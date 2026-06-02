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
Não declare variáveis dentro de while ou de if
Tenha certeza que uma função do tipo inteiro irá retornar algum valor
Cuidado ao escrever expressões e sem atribuir elas para alguma variável

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

