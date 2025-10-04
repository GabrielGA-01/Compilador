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

# 3. Compila o analisador léxico em um arquivo objeto (.o).
echo "[3/4] Compilando o código do scanner..."
gcc -c -o lex.yy.o lex.yy.c

# 4. Linka tudo para criar o executável final.
echo "[4/4] Linkando os arquivos para criar o executável 'compilador'..."
gcc -o compilador lex.yy.o parser.tab.c -lfl

# Limpa os arquivos intermediários que não são mais necessários.
echo "Limpando arquivos temporários..."
rm lex.yy.c lex.yy.o parser.tab.c parser.tab.h

# Mensagem final de sucesso com instruções.
echo "" # Linha em branco para espaçamento.
echo "Sucesso! O executável 'compilador' foi criado."
echo "Para usar, execute por exemplo: ./compilador"