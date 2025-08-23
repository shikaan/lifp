#include "../lib/arena.h"
#include "../lib/profile.h"
#include "../lifp/evaluate.h"
#include "../lifp/fmt.h"
#include "../lifp/node.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"
#include "../vendor/linenoise/linenoise.h"
#include <stdio.h>
#include <string.h>

allocMetricsInit();

// Size of the output buffer
constexpr size_t BUFFER_SIZE = 4096;

// Memory allocated for AST parsing
constexpr size_t AST_MEMORY = (size_t)(1024 * 64);

// Memory allocated for storing transient values across environments
constexpr size_t TEMP_MEMORY = (size_t)(1024 * 64);

const char REPL_COMMAND_CLEAR[] = "clear";

#define printError(Result, InputBuffer, Size, OutputBuffer)                    \
  int _concat(offset_, __LINE__) = 0;                                          \
  formatErrorMessage((Result)->message, (Result)->meta, "repl", InputBuffer,   \
                     Size, OutputBuffer, &_concat(offset_, __LINE__));         \
  fprintf(stdout, "%s\n", OutputBuffer);

#define tryREPL(Action, ...)                                                   \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    printError(&_concat(result, __LINE__), input, BUFFER_SIZE, buffer);        \
    continue;                                                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value;)

#define tryCLI(Action, Destination, ErrorMessage)                              \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    fprintf(stderr, "lifp: %s", ErrorMessage);                                 \
    return 1;                                                                  \
  }                                                                            \
  (Destination) = _concat(result, __LINE__).value;

int main(void) {
  char buffer[BUFFER_SIZE];

  arena_t *ast_arena = nullptr;
  tryCLI(arenaCreate(AST_MEMORY), ast_arena,
         "unable to allocate interpreter memory");

  arena_t *temp_arena = nullptr;
  tryCLI(arenaCreate(TEMP_MEMORY), temp_arena,
         "unable to allocate transient memory");

  environment_t *global_environment = nullptr;
  tryCLI(vmInit(), global_environment, "unable to initialize virtual machine");

  linenoiseSetMultiLine(1);

  profileInit();
  while (true) {
    profileReport();
    arenaReset(ast_arena);
    arenaReset(temp_arena);
    char *input = linenoise("> ");

    if (!input)
      break;

    if (strlen(input) == 0)
      continue;

    if (strcmp(input, REPL_COMMAND_CLEAR) == 0) {
      printf("\e[1;1H\e[2J");
      continue;
    }

    token_list_t *tokens = nullptr;
    tryREPL(tokenize(ast_arena, input), tokens);

    // Add to history only if the string can be tokenized
    linenoiseHistoryAdd(input);

    size_t offset = 0;
    size_t depth = 0;
    node_t *ast = nullptr;
    tryREPL(parse(ast_arena, tokens, &offset, &depth), ast);

    value_t result;
    tryREPL(evaluate(&result, temp_arena, ast, global_environment));

    int buffer_offset = 0;
    formatValue(&result, BUFFER_SIZE, buffer, &buffer_offset);
    printf("~> %s\n", buffer);

    memset(buffer, 0, BUFFER_SIZE);
  }
  profileEnd();
  environmentDestroy(&global_environment);
  arenaDestroy(&temp_arena);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryCLI
#undef tryREPL
#undef printError
