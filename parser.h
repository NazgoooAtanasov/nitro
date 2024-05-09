#ifndef NITRO_PARSER_
#define NITRO_PARSER_

#include "tokenizer.h"
#include <stdint.h>

typedef enum {
  AST_VARIABLE_TYPE_UNKNOWN,
  AST_VARIABLE_TYPE_I32,
  AST_VARIABLE_TYPE_STRING,
  AST_VARIABLE_TYPE_VOID,
} AST_VARIABLE_TYPE;

typedef struct {
  const char* file_path;
  uint32_t row;
  uint32_t col;
} ast_position;

typedef struct _ast_includes {
  const char* file_path;
  struct _ast_includes* next;
  ast_position pos;
} ast_includes;

typedef struct {
  const char* ident;
  AST_VARIABLE_TYPE type;
  const char* value;
  ast_position pos;
} ast_variablebind;

typedef struct _ast_callarguments {
  const char* value;
  AST_VARIABLE_TYPE type;
  bool is_ident;

  ast_position pos;
  struct _ast_callarguments* next;
  struct _ast_callarguments* prev;
} ast_callarguments;

typedef struct {
  const char* funcname;
  ast_callarguments* call_arguments;
  ast_position pos;
} ast_funccall;

typedef struct _ast_stmt {
  ast_variablebind* variablebind;
  ast_funccall* funccall;
  struct _ast_stmt* next;
  ast_position pos;
} ast_stmt;

typedef struct {
  ast_stmt* stmts;
  ast_position pos;
} ast_block;

typedef struct _ast_funcarguments {
  const char* arg_name;
  AST_VARIABLE_TYPE arg_type;
  struct _ast_funcarguments* next;
  ast_position pos;
} ast_funcarguments;

typedef struct _ast_funcdecl {
  const char* func_name;
  const char* return_type;
  ast_funcarguments* arguments;
  ast_block* block;
  ast_position pos;
  struct _ast_funcdecl* next;
} ast_funcdecl;

typedef struct {
  ast_includes* includes;
  ast_funcdecl* funcdecl;
  ast_position pos;
} ast_axiom;

typedef struct {
  Tokenizer* tokenizer;
  uint32_t cur;
  ast_position pos;
} Parser;

Parser parser_create(Tokenizer* tokenizer);
ast_includes* parser_parse_includes(Parser* p);
ast_variablebind* parser_parse_variablebind(Parser* p);
ast_stmt* parser_parse_stmt(Parser* p);
ast_block* parser_parse_block(Parser* p);
ast_funcarguments* parser_parse_funcarguments(Parser* p);
ast_funcdecl* parser_parse_funcdecl(Parser* p);
ast_axiom* parser_parse_axiom(Parser* p);

#endif // NITRO_PARSER_
