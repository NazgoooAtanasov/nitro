#include "tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static inline bool is_intrinsic(const char* word) {
  return strcmp(word, "SYS_CALL") == 0;
}

static inline bool is_ident(const char* word) {
  uint32_t len = strlen(word);
  for (uint32_t i = 0; i < len; ++i) {
    if ((word[i] >= 65 && word[i] <= 90) || (word[i] >= 97 && word[i] <= 122)) {
      continue;
    } else {
      return false;
    }
  }

  return true;
}

static inline bool is_type(const char* word) {
  return strcmp(word, "i32") == 0 ||
    strcmp(word, "void") == 0 ||
    strcmp(word, "string") == 0;
}

static inline bool is_keyword(const char* word) {
  return strcmp(word, "func") == 0 ||
    strcmp(word, "include") == 0 ||
    strcmp(word, "do") == 0 ||
    strcmp(word, "end") == 0 ||
    strcmp(word, "bind") == 0 ||
    strcmp(word, "->") == 0 ||
    is_type(word);
}

const char* token_type_str(TokenType type) {
  switch(type) {
    case TOKEN_TYPE_INTRINSIC: return "TOKEN_TYPE_INTRINSIC";
    case TOKEN_TYPE_KEYWORD: return "TOKEN_TYPE_KEYWORD";
    case TOKEN_TYPE_IDENT: return "TOKEN_TYPE_IDENT";
    case TOKEN_TYPE_CONST: return "TOKEN_TYPE_CONST";
    case TOKEN_TYPE_EQUALS: return "TOKEN_TYPE_EQUALS";
    case TOKEN_TYPE_TYPE: return "TOKEN_TYPE_TYPE";
    case TOKEN_TYPE_RETURN_TYPE_DELIMITER: return "TOKEN_TYPE_RETURN_TYPE_DELIMITER";
    case TOKEN_TYPE_TYPE_DELIMITER: return "TOKEN_TYPE_TYPE_DELIMITER";
    case TOKEN_TYPE_OPENING_BRACE: return "TOKEN_TYPE_OPENING_BRACE";
    case TOKEN_TYPE_CLOSING_BRACE: return "TOKEN_TYPE_CLOSING_BRACE";

    default: return "TOKEN_TYPE_UNKNOWN";
  }
}

Tokenizer tokenizer_craete(const char* file_contents, const char* file_path) {
  Tokenizer tokenizer = {0};
  tokenizer.token_cap = TOKENS_SIZE;
  tokenizer.tokens = malloc(sizeof(Token) * TOKENS_SIZE);
  tokenizer.token_sz = 0;
  tokenizer.file_contents = file_contents;
  tokenizer.file_path = file_path;
  return tokenizer;
}

void tokenizer_free(Tokenizer* tokenizer) {
  free(tokenizer->tokens);
  tokenizer->token_sz = 0;
  tokenizer->file_contents = NULL;
}

bool tokenizer_add_token(Tokenizer* tokenizer, Token token) {
  if (tokenizer->token_sz + 1 >= TOKENS_SIZE) {
    tokenizer->token_cap *= 2;
    void* ptr = realloc(tokenizer->tokens, tokenizer->token_cap * sizeof(Token));
  }

  tokenizer->tokens[tokenizer->token_sz++] = token;
  return true;
}

void tokenizer_print_tokens(Tokenizer* tokenizer) {
  for (uint32_t i = 0; i < tokenizer->token_sz; ++i) {
    Token tok = tokenizer->tokens[i];

    fprintf(stdout, "{ \"str_val\": \"%s\", \"type\": \"%s\" }\n",
            tok.str_val,
            token_type_str(tok.type));
  }
}

const Token* tokenizer_tokenize(Tokenizer* tokenizer) {
  uint32_t i = 0;
  uint32_t row = 1;
  uint32_t col;
  while(tokenizer->file_contents[i] != '\0') {
    char word_buffr[TOKEN_WORD_SIZE] = {0};
    uint32_t word_buffr_i = 0;

    while(
      tokenizer->file_contents[i] != ' ' &&
      tokenizer->file_contents[i] != '\t' &&
      tokenizer->file_contents[i] != '\n' &&
      tokenizer->file_contents[i] != '(' && 
      tokenizer->file_contents[i] != ')' &&
      tokenizer->file_contents[i] != ':'
    ) {
      word_buffr[word_buffr_i++] = tokenizer->file_contents[i++];
      col++;
    }

    if (word_buffr_i <= 0 && (tokenizer->file_contents[i] == ' ' || tokenizer->file_contents[i] == '\t' || tokenizer->file_contents[i] == '\n')) {
      if (tokenizer->file_contents[i] == '\n') {
        row++;
        col = 0;
      }
      i++;
      col++;
      continue;
    }

    Token tok = {0};
    bool keyword = is_keyword(word_buffr);
    bool ident = is_ident(word_buffr);
    bool intrinsic = is_intrinsic(word_buffr);

    if (word_buffr_i <= 0) {
      // here we handle the braces and :
      if (tokenizer->file_contents[i] == '(') {
        word_buffr[word_buffr_i++] = '(';
        tok.type = TOKEN_TYPE_OPENING_BRACE;
        i++;
        col++;
      } else if (tokenizer->file_contents[i] == ')') {
        word_buffr[word_buffr_i++] = ')';
        tok.type = TOKEN_TYPE_CLOSING_BRACE;
        i++;
        col++;
      } else if (tokenizer->file_contents[i] == ':') {
        word_buffr[word_buffr_i++] = ':';
        tok.type = TOKEN_TYPE_TYPE_DELIMITER;
        i++;
        col++;
      } 
    } else if (intrinsic) {
      tok.type = TOKEN_TYPE_INTRINSIC;
    } else if (keyword) {
      if (strcmp(word_buffr, "->") == 0) {
        tok.type = TOKEN_TYPE_RETURN_TYPE_DELIMITER;
      } else if (is_type(word_buffr)) {
        tok.type = TOKEN_TYPE_TYPE;
      } else {
        tok.type = TOKEN_TYPE_KEYWORD;
      }
    } else if (word_buffr[0] == '=') {
      tok.type = TOKEN_TYPE_EQUALS;
    } else if (ident) {
      tok.type = TOKEN_TYPE_IDENT;
    } else {
      tok.type = TOKEN_TYPE_CONST;
    }

    memcpy(tok.str_val, word_buffr, word_buffr_i);
    tok.str_val[word_buffr_i+1] = '\0';
    tok.col = col;
    tok.row = row;

    bool res = tokenizer_add_token(tokenizer, tok);
    if (res != true) {
      fprintf(stderr, "There was a problem parsing %s!\n", tokenizer->file_path);
      exit(1);
    }
  }

  return tokenizer->tokens;
}
