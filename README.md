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
A partir de agora todas as operações terminam em R ou em I

# Próximos passos
Garantir que em operações de bench sejam usandos apenas registradores - OK
Fazer a atualização de operações aritméricas entre R e I - OK
Fazer o retorno das funções - OK
Fazer a chamada de função e preparação dos parâmetros - OK
Atualizar temporários em registradores