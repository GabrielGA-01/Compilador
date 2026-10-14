#!/bin/bash

set -e

echo "Compilando todo o projeto..."

# 1. Garante que a pasta de saída correta existe
mkdir -p testes

# 2. Gera Parser e Scanner
bison -d -v -g parser.y
flex scanner.l

# 3. Compila todos os objetos do projeto (.o)
gcc -g -c -o lex.yy.o lex.yy.c
gcc -g -c -o ast.o ast.c
gcc -g -c -o symtab.o symtab.c
gcc -g -c -o analyze.o analyze.c
gcc -g -c -o cintgen.o cintgen.c
gcc -g -c -o casm.o casm.c
gcc -g -c -o binary.o binary.c
gcc -g -c -o _teste.o _teste.c

# 4. Linka o executável de teste
gcc -g -o teste_exec lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o casm.o binary.o _teste.o -lfl

# 5. Limpeza (opcional)
rm lex.yy.c lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o casm.o binary.o _teste.o

echo "----------------------------------------------------"
echo "Sucesso! Executando _teste..."
echo "----------------------------------------------------"
./teste_exec