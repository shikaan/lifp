#include "../artifacts/docs.h"
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

extern value_map_t *builtins;
extern value_map_t *specials;

const char REPL[] = "repl";

typedef struct {
  size_t ast_memory;
  size_t output_size;
} repl_opts_t;

static constexpr size_t MAX_COMPELTIONS = 64;
static char completions[MAX_COMPELTIONS][MAX_SYMBOL_LENGTH] = {};
static size_t completions_count = 0;

static void sort(size_t len, char array[static len][MAX_SYMBOL_LENGTH]) {
  char aux[MAX_SYMBOL_LENGTH] = {};
  for (size_t i = 1; i < len; i++) {
    for (size_t j = i; j > 0; j--) {
      if (strcmp(array[j], array[j - 1]) < 0) {
        stringCopy(aux, array[j], MAX_SYMBOL_LENGTH);
        stringCopy(array[j], array[j - 1], MAX_SYMBOL_LENGTH);
        stringCopy(array[j - 1], aux, MAX_SYMBOL_LENGTH);
      }
    }
  }
}

static void fillCompletions(environment_t *env) {
  completions_count = 0;
  for (size_t i = 0; i < env->values.capacity; i++) {
    if (env->values.used[i]) {
      memset(completions[completions_count], 0, MAX_SYMBOL_LENGTH);
      stringCopy(completions[completions_count], env->values.keys[i],
                 MAX_SYMBOL_LENGTH);
      completions_count++;
      if (completions_count == MAX_COMPELTIONS)
        goto done;
    }
  }

  for (size_t i = 0; i < builtins->capacity; i++) {
    if (builtins->used[i]) {
      memset(completions[completions_count], 0, MAX_SYMBOL_LENGTH);
      stringCopy(completions[completions_count], builtins->keys[i],
                 MAX_SYMBOL_LENGTH);
      completions_count++;
      if (completions_count == MAX_COMPELTIONS)
        goto done;
    }
  }

  for (size_t i = 0; i < specials->capacity; i++) {
    if (specials->used[i]) {
      memset(completions[completions_count], 0, MAX_SYMBOL_LENGTH);
      stringCopy(completions[completions_count], specials->keys[i],
                 MAX_SYMBOL_LENGTH);
      completions_count++;
      if (completions_count == MAX_COMPELTIONS)
        goto done;
    }
  }
done:
  sort(completions_count, completions);
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
         "    ?            ; lists symbols in this environment\n"
         "    ? io:stdout! ; shows documentation about io:stdout!\n"
         "\n"
         "This REPL supports Tab completion: hit <Tab> for auto-complete.\n"
         "\n"
         "For more information, feedback, or bug reports "
         "https://github.com/shikaan/lifp\n");
}

static char format_buffer[1024] = {0};
static constexpr char REPL_COMMAND_MORE[] = "?";
void more(const char *input, environment_t *env) {
  if (strlen(input) <= 2) {
    size_t term_width = 64;
    struct winsize winsize;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) == 0) {
      term_width = winsize.ws_col;
    }

    size_t divider = term_width / MAX_SYMBOL_LENGTH;

    for (size_t i = 0; i < completions_count; i++) {
      if (i % divider == 0)
        printf("\n");
      printf("%-*s ", (int)MAX_SYMBOL_LENGTH, completions[i]);
    }
    printf("\n");
    return;
  }

  if (input[1] != ' ') {
    printf("Error: invalid whitespace separator. Usage: '? symbol-name'.\n");
    return;
  }

  const char *symbol = &input[2];

  for (size_t i = 0; i < DOCS_COUNT; i++) {
    doc_record_t record = DOCS[i];
    if (strcmp(symbol, record.name) == 0) {
      printf("  %s\n\n  Example:\n    %s\n\n", record.description,
             record.example);
      return;
    }
  }

  const value_t *value = environmentResolveSymbol(env, symbol);
  if (value) {
    int offset = 0;
    formatValue(value, 1024, format_buffer, &offset);
    printf("%s :: (%s) ~> %s\n", symbol, formatValueType(value->type),
           format_buffer);
  } else {
    printf("Error: symbol '%s' not found. Use '?' to see all symbols in "
           "current environment.\n",
           symbol);
  }
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

  vm_t *machine = nullptr;
  tryCLI(vmCreate(), machine, "unable to initialize virtual machine");

  linenoiseSetMultiLine(1);

  char welcome_message[256];
  formatVersion(256, welcome_message);
  printf("%s\nType 'help' for help. Press Ctrl+C to exit.\n\n",
         welcome_message);

  linenoiseSetCompletionCallback(setCompletions);

  fillCompletions(machine->global);

  value_t *result = nullptr;
  profileInit();
  while (true) {
    allocResetMetrics();

    arenaReset(ast_arena);
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
      more(input, machine->global);
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

    tryREPL(evaluate(ast, machine->global), result);

    int buffer_offset = 0;
    formatValue(result, (int)OPTIONS.output_size, buffer, &buffer_offset);
    printf("~> %s\n", buffer);

    fillCompletions(machine->global);

    valueDestroy(&result);
    deallocSafe(&input);
    memset(buffer, 0, OPTIONS.output_size);
    profileReport();
  }

  puts("post while");

  profileEnd();
  valueDestroy(&result);
  vmDestroy(&machine);
  arenaDestroy(&ast_arena);
  return 0;
}

#undef tryREPL
#undef printError
