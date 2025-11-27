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

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( ASTNode * t,
               void (* preProc) (ASTNode *),
               void (* postProc) (ASTNode *) )
{ if (t != NULL)
  { preProc(t);
    
    /* For declaration nodes, we manually handle the children in preProc (insertNode)
       and we don't want to visit the child ID node again as it would count as a reference.
       So we skip children traversal for these specific nodes. */
    int skipChildren = 0;
    if (t->type == NODE_VAR_DECL || t->type == NODE_FUN_DECL || t->type == NODE_PARAM) {
        skipChildren = 1;
    }

    if (!skipChildren) {
        traverse(t->leftChild,preProc,postProc);
        traverse(t->rightChild,preProc,postProc);
    }
    
    postProc(t);
    traverse(t->next,preProc,postProc);
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
        }
        
        if (name != NULL) {
            ExpType type = getExpType(t->leftChild);
            if (st_lookup(name) == -1)
                st_insert(name, t->lineno, location++, type);
            else
                st_insert(name, t->lineno, 0, type); /* Already in table, add ref */
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
            if (st_lookup(name) == -1)
                st_insert(name, t->lineno, location++, type);
            else
                st_insert(name, t->lineno, 0, type);
        }
      }
      break;

    case NODE_PARAM:
      {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        }
        
        if (name != NULL) {
            ExpType type = getExpType(t->leftChild);
            if (st_lookup(name) == -1)
                st_insert(name, t->lineno, location++, type);
            else
                st_insert(name, t->lineno, 0, type);
        }
      }
      break;

    case NODE_VAR:
      {
        if (st_lookup(t->identifier) != -1) {
            st_insert(t->identifier, t->lineno, 0, Void); // Type ignored for reference
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
  traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
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
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(ASTNode * syntaxTree)
{ 
  traverse(syntaxTree,nullProc,checkNode);
}
