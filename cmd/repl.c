#include "../lib/arena.h"
#include "../lib/profile.h"
#include "../lifp/evaluate.h"
#include "../lifp/fmt.h"
#include "../lifp/node.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"
#include "../vendor/linenoise/linenoise.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

const char REPL[] = "repl";

typedef struct {
  size_t ast_memory;
  size_t temp_memory;
  size_t ouput_size;
} repl_opts_t;

static constexpr char REPL_COMMAND_CLEAR[] = "clear";
void clean(void) { printf("\e[1;1H\e[2J"); }
static constexpr char REPL_COMMAND_HELP[] = "help";
void help(void) {
  printf("lifp is a LISP dialect. Its syntax is made of expressions enclosed "
         "in parentheses.\n"
         "Here's your first program:\n"
         "\n"
         "    (io.print (+ 1 2)) ; prints 3\n"
         "\n"
         "To learn about the functions and the variables available in this "
         "environment, use '?':\n"
         "\n"
         "    ?          ; lists functions in this environment\n"
         "    ? io.print ; show documentation and examples for io.print\n"
         "\n"
         "For more information, feedback, or bug reports "
         "https://github.com/shikaan/lifp\n");
}
static constexpr char REPL_COMMAND_MORE[] = "?";
void more(void) { printf("Error: not implemented yet!"); }

#define printError(Result, InputBuffer, Size, OutputBuffer)                    \
  int _concat(offset_, __LINE__) = 0;                                          \
  formatErrorMessage((Result)->message, (Result)->meta, "repl", InputBuffer,   \
                     Size, OutputBuffer, &_concat(offset_, __LINE__));         \
  fprintf(stdout, "%s\n", OutputBuffer);

#define tryREPL(Action, ...)                                                   \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    printError(&_concat(result, __LINE__), input, (int)OPTIONS.ouput_size,     \
               buffer);                                                        \
    continue;                                                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value;)

int repl(const repl_opts_t OPTIONS) {
  char buffer[OPTIONS.ouput_size];

  arena_t *ast_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.ast_memory), ast_arena,
         "unable to allocate interpreter memory");

  arena_t *temp_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.temp_memory), temp_arena,
         "unable to allocate transient memory");

  environment_t *global_environment = nullptr;
  tryCLI(vmInit(), global_environment, "unable to initialize virtual machine");

  linenoiseSetMultiLine(1);

  char welcome_message[256];
  formatVersion(256, welcome_message);
  printf("%s\nType 'help' for help. Press Ctrl+C to exit.\n\n",
         welcome_message);

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
      clean();
      continue;
    }

    if (strcmp(input, REPL_COMMAND_HELP) == 0) {
      help();
      continue;
    }

    if (strcmp(input, REPL_COMMAND_MORE) == 0) {
      more();
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
    formatValue(&result, (int)OPTIONS.ouput_size, buffer, &buffer_offset);
    printf("~> %s\n", buffer);

    memset(buffer, 0, OPTIONS.ouput_size);
  }
  profileEnd();
  environmentDestroy(&global_environment);
  arenaDestroy(&temp_arena);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryREPL
#undef printError
