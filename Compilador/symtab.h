/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <stdio.h>
#include "ast.h"

/* Kind of identifier */
typedef enum { ID_VAR, ID_FUN } IdKind;

/* Maximum number of parameters */
#define MAX_PARAMS 10

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, ExpType type, IdKind kind, int numParams, ExpType *paramTypes );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name );

/* Function st_lookup_type returns the type 
 * of a variable or Void if not found
 */
ExpType st_lookup_type ( char * name );

/* Function st_lookup_kind returns the kind 
 * of a symbol (ID_VAR or ID_FUN)
 */
IdKind st_lookup_kind ( char * name );

/* Function st_lookup_num_params returns the number of parameters
 * of a function symbol
 */
int st_lookup_num_params ( char * name );

/* Function st_lookup_param_type returns the type of the ith parameter
 * of a function symbol
 */
ExpType st_lookup_param_type ( char * name, int paramIndex );

/* Procedure st_add_ref adds a line number to an existing symbol
 * searching all scopes.
 */
void st_add_ref( char * name, int lineno );

/* Scope management functions */
void st_enter_scope(char * scopeName);
void st_exit_scope(void);

/* Function st_lookup_top returns the memory
 * location of a variable in the current scope
 * or -1 if not found. Used for redeclaration checks.
 */
int st_lookup_top ( char * name );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
