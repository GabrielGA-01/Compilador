#!/bin/bash

echo "=========================================="
echo "      EXECUTANDO TESTES DE ERROS"
echo "=========================================="

for i in {1..10}
do
    echo ""
    echo "------------------------------------------"
    echo "TESTE $i: teste_erro$i.txt"
    echo "------------------------------------------"
    # Run compiler and filter for "Type error" lines to keep output clean
    ./compilador teste_erro$i.txt | grep "Type error"
    
    if [ $? -ne 0 ]; then
        echo "(Nenhum erro de tipo detectado - VERIFIQUE SE ISSO ESTÁ CORRETO)"
    fi
done

echo ""
echo "=========================================="
echo "           FIM DOS TESTES"
echo "=========================================="
