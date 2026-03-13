#!/bin/bash

set -e

echo "Iniciando a compilação do compilador..."
echo ""

echo "[1/4] Gerando o parser com Bison..."
bison -d -v -g parser.y

echo "[2/4] Gerando o scanner com Flex..."
flex scanner.l

echo "[3/5] Compilando o código do scanner..."
gcc -c -o lex.yy.o lex.yy.c

echo "[4/5] Compilando o código da AST..."
gcc -c -o ast.o ast.c

echo "[4.5/5] Compilando tabela de símbolos e analisador..."
gcc -c -o symtab.o symtab.c
gcc -c -o analyze.o analyze.c

echo "[4.6/5] Compilando gerador de código intermediário..."
gcc -c -o cgen.o cgen.c

echo "[5/5] Linkando os arquivos para criar o executável 'compilador'..."
gcc -o compilador lex.yy.o parser.tab.c ast.o symtab.o analyze.o cgen.o -lfl

echo "Limpando arquivos temporários..."
rm lex.yy.c lex.yy.o parser.tab.c ast.o symtab.o analyze.o cgen.o

echo ""
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"
