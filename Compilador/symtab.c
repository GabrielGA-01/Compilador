/*******************************************************************************
 * Arquivo: symtab.c
 * 
 * IMPLEMENTAÇÃO DA TABELA DE SÍMBOLOS
 * 
 * Este arquivo implementa a tabela de símbolos usando uma tabela hash
 * com encadeamento (chaining) para resolver colisões.
 * 
 * ESTRUTURA DE DADOS:
 * - Tabela hash de tamanho SIZE (211 buckets)
 * - Cada bucket contém uma lista encadeada de símbolos
 * - Cada símbolo mantém uma lista de linhas onde aparece
 * - Pilha de escopos para gerenciar escopos aninhados
 * 
 * RESOLUÇÃO DE ESCOPO:
 * - Símbolos são buscados primeiro no escopo atual
 * - Se não encontrado, busca no escopo Global
 * - Isso permite que variáveis locais "sombreiem" globais
 * 
 * Baseado no livro "Compiler Construction" de Kenneth C. Louden.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/* Tamanho da tabela hash - número primo para melhor distribuição */
#define SIZE 211

/* Fator de deslocamento para a função hash */
#define SHIFT 4

/* Profundidade máxima de escopos aninhados */
#define MAX_SCOPE_DEPTH 100

/*******************************************************************************
 * FUNÇÃO HASH
 ******************************************************************************/

/**
 * hash: Calcula o índice na tabela hash para uma string.
 * 
 * Usa a técnica de "shifting" onde cada caractere contribui para o
 * hash final através de multiplicação (deslocamento de bits) e soma.
 * 
 * ALGORITMO:
 *   hash = 0
 *   para cada caractere c na string:
 *     hash = (hash * 16 + c) % SIZE
 * 
 * O uso de deslocamento de 4 bits (<< 4) equivale a multiplicar por 16,
 * o que ajuda a distribuir melhor os valores.
 * 
 * PARÂMETROS:
 *   key - String para calcular o hash
 * 
 * RETORNO:
 *   Índice entre 0 e SIZE-1
 */
static int hash ( char * key )
{ 
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { 
    /* Desloca 4 bits (multiplica por 16) e adiciona o caractere */
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/*******************************************************************************
 * ESTRUTURAS DE DADOS INTERNAS
 ******************************************************************************/

/**
 * LineListRec: Lista encadeada de números de linha.
 * 
 * Cada símbolo mantém uma lista de todas as linhas onde é referenciado.
 * Isso é útil para:
 * - Mostrar onde cada variável é usada (debug)
 * - Potencial análise de código morto
 */
typedef struct LineListRec
   { 
     int lineno;                  /* Número da linha */
     struct LineListRec * next;   /* Próximo elemento da lista */
   } * LineList;

/**
 * BucketListRec: Registro completo de um símbolo na tabela.
 * 
 * Contém todas as informações necessárias sobre uma variável ou função.
 * Os buckets são encadeados para resolver colisões na tabela hash.
 */
typedef struct BucketListRec
   { 
     char * name;          /* Nome do identificador */
     LineList lines;       /* Lista de linhas onde aparece */
     int memloc;           /* Localização na memória (índice sequencial) */
     ExpType type;         /* Tipo de dado (Integer, Void) */
     IdKind kind;          /* Categoria (ID_VAR ou ID_FUN) */
     int isArray;          /* 1 se for array, 0 caso contrário */
     int numParams;        /* Número de parâmetros (para funções) */
     ExpType paramTypes[MAX_PARAMS]; /* Tipos dos parâmetros */
     char * scopeName;     /* Nome do escopo onde foi declarado */
     struct BucketListRec * next;  /* Próximo elemento no bucket (colisão) */
   } * BucketList;

/*******************************************************************************
 * VARIÁVEIS GLOBAIS (ESTÁTICAS)
 ******************************************************************************/

/* A tabela hash propriamente dita */
static BucketList hashTable[SIZE];

/* Pilha de nomes de escopo (para gerenciar escopos aninhados) */
static char * scopeNameStack[MAX_SCOPE_DEPTH];
static int scopeStackTop = 0;

/*******************************************************************************
 * FUNÇÕES DE GERENCIAMENTO DE ESCOPO
 ******************************************************************************/

/**
 * initScopeStack: Inicializa a pilha de escopos.
 * 
 * Garante que o escopo "Global" sempre seja o primeiro na pilha.
 * Esta função é chamada internamente antes de qualquer operação
 * que dependa do escopo atual.
 */
static void initScopeStack() {
    if (scopeStackTop == 0) {
        /* Empilha o escopo Global como base */
        scopeNameStack[scopeStackTop++] = "Global";
    }
}

/**
 * st_enter_scope: Entra em um novo escopo.
 * 
 * Empilha o nome do novo escopo. Chamado quando encontramos
 * uma declaração de função - todos os parâmetros e variáveis
 * locais serão registrados neste novo escopo.
 * 
 * PARÂMETROS:
 *   scopeName - Nome do escopo (tipicamente o nome da função)
 */
void st_enter_scope(char * scopeName) {
    initScopeStack();
    
    /* Verifica overflow da pilha */
    if (scopeStackTop >= MAX_SCOPE_DEPTH) {
        fprintf(stderr, "Error: Max scope depth exceeded\n");
        exit(1);
    }
    
    /* Empilha o novo escopo */
    scopeNameStack[scopeStackTop++] = scopeName;
}

/**
 * st_exit_scope: Sai do escopo atual.
 * 
 * Desempilha o escopo atual. O escopo Global (base) nunca é removido.
 */
void st_exit_scope(void) {
    if (scopeStackTop > 1) { /* Não remove o escopo Global */
        scopeStackTop--;
    }
}

/*******************************************************************************
 * FUNÇÕES DE INSERÇÃO
 ******************************************************************************/

/**
 * st_insert: Insere um símbolo na tabela ou adiciona referência.
 * 
 * COMPORTAMENTO:
 * 1. Se o símbolo NÃO existe no escopo atual:
 *    - Cria nova entrada com todas as informações
 *    - Adiciona ao início do bucket (mais eficiente)
 * 
 * 2. Se o símbolo JÁ existe no escopo atual:
 *    - Apenas adiciona o número da linha à lista de referências
 *    - (indica que o símbolo foi usado novamente)
 * 
 * PARÂMETROS:
 *   name       - Nome do identificador
 *   lineno     - Linha da declaração/uso
 *   loc        - Localização na memória
 *   type       - Tipo (Integer, Void)
 *   kind       - Categoria (ID_VAR, ID_FUN)
 *   isArray    - 1 se array, 0 caso contrário
 *   numParams  - Número de parâmetros (funções)
 *   paramTypes - Array de tipos dos parâmetros
 */
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int isArray, int numParams, ExpType *paramTypes )
{ 
  /* Calcula o índice na tabela hash */
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  /* Obtém o nome do escopo atual */
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Procura o símbolo no escopo atual */
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
    
  if (l == NULL) /* Símbolo não existe no escopo atual - criar novo */
  { 
    /* Aloca memória para o novo registro */
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    
    /* Cria a lista de linhas com a primeira ocorrência */
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;
    
    /* Preenche os campos do símbolo */
    l->memloc = loc;
    l->type = type;
    l->kind = kind;
    l->isArray = isArray;
    l->numParams = numParams;
    
    /* Copia os tipos dos parâmetros (para funções) */
    if (paramTypes != NULL) {
        for (int i = 0; i < numParams && i < MAX_PARAMS; i++) {
            l->paramTypes[i] = paramTypes[i];
        }
    }
    
    l->scopeName = currentScopeName;
    
    /* Insere no início do bucket (O(1)) */
    l->next = hashTable[h];
    hashTable[h] = l; 
  }
  else /* Símbolo já existe - adicionar linha de referência */
  { 
    /* Percorre até o final da lista de linhas */
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    
    /* Adiciona nova linha */
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

/**
 * st_add_ref: Adiciona uma referência a um símbolo existente.
 * 
 * Diferente de st_insert, esta função APENAS adiciona uma linha
 * de referência - nunca cria um novo símbolo. Usada quando
 * encontramos um uso de uma variável já declarada.
 * 
 * A busca considera: Escopo Atual -> Escopo Global
 * 
 * PARÂMETROS:
 *   name   - Nome do identificador
 *   lineno - Linha onde foi referenciado
 */
void st_add_ref( char * name, int lineno )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList found = NULL;
  
  /* PRIMEIRA BUSCA: No escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
  /* SEGUNDA BUSCA: No escopo Global (se não encontrou localmente) */
  if (found == NULL) {
      temp = l;
      while (temp != NULL) {
          if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
              found = temp;
              break;
          }
          temp = temp->next;
      }
  }
  
  /* Se encontrou, adiciona a linha de referência */
  if (found != NULL) {
    LineList t = found->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

/*******************************************************************************
 * FUNÇÕES DE BUSCA
 ******************************************************************************/

/**
 * st_lookup: Busca um símbolo na tabela.
 * 
 * Implementa a regra de escopo: primeiro busca no escopo atual,
 * depois no escopo Global. Isso permite que variáveis locais
 * "sombreiem" variáveis globais com o mesmo nome.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   Localização na memória se encontrado, -1 se não existe
 */
int st_lookup ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* PRIMEIRA BUSCA: Escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  /* SEGUNDA BUSCA: Escopo Global */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  return -1;  /* Não encontrado */
}

/**
 * st_lookup_top: Busca apenas no escopo atual (topo da pilha).
 * 
 * Usado para detectar redeclarações no mesmo escopo.
 * NÃO busca no escopo Global.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   Localização na memória se encontrado no escopo atual, -1 caso contrário
 */
int st_lookup_top ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Busca APENAS no escopo atual */
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
    
  if (l == NULL) return -1;
  else return l->memloc;
}

/**
 * st_lookup_type: Retorna o tipo de um símbolo.
 * 
 * Usa a mesma estratégia de busca: Escopo Atual -> Global.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   Tipo do símbolo (Integer, Void), ou Void se não encontrado
 */
ExpType st_lookup_type ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Busca no escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  /* Busca no escopo Global */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  return Void;  /* Não encontrado - retorna Void como padrão */
}

/**
 * st_lookup_kind: Retorna a categoria de um símbolo.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   ID_VAR ou ID_FUN, ou ID_VAR como padrão se não encontrado
 */
IdKind st_lookup_kind ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Busca no escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  /* Busca no escopo Global */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  return ID_VAR; /* Padrão: variável */
}

/**
 * st_lookup_num_params: Retorna o número de parâmetros de uma função.
 * 
 * PARÂMETROS:
 *   name - Nome da função
 * 
 * RETORNO:
 *   Número de parâmetros, ou 0 se não encontrada
 */
int st_lookup_num_params ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Busca no escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  /* Busca no escopo Global */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  return 0;  /* Não encontrada */
}

/**
 * st_lookup_param_type: Retorna o tipo de um parâmetro específico.
 * 
 * PARÂMETROS:
 *   name       - Nome da função
 *   paramIndex - Índice do parâmetro (0-based)
 * 
 * RETORNO:
 *   Tipo do parâmetro, ou Void se não encontrado ou índice inválido
 */
ExpType st_lookup_param_type ( char * name, int paramIndex )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList found = NULL;
  
  /* Busca no escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
  /* Busca no escopo Global */
  if (found == NULL) {
      temp = l;
      while (temp != NULL) {
          if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
              found = temp;
              break;
          }
          temp = temp->next;
      }
  }
  
  /* Verifica se encontrou e se o índice é válido */
  if (found == NULL || paramIndex >= found->numParams) return Void;
  else return found->paramTypes[paramIndex];
}

/**
 * st_lookup_is_array: Verifica se um símbolo é um array.
 * 
 * PARÂMETROS:
 *   name - Nome do identificador
 * 
 * RETORNO:
 *   1 se for array, 0 caso contrário ou se não encontrado
 */
int st_lookup_is_array ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Busca no escopo atual */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->isArray;
      }
      temp = temp->next;
  }
  
  /* Busca no escopo Global */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->isArray;
      }
      temp = temp->next;
  }
  
  return 0;  /* Não é array (ou não encontrado) */
}

/*******************************************************************************
 * FUNÇÕES DE IMPRESSÃO
 ******************************************************************************/

/**
 * printSymTab: Imprime a tabela de símbolos formatada.
 * 
 * Gera uma tabela com as seguintes colunas:
 * - Hash: Índice do bucket na tabela hash
 * - Name: Nome do identificador
 * - Type: Tipo (Integer, Void, Boolean)
 * - Kind: Categoria (Var, Func)
 * - Scope: Nome do escopo onde foi declarado
 * - Line Numbers: Lista de linhas onde aparece
 * 
 * PARÂMETROS:
 *   listing - Arquivo onde imprimir (pode ser stdout)
 */
void printSymTab(FILE * listing)
{ 
  int i;
  
  /* Imprime cabeçalho da tabela */
  fprintf(listing,"%-15s %-15s %-15s %-15s %-15s %-15s\n", 
          "Hash", "Name", "Type", "Kind", "Scope", "Line Numbers");
  fprintf(listing,"%-15s %-15s %-15s %-15s %-15s %-15s\n", 
          "----", "----", "----", "----", "-----", "------------");
  
  /* Percorre todos os buckets da tabela hash */
  for (i=0;i<SIZE;++i)
  { 
    if (hashTable[i] != NULL)
    { 
      BucketList l = hashTable[i];
      
      /* Percorre todos os símbolos no bucket (lista encadeada) */
      while (l != NULL)
      { 
        LineList t = l->lines;
        
        /* Imprime índice do bucket (hash) */
        fprintf(listing,"%-15d ",i);
        
        /* Imprime nome do símbolo */
        fprintf(listing,"%-15s ",l->name);
        
        /* Imprime tipo */
        switch(l->type) {
            case Integer: fprintf(listing, "%-15s ", "Integer"); break;
            case Void:    fprintf(listing, "%-15s ", "Void"); break;
            case Boolean: fprintf(listing, "%-15s ", "Boolean"); break;
            default:      fprintf(listing, "%-15s ", "Unknown"); break;
        }
        
        /* Imprime categoria */
        switch(l->kind) {
            case ID_VAR: fprintf(listing, "%-15s ", "Var"); break;
            case ID_FUN: fprintf(listing, "%-15s ", "Func"); break;
            default:     fprintf(listing, "%-15s ", "Unknown"); break;
        }
        
        /* Imprime escopo */
        fprintf(listing, "%-15s ", l->scopeName);
        
        /* Imprime lista de linhas */
        while (t != NULL)
        { 
          fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        
        l = l->next;  /* Próximo símbolo no bucket */
      }
    }
  }
}
