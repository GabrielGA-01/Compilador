#!/bin/bash

set -e

echo "Iniciando a compilação do compilador..."
echo ""

echo "[1/4] Gerando o parser com Bison..."
bison -d -v -g parser.y

echo "[2/4] Gerando o scanner com Flex..."
flex scanner.l

# ... (partes anteriores iguais)

echo "[3/5] Compilando o código do scanner..."
gcc -g -c -o lex.yy.o lex.yy.c

echo "[4/5] Compilando o código da AST..."
gcc -g -c -o ast.o ast.c

echo "[4.5/5] Compilando tabela de símbolos e analisador..."
gcc -g -c -o symtab.o symtab.c
gcc -g -c -o analyze.o analyze.c

echo "[4.6/5] Compilando gerador de código intermediário..."
gcc -g -c -o cintgen.o cintgen.c

echo "[5/5] Linkando os arquivos..."
gcc -g -o compilador lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o -lfl

echo "Limpando arquivos temporários..."
rm lex.yy.c lex.yy.o parser.tab.c ast.o symtab.o analyze.o cintgen.o

echo ""
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"
