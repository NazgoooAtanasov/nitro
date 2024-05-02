#ifndef NITRO_SEMANTIC_
#define NITRO_SEMANTIC_

#include "parser.h"

typedef struct {
  ast_axiom* ast;
} Semantic;

Semantic semantic_create(ast_axiom* ast);
void semantic_check_includes(Semantic* s, ast_includes* includes);
void semantic_check_block(Semantic* s, ast_block* block);
void semantic_check_funcdecl(Semantic* s, ast_funcdecl* funcdecl);
void semantic_check(Semantic* s);

#endif // NITRO_SEMANTIC_
