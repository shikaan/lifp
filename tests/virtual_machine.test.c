#include "test.h"
#include "utils.h"

#include "../lib/map.h"
#include "../lifp/virtual_machine.h"

static environment_t *global;
static arena_t *test_arena;

void resolutions() {
  tryAssert(vmInit(), global);

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
}

int main(void) {
  tryAssert(arenaCreate((size_t)1024 * 1024), test_arena);
  suite(resolutions);
  arenaDestroy(&test_arena);
  environmentDestroy(&global);
  return report();
}
