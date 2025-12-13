/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* MAX_SCOPE_DEPTH is the maximum nesting of scopes */
#define MAX_SCOPE_DEPTH 100

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     LineList lines;
     int memloc ; /* memory location for variable */
     ExpType type; /* data type for variable */
     IdKind kind; /* kind of identifier */
     int numParams; /* number of parameters (for functions) */
     ExpType paramTypes[MAX_PARAMS]; /* types of parameters */
     char * scopeName; /* scope name of this variable */
     struct BucketListRec * next;
   } * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Scope Stack for Names */
static char * scopeNameStack[MAX_SCOPE_DEPTH];
static int scopeStackTop = 0;

/* Initialize with Global scope */
static void initScopeStack() {
    if (scopeStackTop == 0) {
        scopeNameStack[scopeStackTop++] = "Global";
    }
}

void st_enter_scope(char * scopeName) {
    initScopeStack();
    if (scopeStackTop >= MAX_SCOPE_DEPTH) {
        fprintf(stderr, "Error: Max scope depth exceeded\n");
        exit(1);
    }
    scopeNameStack[scopeStackTop++] = scopeName;
}

void st_exit_scope(void) {
    if (scopeStackTop > 1) { /* Don't pop Global */
        scopeStackTop--;
    }
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int numParams, ExpType *paramTypes )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Check if variable is already in the CURRENT scope */
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
    
  if (l == NULL) /* variable not yet in table for this scope */
  { 
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->type = type;
    l->kind = kind;
    l->numParams = numParams;
    if (paramTypes != NULL) {
        for (int i = 0; i < numParams && i < MAX_PARAMS; i++) {
            l->paramTypes[i] = paramTypes[i];
        }
    }
    l->scopeName = currentScopeName;
    l->lines->next = NULL;
    l->next = hashTable[h]; /* Add to head */
    hashTable[h] = l; 
  }
  else /* found in table in current scope, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Procedure st_add_ref adds a line number to an existing symbol
 * searching all scopes.
 */
void st_add_ref( char * name, int lineno )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Search priority: Current Scope -> Global Scope */
  BucketList found = NULL;
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope if not found */
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

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Search priority: Current Scope -> Global Scope */
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->memloc;
      }
      temp = temp->next;
  }
  
  return -1;
}

/* Function st_lookup_top returns the memory
 * location of a variable in the current scope
 * or -1 if not found.
 */
int st_lookup_top ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(l->scopeName, currentScopeName) != 0)))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

/* Function st_lookup_type returns the type 
 * of a variable or Void if not found
 */
ExpType st_lookup_type ( char * name )
{ 
  int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Search priority: Current Scope -> Global Scope */
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->type;
      }
      temp = temp->next;
  }
  
  return Void;
}

IdKind st_lookup_kind ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Search priority: Current Scope -> Global Scope */
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->kind;
      }
      temp = temp->next;
  }
  
  return ID_VAR; /* Default */
}

int st_lookup_num_params ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  /* Search priority: Current Scope -> Global Scope */
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope */
  temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, "Global") == 0) {
          return temp->numParams;
      }
      temp = temp->next;
  }
  
  return 0;
}

ExpType st_lookup_param_type ( char * name, int paramIndex )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  initScopeStack();
  char * currentScopeName = scopeNameStack[scopeStackTop-1];
  
  BucketList found = NULL;
  
  /* First pass: Check current scope */
  BucketList temp = l;
  while (temp != NULL) {
      if (strcmp(name, temp->name) == 0 && strcmp(temp->scopeName, currentScopeName) == 0) {
          found = temp;
          break;
      }
      temp = temp->next;
  }
  
  /* Second pass: Check Global scope */
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

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"%-10s %-10s %-10s %-10s %-10s %-10s\n", "Hash", "Name", "Type", "Kind", "Scope", "Line Numbers");
  fprintf(listing,"%-10s %-10s %-10s %-10s %-10s %-10s\n", "----", "----", "----", "----", "-----", "------------");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-10d ",i); /* Print Hash (Bucket Index) */
        fprintf(listing,"%-10s ",l->name);
        
        switch(l->type) {
            case Integer: fprintf(listing, "%-10s ", "Integer"); break;
            case Void:    fprintf(listing, "%-10s ", "Void"); break;
            case Boolean: fprintf(listing, "%-10s ", "Boolean"); break;
            default:      fprintf(listing, "%-10s ", "Unknown"); break;
        }
        
        switch(l->kind) {
            case ID_VAR: fprintf(listing, "%-10s ", "Var"); break;
            case ID_FUN: fprintf(listing, "%-10s ", "Func"); break;
            default:     fprintf(listing, "%-10s ", "Unknown"); break;
        }
        
        fprintf(listing, "%-10s ", l->scopeName);
        
        while (t != NULL)
        { fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
} /* printSymTab */
