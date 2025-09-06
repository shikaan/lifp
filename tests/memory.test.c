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
static arena_t *test_result_arena;
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
    tryAssert(tokenize(test_ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *ast = nullptr;
    tryAssert(parse(test_ast_arena, tokens, &offset, &depth), ast);

    value_t result;
    evaluate(&result, test_result_arena, test_temp_arena, ast, global);

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
  size_t usage = getArenaMemoryUsage(test_result_arena);

  case("inlined values");
  execute("3");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
  "not persisted for simple evaluations");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(+ 1 2 3)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
  "not persisted for builtins invocations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  execute("((fn (a b) (+ a b)) 10 20)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
  "not persisted for closure invocations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  execute("(+ (+ 1 2) (+ 3 4))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
  "not persisted for nested invocations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  case("allocated values");
  execute("(1 2 3 4 5)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + (sizeof(value_t) * 5) +
                    sizeof(List(value_t)),
                "persisted for simple evaluations");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(list.from 1 2 3 4 5)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + (sizeof(value_t) * 5) +
                    sizeof(List(value_t)),
                "persisted for builtin invocations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  execute("((fn (a) (1 a)) 2)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + (sizeof(value_t) * 2) +
                sizeof(List(value_t)),
                "persisted for closure invocations");
  usage = getArenaMemoryUsage(test_result_arena);

  // TODO: there is no way of nesting builtins for non stack allocated values yet
  
  arenaReset(test_ast_arena);
  arenaReset(test_result_arena);
}

void letBindingsMemory(void) {
  size_t usage = getArenaMemoryUsage(test_result_arena);

  case("inlined values");
  execute("(let ((x 10)) x)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for simple evaluations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  execute("(let ((x 10) (y 20)) (+ x y))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for builtin invocations");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(let ((x 1)) (let ((y 2)) (+ x y)))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted after nested let bindings");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(let ((f (fn (a) (+ a 1)))) (f 10))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted after let with closures");
  usage = getArenaMemoryUsage(test_result_arena);
  
  case("allocated values");
  execute("(let ((x 10)) (x))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), 
                usage + sizeof(List(value_t)) + sizeof(value_t),
                "persisted for simple evaluations");
  usage = getArenaMemoryUsage(test_result_arena);
  
  execute("(let ((x 10) (y 20)) (list.from x y))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t) * 2),
                "persisted for builtin invocations");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(let ((x 1)) (let ((y 2)) (x y)))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t) * 2),
                "persisted for nested let bindings");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(let ((f (fn (a) (+ a 1)))) (list.from (f 10)))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t)),
                "persisted for let with closures");

  arenaReset(test_ast_arena);
  arenaReset(test_result_arena);
}

void conditionalMemory(void) {
  size_t usage = getArenaMemoryUsage(test_result_arena);

  case("inlined values");
  execute("(cond ((= 1 1) 42) 0)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for conditional");

  execute("(cond ((= 1 2) (+ 1 2)) ((= 2 2) (+ 3 4)) (+ 5 6))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for multiple conditions");

  execute("(cond ((= 1 1) (let ((x 10)) x)) 0)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for conditional with let");
  
  case("allocated values");
  execute("(cond ((= 1 1) (42)) 0)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t)),
                "persisted for conditional");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(cond ((= 1 2) (1 2)) ((= 2 2) (3 4)) (5 6))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t) * 2),
                "persisted for multiple conditions");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("(cond ((= 1 1) (let ((x 10)) (x))) (0))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage + sizeof(List(value_t)) + (sizeof(value_t)),
                "persisted for conditional with let");

  arenaReset(test_ast_arena);
  arenaReset(test_result_arena);
}

void closureMemory(void) {
  size_t usage = getArenaMemoryUsage(test_result_arena);

  case("inlined values");
  execute("(def! multiply (fn (a b) (* a b)))");
  size_t after_def_temp = getArenaMemoryUsage(test_result_arena);

  execute("(multiply 5 6)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), after_def_temp,
                "not persisted for closure invocation");

  execute("(multiply 1 2)");
  execute("(multiply 3 4)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), after_def_temp,
                "not persisted for multiple closure invocations");

  execute("(def! complex (fn (x) (let ((y (+ x 1))) (cond ((= y 5) y) x))))");
  execute("(complex 4)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "not persisted for complex closure");

  case("allocated values");
  execute("(fn (a) a)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage 
                  + sizeof(node_t) // form
                  + (sizeof(char)*8) // form symbol
                  + sizeof(List(node_t)) + (sizeof(node_t) * 2) // arguments 
                  + (sizeof(char)*8), // argument symbol
                "persisted closure");
  usage = getArenaMemoryUsage(test_result_arena);

  execute("((fn (a) a) (1 2))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage 
                + sizeof(List(value_t)) + (sizeof(value_t) * 2),
                "only persists returned value on immediate invocations");
  usage = getArenaMemoryUsage(test_result_arena);

  execute(
    "(def! complex2 (fn (x) (let ((y (+ x 1))) (cond ((= y 5) (y)) (x)))))");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage,
                "not persisted for definitions");

  usage = getArenaMemoryUsage(test_result_arena);
  execute("(complex2 4)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena),
                usage 
                + sizeof(List(value_t)) + (sizeof(value_t)),
                "only persists returned value on persistent invocations");

  arenaReset(test_ast_arena);
  arenaReset(test_result_arena);
}

void recursiveMemory(void) {
  size_t usage = getArenaMemoryUsage(test_result_arena);

  case("inlined values");
  execute("(def! count (fn (n) (cond ((= n 0) 0) (count (- n 1)))))");
  execute("(count 20)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "no persistence for recursion");

  case("allocated values");
  execute("(def! count2 (fn (n) (cond ((= n 0) (0)) (count2 (- n 1)))))");
  execute("(count2 20)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), 
                usage 
                + sizeof(List(value_t)) + (sizeof(value_t)),
                "only persists returned value");
  
  arenaReset(test_ast_arena);
  arenaReset(test_result_arena);
}

void errorHandlingMemory(void) {
  size_t usage = getArenaMemoryUsage(test_result_arena);
  size_t initial_safe_alloc = getTotalAllocatedBytes();

  execute("(def! add2 (fn (a b) (+ a b)))");
  execute("(add2 1)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "cleans arena after arity error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from arity error");

  execute("undefined");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "cleans arena after undefined symbol error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from undefined symbol error");

  execute("((fn (1) a) 1)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "cleans arena after invalid closure invocation");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from invalid closure");

  execute("(let ((x undefined_var)) x)");
  expectEqlSize(getArenaMemoryUsage(test_result_arena), usage,
                "cleans arena after let binding error");
  expectEqlSize(getTotalAllocatedBytes(), initial_safe_alloc,
                "no memory leaks from let binding error");
}

void danglingArenas(void) {
  execute("((fn (a) a) 1 2)");
  expectEqlSize(getUsedArenas(), 4, "no dangling arena on failure");

  execute("(def! rec (fn (a) (cond ((= a 0) 1) (rec (- a 1)))))\n(rec 10)");
  expectEqlSize(getUsedArenas(), 4, "no dangling arena on recursive calls");

  execute("(def! rec (fn (a) (cond ((= a 0) 1) (lol (- a 1)))))\n(rec 10)");
  expectEqlSize(getUsedArenas(), 4, "no dangling arena on nested errors");
}

int main(void) {
  tryAssert(arenaCreate((size_t)(64 * 1024)), test_ast_arena);
  tryAssert(arenaCreate((size_t)(32 * 1024)), test_temp_arena);
  tryAssert(arenaCreate((size_t)(32 * 1024)), test_result_arena);
  tryAssert(vmInit(), global);

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
  printf("Error: This test can only run with PROFILE=1\n"
         "  Run again with:\n"
         "    make PROFILE=1 clean tests/memory.test && tests/memory.test\n");

  return 1;
}
#endif
