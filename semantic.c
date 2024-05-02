#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Semantic semantic_create(ast_axiom* ast) {
  Semantic semantic = {0};
  semantic.ast = ast;
  return semantic;
}

void semantic_check_includes(Semantic* s, ast_includes* includes) {
  ast_includes* include = includes;

  while (includes != NULL) {
    if (includes->file_path[0] != '"') {
      fprintf(stderr, "%s:%d:%d [Semantic error]: 'include' expects a string value but got '%s'\n\tTry '\"%s\"'\n",
              includes->pos.file_path,
              includes->pos.row,
              includes->pos.col,
              includes->file_path,
              includes->file_path);
      exit(1);
    }

    includes = includes->next;
  }
}

void semantic_check_block(Semantic* s, ast_block* block) {
  ast_stmt* stmt = block->stmts;
  while(stmt != NULL) {
    if (stmt->variablebind != NULL) {
      if (stmt->variablebind->type == AST_VARIABLE_TYPE_VOID) {
        fprintf(stderr, "%s:%d:%d [Semantic error]: 'void' cannot be used as a variable type.\n",
                stmt->variablebind->pos.file_path,
                stmt->variablebind->pos.row,
                stmt->variablebind->pos.col);
        exit(1);
      }

      if (stmt->variablebind->type == AST_VARIABLE_TYPE_I32 && stmt->variablebind->value[0] == '"') {
        fprintf(stderr, "%s:%d:%d [Semantic error]: variable of type '%s' cannot be assigned to '%s'.\n",
                stmt->variablebind->pos.file_path,
                stmt->variablebind->pos.row,
                stmt->variablebind->pos.col,
                "i32",
                stmt->variablebind->value);
        exit(1);
      }

      if (stmt->variablebind->type == AST_VARIABLE_TYPE_STRING && stmt->variablebind->value[0] != '"') {
        fprintf(stderr, "%s:%d:%d [Semantic error]: variable of type '%s' cannot be assigned to '%s'.\n",
                stmt->variablebind->pos.file_path,
                stmt->variablebind->pos.row,
                stmt->variablebind->pos.col, "string", stmt->variablebind->value);
        exit(1);
      }
    } else if (stmt->funccall) {
      if (strcmp(stmt->funccall->funcname, "SYS_CALL") == 0) {
        uint32_t max_arg_count = 2;
        uint32_t arg_count = 0;
        ast_callarguments* arg = stmt->funccall->call_arguments;
        while(arg != NULL) {
          arg_count++;

          if (arg->type != AST_VARIABLE_TYPE_I32) {
            fprintf(stderr, "%s:%d:%d [Semantic error]: No such function signiture for SYS_CALL intrinsic.\n\tThe right signiture is SYS_CALL(i32 i32)\n",
                    stmt->funccall->pos.file_path,
                    stmt->funccall->pos.row,
                    stmt->funccall->pos.col);
            exit(1);
          }

          arg = arg->next;
        }

        if (arg_count > max_arg_count || arg_count < max_arg_count) {
          fprintf(stderr, "%s:%d:%d [Semantic error]: Insufficient arguemnts for intrinsic SYS_CALL.\n\tThe signiture is SYS_CALL(i32 i32)\n",
                  stmt->funccall->pos.file_path,
                  stmt->funccall->pos.row,
                  stmt->funccall->pos.col);
          exit(1);
        }
      }
    }

    stmt = stmt->next;
  }
}

void semantic_check_funcdecl(Semantic* s, ast_funcdecl* funcdecl) {
  semantic_check_block(s, funcdecl->block);
}

void semantic_check(Semantic* s) {
  if (s->ast->includes) {
    semantic_check_includes(s, s->ast->includes);
  }

  semantic_check_funcdecl(s, s->ast->funcdecl);
}
