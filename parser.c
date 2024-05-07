#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline AST_VARIABLE_TYPE get_type(const char* word) {
  if (word[0] == '"') {
    return AST_VARIABLE_TYPE_STRING;
  }

  bool is_i32 = true;
  for (uint32_t i = 0; i < strlen(word); ++i) {
    if (word[i] < 48 || word[i] > 57) {
      is_i32 = false;
      break;
    }
  }

  if (is_i32) return AST_VARIABLE_TYPE_I32;

  assert(false && "This should be unreachablet.If this is hit, then there is a new type that has not been handled.");
  return AST_VARIABLE_TYPE_VOID;
}

Parser parser_create(Tokenizer* tokenizer) {
  Parser parser = {0};
  parser.tokenizer = tokenizer;
  return parser;
}
Token* parser_peek_token(Parser* p) {
  return &p->tokenizer->tokens[p->cur];
}
Token* parser_pop_token(Parser* p) {
  Token* tok = parser_peek_token(p);
  p->cur++;
  return tok;
};

ast_includes* parser_parse_includes(Parser* p) {
  ast_includes* includes = malloc(sizeof(ast_includes));
  ast_includes* level = NULL;

  Token* t = parser_peek_token(p);
  while(t->type == TOKEN_TYPE_KEYWORD && strcmp(t->str_val, "include") == 0) {
    if (level == NULL) {
      level = includes;
    } else {
      level->next = malloc(sizeof(ast_includes));
      level = level->next;
    }

    parser_pop_token(p); // consuming the peeked token before

    t = parser_pop_token(p);
    if (t->type != TOKEN_TYPE_CONST) {
      fprintf(stderr,
              "%s:%d:%d [Compile error]: Unexpected token '%s', where constant or immediate value is expected.\n\tMaybe change '%s' to '\"%s\"'\n",
              p->tokenizer->file_path, t->row, t->col, t->str_val, t->str_val, t->str_val);
      exit(1);
    }
    level->file_path = t->str_val;
    level->pos = (ast_position){
      .file_path = p->tokenizer->file_path,
      .col = t->col,
      .row = t->row
    };

    t = parser_peek_token(p);
  }

  return includes;
}

ast_variablebind* parser_parse_variablebind(Parser* p) {
  ast_variablebind* variablebind = malloc(sizeof(ast_variablebind));

  Token* t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_KEYWORD || strcmp(t->str_val, "bind") != 0) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', in variable binding.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_IDENT) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where variable identifier is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }
  variablebind->ident = t->str_val;

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_TYPE_DELIMITER) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where variable type delimiter is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_TYPE) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where variable type is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  if (strcmp(t->str_val, "i32") == 0) {
    variablebind->type = AST_VARIABLE_TYPE_I32;
  } else if (strcmp(t->str_val, "string") == 0) {
    variablebind->type = AST_VARIABLE_TYPE_STRING;
  } else if (strcmp(t->str_val, "void") == 0) {
    variablebind->type = AST_VARIABLE_TYPE_VOID;
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_EQUALS) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where '=' is expected for variable binding.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_CONST) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where constant value is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }
  variablebind->value = t->str_val;
  variablebind->pos = (ast_position){
    .file_path = p->tokenizer->file_path,
    .col = t->col,
    .row = t->row
  };

  return variablebind;
}

ast_callarguments* parser_parse_callarguments(Parser* p) {
  ast_callarguments* call_arguments = malloc(sizeof(ast_callarguments));
  ast_callarguments* level = NULL;

  uint32_t arg_count = 0;
  Token* t = parser_peek_token(p);
  while (t->type == TOKEN_TYPE_CONST || t->type == TOKEN_TYPE_IDENT) {
    if (level == NULL) {
      level = call_arguments;
    } else {
      level->next = malloc(sizeof(ast_callarguments));
      level->next->prev = level;
      level = level->next;
    }
    parser_pop_token(p);

    level->value = t->str_val;

    if (t->type == TOKEN_TYPE_CONST) {
      level->type = get_type(level->value);
    }

    level->pos = (ast_position) {
      .file_path = p->tokenizer->file_path,
      .row = t->row,
      .col = t->col
    };

    arg_count++;
    t = parser_peek_token(p);
  }

  return arg_count > 0 ? call_arguments : NULL;
}

ast_funccall* parser_parse_funccal(Parser* p) {
  ast_funccall* funccall = malloc(sizeof(ast_funccall));

  Token* t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_INTRINSIC && t->type != TOKEN_TYPE_IDENT) {
    fprintf(stderr, "%s:%d:%d [Compile error]: Name '%s' is not callable.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }
  funccall->funcname = t->str_val;

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_OPENING_BRACE) {
    fprintf(stderr, "%s:%d:%d [Compile error]: Unexpected token '%s', where '(' is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  funccall->call_arguments = parser_parse_callarguments(p);

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_CLOSING_BRACE) {
    fprintf(stderr, "%s:%d:%d [Compile error]: Unexpected token '%s', where ')' is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }
  funccall->pos = (ast_position) {
    .file_path = p->tokenizer->file_path,
    .row = t->row,
    .col = t->col
  };

  return funccall;
}

ast_stmt* parser_parse_stmt(Parser* p) {
  ast_stmt* stmts = malloc(sizeof(ast_stmt));
  ast_stmt* level = NULL;

  Token* t = parser_peek_token(p);
  while ((t->type == TOKEN_TYPE_KEYWORD && strcmp(t->str_val, "bind") == 0) || (t->type == TOKEN_TYPE_INTRINSIC || t->type == TOKEN_TYPE_IDENT)) {
    if (level == NULL) {
      level = stmts;
    } else {
      level->next = malloc(sizeof(ast_stmt));
      level = level->next;
    }

    if (t->type == TOKEN_TYPE_KEYWORD && strcmp(t->str_val, "bind") == 0) {
      level->variablebind = parser_parse_variablebind(p);
    } else if (t->type == TOKEN_TYPE_INTRINSIC || t->type == TOKEN_TYPE_IDENT) { // @TODO: to handle normal funccalls here
      level->funccall = parser_parse_funccal(p);
    }

    level->pos = (ast_position){
      .file_path = p->tokenizer->file_path,
      .col = t->col,
      .row = t->row
    };
    t = parser_peek_token(p);
  }

  return stmts;
}

ast_block* parser_parse_block(Parser* p) {
  ast_block* block = malloc(sizeof(ast_block));

  Token* t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_KEYWORD || strcmp(t->str_val, "do") != 0) {
      fprintf(stderr,
              "%s:%d:%d [Compile error]: Unexpected token '%s', where 'do' (block start) is expected.\n",
              p->tokenizer->file_path, t->row, t->col, t->str_val);
      exit(1);
  }

  block->stmts = parser_parse_stmt(p);

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_KEYWORD || strcmp(t->str_val, "end") != 0) {
      fprintf(stderr,
              "%s:%d:%d [Compile error]: Unexpected token '%s', where 'end' (block end) is expected.\n",
              p->tokenizer->file_path, t->row, t->col, t->str_val);
      exit(1);
  }

  block->pos = (ast_position){
    .file_path = p->tokenizer->file_path,
    .col = t->col,
    .row = t->row
  };

  return block;
}

ast_funcarguments* parser_parse_funcarguments(Parser* p) {
  ast_funcarguments* arguments = malloc(sizeof(ast_funcarguments));
  ast_funcarguments* level = NULL;

  Token* t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_OPENING_BRACE) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where opening brackets for function arguments is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  uint32_t arg_count = 0;
  t = parser_peek_token(p);
  while(t->type == TOKEN_TYPE_IDENT) {
    if (level == NULL) {
      level = arguments;
    } else {
      level->next = malloc(sizeof(ast_funcarguments));
      level = level->next;
    }
    parser_pop_token(p); // consuming the already checked token

    level->arg_name = t->str_val;
    t = parser_pop_token(p);
    if (t->type != TOKEN_TYPE_TYPE_DELIMITER) {
      fprintf(stderr,
              "%s:%d:%d [Compile error]: Unexpected token '%s', where type delimiter for function argument '%s' is expected.\n\tTry replacing '%s' with ':'.\n",
              p->tokenizer->file_path, t->row, t->col, t->str_val, level->arg_name, t->str_val);
      exit(1);
    }

    t = parser_pop_token(p);
    if (t->type != TOKEN_TYPE_TYPE) {
      fprintf(stderr,
              "%s:%d:%d [Compile error]: Unexpected token '%s', where type function argument '%s' is expected.\n",
              p->tokenizer->file_path, t->row, t->col, t->str_val, level->arg_name);
      exit(1);
    }

    if (strcmp(t->str_val, "i32") == 0) {
      level->arg_type = AST_VARIABLE_TYPE_I32;
    } else if (strcmp(t->str_val, "string") == 0) {
      level->arg_type = AST_VARIABLE_TYPE_STRING;
    } else if (strcmp(t->str_val, "void") == 0) {
      level->arg_type = AST_VARIABLE_TYPE_VOID;
    }

    level->pos = (ast_position){
      .file_path = p->tokenizer->file_path,
      .col = t->col,
      .row = t->row
    };

    arg_count++;
    t = parser_peek_token(p);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_CLOSING_BRACE) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where closing brackets for function arguments is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  return arg_count > 0 ? arguments : NULL;
}

ast_funcdecl* parser_parse_funcdecl(Parser* p) {
  ast_funcdecl* funcdecl = malloc(sizeof(ast_funcdecl));

  Token* t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_KEYWORD || strcmp(t->str_val, "func") != 0) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where function declaration is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_IDENT) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where function name(identifier) is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  funcdecl->func_name = t->str_val;
  funcdecl->arguments = parser_parse_funcarguments(p);

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_RETURN_TYPE_DELIMITER) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where return type delimiter is expected.\n\tTry replacing '%s' with '->'.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val, t->str_val);
    exit(1);
  }

  t = parser_pop_token(p);
  if (t->type != TOKEN_TYPE_TYPE) {
    fprintf(stderr,
            "%s:%d:%d [Compile error]: Unexpected token '%s', where type is expected.\n",
            p->tokenizer->file_path, t->row, t->col, t->str_val);
    exit(1);
  }

  funcdecl->return_type = t->str_val;

  funcdecl->block = parser_parse_block(p);
  funcdecl->pos = (ast_position){
    .file_path = p->tokenizer->file_path,
    .col = t->col,
    .row = t->row
  };

  return funcdecl;
}

ast_axiom* parser_parse_axiom(Parser* p) {
  ast_axiom* axiom = malloc(sizeof(ast_axiom));

  Token* t = parser_peek_token(p);
  if (t->type == TOKEN_TYPE_KEYWORD && strcmp(t->str_val, "include") == 0) {
    axiom->includes = parser_parse_includes(p);
  }

  axiom->funcdecl = parser_parse_funcdecl(p);
  ast_funcdecl* level = axiom->funcdecl;

  t = parser_peek_token(p);
  while(t->type == TOKEN_TYPE_KEYWORD && strcmp(t->str_val, "func") == 0) {
    level->next = parser_parse_funcdecl(p);
    level = level->next;
    t = parser_peek_token(p);
  }

  axiom->pos = (ast_position){
    .file_path = p->tokenizer->file_path,
    .col = t->col,
    .row = t->row
  };

  return axiom;
}
