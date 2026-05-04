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
