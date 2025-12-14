#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "ast.h"

void buildSymtab(ASTNode *);

void typeCheck(ASTNode *);

#endif
