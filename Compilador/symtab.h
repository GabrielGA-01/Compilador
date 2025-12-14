/*******************************************************************************
 * Arquivo: symtab.h
 * 
 * INTERFACE DA TABELA DE SÍMBOLOS
 * 
 * Este header define a interface pública para a tabela de símbolos do compilador.
 * A tabela de símbolos armazena informações sobre todos os identificadores
 * (variáveis, funções, parâmetros) encontrados no código fonte.
 * 
 * INFORMAÇÕES ARMAZENADAS PARA CADA SÍMBOLO:
 * - Nome do identificador
 * - Tipo (Integer, Void)
 * - Categoria (variável ou função)
 * - Escopo onde foi declarado
 * - Linhas onde é referenciado
 * - Informações específicas (se é array, número de parâmetros, etc.)
 * 
 * USOS DA TABELA DE SÍMBOLOS:
 * 1. Verificar se variável foi declarada antes de ser usada
 * 2. Detectar declarações duplicadas no mesmo escopo
 * 3. Verificar tipos em expressões e atribuições
 * 4. Validar chamadas de função (número e tipos de argumentos)
 * 
 * Baseado no livro "Compiler Construction" de Kenneth C. Louden.
 ******************************************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <stdio.h>
#include "ast.h"   /* Para usar ExpType (Void, Integer, Boolean) */

/*******************************************************************************
 * ENUMERAÇÃO IdKind
 * 
 * Define as categorias de identificadores que podem ser armazenados.
 ******************************************************************************/
typedef enum { 
    ID_VAR,   /* Identificador de variável (simples ou array) */
    ID_FUN    /* Identificador de função */
} IdKind;

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/* Número máximo de parâmetros que uma função pode ter */
#define MAX_PARAMS 10

/*******************************************************************************
 * FUNÇÕES DE INSERÇÃO
 ******************************************************************************/

/**
 * st_insert: Insere um novo símbolo na tabela ou adiciona uma referência.
 * 
 * Se o símbolo não existe no escopo atual, cria uma nova entrada.
 * Se já existe no escopo atual, apenas adiciona o número da linha
 * à lista de referências (indica que o símbolo foi usado novamente).
 * 
 * PARÂMETROS:
 *   name       - Nome do identificador
 *   lineno     - Linha onde foi declarado/usado
 *   loc        - Localização na memória (índice sequencial)
 *   type       - Tipo do símbolo (Integer, Void)
 *   kind       - Categoria (ID_VAR ou ID_FUN)
 *   isArray    - 1 se for array, 0 caso contrário
 *   numParams  - Número de parâmetros (para funções)
 *   paramTypes - Array com os tipos dos parâmetros (para funções)
 */
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int isArray, int numParams, ExpType *paramTypes );

/*******************************************************************************
 * FUNÇÕES DE BUSCA
 ******************************************************************************/

/**
 * st_lookup: Busca um símbolo na tabela.
 * 
 * Procura primeiro no escopo atual, depois no escopo global.
 * Isso implementa a regra de "shadowing" onde variáveis locais
 * podem ocultar variáveis globais com o mesmo nome.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador a buscar
 * 
 * RETORNO:
 *   Localização na memória se encontrado, -1 se não existe
 */
int st_lookup ( char * name );

/**
 * st_lookup_type: Retorna o tipo de um símbolo.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   Tipo do símbolo (Integer, Void) ou Void se não encontrado
 */
ExpType st_lookup_type ( char * name );

/**
 * st_lookup_kind: Retorna a categoria de um símbolo.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   ID_VAR se for variável, ID_FUN se for função
 */
IdKind st_lookup_kind ( char * name );

/**
 * st_lookup_num_params: Retorna o número de parâmetros de uma função.
 * 
 * PARÂMETROS:
 *   name - Nome da função
 * 
 * RETORNO:
 *   Número de parâmetros, ou 0 se não encontrada
 */
int st_lookup_num_params ( char * name );

/**
 * st_lookup_param_type: Retorna o tipo de um parâmetro específico.
 * 
 * PARÂMETROS:
 *   name       - Nome da função
 *   paramIndex - Índice do parâmetro (0-based)
 * 
 * RETORNO:
 *   Tipo do parâmetro, ou Void se não encontrado
 */
ExpType st_lookup_param_type ( char * name, int paramIndex );

/**
 * st_lookup_is_array: Verifica se um símbolo é um array.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   1 se for array, 0 caso contrário
 */
int st_lookup_is_array ( char * name );

/**
 * st_add_ref: Adiciona uma referência a um símbolo existente.
 * 
 * Usado quando encontramos um uso de uma variável que já foi
 * declarada. Simplesmente adiciona o número da linha à lista
 * de referências do símbolo.
 * 
 * PARÂMETROS:
 *   name   - Nome do identificador
 *   lineno - Linha onde foi referenciado
 */
void st_add_ref( char * name, int lineno );

/*******************************************************************************
 * FUNÇÕES DE GERENCIAMENTO DE ESCOPO
 ******************************************************************************/

/**
 * st_enter_scope: Entra em um novo escopo.
 * 
 * Chamado quando encontramos o início de uma função.
 * O nome do escopo é tipicamente o nome da função.
 * 
 * PARÂMETROS:
 *   scopeName - Nome do novo escopo (normalmente nome da função)
 */
void st_enter_scope(char * scopeName);

/**
 * st_exit_scope: Sai do escopo atual.
 * 
 * Chamado quando terminamos de processar uma função.
 * O escopo "Global" nunca é removido.
 */
void st_exit_scope(void);

/**
 * st_lookup_top: Busca um símbolo apenas no escopo atual (topo).
 * 
 * Diferente de st_lookup, não busca no escopo Global.
 * Usado para detectar redeclarações no mesmo escopo.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   Localização se encontrado no escopo atual, -1 caso contrário
 */
int st_lookup_top ( char * name );

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO
 ******************************************************************************/

/**
 * printSymTab: Imprime o conteúdo da tabela de símbolos.
 * 
 * Gera uma tabela formatada com todas as informações de cada símbolo.
 * 
 * PARÂMETROS:
 *   listing - Arquivo onde a tabela será impressa
 */
void printSymTab(FILE * listing);

#endif /* _SYMTAB_H_ */
