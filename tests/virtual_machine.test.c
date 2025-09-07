#include "test.h"
#include "utils.h"

#include "../lib/map.h"
#include "../lifp/virtual_machine.h"

static environment_t *global;
static arena_t *test_arena;

void resolutions(void) {
  tryAssert(vmInit(VM_TEST_OPTIONS), global);

  const value_t *builtin = environmentResolveSymbol(global, "+");
  expectNotNull(builtin, "resolves builtin");
  expectEqlUint(builtin->type, VALUE_TYPE_BUILTIN, "with correct type");

  const value_t *special = environmentResolveSymbol(global, "def!");
  expectNotNull(special, "resolves special");
  expectEqlUint(special->type, VALUE_TYPE_SPECIAL, "with correct type");

  value_t custom_value;
  valueInit(&custom_value, test_arena, 12.0);

  mapSet(value_t, global->values, "custom", &custom_value);
  const value_t *custom = environmentResolveSymbol(global, "custom");
  expectNotNull(custom, "resolves custom");
  expectEqlUint(custom->type, VALUE_TYPE_NUMBER, "with correct type");

  environmentDestroy(&global);
}

void callStack(void) {
  vm_opts_t options = {
      .max_call_stack_size = 2,
      .environment_size = VM_TEST_OPTIONS.environment_size,
  };
  tryAssert(vmInit(options), global);

  environment_t *env;
  tryAssert(environmentCreate(global), env);

  result_ref_t result;
  tryFail(environmentCreate(global), result);
  expectIncludeString(result.message, "call stack size",
                      "throws when exceeding max call stack size");

  environmentDestroy(&env);
  environmentDestroy(&global);
}

int main(void) {
  tryAssert(arenaCreate((size_t)1024 * 1024), test_arena);
  suite(resolutions);
  suite(callStack);
  arenaDestroy(&test_arena);
  return report();
}
