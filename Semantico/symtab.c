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
     int scope; /* scope depth of this variable */
     struct BucketListRec * next;
   } * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Current scope depth */
static int currentScope = 0;

/* List of symbols added in each scope, to allow easy removal */
typedef struct ScopeListRec {
    char *name;
    struct ScopeListRec *next;
} *ScopeList;

static ScopeList scopeStack[MAX_SCOPE_DEPTH];

void st_enter_scope(void) {
    currentScope++;
    if (currentScope >= MAX_SCOPE_DEPTH) {
        fprintf(stderr, "Error: Max scope depth exceeded\n");
        exit(1);
    }
    scopeStack[currentScope] = NULL;
}

void st_exit_scope(void) {
    ScopeList s = scopeStack[currentScope];
    while (s != NULL) {
        /* Remove from hash table */
        int h = hash(s->name);
        BucketList l = hashTable[h];
        BucketList prev = NULL;
        
        while (l != NULL) {
            if (strcmp(l->name, s->name) == 0 && l->scope == currentScope) {
                break;
            }
            prev = l;
            l = l->next;
        }
        
        if (l != NULL) {
            if (prev == NULL) {
                hashTable[h] = l->next;
            } else {
                prev->next = l->next;
            }
        }
        
        s = s->next;
    }
    currentScope--;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int numParams, ExpType *paramTypes )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  
  /* Check if variable is already in the CURRENT scope */
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (l->scope != currentScope)))
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
    l->scope = currentScope;
    l->lines->next = NULL;
    l->next = hashTable[h]; /* Add to head */
    hashTable[h] = l; 
    
    /* Add to scope stack for cleanup */
    ScopeList s = (ScopeList) malloc(sizeof(struct ScopeListRec));
    s->name = name;
    s->next = scopeStack[currentScope];
    scopeStack[currentScope] = s;
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
  /* Search for the first match (most recent scope) */
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
    
  if (l != NULL) {
    LineList t = l->lines;
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
  /* Search for the first match (most recent scope because we insert at head) */
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

/* Function st_lookup_top returns the memory
 * location of a variable in the current scope
 * or -1 if not found.
 */
int st_lookup_top ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (l->scope != currentScope)))
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
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return Void;
  else return l->type;
}

IdKind st_lookup_kind ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return ID_VAR; /* Default */
  else return l->kind;
}

int st_lookup_num_params ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return 0;
  else return l->numParams;
}

ExpType st_lookup_param_type ( char * name, int paramIndex )
{ int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL || paramIndex >= l->numParams) return Void;
  else return l->paramTypes[paramIndex];
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Variable Name  Location   Type       Kind       Scope      Line Numbers\n");
  fprintf(listing,"-------------  --------   ----       ----       -----      ------------\n");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-8d  ",l->memloc);
        
        switch(l->type) {
            case Integer: fprintf(listing, "Integer    "); break;
            case Void:    fprintf(listing, "Void       "); break;
            case Boolean: fprintf(listing, "Boolean    "); break;
            default:      fprintf(listing, "Unknown    "); break;
        }
        
        switch(l->kind) {
            case ID_VAR: fprintf(listing, "Var        "); break;
            case ID_FUN: fprintf(listing, "Func       "); break;
            default:     fprintf(listing, "Unknown    "); break;
        }
        
        fprintf(listing, "%-10d ", l->scope);
        
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
