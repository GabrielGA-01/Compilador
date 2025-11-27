#!/bin/bash

# Encerra o script imediatamente se qualquer comando falhar.
set -e

echo "Iniciando a compilação do seu compilador..."
echo ""

# Gera o parser C (parser.tab.c) e o header (parser.tab.h).
echo "[1/4] Gerando o parser com Bison..."
bison -d -v -g parser.y

# Gera o analisador léxico C (lex.yy.c).
echo "[2/4] Gerando o scanner com Flex..."
flex scanner.l

# Compila os arquivos C em arquivos objeto (.o).
echo "[3/5] Compilando o código do scanner..."
gcc -c -o lex.yy.o lex.yy.c

# Compila o código da AST.
echo "[4/5] Compilando o código da AST..."
gcc -c -o ast.o ast.c

# Compila a tabela de símbolos e o analisador semântico.
echo "[4.5/5] Compilando tabela de símbolos e analisador..."
gcc -c -o symtab.o symtab.c
gcc -c -o analyze.o analyze.c

# Linka tudo para criar o executável final.
echo "[5/5] Linkando os arquivos para criar o executável 'compilador'..."
gcc -o compilador lex.yy.o parser.tab.c ast.o symtab.o analyze.o -lfl

# Limpa os arquivos intermediários.
echo "Limpando arquivos temporários..."
rm lex.yy.c lex.yy.o parser.tab.c parser.tab.h ast.o symtab.o analyze.o

echo ""
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"
