#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

#define SIZE 211
#define SHIFT 4
#define MAX_SCOPE_DEPTH 100

// funcao hash simples
static int hash ( char * key )
{ 
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { 
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

typedef struct LineListRec
   { 
     int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { 
     char * name;
     LineList lines;
     int memloc;
     ExpType type;
     IdKind kind;
     int isArray;
     int numParams;
     ExpType paramTypes[MAX_PARAMS];
     char * scopeName;
     struct BucketListRec * next;
   } * BucketList;

static BucketList hashTable[SIZE];

static char * scopeNameStack[MAX_SCOPE_DEPTH];
static int scopeStackTop = 0;

static void initScopeStack() {
    if (scopeStackTop == 0) {
        scopeNameStack[scopeStackTop++] = "Global";
    }
}

// entra num novo escopo
void st_enter_scope(char * scopeName) {
    initScopeStack();
    if (scopeStackTop >= MAX_SCOPE_DEPTH) {
        fprintf(stderr, "Error: Max scope depth exceeded\n");
        exit(1);
    }
    scopeNameStack[scopeStackTop++] = scopeName;
}

// sai do escopo atual
void st_exit_scope(void) {
    if (scopeStackTop > 1) {
        scopeStackTop--;
    }
}

// insere simbolo ou adiciona referencia se ja existe
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int isArray, int numParams, ExpType *paramTypes )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
    
  if (l == NULL)
  { 
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;
    l->memloc = loc;
    l->type = type;
    l->kind = kind;
    l->isArray = isArray;
    l->numParams = numParams;
    
    if (paramTypes != NULL) {
        for (int i = 0; i < numParams && i < MAX_PARAMS; i++) {
            l->paramTypes[i] = paramTypes[i];
        }
    }
    
    l->scopeName = currentScopeName;
    l->next = hashTable[h];
    hashTable[h] = l; 
  }
  else
  { 
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

// adiciona referencia a simbolo existente
void st_add_ref( char * name, int lineno )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList found = NULL;
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
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
  
  if (found != NULL) {
    LineList t = found->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

// busca simbolo no escopo atual e depois no global
int st_lookup ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  return -1;
}

// busca so no escopo atual (pra detectar redeclaracao)
int st_lookup_top ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
    
  if (l == NULL) return -1;
  else return l->memloc;
}

// retorna tipo do simbolo
ExpType st_lookup_type ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  return Void;
}

// retorna categoria (var ou func)
IdKind st_lookup_kind ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  return ID_VAR;
}

// retorna numero de parametros da funcao
int st_lookup_num_params ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  return 0;
}

// retorna tipo de um parametro especifico
ExpType st_lookup_param_type ( char * name, int paramIndex )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList found = NULL;
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
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
  
  if (found == NULL || paramIndex >= found->numParams) return Void;
  else return found->paramTypes[paramIndex];
}

// verifica se simbolo eh array
int st_lookup_is_array ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->isArray;
      }
      temp = temp->next;
  }
  
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->isArray;
      }
      temp = temp->next;
  }
  
  return 0;
}

// imprime tabela formatada
void printSymTab(FILE * listing)
{ 
  int i;
  
  fprintf(listing,"%-15s %-15s %-15s %-15s %-15s %-15s\n", 
          "Hash", "Name", "Type", "Kind", "Scope", "Line Numbers");
  fprintf(listing,"%-15s %-15s %-15s %-15s %-15s %-15s\n", 
          "----", "----", "----", "----", "-----", "------------");
  
  for (i=0;i<SIZE;++i)
  { 
    if (hashTable[i] != NULL)
    { 
      BucketList l = hashTable[i];
      
      while (l != NULL)
      { 
        LineList t = l->lines;
        
        fprintf(listing,"%-15d ",i);
        fprintf(listing,"%-15s ",l->name);
        
        switch(l->type) {
            case Integer: fprintf(listing, "%-15s ", "Integer"); break;
            case Void:    fprintf(listing, "%-15s ", "Void"); break;
            case Boolean: fprintf(listing, "%-15s ", "Boolean"); break;
            default:      fprintf(listing, "%-15s ", "Unknown"); break;
        }
        
        switch(l->kind) {
            case ID_VAR: fprintf(listing, "%-15s ", "Var"); break;
            case ID_FUN: fprintf(listing, "%-15s ", "Func"); break;
            default:     fprintf(listing, "%-15s ", "Unknown"); break;
        }
        
        fprintf(listing, "%-15s ", l->scopeName);
        
        while (t != NULL)
        { 
          fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        
        l = l->next;
      }
    }
  }
}
