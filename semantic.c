#include "semantic.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline const char* type_to_str(AST_VARIABLE_TYPE type) {
  switch(type) {
    case AST_VARIABLE_TYPE_I32: return "i32";
    case AST_VARIABLE_TYPE_STRING: return "string";
    case AST_VARIABLE_TYPE_VOID: return "void";
    default: return "UNKNOWN";
  }
}

Semantic semantic_create(ast_axiom* ast) {
  Semantic semantic = {0};
  semantic.ast = ast;
  semantic.funcdecls = NULL;
  return semantic;
}

void semantic_add_funcdecl(Semantic* s, ast_funcdecl* funcdecl) {
  shput(s->funcdecls, funcdecl->func_name, funcdecl);
}

ast_funcdecl* semantic_get_funcdecl(Semantic* s, const char* funcname) {
  ast_funcdecl* funcdecl = shget(s->funcdecls, funcname);
  return funcdecl;
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
    } else if (stmt->funccall) { // intrinsic SYS_CALL with two arguments
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
      } else { // every other funccall
        ast_funcdecl* funcdecl = semantic_get_funcdecl(s, stmt->funccall->funcname);
        if (funcdecl == NULL) {
          fprintf(stderr, "%s:%d:%d [Semantic error]: Undefined reference to '%s'. '%s' is not callable.\n",
                  stmt->funccall->pos.file_path,
                  stmt->funccall->pos.row,
                  stmt->funccall->pos.col,
                  stmt->funccall->funcname,
                  stmt->funccall->funcname);
          exit(1);
        }

        uint32_t arg_pos = 1;
        ast_funcarguments* declarg = funcdecl->arguments;
        ast_callarguments* callarg = stmt->funccall->call_arguments;
        while(declarg != NULL && callarg != NULL) {
          if (callarg->type != declarg->arg_type) {
            fprintf(stderr, "%s:%d:%d [Semantic error]: Argument in function call '%s', on position %d should be of type '%s', not '%s'.\n",
                    stmt->funccall->pos.file_path,
                    stmt->funccall->pos.row,
                    stmt->funccall->pos.col,
                    stmt->funccall->funcname,
                    arg_pos,
                    type_to_str(declarg->arg_type), 
                    type_to_str(callarg->type));
            exit(1);
          }

          arg_pos++;
          declarg = declarg->next;
          callarg = callarg->next;
        }

        if (declarg != NULL && callarg == NULL) {
          fprintf(stderr, "%s:%d:%d [Semantic error]: Insufficient amount of arguments for '%s'.\n",
                  stmt->funccall->pos.file_path,
                  stmt->funccall->pos.row,
                  stmt->funccall->pos.col,
                  stmt->funccall->funcname);
          exit(1);
        }

        if (declarg == NULL && callarg != NULL) {
          fprintf(stderr, "%s:%d:%d [Semantic error]: Call to '%s' has too much arguments.\n",
                  stmt->funccall->pos.file_path,
                  stmt->funccall->pos.row,
                  stmt->funccall->pos.col,
                  stmt->funccall->funcname);
          exit(1);
        }
      }
    }

    stmt = stmt->next;
  }
}

void semantic_check_funcdecl(Semantic* s, ast_funcdecl* funcdecl) {
  semantic_add_funcdecl(s, funcdecl);
  semantic_check_block(s, funcdecl->block);
}

void semantic_check(Semantic* s) {
  if (s->ast->includes) {
    semantic_check_includes(s, s->ast->includes);
  }

  ast_funcdecl* level = s->ast->funcdecl;
  while (level != NULL) {
    semantic_check_funcdecl(s, level);
    level = level->next;
  }
}
