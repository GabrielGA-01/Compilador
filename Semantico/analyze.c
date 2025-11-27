/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

/* #include "globals.h" */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "analyze.h"
#include "ast.h"
#include "parser.tab.h"

/* Global variables that were likely in globals.h */
FILE * listing = NULL;
int Error = 0;
int TraceAnalyze = 1; /* Enable trace by default for now */

/* counter for variable memory locations */
static int location = 0;

static void typeError(ASTNode * t, char * message);
static void checkNode(ASTNode * t);

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( ASTNode * t,
               void (* preProc) (ASTNode *),
               void (* postProc) (ASTNode *) )
{ if (t != NULL)
  { 
    int enteredScope = 0;
    
    preProc(t);
    
    /* Handle Scope Entry */
    if (t->type == NODE_FUN_DECL) {
        st_enter_scope();
        enteredScope = 1;
    }
    
    /* Handle Children */
    int skipChildren = 0;
    if (t->type == NODE_VAR_DECL || t->type == NODE_FUN_DECL || t->type == NODE_PARAM) {
        skipChildren = 1;
    }

    if (!skipChildren) {
        if (t->type == NODE_COMPOUND_STMT) st_enter_scope();
        
        traverse(t->leftChild,preProc,postProc);
        traverse(t->rightChild,preProc,postProc);
        
        if (t->type == NODE_COMPOUND_STMT) st_exit_scope();
    }
    
    postProc(t);
    
    /* Handle Next */
    if (t->type == NODE_FUN_DECL && enteredScope) {
        /* Special handling for FUN_DECL to visit body inside scope */
        if (t->next != NULL && t->next->type == NODE_FUN_BODY) {
            ASTNode *body = t->next;
            
            /* Manually traverse body to control scope */
            preProc(body);
            
            /* Body Children (Params, Block) */
            traverse(body->leftChild, preProc, postProc);
            traverse(body->rightChild, preProc, postProc);
            
            postProc(body);
            
            /* Exit function scope */
            st_exit_scope();
            
            /* Now visit Next Function (body->next) */
            traverse(body->next, preProc, postProc);
        } else {
            /* No body? Just exit scope and continue */
            st_exit_scope();
            traverse(t->next, preProc, postProc);
        }
    } else {
        /* Generic traversal for next sibling */
        traverse(t->next,preProc,postProc);
    }
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(ASTNode * t)
{ if (t==NULL) return;
  else return;
}

static ExpType getExpType(ASTNode* typeNode) {
    if (typeNode == NULL) return Void;
    if (typeNode->number == INT) return Integer;
    if (typeNode->number == VOID) return Void;
    return Void;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( ASTNode * t)
{ 
  switch (t->type)
  { case NODE_VAR_DECL:
      {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        } else if (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) {
             /* Handle array declaration */
             if (t->rightChild->leftChild != NULL)
                name = t->rightChild->leftChild->identifier;
        }
        
        if (name != NULL) {
            ExpType type = getExpType(t->leftChild);
            
            /* Case 3: Error if variable declared as Void */
            if (type == Void) {
                typeError(t, "Variable declared as Void");
            }
            
            /* Case 4: Error if already declared in CURRENT scope */
            if (st_lookup_top(name) != -1) {
                typeError(t, "Variable already declared");
            } else {
                /* Case 7: Check if name is already a FUNCTION (Shadowing check) */
                if (strcmp(name, "xyz") == 0) {
                    printf("DEBUG: Checking shadowing for xyz. Lookup: %d, Kind: %d\n", st_lookup(name), st_lookup_kind(name));
                }
                if (st_lookup(name) != -1 && st_lookup_kind(name) == ID_FUN) {
                     typeError(t, "Variable name shadows a function");
                }
                
                st_insert(name, t->lineno, location++, type, ID_VAR, 0, NULL);
            }
        }
      }
      break;
      
    case NODE_FUN_DECL:
      {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        }
        
        if (name != NULL) {
             ExpType type = getExpType(t->leftChild);
             if (st_lookup_top(name) != -1) {
                  typeError(t, "Function already declared");
             } else {
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
                 
                 st_insert(name, t->lineno, location++, type, ID_FUN, numParams, paramTypes);
             }
         }
      }
      break;

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
             if (type == Void) {
                typeError(t, "Parameter declared as Void");
            }
            
            if (st_lookup_top(name) != -1) {
                 typeError(t, "Parameter already declared");
            } else {
                st_insert(name, t->lineno, location++, type, ID_VAR, 0, NULL);
            }
        }
      }
      break;

    case NODE_VAR:
      {
        /* Case 1: Error if variable not declared */
        if (st_lookup(t->identifier) == -1) {
            typeError(t, "Variable not declared");
        } else {
            st_add_ref(t->identifier, t->lineno); // Add reference
        }
      }
      break;
      
    case NODE_FUN_CALL:
      {
          /* Case 5: Error if function not declared */
          if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
              if (st_lookup(t->leftChild->identifier) == -1) {
                  typeError(t, "Function not declared");
              } else {
                  st_add_ref(t->leftChild->identifier, t->lineno);
                  
                  /* Case 8 & 9: Check Parameters */
                  int expectedParams = st_lookup_num_params(t->leftChild->identifier);
                  int actualParams = 0;
                  ASTNode *arg = t->rightChild; /* args */
                  
                  /* Check param count first */
                  ASTNode *tempArg = arg;
                  while (tempArg != NULL) {
                      actualParams++;
                      tempArg = tempArg->next;
                  }
                  
                  printf("DEBUG: Calling function '%s'. Expected: %d, Actual: %d\n", t->leftChild->identifier, expectedParams, actualParams);
                  
                  if (actualParams != expectedParams) {
                      typeError(t, "Invalid number of parameters");
                  } else {
                      /* Check param types */
                      /* Note: We can only check types if we have computed expression types.
                         But insertNode happens BEFORE typeCheck.
                         So we can't check types here!
                         We must check types in checkNode (post-order).
                      */
                  }
              }
          }
      }
      break;

    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(ASTNode * syntaxTree)
{ 
  if (listing == NULL) listing = stdout;
  /* Combine insertion and type checking in one pass */
  traverse(syntaxTree,insertNode,checkNode);
  
  /* Case 6: Check if main is declared */
  if (st_lookup("main") == -1) {
      fprintf(listing, "Type error: Function main not declared\n");
      Error = 1;
  }
  
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

/* Procedure typeError reports a type error */
static void typeError(ASTNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = 1;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(ASTNode * t)
{ switch (t->type)
  { case NODE_BINARY_OP:
    case NODE_ASSIGN_EXPR:
      if (t->type == NODE_ASSIGN_EXPR) {
          /* Case 2: Check for Void assignment */
          if (t->rightChild->expType == Void) {
              typeError(t, "Invalid assignment (Void value)");
          }
      }
      
      if (t->leftChild->expType != Integer || t->rightChild->expType != Integer)
        typeError(t,"Op applied to non-integer");
      if ((t->type == NODE_BINARY_OP) && (t->number == EQ || t->number == DIF || t->number == LT || t->number == GT || t->number == LET || t->number == GET))
        t->expType = Boolean;
      else
        t->expType = Integer;
      break;
    case NODE_NUM:
      t->expType = Integer;
      break;
    case NODE_VAR:
      t->expType = st_lookup_type(t->identifier);
      break;
    case NODE_FUN_CALL:
      if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
          t->expType = st_lookup_type(t->leftChild->identifier);
          
          /* Case 9: Check Parameter Types */
          int expectedParams = st_lookup_num_params(t->leftChild->identifier);
          ASTNode *arg = t->rightChild;
          int i = 0;
          while (arg != NULL && i < expectedParams) {
              ExpType expectedType = st_lookup_param_type(t->leftChild->identifier, i);
              if (arg->expType != expectedType) {
                  typeError(t, "Invalid parameter type");
              }
              arg = arg->next;
              i++;
          }
      }
      break;
    default:
      break;
  }
}

static void checkUnusedReturn(ASTNode *t) {
    if (t == NULL) return;
    
    if (t->type == NODE_COMPOUND_STMT) {
        /* Iterate statements */
        ASTNode *stmt = t->rightChild; /* statement_lista */
        while (stmt != NULL) {
            /* If statement is a FUN_CALL directly */
            if (stmt->type == NODE_FUN_CALL) {
                if (stmt->expType != Void) {
                    typeError(stmt, "Function return value not used");
                }
            }
            /* Recurse into statement */
            checkUnusedReturn(stmt);
            stmt = stmt->next;
        }
    } else {
        /* Recurse children */
        checkUnusedReturn(t->leftChild);
        checkUnusedReturn(t->rightChild);
        checkUnusedReturn(t->next);
    }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(ASTNode * syntaxTree)
{ 
  /* traverse(syntaxTree,nullProc,checkNode); */
  /* Already done in buildSymtab */
  checkUnusedReturn(syntaxTree);
}
