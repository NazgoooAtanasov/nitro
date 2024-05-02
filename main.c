#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tokenizer.h"
#include "parser.h"
#include "semantic.h"
#include "generator.h"

int main(int argc, char** argv) {
  if (argc <= 1) {
    fprintf(stderr, "Please provide a file to read.\n\tUsage: nitroc file.nitro\n");
    return 1;
  }

  const char* file_path = argv[1];

  const char* dot = strrchr(file_path, '.');
  if (strcmp(dot, ".nitro") != 0) {
    fprintf(stderr, "The provided file is not a nitro file.\n");
    return 1;
  }

  FILE* file = fopen(file_path, "r");
  if (!file) {
    fprintf(stderr, "There was an error opening %s: %s", file_path, strerror(errno));
    return 1;
  }

  char file_contents[MAX_NITRO_FILE_SIZE];
  fread(file_contents, sizeof(char), MAX_NITRO_FILE_SIZE, file);

  Tokenizer tokenizer = tokenizer_craete(file_contents, file_path);
  const Token* tokens = tokenizer_tokenize(&tokenizer);

  Parser parser = parser_create(&tokenizer);
  ast_axiom* axiom = parser_parse_axiom(&parser);

  Semantic semantic = semantic_create(axiom);
  semantic_check(&semantic);

  Generator generator = generator_create(axiom);
  generator_gen_top_level(&generator);

  fprintf(stdout, "%s\n", generator.asm_buffr);
  return 0;
}
