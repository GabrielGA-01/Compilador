/*******************************************************************************
 * Arquivo: analyze.c
 * 
 * IMPLEMENTAÇÃO DO ANALISADOR SEMÂNTICO
 * 
 * Este arquivo implementa a análise semântica do compilador C-.
 * A análise semântica verifica regras que vão além da sintaxe:
 * 
 * VERIFICAÇÕES REALIZADAS:
 * 1. Declaração de variáveis antes do uso
 * 2. Declarações duplicadas no mesmo escopo
 * 3. Compatibilidade de tipos em expressões
 * 4. Número e tipo de argumentos em chamadas de função
 * 5. Tipo de retorno compatível com declaração da função
 * 6. Existência da função main
 * 7. Uso correto de arrays (sempre com índice)
 * 8. Variáveis não podem ser declaradas como void
 * 
 * ESTRATÉGIA DE IMPLEMENTAÇÃO:
 * - Uma única passagem pela AST
 * - Em pre-order: insere símbolos na tabela
 * - Em post-order: verifica tipos das expressões
 * 
 * Baseado no livro "Compiler Construction" de Kenneth C. Louden.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"      /* Funções da tabela de símbolos */
#include "analyze.h"     /* Interface deste módulo */
#include "ast.h"         /* Estruturas da AST */
#include "parser.tab.h"  /* Constantes de tokens (INT, VOID, etc.) */

/*******************************************************************************
 * VARIÁVEIS GLOBAIS
 ******************************************************************************/

FILE * listing = NULL;   /* Arquivo para saída de mensagens (normalmente stdout) */
int Error = 0;           /* Flag de erro: 1 se algum erro foi encontrado */
int TraceAnalyze = 1;    /* Flag para ativar mensagens de debug */

/* Contador para localização de memória dos símbolos */
static int location = 0;

/* Nome da função atual sendo analisada (para verificar escopo) */
static char * currentFuncName = "Global";

/*******************************************************************************
 * PROTÓTIPOS DE FUNÇÕES INTERNAS
 ******************************************************************************/

static void typeError(ASTNode * t, char * message);
static void checkNode(ASTNode * t);

/*******************************************************************************
 * FUNÇÃO DE TRAVESSIA DA AST
 ******************************************************************************/

/**
 * traverse: Percorre a AST aplicando funções em pre-order e post-order.
 * 
 * Esta é uma função genérica de travessia que visita todos os nós da AST.
 * Permite especificar funções a serem executadas antes (preProc) e depois
 * (postProc) de visitar os filhos de cada nó.
 * 
 * ORDEM DE EXECUÇÃO PARA CADA NÓ:
 * 1. preProc(nó)          - Processa antes dos filhos (pre-order)
 * 2. traverse(filhos)      - Recursão para os filhos
 * 3. postProc(nó)         - Processa após os filhos (post-order)
 * 4. traverse(irmãos)      - Continua para o próximo da lista
 * 
 * TRATAMENTO ESPECIAL DE ESCOPOS:
 * - NODE_FUN_DECL: Entra no escopo da função
 * - NODE_COMPOUND_STMT: Pode criar escopo aninhado
 * - Filhos de declarações são tratados especialmente
 * 
 * PARÂMETROS:
 *   t       - Nó raiz da subárvore a percorrer
 *   preProc - Função a executar antes dos filhos (pode ser nullProc)
 *   postProc - Função a executar após os filhos (pode ser nullProc)
 */
static void traverse( ASTNode * t,
               void (* preProc) (ASTNode *),
               void (* postProc) (ASTNode *) )
{ 
  if (t != NULL)
  { 
    int enteredScope = 0;
    
    /* FASE 1: Processamento pre-order */
    preProc(t);
    
    /*-------------------------------------------------------------------------
     * Gerenciamento de Escopo
     * 
     * Quando encontramos uma declaração de função, entramos em seu escopo.
     * Todos os parâmetros e variáveis locais serão registrados nesse escopo.
     *------------------------------------------------------------------------*/
    if (t->type == NODE_FUN_DECL) {
        /* Extrai o nome da função do nó filho direito */
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        }
        if (name != NULL) currentFuncName = name;
        
        /* Entra no novo escopo */
        st_enter_scope(currentFuncName);
        enteredScope = 1;
    }
    
    /*-------------------------------------------------------------------------
     * Tratamento Especial de Filhos
     * 
     * Alguns tipos de nós têm filhos que não devem ser visitados
     * automaticamente pela recursão genérica porque precisam de
     * tratamento especial (ex: declarações processam seus filhos
     * na função insertNode).
     *------------------------------------------------------------------------*/
    int skipChildren = 0;
    if (t->type == NODE_VAR_DECL || t->type == NODE_FUN_DECL || 
        t->type == NODE_PARAM || t->type == NODE_FUN_CALL) {
        skipChildren = 1;
    }

    /* FASE 2: Recursão para filhos (se não for pular) */
    if (!skipChildren) {
        /* Blocos compostos criam escopos para variáveis locais */
        if (t->type == NODE_COMPOUND_STMT) st_enter_scope(currentFuncName);
        
        traverse(t->leftChild, preProc, postProc);
        traverse(t->rightChild, preProc, postProc);
        
        if (t->type == NODE_COMPOUND_STMT) st_exit_scope();
    }
    
    /*-------------------------------------------------------------------------
     * Tratamento de Chamadas de Função
     * 
     * Para NODE_FUN_CALL, precisamos percorrer os argumentos (filho direito)
     * mas NÃO o nome da função (filho esquerdo), pois isso já foi tratado.
     *------------------------------------------------------------------------*/
    if (t->type == NODE_FUN_CALL) {
        traverse(t->rightChild, preProc, postProc);
    }
    
    /* FASE 3: Processamento post-order */
    postProc(t);
    
    /*-------------------------------------------------------------------------
     * Processamento do Corpo da Função
     * 
     * Funções têm estrutura especial: FUN_DECL -> next -> FUN_BODY
     * O corpo deve ser processado DENTRO do escopo da função.
     *------------------------------------------------------------------------*/
    if (t->type == NODE_FUN_DECL && enteredScope) {
        if (t->next != NULL && t->next->type == NODE_FUN_BODY) {
            ASTNode *body = t->next;
            
            /* Processa o corpo manualmente dentro do escopo */
            preProc(body);
            
            /* Parâmetros (filho esquerdo) */
            traverse(body->leftChild, preProc, postProc);
            /* Bloco de código (filho direito) */
            traverse(body->rightChild, preProc, postProc);
            
            postProc(body);
            
            /* Sai do escopo da função */
            st_exit_scope();
            currentFuncName = "Global";
            
            /* Continua para a próxima declaração (pula o FUN_BODY já processado) */
            traverse(body->next, preProc, postProc);
        } else {
            st_exit_scope();
            currentFuncName = "Global";
            traverse(t->next, preProc, postProc);
        }
    } else {
        /* FASE 4: Continua para o próximo irmão */
        traverse(t->next, preProc, postProc);
    }
  }
}

/**
 * nullProc: Função vazia usada para travessias parciais.
 * 
 * Usada quando queremos apenas pre-order ou apenas post-order.
 */
static void nullProc(ASTNode * t)
{ 
  if (t==NULL) return;
  else return;
}

/*******************************************************************************
 * FUNÇÕES AUXILIARES
 ******************************************************************************/

/**
 * getExpType: Converte um nó TYPE para ExpType.
 * 
 * PARÂMETROS:
 *   typeNode - Nó do tipo NODE_TYPE contendo INT ou VOID
 * 
 * RETORNO:
 *   Integer se INT, Void caso contrário
 */
static ExpType getExpType(ASTNode* typeNode) {
    if (typeNode == NULL) return Void;
    if (typeNode->number == INT) return Integer;
    if (typeNode->number == VOID) return Void;
    return Void;
}

/*******************************************************************************
 * FUNÇÃO insertNode - PRE-ORDER
 * 
 * Esta função é chamada em pre-order (antes de visitar os filhos).
 * Sua responsabilidade é inserir símbolos na tabela e verificar
 * regras de declaração.
 ******************************************************************************/

/**
 * insertNode: Insere identificadores na tabela de símbolos.
 * 
 * Esta função processa diferentes tipos de nós para:
 * 1. Registrar declarações na tabela de símbolos
 * 2. Verificar redeclarações
 * 3. Verificar uso de identificadores não declarados
 * 4. Verificar número de parâmetros em chamadas
 * 
 * PARÂMETROS:
 *   t - Nó atual sendo processado
 */
static void insertNode( ASTNode * t)
{ 
  switch (t->type)
  { 
    /*=========================================================================
     * CASO 1: DECLARAÇÃO DE VARIÁVEL
     *========================================================================*/
    case NODE_VAR_DECL:
      {
        /* Extrai o nome da variável */
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        } else if (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) {
             /* Variável é um array - nome está no filho esquerdo do ARRAY_DECL */
             if (t->rightChild->leftChild != NULL)
                name = t->rightChild->leftChild->identifier;
        }
        
        if (name != NULL) {
            ExpType type = getExpType(t->leftChild);
            
            /*-----------------------------------------------------------------
             * ERRO: Variável declarada como Void
             * 
             * Em C-, apenas funções podem ser void. Variáveis devem ser int.
             *----------------------------------------------------------------*/
            if (type == Void) {
                char buf[256];
                sprintf(buf, "Variable '%s' declared as Void", name);
                typeError(t, buf);
            }
            
            /*-----------------------------------------------------------------
             * ERRO: Variável já declarada no mesmo escopo
             * 
             * st_lookup_top só verifica o escopo atual, permitindo
             * shadowing de variáveis globais.
             *----------------------------------------------------------------*/
            if (st_lookup_top(name) != -1) {
                char buf[256];
                sprintf(buf, "Variable '%s' already declared", name);
                typeError(t, buf);
            } else {
                /*-------------------------------------------------------------
                 * ERRO: Variável com nome de função (shadowing)
                 * 
                 * Não permitimos que uma variável local tenha o mesmo
                 * nome de uma função já declarada.
                 *------------------------------------------------------------*/
                if (st_lookup(name) != -1 && st_lookup_kind(name) == ID_FUN) {
                     char buf[256];
                     sprintf(buf, "Variable name '%s' shadows a function", name);
                     typeError(t, buf);
                }
                
                /* Insere na tabela de símbolos */
                int isArray = (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) ? 1 : 0;
                st_insert(name, t->lineno, location++, type, ID_VAR, isArray, 0, NULL);
            }
        }
      }
      break;
      
    /*=========================================================================
     * CASO 2: DECLARAÇÃO DE FUNÇÃO
     *========================================================================*/
    case NODE_FUN_DECL:
      {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        }
        
        if (name != NULL) {
             ExpType type = getExpType(t->leftChild);
             
             /*-----------------------------------------------------------------
              * ERRO: Função já declarada
              *----------------------------------------------------------------*/
             if (st_lookup_top(name) != -1) {
                  char buf[256];
                  sprintf(buf, "Function '%s' already declared", name);
                  typeError(t, buf);
             } else {
                 /*-------------------------------------------------------------
                  * Coleta informações dos parâmetros
                  * 
                  * Precisamos saber número e tipos dos parâmetros para
                  * verificar chamadas à função posteriormente.
                  *------------------------------------------------------------*/
                 int numParams = 0;
                 ExpType paramTypes[MAX_PARAMS];
                 
                 if (t->next != NULL && t->next->type == NODE_FUN_BODY) {
                     ASTNode *params = t->next->leftChild;
                     while (params != NULL) {
                         if (params->type == NODE_PARAM) {
                             if (numParams < MAX_PARAMS) {
                                 paramTypes[numParams++] = getExpType(params->leftChild);
                             }
                         }
                         params = params->next;
                     }
                 }
                 
                 /* Insere função na tabela */
                 st_insert(name, t->lineno, location++, type, ID_FUN, 0, numParams, paramTypes);
             }
         }
      }
      break;

    /*=========================================================================
     * CASO 3: PARÂMETRO DE FUNÇÃO
     *========================================================================*/
    case NODE_PARAM:
      {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        } else if (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) {
             if (t->rightChild->leftChild != NULL)
                name = t->rightChild->leftChild->identifier;
        }
        
        if (name != NULL) {
            ExpType type = getExpType(t->leftChild);
            
            /* ERRO: Parâmetro declarado como void */
            if (type == Void) {
                char buf[256];
                sprintf(buf, "Parameter '%s' declared as Void", name);
                typeError(t, buf);
            }
            
            /* ERRO: Parâmetro duplicado */
            if (st_lookup_top(name) != -1) {
                 char buf[256];
                 sprintf(buf, "Parameter '%s' already declared", name);
                 typeError(t, buf);
            } else {
                int isArray = (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) ? 1 : 0;
                st_insert(name, t->lineno, location++, type, ID_VAR, isArray, 0, NULL);
            }
        }
      }
      break;

    /*=========================================================================
     * CASO 4: USO DE VARIÁVEL
     *========================================================================*/
    case NODE_VAR:
      {
        /*---------------------------------------------------------------------
         * ERRO: Variável não declarada
         * 
         * Toda variável usada deve ter sido declarada anteriormente.
         *--------------------------------------------------------------------*/
        if (st_lookup(t->identifier) == -1) {
            char buf[256];
            sprintf(buf, "Variable '%s' not declared", t->identifier);
            typeError(t, buf);
        } else {
            /* Registra uso da variável (adiciona linha de referência) */
            st_add_ref(t->identifier, t->lineno);
        }
      }
      break;
      
    /*=========================================================================
     * CASO 5: CHAMADA DE FUNÇÃO
     *========================================================================*/
    case NODE_FUN_CALL:
      {
          if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
              /*---------------------------------------------------------------
               * ERRO: Função não declarada
               *--------------------------------------------------------------*/
              if (st_lookup(t->leftChild->identifier) == -1) {
                  char buf[256];
                  sprintf(buf, "Function '%s' not declared", t->leftChild->identifier);
                  typeError(t, buf);
              } else {
                  st_add_ref(t->leftChild->identifier, t->lineno);
                  
                  /*-----------------------------------------------------------
                   * Verifica número de parâmetros
                   * 
                   * Conta argumentos passados e compara com a declaração.
                   *----------------------------------------------------------*/
                  int expectedParams = st_lookup_num_params(t->leftChild->identifier);
                  int actualParams = 0;
                  ASTNode *arg = t->rightChild;
                  
                  /* Conta argumentos */
                  ASTNode *tempArg = arg;
                  while (tempArg != NULL) {
                      actualParams++;
                      tempArg = tempArg->next;
                  }
                  
                  /* ERRO: Número incorreto de parâmetros */
                  if (actualParams != expectedParams) {
                      char buf[256];
                      sprintf(buf, "Invalid number of parameters for function '%s'", t->leftChild->identifier);
                      typeError(t, buf);
                  }
                  /* Nota: Tipos dos parâmetros são verificados em checkNode (post-order) */
              }
          }
      }
      break;

    default:
      break;
  }
}

/*******************************************************************************
 * FUNÇÃO buildSymtab - PONTO DE ENTRADA
 ******************************************************************************/

/**
 * buildSymtab: Constrói a tabela de símbolos.
 * 
 * Esta é a primeira fase da análise semântica. Percorre toda a AST
 * inserindo símbolos e verificando declarações.
 * 
 * FUNÇÕES BUILT-IN:
 * Antes de processar o código do usuário, insere as funções built-in
 * input() e output() que são fornecidas pelo runtime C-.
 * 
 * PARÂMETROS:
 *   syntaxTree - Raiz da AST gerada pelo parser
 */
void buildSymtab(ASTNode * syntaxTree)
{ 
  if (listing == NULL) listing = stdout;
  
  /*---------------------------------------------------------------------------
   * Insere funções built-in
   * 
   * input(): Lê um inteiro da entrada padrão
   *          - Tipo retorno: Integer
   *          - Parâmetros: nenhum
   * 
   * output(): Escreve um inteiro na saída padrão
   *           - Tipo retorno: Void
   *           - Parâmetros: 1 (Integer)
   *--------------------------------------------------------------------------*/
  ExpType outputParamTypes[1] = { Integer };
  st_insert("input", 0, location++, Integer, ID_FUN, 0, 0, NULL);
  st_insert("output", 0, location++, Void, ID_FUN, 0, 1, outputParamTypes);
  
  /*---------------------------------------------------------------------------
   * Travessia combinada
   * 
   * Fazemos inserção e verificação de tipos em uma única passagem:
   * - Pre-order (insertNode): Insere símbolos na tabela
   * - Post-order (checkNode): Verifica tipos das expressões
   *--------------------------------------------------------------------------*/
  traverse(syntaxTree, insertNode, checkNode);
  
  /*---------------------------------------------------------------------------
   * ERRO: Função main não declarada
   * 
   * Em C-, todo programa deve ter uma função main como ponto de entrada.
   *--------------------------------------------------------------------------*/
  if (st_lookup("main") == -1) {
      fprintf(listing, "[Semantic Error] Function main not declared\n");
      Error = 1;
  }
  
  if (TraceAnalyze)
  { 
      /* Impressão da tabela movida para main */
  }
}

/*******************************************************************************
 * FUNÇÃO typeError
 ******************************************************************************/

/**
 * typeError: Reporta um erro semântico.
 * 
 * Imprime uma mensagem de erro formatada e marca a flag de erro.
 * 
 * PARÂMETROS:
 *   t       - Nó onde o erro foi detectado (para obter a linha)
 *   message - Descrição do erro
 */
static void typeError(ASTNode * t, char * message)
{ 
  fprintf(listing, "[Semantic Error] Line %d: %s\n", t->lineno, message);
  Error = 1;
}

/*******************************************************************************
 * FUNÇÃO checkNode - POST-ORDER
 * 
 * Esta função é chamada em post-order (após visitar os filhos).
 * Quando ela processa um nó, todos os filhos já têm seus tipos calculados.
 * Isso permite verificar compatibilidade de tipos em expressões.
 ******************************************************************************/

/**
 * checkNode: Verifica tipos em um nó.
 * 
 * Realiza verificação de tipos dependendo do tipo de nó:
 * - BINARY_OP: Ambos operandos devem ser Integer
 * - ASSIGN_EXPR: Valor atribuído não pode ser Void
 * - RETURN_STMT: Tipo de retorno deve corresponder à função
 * - FUN_CALL: Tipos dos argumentos devem corresponder aos parâmetros
 * 
 * PARÂMETROS:
 *   t - Nó atual sendo verificado
 */
static void checkNode(ASTNode * t)
{ 
  switch (t->type)
  { 
    /*=========================================================================
     * OPERAÇÕES BINÁRIAS E ATRIBUIÇÕES
     *========================================================================*/
    case NODE_BINARY_OP:
    case NODE_ASSIGN_EXPR:
      if (t->type == NODE_ASSIGN_EXPR) {
          /*-------------------------------------------------------------------
           * Verificação de atribuição a array sem índice
           * 
           * Em C-, não podemos atribuir a um array inteiro, apenas a
           * elementos individuais (arr[i] = valor).
           *------------------------------------------------------------------*/
          if (t->leftChild->type == NODE_VAR) {
              if (st_lookup_is_array(t->leftChild->identifier)) {
                  char buf[256];
                  sprintf(buf, "Cannot assign to array '%s' without index", t->leftChild->identifier);
                  typeError(t, buf);
              }
          }
          
          /*-------------------------------------------------------------------
           * Verificação de uso de array sem índice no lado direito
           *------------------------------------------------------------------*/
          if (t->rightChild->type == NODE_VAR) {
              if (st_lookup_is_array(t->rightChild->identifier)) {
                  char buf[256];
                  sprintf(buf, "Cannot use array '%s' without index", t->rightChild->identifier);
                  typeError(t, buf);
              }
          }
          
          /*-------------------------------------------------------------------
           * Verificação de atribuição de valor Void
           * 
           * Não podemos atribuir o resultado de uma função void a uma variável.
           * Ex: x = output(y);  // ERRO: output retorna void
           *------------------------------------------------------------------*/
          int skipVoidError = 0;
          if (t->rightChild->type == NODE_FUN_CALL && 
              t->rightChild->leftChild != NULL &&
              t->rightChild->leftChild->type == NODE_VAR) {
              /* Se a função não foi declarada, já reportamos erro antes */
              if (st_lookup(t->rightChild->leftChild->identifier) == -1) {
                  skipVoidError = 1;
              }
          }
          
          if (!skipVoidError && t->rightChild->expType == Void) {
              char buf[256];
              char *name = "unknown";
              if (t->leftChild->type == NODE_VAR) {
                  name = t->leftChild->identifier;
              } else if (t->leftChild->type == NODE_ARRAY_ACCESS && 
                         t->leftChild->leftChild != NULL &&
                         t->leftChild->leftChild->type == NODE_VAR) {
                  name = t->leftChild->leftChild->identifier;
              }
              sprintf(buf, "Invalid assignment to '%s' (Void value)", name);
              typeError(t, buf);
          } else if (t->leftChild->expType != Integer || t->rightChild->expType != Integer) {
             if (!skipVoidError) typeError(t,"Op applied to non-integer");
          }
      } else {
          /* Operação binária: ambos operandos devem ser Integer */
          if (t->leftChild->expType != Integer || t->rightChild->expType != Integer)
            typeError(t,"Op applied to non-integer");
      }
      
      /*-----------------------------------------------------------------------
       * Define o tipo resultante da expressão
       * 
       * - Comparações (==, !=, <, >, <=, >=): Boolean
       * - Aritméticas (+, -, *, /) e atribuições: Integer
       *----------------------------------------------------------------------*/
      if ((t->type == NODE_BINARY_OP) && (t->number == EQ || t->number == DIF || 
          t->number == LT || t->number == GT || t->number == LET || t->number == GET))
        t->expType = Boolean;
      else
        t->expType = Integer;
      break;
      
    /*=========================================================================
     * LITERAL NUMÉRICO
     *========================================================================*/
    case NODE_NUM:
      t->expType = Integer;  /* Números são sempre Integer */
      break;
      
    /*=========================================================================
     * REFERÊNCIA A VARIÁVEL
     *========================================================================*/
    case NODE_VAR:
      /* Obtém tipo da tabela de símbolos */
      if (st_lookup(t->identifier) == -1)
          t->expType = Integer;  /* Padrão se não encontrado */
      else
          t->expType = st_lookup_type(t->identifier);
      break;
      
    /*=========================================================================
     * CHAMADA DE FUNÇÃO
     *========================================================================*/
    case NODE_FUN_CALL:
      if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
          /* O tipo da chamada é o tipo de retorno da função */
          t->expType = st_lookup_type(t->leftChild->identifier);
          
          /*-------------------------------------------------------------------
           * Verificação de tipos dos parâmetros
           * 
           * Compara o tipo de cada argumento passado com o tipo esperado.
           *------------------------------------------------------------------*/
          int expectedParams = st_lookup_num_params(t->leftChild->identifier);
          ASTNode *arg = t->rightChild;
          int i = 0;
          while (arg != NULL && i < expectedParams) {
              ExpType expectedType = st_lookup_param_type(t->leftChild->identifier, i);
              if (arg->expType != expectedType) {
                  char buf[256];
                  sprintf(buf, "Invalid parameter type in call to '%s'", t->leftChild->identifier);
                  typeError(t, buf);
              }
              arg = arg->next;
              i++;
          }
      }
      break;
      
    /*=========================================================================
     * ACESSO A ARRAY
     *========================================================================*/
    case NODE_ARRAY_ACCESS:
      /* Acesso a array retorna Integer (tipo dos elementos) */
      t->expType = Integer;
      break;
      
    /*=========================================================================
     * COMANDO RETURN
     *========================================================================*/
    case NODE_RETURN_STMT:
      {
          /* Obtém o tipo de retorno esperado da função atual */
          ExpType funcType = st_lookup_type(currentFuncName);
          
          if (funcType == Void) {
              /* Função void não pode retornar valor */
              if (t->leftChild != NULL) {
                  typeError(t, "Void function cannot return a value");
              }
          } else {
              /* Função não-void deve retornar valor */
              if (t->leftChild == NULL) {
                  typeError(t, "Function expected to return a value");
              } else if (t->leftChild->expType != funcType) {
                  typeError(t, "Invalid return type");
              }
          }
      }
      break;
      
    default:
      break;
  }
}

/*******************************************************************************
 * VERIFICAÇÃO DE RETORNO NÃO USADO
 ******************************************************************************/

/**
 * checkUnusedReturn: Verifica chamadas de função com retorno não usado.
 * 
 * Em alguns contextos, chamar uma função que retorna um valor sem usar
 * esse valor pode ser um erro de programação. Esta função detecta esses casos.
 * 
 * EXEMPLO:
 *   gcd(u, v);  // AVISO: retorno de gcd não está sendo usado
 * 
 * PARÂMETROS:
 *   t - Nó raiz da subárvore a verificar
 */
static void checkUnusedReturn(ASTNode *t) {
    if (t == NULL) return;
    
    if (t->type == NODE_COMPOUND_STMT) {
        /* Itera pelos statements do bloco */
        ASTNode *stmt = t->rightChild;
        while (stmt != NULL) {
            /* Se um statement é diretamente uma chamada de função */
            if (stmt->type == NODE_FUN_CALL) {
                if (stmt->expType != Void) {
                    typeError(stmt, "Function return value not used");
                }
            }
            /* Recursão para statements aninhados */
            checkUnusedReturn(stmt);
            stmt = stmt->next;
        }
    } else {
        /* Recursão para filhos e irmãos */
        checkUnusedReturn(t->leftChild);
        checkUnusedReturn(t->rightChild);
        checkUnusedReturn(t->next);
    }
}

/*******************************************************************************
 * FUNÇÃO typeCheck - PONTO DE ENTRADA
 ******************************************************************************/

/**
 * typeCheck: Verifica tipos em toda a AST.
 * 
 * Esta é a segunda fase da análise semântica. Na implementação atual,
 * a verificação de tipos é feita junto com buildSymtab em uma única
 * passagem. Esta função realiza verificações adicionais.
 * 
 * PARÂMETROS:
 *   syntaxTree - Raiz da AST
 */
void typeCheck(ASTNode * syntaxTree)
{ 
  /* A verificação principal já foi feita em buildSymtab */
  /* Verifica retornos não usados como verificação adicional */
  checkUnusedReturn(syntaxTree);
}
