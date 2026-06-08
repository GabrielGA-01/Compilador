#!/bin/bash

set -e

echo "Iniciando a compilação do compilador..."
echo ""

echo "[1/9] Gerando o parser com Bison..."
bison -d -v -g parser.y

echo "[2/9] Gerando o scanner com Flex..."
flex scanner.l

# ... (partes anteriores iguais)

echo "[3/9] Compilando o código do scanner..."
gcc -g -c -o lex.yy.o lex.yy.c

echo "[4/9] Compilando o código da AST..."
gcc -g -c -o ast.o ast.c

echo "[5/9] Compilando tabela de símbolos e analisador..."
gcc -g -c -o symtab.o symtab.c
gcc -g -c -o analyze.o analyze.c

echo "[6/9] Compilando gerador de código intermediário..."
gcc -g -c -o cintgen.o cintgen.c

echo "[7/9] Compilando gerador de assembly..."
gcc -g -c -o casm.o casm.c

echo "[8/9] Compilando gerador de binário..."
gcc -g -c -o binary.o binary.c

echo "[9/9] Linkando os arquivos..."
gcc -g -o compilador lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o casm.o binary.o -lfl

echo "Limpando arquivos temporários..."
rm lex.yy.c lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o casm.o binary.o

echo ""
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"
