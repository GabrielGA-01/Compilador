/*******************************************************************************
 * Arquivo: analyze.h
 * 
 * INTERFACE DO ANALISADOR SEMÂNTICO
 * 
 * Este header define a interface pública para a análise semântica do compilador.
 * A análise semântica é a terceira fase do compilador, responsável por verificar
 * regras que não podem ser expressas na gramática, como:
 * 
 * - Variáveis devem ser declaradas antes de serem usadas
 * - Tipos devem ser compatíveis em operações e atribuições
 * - Funções devem ser chamadas com o número correto de argumentos
 * - A função main deve existir
 * 
 * FASES DA ANÁLISE SEMÂNTICA:
 * 1. buildSymtab - Constrói tabela de símbolos (primeira passagem)
 * 2. typeCheck   - Verifica tipos (segunda passagem)
 * 
 * Baseado no livro "Compiler Construction" de Kenneth C. Louden.
 ******************************************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "ast.h"   /* Estruturas da AST (ASTNode) */

/*******************************************************************************
 * FUNÇÕES PÚBLICAS
 ******************************************************************************/

/**
 * buildSymtab: Constrói a tabela de símbolos.
 * 
 * Percorre a AST em pre-order (visita o nó antes dos filhos) e:
 * 1. Insere declarações de variáveis e funções na tabela de símbolos
 * 2. Verifica declarações duplicadas no mesmo escopo
 * 3. Verifica uso de variáveis não declaradas
 * 4. Gerencia entrada e saída de escopos
 * 5. Verifica se a função main foi declarada
 * 
 * ERROS DETECTADOS:
 * - Variável não declarada
 * - Variável declarada duas vezes no mesmo escopo
 * - Variável declarada como void
 * - Função não declarada
 * - Função main não existe
 * - Número incorreto de parâmetros em chamadas
 * 
 * PARÂMETROS:
 *   syntaxTree - Ponteiro para a raiz da AST
 */
void buildSymtab(ASTNode *);

/**
 * typeCheck: Verifica compatibilidade de tipos.
 * 
 * Percorre a AST em post-order (visita filhos antes do nó) e:
 * 1. Atribui tipos às expressões (Integer, Void, Boolean)
 * 2. Verifica compatibilidade em operações aritméticas
 * 3. Verifica tipo de retorno de funções
 * 4. Verifica uso correto de arrays (com índice)
 * 5. Verifica atribuições válidas (não void)
 * 
 * ERROS DETECTADOS:
 * - Operação aplicada a tipo não-inteiro
 * - Atribuição de valor void
 * - Retorno incompatível com tipo da função
 * - Uso de array sem índice
 * - Tipos de parâmetros incompatíveis
 * 
 * PARÂMETROS:
 *   syntaxTree - Ponteiro para a raiz da AST
 * 
 * OBSERVAÇÃO:
 *   Esta função deve ser chamada APÓS buildSymtab, pois
 *   depende da tabela de símbolos já preenchida.
 */
void typeCheck(ASTNode *);

#endif /* _ANALYZE_H_ */
