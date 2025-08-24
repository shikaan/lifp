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
#include <string.h>
allocMetricsInit();

static arena_t *test_ast_arena;
static arena_t *test_temp_arena;
static environment_t *global;

// This execute function does stack allocations, hence why we expect no
// allocations in the following tests
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

size_t getTotalAllocatedBytes(void) { return allocGetMetrics().bytes; }

size_t getArenaMemoryUsage(arena_t *arena) { return arena->offset; }

void transientAllocations(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);

  execute("(+ 1 2 3)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after simple invocation");

  execute("(def! add (fn (a b) (+ a b)))");
  execute("(add 10 20)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after closure invocation");

  execute("(+ (+ 1 2) (+ 3 4))");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after nested evaluations");

  execute("(1 2 3 4 5)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after list evaluation");

  arenaReset(test_ast_arena);
  arenaReset(test_temp_arena);
}

void letBindingsMemory(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);

  execute("(let ((x 10) (y 20)) (+ x y))");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after let binding");

  execute("(let ((x 1)) (let ((y 2)) (+ x y)))");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after nested let bindings");

  execute("(let ((f (fn (a) (+ a 1)))) (f 10))");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after let with closures");

  arenaReset(test_ast_arena);
  arenaReset(test_temp_arena);
}

void conditionalMemory(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);

  execute("(cond ((= 1 1) 42) 0)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after conditional");

  execute("(cond ((= 1 2) (+ 1 2)) ((= 2 2) (+ 3 4)) (+ 5 6))");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after multiple conditions");

  execute("(cond ((= 1 1) (let ((x 10)) x)) 0)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after conditional with let");

  arenaReset(test_ast_arena);
  arenaReset(test_temp_arena);
}

void closureMemory(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);

  execute("(def! multiply (fn (a b) (* a b)))");
  size_t after_def_temp = getArenaMemoryUsage(test_temp_arena);

  execute("(multiply 5 6)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), after_def_temp,
                "cleans arena after closure invocation");

  execute("(multiply 1 2)");
  execute("(multiply 3 4)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), after_def_temp,
                "cleans arena after multiple closure invocations");

  execute("(def! complex (fn (x) (let ((y (+ x 1))) (cond ((= y 5) y) x))))");
  execute("(complex 4)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after complex closure");
}

void recursiveMemory(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);

  execute("(def! countdown (fn (n) (cond ((= n 0) 0) (countdown (- n 1)))))");
  execute("(countdown 5)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after recursive calls");

  execute("(countdown 20)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after deep recursion");
}

void errorHandlingMemory(void) {
  size_t initial_temp_usage = getArenaMemoryUsage(test_temp_arena);
  size_t initial_safe_alloc = getTotalAllocatedBytes();

  execute("(def! add2 (fn (a b) (+ a b)))");
  execute("(add2 1)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after arity error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from arity error");

  execute("undefined");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after undefined symbol error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from undefined symbol error");

  execute("((fn (1) a) 1)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after invalid closure invocation");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from invalid closure");

  execute("(let ((x undefined_var)) x)");
  expectEqlSize(getArenaMemoryUsage(test_temp_arena), initial_temp_usage,
                "cleans arena after let binding error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from let binding error");
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

  suite(transientAllocations);
  suite(letBindingsMemory);
  suite(conditionalMemory);
  suite(closureMemory);
  suite(recursiveMemory);
  suite(errorHandlingMemory);
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
