#ifndef NITRO_SEMANTIC_
#define NITRO_SEMANTIC_

#include "parser.h"

typedef struct {
  char* key;
  ast_funcdecl* value;
} FunctionDecls;

typedef struct {
  char* key;
  ast_variablebind* value;
} VariableBinds;

typedef struct {
  ast_axiom* ast;
  FunctionDecls* funcdecls;
  VariableBinds* variablebinds;
} Semantic;

Semantic semantic_create(ast_axiom* ast);
void semantic_add_funcdecl(Semantic* s, ast_funcdecl* funcdecl);
ast_funcdecl* semantic_get_funcdecl(Semantic* s, const char* funcname);
void semantic_add_variablebind(Semantic* s, ast_variablebind* variablebind);
ast_variablebind* semantic_get_variablebind(Semantic* s, const char* variablebindname);
void semantic_check_includes(Semantic* s, ast_includes* includes);
void semantic_check_block(Semantic* s, ast_block* block);
void semantic_check_funcdecl(Semantic* s, ast_funcdecl* funcdecl);
void semantic_check(Semantic* s);

#endif // NITRO_SEMANTIC_
