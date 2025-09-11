#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"

#include "../lib/profile.h"

#include "utils.h"

#include <fcntl.h> // open
#include <stddef.h>
#include <stdio.h> // sprint
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char RUN[] = "run";

typedef struct {
  size_t ast_memory;
  size_t temp_memory;
  size_t file_size;
  size_t call_stack_size;
  size_t environment_size;
  const char *filename;
} run_opts_t;

#define tryRun(Action, ...)                                                    \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    error("%s", _concat(result, __LINE__).message);                            \
    profileReport();                                                           \
    return 1;                                                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value;)

static void readStatement(ssize_t size, char line_buffer[static size],
                          const char input_buffer[static size],
                          ssize_t *offset) {
  size_t line_buffer_offset = 0;
  int depth = 0;

  for (; *offset < size; (*offset)++) {
    const char current = input_buffer[(*offset)];

    // end of an atom statement
    if (current == '\n' && depth == 0) {
      (*offset)++;
      return;
    }

    line_buffer[line_buffer_offset++] = current;

    // end of a list statement
    if (current == RPAREN) {
      if (depth == 1) {
        (*offset)++;
        return;
      }
      depth--;
    }

    if (current == LPAREN) {
      depth++;
    }
  }
}

int run(const run_opts_t OPTIONS) {
  int file_descriptor = open(OPTIONS.filename, O_RDONLY, 0644);
  if (file_descriptor < 0) {
    error("cannot open '%s'", OPTIONS.filename);
    return 1;
  }

  char *file_buffer;
  tryCLI(allocSafe(OPTIONS.file_size), file_buffer,
         "cannot allocate file buffer");

  ssize_t file_offset = 0;
  ssize_t file_length = read(file_descriptor, file_buffer, OPTIONS.file_size);
  if (file_length == 0) {
    error("provided file is empty");
    return 1;
  }

  char *statement_buffer;
  tryCLI(allocSafe(OPTIONS.file_size), statement_buffer,
         "cannot allocate file buffer");

  profileInit();
  arena_t *ast_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.ast_memory), ast_arena,
         "unable to allocate interpreter memory");

  arena_t *scratch_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.temp_memory), scratch_arena,
         "unable to allocate transient memory");

  arena_t *result_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.temp_memory), result_arena,
         "unable to allocate transient memory");

  vm_opts_t vm_options = {
      .max_call_stack_size = OPTIONS.call_stack_size,
      .environment_size = OPTIONS.environment_size,
  };
  environment_t *global_environment = nullptr;
  tryCLI(vmInit(vm_options), global_environment,
         "unable to initialize virtual machine");

  do {
    memset(statement_buffer, 0, (size_t)file_length);
    arenaReset(ast_arena);
    arenaReset(scratch_arena);
    readStatement(file_length, statement_buffer, file_buffer, &file_offset);

    if (strlen(statement_buffer) == 0)
      continue;

    token_list_t *tokens = nullptr;
    tryRun(tokenize(ast_arena, statement_buffer), tokens);

    size_t line_offset = 0;
    size_t depth = 0;
    node_t *syntax_tree = nullptr;
    tryRun(parse(ast_arena, tokens, &line_offset, &depth), syntax_tree);

    if (syntax_tree) {
      value_t reduced;
      tryRun(evaluate(&reduced, result_arena, scratch_arena, syntax_tree,
                      global_environment));
    }
  } while (file_offset < file_length);

  profileReport();

  deallocSafe(&statement_buffer);
  deallocSafe(&file_buffer);

  environmentDestroy(&global_environment);
  arenaDestroy(&result_arena);
  arenaDestroy(&scratch_arena);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryCLI
#undef tryREPL
