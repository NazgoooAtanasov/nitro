#include "generator.h"
#include <stdio.h>
#include <string.h>

Generator generator_create(ast_axiom* ast) {
  Generator gen = {0};
  gen.ast = ast;
  gen.cur = 0;
  memset(gen.asm_buffr, '\0', MAX_NITRO_FILE_SIZE);
  memset(gen.strings, '\0', 1024);
  return gen;
}
static inline void generator_add_asm(Generator* g, const char* asm_text) {
  sprintf(&g->asm_buffr[g->cur], "%s", asm_text);
  g->cur += strlen(asm_text);
}

static inline void generator_add_string_asm(Generator* g, const char* asm_text) {
  sprintf(&g->strings[g->strings_cur], "%s", asm_text);
  g->strings_cur += strlen(asm_text);
}

void generator_gen_block(Generator* g, ast_block* block) {
  ast_stmt* stmt = block->stmts;

  generator_add_asm(g, "\tpush rbp\n");
  generator_add_asm(g, "\tmov rbp, rsp\n");

  // finding the amount of bytes for local variables
  uint32_t varaibles_size = 0;
  while (stmt != NULL) {
    if (stmt->variablebind) {
      if (stmt->variablebind->type == AST_VARIABLE_TYPE_I32) {
        varaibles_size += 4;
      }
    }
    stmt = stmt->next;
  }

  char integer_buffr[1024] = {0};
  sprintf(integer_buffr, "%d", varaibles_size);
  generator_add_asm(g, "\tsub rsp, ");
  generator_add_asm(g, integer_buffr);
  generator_add_asm(g, "\n");

  stmt = block->stmts;
  uint32_t variablebind_generated = 1;
  while(stmt != NULL) {
    if (stmt->variablebind) {
      if (stmt->variablebind->type == AST_VARIABLE_TYPE_I32) {
        generator_add_asm(g, "\tmov dword [rbp-");
        memset(integer_buffr, '\0', 1024);
        sprintf(integer_buffr, "%d", variablebind_generated * 4);
        generator_add_asm(g, integer_buffr);
        generator_add_asm(g, "], ");

        generator_add_asm(g, stmt->variablebind->value);
        generator_add_asm(g, "\n");
      } else if (stmt->variablebind->type == AST_VARIABLE_TYPE_STRING) {
        generator_add_string_asm(g, "\t_nitro_str_");
        generator_add_string_asm(g, stmt->variablebind->ident);
        generator_add_string_asm(g, " db ");
        generator_add_string_asm(g, stmt->variablebind->value);
        generator_add_string_asm(g, ",0\n");

        generator_add_string_asm(g, "\t_nitro_str_");
        generator_add_string_asm(g, stmt->variablebind->ident);
        generator_add_string_asm(g, "_len equ $ -");
        generator_add_string_asm(g, "_nitro_str_");
        generator_add_string_asm(g, stmt->variablebind->ident);
      }

      variablebind_generated++;
    } else if (stmt->funccall) {
      if (strcmp(stmt->funccall->funcname, "SYS_CALL") == 0) {
        ast_callarguments* arg = stmt->funccall->call_arguments;
        uint32_t argidx = 0;
        while (arg != NULL) {
          if (argidx == 0) {
            generator_add_asm(g, "\tmov rax, ");
          } else if (argidx == 1) {
            generator_add_asm(g, "\tmov rdi, ");
          }

          generator_add_asm(g, arg->value);
          generator_add_asm(g, "\n");

          argidx++;
          arg = arg->next;
        }
        generator_add_asm(g, "\tsyscall\n");
      } else {
        ast_callarguments* arg = stmt->funccall->call_arguments;
        while (arg->next != NULL) {
          arg = arg->next;
        }
        
        while(arg != NULL) {
          generator_add_asm(g, "\tpush ");
          generator_add_asm(g, arg->value);
          generator_add_asm(g, "\n");
          arg = arg->prev;
        }

        generator_add_asm(g, "\tcall _nitro_");
        generator_add_asm(g, stmt->funccall->funcname);
        generator_add_asm(g, "\n");
      }
    }

    stmt = stmt->next;
  }

  generator_add_asm(g, "\tmov rsp, rbp\n");
  generator_add_asm(g, "\tpop rbp\n");
  generator_add_asm(g, "\tret\n");
}
void generator_gen_funcdecl(Generator* g, ast_funcdecl* funcdecl) {
  generator_add_asm(g, "_nitro_");
  generator_add_asm(g, funcdecl->func_name);
  generator_add_asm(g, ":\n");

  generator_gen_block(g, funcdecl->block);
}
void generator_gen_top_level(Generator* g) {
  generator_add_asm(g, "BITS 64\n");
  generator_add_asm(g, "global _start\n");

  if (g->ast->includes) {
    // handle includes
  }

  generator_gen_funcdecl(g, g->ast->funcdecl);
  ast_funcdecl* level = g->ast->funcdecl->next;
  while(level != NULL) {
    generator_gen_funcdecl(g, level);
    level = level->next;
  }

  generator_add_asm(g, "\n");
  generator_add_asm(g, "_start:\n");
  generator_add_asm(g, "\tcall _nitro_main\n");
  generator_add_asm(g, "\tmov rax, 60\n");
  generator_add_asm(g, "\tmov rdi, 0\n");
  generator_add_asm(g, "\tsyscall\n");

  generator_add_asm(g, "\nsection .data\n");
  generator_add_asm(g, g->strings);
}
