#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "analyze.h"
#include "ast.h"
#include "parser.tab.h"

FILE * listing = NULL;
int Error = 0;
int TraceAnalyze = 1;

static int location = 0;

static char * currentFuncName = "Global";

static void typeError(ASTNode * t, char * message);
static void checkNode(ASTNode * t);

static void traverse( ASTNode * t,
               void (* preProc) (ASTNode *),
               void (* postProc) (ASTNode *) )
{ 
  if (t != NULL)
  { 
    int enteredScope = 0;
    
    preProc(t);
    
    if (t->type == NODE_FUN_DECL) {
        char *name = NULL;
        if (t->rightChild != NULL && t->rightChild->type == NODE_VAR) {
            name = t->rightChild->identifier;
        }
        if (name != NULL) currentFuncName = name;
        
        st_enter_scope(currentFuncName);
        enteredScope = 1;
    }
    
    int skipChildren = 0;
    if (t->type == NODE_VAR_DECL || t->type == NODE_FUN_DECL || 
        t->type == NODE_PARAM || t->type == NODE_FUN_CALL) {
        skipChildren = 1;
    }

    if (!skipChildren) {
        if (t->type == NODE_COMPOUND_STMT) st_enter_scope(currentFuncName);
        
        traverse(t->leftChild, preProc, postProc);
        traverse(t->rightChild, preProc, postProc);
        
        if (t->type == NODE_COMPOUND_STMT) st_exit_scope();
    }
    
    if (t->type == NODE_FUN_CALL) {
        traverse(t->rightChild, preProc, postProc);
    }
    
    postProc(t);
    
    if (t->type == NODE_FUN_DECL && enteredScope) {
        if (t->next != NULL && t->next->type == NODE_FUN_BODY) {
            ASTNode *body = t->next;
            
            preProc(body);
            
            traverse(body->leftChild, preProc, postProc);
            traverse(body->rightChild, preProc, postProc);
            
            postProc(body);
            
            st_exit_scope();
            currentFuncName = "Global";
            
            traverse(body->next, preProc, postProc);
        } else {
            st_exit_scope();
            currentFuncName = "Global";
            traverse(t->next, preProc, postProc);
        }
    } else {
        traverse(t->next, preProc, postProc);
    }
  }
}

static void nullProc(ASTNode * t)
{ 
  if (t==NULL) return;
  else return;
}

static ExpType getExpType(ASTNode* typeNode) {
    if (typeNode == NULL) return Void;
    if (typeNode->number == INT) return Integer;
    if (typeNode->number == VOID) return Void;
    return Void;
}

static void insertNode( ASTNode * t)
{ 
  switch (t->type)
  { 
    case NODE_VAR_DECL:
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
                char buf[256];
                sprintf(buf, "Variable '%s' declared as Void", name);
                typeError(t, buf);
            }
            
            if (st_lookup_top(name) != -1) {
                char buf[256];
                sprintf(buf, "Variable '%s' already declared", name);
                typeError(t, buf);
            } else {
                if (st_lookup(name) != -1 && st_lookup_kind(name) == ID_FUN) {
                     char buf[256];
                     sprintf(buf, "Variable name '%s' shadows a function", name);
                     typeError(t, buf);
                }
                
                int isArray = (t->rightChild != NULL && t->rightChild->type == NODE_ARRAY_DECL) ? 1 : 0;
                st_insert(name, t->lineno, location++, type, ID_VAR, isArray, 0, NULL);
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
                  char buf[256];
                  sprintf(buf, "Function '%s' already declared", name);
                  typeError(t, buf);
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
                 
                 st_insert(name, t->lineno, location++, type, ID_FUN, 0, numParams, paramTypes);
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
                char buf[256];
                sprintf(buf, "Parameter '%s' declared as Void", name);
                typeError(t, buf);
            }
            
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

    case NODE_VAR:
      {
        if (st_lookup(t->identifier) == -1) {
            char buf[256];
            sprintf(buf, "Variable '%s' not declared", t->identifier);
            typeError(t, buf);
        } else {
            st_add_ref(t->identifier, t->lineno);
        }
      }
      break;
      
    case NODE_FUN_CALL:
      {
          if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
              if (st_lookup(t->leftChild->identifier) == -1) {
                  char buf[256];
                  sprintf(buf, "Function '%s' not declared", t->leftChild->identifier);
                  typeError(t, buf);
              } else {
                  st_add_ref(t->leftChild->identifier, t->lineno);
                  
                  int expectedParams = st_lookup_num_params(t->leftChild->identifier);
                  int actualParams = 0;
                  ASTNode *arg = t->rightChild;
                  
                  ASTNode *tempArg = arg;
                  while (tempArg != NULL) {
                      actualParams++;
                      tempArg = tempArg->next;
                  }
                  
                  if (actualParams != expectedParams) {
                      char buf[256];
                      sprintf(buf, "Invalid number of parameters for function '%s'", t->leftChild->identifier);
                      typeError(t, buf);
                  }
              }
          }
      }
      break;

    default:
      break;
  }
}

void buildSymtab(ASTNode * syntaxTree)
{ 
  if (listing == NULL) listing = stdout;
  
  ExpType outputParamTypes[1] = { Integer };
  st_insert("input", 0, location++, Integer, ID_FUN, 0, 0, NULL);
  st_insert("output", 0, location++, Void, ID_FUN, 0, 1, outputParamTypes);
  
  traverse(syntaxTree, insertNode, checkNode);
  
  if (st_lookup("main") == -1) {
      fprintf(listing, "[Semantic Error] Function main not declared\n");
      Error = 1;
  }
  
  if (TraceAnalyze)
  { 
  }
}

static void typeError(ASTNode * t, char * message)
{ 
  fprintf(listing, "[Semantic Error] Line %d: %s\n", t->lineno, message);
  Error = 1;
}

static void checkNode(ASTNode * t)
{ 
  switch (t->type)
  { 
    case NODE_BINARY_OP:
    case NODE_ASSIGN_EXPR:
      if (t->type == NODE_ASSIGN_EXPR) {
          if (t->leftChild->type == NODE_VAR) {
              if (st_lookup_is_array(t->leftChild->identifier)) {
                  char buf[256];
                  sprintf(buf, "Cannot assign to array '%s' without index", t->leftChild->identifier);
                  typeError(t, buf);
              }
          }
          
          if (t->rightChild->type == NODE_VAR) {
              if (st_lookup_is_array(t->rightChild->identifier)) {
                  char buf[256];
                  sprintf(buf, "Cannot use array '%s' without index", t->rightChild->identifier);
                  typeError(t, buf);
              }
          }
          
          int skipVoidError = 0;
          if (t->rightChild->type == NODE_FUN_CALL && 
              t->rightChild->leftChild != NULL &&
              t->rightChild->leftChild->type == NODE_VAR) {
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
          if (t->leftChild->expType != Integer || t->rightChild->expType != Integer)
            typeError(t,"Op applied to non-integer");
      }
      
      if ((t->type == NODE_BINARY_OP) && (t->number == EQ || t->number == DIF || 
          t->number == LT || t->number == GT || t->number == LET || t->number == GET))
        t->expType = Boolean;
      else
        t->expType = Integer;
      break;
      
    case NODE_NUM:
      t->expType = Integer;
      break;
      
    case NODE_VAR:
      if (st_lookup(t->identifier) == -1)
          t->expType = Integer;
      else
          t->expType = st_lookup_type(t->identifier);
      break;
      
    case NODE_FUN_CALL:
      if (t->leftChild != NULL && t->leftChild->type == NODE_VAR) {
          t->expType = st_lookup_type(t->leftChild->identifier);
          
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
      
    case NODE_ARRAY_ACCESS:
      t->expType = Integer;
      break;
      
    case NODE_RETURN_STMT:
      {
          ExpType funcType = st_lookup_type(currentFuncName);
          
          if (funcType == Void) {
              if (t->leftChild != NULL) {
                  typeError(t, "Void function cannot return a value");
              }
          } else {
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

static void checkUnusedReturn(ASTNode *t) {
    if (t == NULL) return;
    
    if (t->type == NODE_COMPOUND_STMT) {
        ASTNode *stmt = t->rightChild;
        while (stmt != NULL) {
            if (stmt->type == NODE_FUN_CALL) {
                if (stmt->expType != Void) {
                    typeError(stmt, "Function return value not used");
                }
            }
            checkUnusedReturn(stmt);
            stmt = stmt->next;
        }
    } else {
        checkUnusedReturn(t->leftChild);
        checkUnusedReturn(t->rightChild);
        checkUnusedReturn(t->next);
    }
}

void typeCheck(ASTNode * syntaxTree)
{ 
  checkUnusedReturn(syntaxTree);
}
