#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"

#include "../lib/profile.h"

#include <fcntl.h> // open
#include <stddef.h>
#include <stdio.h> // sprint
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Size of the output buffer
constexpr size_t FILE_BUFFER_SIZE = (size_t)1024 * 1024;

// Memory allocated for AST parsing
constexpr size_t AST_MEMORY = (size_t)(1024 * 64);

// Memory allocated for storing transient values across environments
constexpr size_t TEMP_MEMORY = (size_t)(1024 * 64);

#define error(Fmt, ...)                                                        \
  {                                                                            \
    fprintf(stderr, "lifp: ");                                                 \
    fprintf(stderr, Fmt __VA_OPT__(, ) __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                                     \
  }

#define tryRun(Action, ...)                                                    \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    error("%s", _concat(result, __LINE__).message);                            \
    profileReport();                                                           \
    return 1;                                                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value;)

#define tryCLI(Action, Destination, ErrorMessage)                              \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    error("%s", ErrorMessage);                                                 \
    return 1;                                                                  \
  }                                                                            \
  (Destination) = _concat(result, __LINE__).value;

void readLine(ssize_t size, char line_buffer[static size],
              const char input_buffer[static size], ssize_t *offset) {
  size_t line_buffer_offset = 0;
  bool skip_current_line = false;
  int depth = 0;

  for (; *offset < size; (*offset)++) {
    const char current = input_buffer[(*offset)];

    // FIXME: the lexer should know about comments, instead of hanling it here
    if (current == ';') {
      skip_current_line = true;
      continue;
    }

    if (current == '\n') {
      skip_current_line = false;
      continue;
    }

    if (skip_current_line)
      continue;

    line_buffer[line_buffer_offset++] = current;

    if (current == LPAREN) {
      depth++;
    } else if (current == RPAREN) {
      depth--;
    }

    // FIXME: this does not allow having top-level atoms
    if (depth == 0) {
      (*offset)++;
      return;
    }
  }
}

allocMetricsInit();

int main(int argc, char **argv) {
  if (argc != 2) {
    error("'run' takes one argument");
    return 1;
  }

  const char *file_name = argv[1];
  int file_descriptor = open(file_name, O_RDONLY, 0644);
  if (file_descriptor < 0) {
    error("cannot open '%s'", file_name);
    return 1;
  }

  char *file_buffer = malloc(sizeof(char) * FILE_BUFFER_SIZE);
  if (!file_buffer) {
    error("cannot allocate file buffer");
    return 1;
  }
  ssize_t len = read(file_descriptor, file_buffer, FILE_BUFFER_SIZE);
  if (len == 0) {
    error("provided file is empty");
    return 1;
  }

  ssize_t file_offset = 0;
  char *line_buffer = malloc(sizeof(char) * FILE_BUFFER_SIZE);
  if (!line_buffer) {
    error("cannot allocate file buffer");
    return 1;
  }

  profileInit();
  arena_t *ast_arena = nullptr;
  tryCLI(arenaCreate(AST_MEMORY), ast_arena,
         "unable to allocate interpreter memory");

  arena_t *temp_arena = nullptr;
  tryCLI(arenaCreate(TEMP_MEMORY), temp_arena,
         "unable to allocate transient memory");

  environment_t *global_environment = nullptr;
  tryCLI(vmInit(), global_environment, "unable to initialize virtual machine");

  do {
    memset(line_buffer, 0, (size_t)len);
    arenaReset(ast_arena);
    arenaReset(temp_arena);
    readLine(len, line_buffer, file_buffer, &file_offset);

    if (strlen(line_buffer) == 0)
      continue;

    token_list_t *tokens = nullptr;
    tryRun(tokenize(ast_arena, line_buffer), tokens);

    size_t line_offset = 0;
    size_t depth = 0;
    node_t *syntax_tree = nullptr;
    tryRun(parse(ast_arena, tokens, &line_offset, &depth), syntax_tree);

    value_t reduced;
    tryRun(evaluate(&reduced, temp_arena, syntax_tree, global_environment));
  } while (strlen(line_buffer) > 0);

  profileReport();

  environmentDestroy(&global_environment);
  arenaDestroy(&temp_arena);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryCLI
#undef tryREPL
#undef printError
