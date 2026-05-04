#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <stdio.h>
#include "ast.h"

typedef enum { 
    ID_VAR,
    ID_FUN
} IdKind;

#define MAX_PARAMS 10

void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int isArray, int numParams, ExpType *paramTypes );
int st_lookup ( char * name );
ExpType st_lookup_type ( char * name );
IdKind st_lookup_kind ( char * name );
int st_lookup_num_params ( char * name );
ExpType st_lookup_param_type ( char * name, int paramIndex );
int st_lookup_is_array ( char * name );
int st_lookup_is_array_scope ( char * name, char * scopeName );
void st_add_ref( char * name, int lineno );
void st_enter_scope(char * scopeName);
void st_exit_scope(void);
int st_lookup_top ( char * name );
void printSymTab(FILE * listing);

#endif
