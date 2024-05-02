#ifndef NITRO_TOKENIZER_
#define NITRO_TOKENIZER_

#include <stdint.h>
#include <stdbool.h>

#define TOKEN_WORD_SIZE 1024
#define TOKENS_SIZE 100

typedef enum {
  TOKEN_TYPE_INTRINSIC,
  TOKEN_TYPE_TYPE,
  TOKEN_TYPE_RETURN_TYPE_DELIMITER,
  TOKEN_TYPE_TYPE_DELIMITER,
  TOKEN_TYPE_OPENING_BRACE,
  TOKEN_TYPE_CLOSING_BRACE,
  TOKEN_TYPE_EQUALS,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_IDENT,
  TOKEN_TYPE_CONST,
} TokenType;

typedef struct {
  char str_val[TOKEN_WORD_SIZE];
  uint32_t row;
  uint32_t col;
  TokenType type;
} Token;
const char* token_type_str(TokenType type);

typedef struct {
  const char* file_contents;
  const char* file_path;
  Token* tokens;
  uint32_t token_sz;
  uint32_t token_cap;
} Tokenizer;
Tokenizer tokenizer_craete(const char* file_contents, const char* file_path);
void tokenizer_free(Tokenizer* tokenizer);
bool tokenizer_add_token(Tokenizer* tokenizer, Token token);
void tokenizer_print_tokens(Tokenizer* tokenizer);
const Token* tokenizer_tokenize(Tokenizer* tokenizer);

#endif // NITRO_TOKENIZER_
