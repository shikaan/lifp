#ifdef MEMORY_PROFILE
#include "../lib/arena.h"
#include "../lib/profile.h"
#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"
#include "test.h"
#include "utils.h"

#include <stddef.h>
allocMetricsInit();

static arena_t *test_ast_arena;
static arena_t *test_temp_arena;
static environment_t *global;

void execute(const char *input) {
  char input_copy[1024];
  strcpy(input_copy, input);

  char *line = strtok(input_copy, "\n");

  while (line != NULL) {
    arenaReset(test_ast_arena);
    arenaReset(test_temp_arena);
    token_list_t *tokens = nullptr;
    tryAssertAssign(tokenize(test_ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *ast = nullptr;
    tryAssertAssign(parse(test_ast_arena, tokens, &offset, &depth), ast);

    value_t result;
    evaluate(&result, test_temp_arena, ast, global);

    line = strtok(nullptr, "\n");
  }
}

size_t getUsedArenas(void) {
  size_t freed = 0;
  for (size_t i = 0; i < arena_metrics.arenas_count; i++) {
    if (arena_metrics.freed[i])
      freed++;
  }
  return arena_metrics.arenas_count - freed;
}

void danglingArenas(void) {
  execute("((fn (a) a) 1 2)");
  expectEqlSize(getUsedArenas(), 3, "no dangling arena on failure");

  execute("(def! rec (fn (a) (cond ((= a 0) 1) (rec (- a 1)))))\n(rec 10)");
  expectEqlSize(getUsedArenas(), 3, "no dangling arena on recursive calls");

  execute("(def! rec (fn (a) (cond ((= a 0) 1) (lol (- a 1)))))\n(rec 10)");
  expectEqlSize(getUsedArenas(), 3, "no dangling arena on nested errors");
}

int main(void) {
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_ast_arena);
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_temp_arena);
  tryAssertAssign(vmInit(), global);

  profileInit();

  suite(danglingArenas);

  return report();
}
#else
#include <stdio.h>

int main(void) {
  printf("Error: This test can only with PROFILE=1\n"
         "  Run again with:\n"
         "    make PROFILE=1 clean tests/memory.test && tests/memory.test\n");

  return 1;
}
#endif
