# Documentação: Uso de `%locations` no Bison

Este documento explica o motivo da utilização da diretiva `%locations` no arquivo `parser.y` e como ela resolveu o problema da precisão dos números de linha na Tabela de Símbolos.

## O Problema
Anteriormente, usávamos apenas a variável global `yylineno` (fornecida pelo Flex) para rastrear o número da linha.
O problema é que `yylineno` armazena a linha **atual** que o scanner está lendo.

Ao processar uma regra gramatical grande, como a declaração de uma função:
```c
void funcaoTeste(...) {
    // ... várias linhas de código ...
} // <--- O parser só "termina" de ler a função aqui
```
Quando o parser reduzia a regra `fun_declaracao`, ele já tinha lido todo o corpo da função. Nesse momento, `yylineno` apontava para a linha do fechamento `}`.
Como resultado, na tabela de símbolos, a função `funcaoTeste` aparecia com a linha do final do arquivo ou do final da função, e não a linha onde o nome `funcaoTeste` foi escrito.

## A Solução: `%locations`
A diretiva `%locations` instrui o Bison a gerar código para rastrear a localização (linha e coluna) de **cada token individualmente**, e não apenas a posição atual do cursor.

### Implementação

1.  **No `parser.y`**:
    -   Adicionamos `%locations`.
    -   Isso disponibiliza variáveis especiais como `@1`, `@2`, `@3`, etc., que contêm as informações de localização (linha inicial, linha final, coluna) para o 1º, 2º, 3º componente da regra.
    -   Alteramos a regra para pegar a linha exata do identificador (o segundo componente, então `@2`):
        ```yacc
        fun_declaracao: tipo_especificador ID ...
        {
            // @2.first_line é a linha onde o token ID foi encontrado
            func_node->lineno = @2.first_line; 
        }
        ```

2.  **No `scanner.l`**:
    -   Precisamos dizer ao Flex para manter essas informações de localização atualizadas a cada token lido.
    -   Usamos a macro `YY_USER_ACTION`, que é executada antes de cada ação de regra:
        ```c
        #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
        ```
    -   Isso copia o `yylineno` atual para a estrutura `yylloc` do token atual.

## Resumo
Com essa mudança, conseguimos "voltar no tempo" e saber exatamente em qual linha o nome da função (ou variável) estava quando foi lido, independentemente de quantas linhas de código vieram depois dele dentro da mesma regra gramatical.
