#include "../lib/arena.h"
#include "../lib/profile.h"
#include "../lib/string.h"
#include "../lifp/evaluate.h"
#include "../lifp/fmt.h"
#include "../lifp/node.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"
#include "../vendor/linenoise/linenoise.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern Map(value_t) * builtins;
extern Map(value_t) * specials;

const char REPL[] = "repl";

typedef struct {
  size_t ast_memory;
  size_t temp_memory;
  size_t output_size;
  size_t environment_size;
} repl_opts_t;

static constexpr char REPL_COMMAND_CLEAR[] = "clear";
void clean(void) { linenoiseClearScreen(); }
static constexpr char REPL_COMMAND_HELP[] = "help";
void help(void) {
  printf("lifp is practical functional programming language belonging to the "
         "LISP family. It features a REPL, file execution, a standard library"
         " and modern conveniences. Here's your first program:\n"
         "\n"
         "    (io:stdout! \"Hello world!\") ; prints \"Hello World\"\n"
         "\n"
         "To learn about the symbols in this environment, use '?':\n"
         "\n"
         "    ?           ; lists symbols in this environment\n"
         "\n"
         "This REPL supports Tab completion: hit <Tab> for auto-complete.\n"
         "\n"
         "For more information, feedback, or bug reports "
         "https://github.com/shikaan/lifp\n");
}

static char completions[64][32] = {};
static size_t completions_count = 0;

static void fillCompletions(environment_t *env) {
  completions_count = 0;
  for (size_t i = 0; i < env->values->capacity; i++) {
    if (env->values->used[i]) {
      memset(completions[completions_count], 0, 32);
      stringCopy(completions[completions_count], env->values->keys[i], 32);
      completions_count++;
      if (completions_count == 64)
        return;
    }
  }

  for (size_t i = 0; i < builtins->capacity; i++) {
    if (builtins->used[i]) {
      memset(completions[completions_count], 0, 32);
      stringCopy(completions[completions_count], builtins->keys[i], 32);
      completions_count++;
      if (completions_count == 64)
        return;
    }
  }

  for (size_t i = 0; i < specials->capacity; i++) {
    if (specials->used[i]) {
      memset(completions[completions_count], 0, 32);
      stringCopy(completions[completions_count], specials->keys[i], 32);
      completions_count++;
      if (completions_count == 64)
        return;
    }
  }
}

char completion_buffer[2048] = {};
static void setCompletions(const char *input, linenoiseCompletions *lnc) {
  memset(completion_buffer, 0, 2048);
  size_t input_len = strlen(input);
  const char *last_token = nullptr;

  if (input_len == 0) {
    last_token = input;
  } else {
    char last = input[input_len - 1];

    if (last == '(') {
      stringConcat(2048, completion_buffer, input_len, input, 1, ")");
      linenoiseAddCompletion(lnc, completion_buffer);
      return;
    }

    // This allows completion within parenthesis, e.g. (something
    // COMPLETE_HERE). It goes from the end of the string and stops at blank
    // spaces or opening parens to identify the last token the user is typing in
    for (int i = (int)input_len - 1; i >= 0; i--) {
      char current = input[i];
      if (current == '(' || isspace(current)) {
        last_token = &input[i];
        break;
      }
    }

    last_token = last_token ? last_token + 1 : input;
  }

  size_t last_token_len = strlen(last_token);
  for (size_t i = 0; i < completions_count; i++) {
    char *completion = completions[i];
    if (stringStartsWith(completion, last_token)) {
      size_t len = input_len - last_token_len;
      stringConcat(2048, completion_buffer, len, input, 32, completion);
      linenoiseAddCompletion(lnc, completion_buffer);
    }
  }
}

static constexpr char REPL_COMMAND_MORE[] = "?";
void more(const char *input) {
  (void)input;
  size_t term_width = 64;
  struct winsize winsize;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) == 0) {
    term_width = winsize.ws_col;
  }

  size_t divider = term_width / 33U;

  for (size_t i = 0; i < completions_count; i++) {
    if (i % divider == 0)
      printf("\n");
    printf("%-32s ", completions[i]);
  }
  printf("\n");
}

#define printError(Result, InputBuffer, Size, OutputBuffer)                    \
  int _concat(offset_, __LINE__) = 0;                                          \
  formatErrorMessage((Result)->message, (Result)->meta, "repl", InputBuffer,   \
                     Size, OutputBuffer, &_concat(offset_, __LINE__));         \
  fprintf(stdout, "%s\n", OutputBuffer);

#define tryREPL(Action, ...)                                                   \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    printError(&_concat(result, __LINE__), input, (int)OPTIONS.output_size,    \
               buffer);                                                        \
    continue;                                                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value;)

int repl(const repl_opts_t OPTIONS) {
  char buffer[OPTIONS.output_size];

  arena_t *ast_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.ast_memory), ast_arena,
         "unable to allocate interpreter memory");

  arena_t *scratch_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.temp_memory / 2), scratch_arena,
         "unable to allocate transient memory");

  arena_t *result_arena = nullptr;
  tryCLI(arenaCreate(OPTIONS.temp_memory / 4), result_arena,
         "unable to allocate transient memory");

  vm_options_t vm_options = {
      .environment_size = OPTIONS.environment_size,
      .vm_size = OPTIONS.temp_memory / 4,
  };
  vm_t *machine = nullptr;
  tryCLI(vmCreate(vm_options), machine, "unable to initialize virtual machine");

  linenoiseSetMultiLine(1);

  char welcome_message[256];
  formatVersion(256, welcome_message);
  printf("%s\nType 'help' for help. Press Ctrl+C to exit.\n\n",
         welcome_message);

  linenoiseSetCompletionCallback(setCompletions);

  fillCompletions(machine->global);

  profileInit();
  while (true) {
    profileReport();
    arenaReset(ast_arena);
    arenaReset(scratch_arena);
    arenaReset(result_arena);
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

    if (strstr(input, REPL_COMMAND_MORE) != nullptr && input[0] == '?') {
      more(input);
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
    tryREPL(
        evaluate(&result, result_arena, scratch_arena, ast, machine->global));

    int buffer_offset = 0;
    formatValue(&result, (int)OPTIONS.output_size, buffer, &buffer_offset);
    printf("~> %s\n", buffer);

    fillCompletions(machine->global);
    memset(buffer, 0, OPTIONS.output_size);
  }
  profileEnd();
  arenaDestroy(&result_arena);
  arenaDestroy(&scratch_arena);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryREPL
#undef printError
