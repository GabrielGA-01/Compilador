# Relatório de Mudanças - Integração da Tabela de Símbolos

Este documento detalha todas as alterações realizadas para integrar a tabela de símbolos ao analisador semântico do compilador.

## 1. Visão Geral
O objetivo foi adaptar o analisador semântico (originalmente baseado no compilador Tiny) para funcionar com a estrutura de AST (`ASTNode`) do seu projeto e integrar a tabela de símbolos para rastrear variáveis e funções.

## 2. Arquivos Alterados

### `Semantico/symtab.h`
- **Mudança**: Adicionada a inclusão de `<stdio.h>`.
- **Motivo**: O arquivo usava o tipo `FILE*` na função `printSymTab`, mas não incluía a biblioteca necessária, causando dependência de outros arquivos. Agora ele é independente.

### `Semantico/analyze.h`
- **Mudança**: Substituído `TreeNode*` por `ASTNode*` nos protótipos das funções `buildSymtab` e `typeCheck`.
- **Mudança**: Incluído `ast.h`.
- **Motivo**: A estrutura de dados da sua árvore sintática é `ASTNode`, não `TreeNode`.

### `Semantico/analyze.c`
- **Reescrita Completa**: O arquivo foi reescrito para adaptar a lógica de travessia da árvore.
- **Função `traverse`**: Adaptada para percorrer `leftChild`, `rightChild` e `next` (estrutura da sua AST).
- **Função `insertNode`**:
    - Identifica declarações de variáveis (`NODE_VAR_DECL`) e insere na tabela.
    - Identifica declarações de funções (`NODE_FUN_DECL`) e insere na tabela.
    - Identifica parâmetros (`NODE_PARAM`) e insere na tabela.
    - Identifica usos de variáveis (`NODE_VAR`) e atualiza a tabela com o número da linha (referência).
- **Função `buildSymtab`**: Inicia a travessia da árvore a partir da raiz.
- **Correção**: Removido `#include "globals.h"` que não existia no projeto.

### `Semantico/parser.y`
- **Mudança**: Incluído `analyze.h`.
- **Mudança na `main`**:
    - Adicionado suporte a argumentos de linha de comando (`argc`, `argv`) para permitir passar o arquivo de entrada (ex: `./compilador gcd.txt`).
    - Adicionada a chamada `buildSymtab(root)` logo após a impressão da AST para gerar e exibir a tabela de símbolos.

### `Semantico/scanner.l`
- **Mudança**: Removida a função `abrirArq` que forçava a abertura do arquivo `sort.txt`.
- **Motivo**: Permitir que o compilador funcione com qualquer arquivo passado por argumento.

### `Semantico/compilar.sh`
- **Mudança**: Adicionados passos para compilar `symtab.c` e `analyze.c`.
- **Mudança**: Atualizado o comando de linkagem para incluir `symtab.o` e `analyze.o` no executável final.

## 3. Verificação Realizada

### Teste com `gcd.txt`
- **Comando**: `./compilador gcd.txt`
- **Resultado**: A tabela de símbolos foi gerada corretamente, listando a função `gcd`, `main` e as variáveis `u`, `v`, `x`, `y`.

### Teste com `sort.txt`
- **Comando**: `./compilador sort.txt`
- **Resultado**: A tabela de símbolos listou corretamente todas as variáveis globais, locais e funções do algoritmo de ordenação.

## 4. Próximos Passos Sugeridos
1.  **Verificação de Tipos**: Implementar a lógica em `analyze.c` para verificar se operações (soma, atribuição, etc.) estão usando tipos compatíveis.
2.  **Escopos**: Implementar suporte a escopos aninhados (atualmente a tabela é global), para permitir que variáveis locais tenham o mesmo nome de variáveis globais sem conflito.

## 5. Atualização: Propagação de Números de Linha
Para corrigir os números de linha zerados na tabela de símbolos, foram feitas as seguintes alterações adicionais:

### `Semantico/ast.h`
- **Mudança**: Adicionado o campo `int lineno` na estrutura `ASTNode`.

### `Semantico/ast.c`
- **Mudança**: Atualizada a função `create_node` para inicializar `lineno` com o valor da variável global `yylineno` (fornecida pelo Flex).

### `Semantico/analyze.c`
- **Mudança**: Atualizada a função `insertNode` para usar `t->lineno` ao chamar `st_insert`, garantindo que o número da linha correto seja registrado na tabela de símbolos.
- **Mudança**: Atualizada a função `traverse` para não visitar os filhos de nós de declaração (`NODE_VAR_DECL`, `NODE_FUN_DECL`, `NODE_PARAM`). Isso evita que o identificador seja processado duas vezes (uma na declaração e outra como filho), corrigindo a duplicação na tabela de símbolos.

## 6. Atualização: Contagem de Linhas (Comentários e Espaços)
Para garantir que a contagem de linhas considere comentários e linhas em branco:

### `Semantico/scanner.l`
- **Mudança**: Adicionada lógica dentro do bloco de tratamento de comentários `/* ... */` para incrementar `yylineno` sempre que um caractere `\n` for encontrado.
- **Mudança**: Substituída a regra `quebralinha` (que podia agrupar vários `\n` em um só token) por uma regra explícita `\n` que incrementa `yylineno` a cada ocorrência. Isso garante precisão na contagem.

## 7. Atualização: Precisão da Linha de Funções
Para corrigir o problema onde a linha da função apontava para o final do corpo (fechamento `}`) em vez do nome da função:

### `Semantico/parser.y`
- **Mudança**: Habilitado `%locations` para rastrear a localização exata de cada token.
- **Mudança**: Atualizadas as regras gramaticais (`fun_declaracao`, `var_declaracao`, `param`) para atribuir explicitamente a linha do identificador (`@2.first_line`) ao nó da AST, em vez de usar a linha global `yylineno` (que estaria avançada no final da regra).

### `Semantico/scanner.l`
- **Mudança**: Adicionada a macro `YY_USER_ACTION` para sincronizar a estrutura de localização `yylloc` com a variável `yylineno` a cada token lido.

## 8. Implementação: Verificação de Tipos
Implementada a lógica para verificar a compatibilidade de tipos em expressões e atribuições.

### `Semantico/ast.h`
- **Mudança**: Adicionado o enum `ExpType` (Void, Integer, Boolean) e o campo `expType` na estrutura `ASTNode`.

### `Semantico/symtab.h` & `Semantico/symtab.c`
- **Mudança**: Atualizada a tabela de símbolos para armazenar o tipo de dados (`ExpType`) de cada variável e função.
- **Mudança**: Adicionada a coluna "Type" na impressão da tabela de símbolos.

### `Semantico/analyze.c`
- **Mudança**: Atualizada a função `insertNode` para extrair e salvar o tipo das declarações na tabela de símbolos.
- **Mudança**: Implementada a função `checkNode` que verifica:
    - Se operações aritméticas e atribuições envolvem apenas inteiros.
    - Se o tipo de retorno de chamadas de função é compatível com a expressão onde são usadas.
- **Mudança**: Adicionada a chamada `typeCheck(root)` no `parser.y` após a construção da tabela de símbolos.
