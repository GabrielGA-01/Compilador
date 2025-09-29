#!/bin/bash

# Este script compila um arquivo de scanner léxico (.l) usando flex e gcc.

# 1. Executa o flex no arquivo AL.l
#    Isso gera um arquivo em C chamado lex.yy.c, que contém o código do analisador léxico.
echo "Executando o flex em AL.l..."
flex AL.l

# Verifica se o flex foi executado com sucesso
if [ $? -ne 0 ]; then
    echo "Erro ao executar o flex. Verifique se o flex está instalado e se o arquivo AL.l está correto."
    exit 1
fi

# 2. Compila o arquivo C gerado (lex.yy.c) usando o gcc
#    -o exec: Define o nome do arquivo executável de saída como "exec".
#    -lfl:    Faz o link com a biblioteca do flex (libfl), que é necessária para o funcionamento do analisador.
echo "Compilando o arquivo lex.yy.c com o gcc..."
gcc -o exec lex.yy.c -lfl

# Verifica se a compilação foi bem-sucedida
if [ $? -eq 0 ]; then
    echo "Compilação concluída com sucesso! Executável 'exec' criado."
else
    echo "Erro durante a compilação com o gcc."
fi
