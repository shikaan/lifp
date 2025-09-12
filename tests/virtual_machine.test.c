#include "test.h"
#include "utils.h"

#include "../lib/map.h"
#include "../lifp/virtual_machine.h"

void resolutions(void) {
  virtual_machine_t *machine;
  tryAssert(vmInit(VM_TEST_OPTIONS), machine);

  const value_t *builtin = environmentResolveSymbol(machine->global, "+");
  expectNotNull(builtin, "resolves builtin");
  expectEqlUint(builtin->type, VALUE_TYPE_BUILTIN, "with correct type");

  const value_t *special = environmentResolveSymbol(machine->global, "def!");
  expectNotNull(special, "resolves special");
  expectEqlUint(special->type, VALUE_TYPE_SPECIAL, "with correct type");

  value_t custom_value;
  valueInit(&custom_value, machine->arena, 12.0);

  mapSet(value_t, machine->global->values, "custom", &custom_value);
  const value_t *custom = environmentResolveSymbol(machine->global, "custom");
  expectNotNull(custom, "resolves custom");
  expectEqlUint(custom->type, VALUE_TYPE_NUMBER, "with correct type");
  vmStop();
}

void callStack(void) {
  vm_opts_t options = {
      .max_call_stack_size = 2,
      .environment_size = VM_TEST_OPTIONS.environment_size,
  };
  virtual_machine_t *machine;
  tryAssert(vmInit(options), machine);

  environment_t *env;
  tryAssert(environmentCreate(machine->arena, machine->global), env);
  (void)env;

  result_ref_t result;
  tryFail(environmentCreate(machine->arena, machine->global), result);
  expectIncludeString(result.message, "call stack size",
                      "throws when exceeding max call stack size");
}

int main(void) {
  suite(resolutions);
  suite(callStack);
  return report();
}
