#ifndef NITRO_GENERATOR_
#define NITRO_GENERATOR_

#include "parser.h"
#include <stdint.h>

#define MAX_NITRO_FILE_SIZE 1024 * 1024

typedef struct {
  ast_axiom* ast;
  char asm_buffr[MAX_NITRO_FILE_SIZE];
  uint32_t cur;
  char strings[1024];
  uint32_t strings_cur;
} Generator;

Generator generator_create(ast_axiom* ast);
void generator_gen_block(Generator* g, ast_block* block);
void generator_gen_funcdecl(Generator* g, ast_funcdecl* funcdecl);
void generator_gen_top_level(Generator* g);
#endif // NITRO_GENERATOR_
