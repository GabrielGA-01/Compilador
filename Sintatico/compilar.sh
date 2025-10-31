#!/bin/bash

# Encerra o script imediatamente se qualquer comando falhar.
set -e

echo "Iniciando a compilação do seu compilador..."
echo "" # Linha em branco para espaçamento.

# 1. Gera o parser C (parser.tab.c) e o header (parser.tab.h).
echo "[1/4] Gerando o parser com Bison..."
bison -d -v -g parser.y

# 2. Gera o analisador léxico C (lex.yy.c).
echo "[2/4] Gerando o scanner com Flex..."
flex scanner.l

# 3. Compila os arquivos C em arquivos objeto (.o).
echo "[3/5] Compilando o código do scanner..."
gcc -c -o lex.yy.o lex.yy.c
echo "[4/5] Compilando o código da AST..."
gcc -c -o ast.o ast.c

# 5. Linka tudo para criar o executável final.
echo "[5/5] Linkando os arquivos para criar o executável 'compilador'..."
gcc -o compilador lex.yy.o parser.tab.c ast.o -lfl

# Limpa os arquivos intermediários que não são mais necessários.
echo "Limpando arquivos temporários..."
# rm lex.yy.c lex.yy.o parser.tab.c parser.tab.h ast.o

# Mensagem final de sucesso com instruções.
echo "" # Linha em branco para espaçamento.
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"
