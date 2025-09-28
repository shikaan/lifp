#include "test.h"
#include "utils.h"

#include "../lifp/error.h"
#include "../lifp/virtual_machine.h"

static arena_t *test_arena;

void createDestroy(void) {
  vm_t *machine = nullptr;

  case("create");
  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  expectNotNull(machine, "creates a virtual machine");
  expectNotNull(machine->global, "machine has global environment");
  expectEqlSize(machine->options.vm_size, VM_TEST_OPTIONS.vm_size,
    "sets vm_size correctly");
  expectEqlSize(machine->options.environment_size,
    VM_TEST_OPTIONS.environment_size,
    "sets environment_size correctly");
      
  case("destroy");
  vmDestroy(&machine);
  expectNull(machine, "sets pointer to null");
}

void environmentCreateDestroy(void) {
  environment_t *env = nullptr;
  case("create env");
  tryAssert(environmentCreate(nullptr), env);
  expectNotNull(env, "creates an environment");
  expectEqlSize(env->values.capacity, 8, "initial map capacity");

  case("destroy env");
  environmentDestroy(&env);
  expectNull(env, "sets pointer to null");
}

void resolutions(void) {
  vm_t *machine;
  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);

  const value_t *builtin = environmentResolveSymbol(machine->global, "+");
  expectNotNull(builtin, "resolves builtin");
  expectEqlUint(builtin->type, VALUE_TYPE_BUILTIN, "with correct type");

  const value_t *special = environmentResolveSymbol(machine->global, "def!");
  expectNotNull(special, "resolves special");
  expectEqlUint(special->type, VALUE_TYPE_SPECIAL, "with correct type");

  value_t value = {VALUE_TYPE_NUMBER, .as.number = 12.0};
  tryAssert(environmentRegisterSymbol(machine->global, "twelve", &value));

  const value_t *custom = environmentResolveSymbol(machine->global, "twelve");
  expectNotNull(custom, "allows defining custom symbol");
  expectEqlUint(custom->type, VALUE_TYPE_NUMBER, "with correct type");
  expectEqlDouble(custom->as.number, 12.0, "with correct value");

  result_void_t result;
  tryFail(environmentRegisterSymbol(machine->global, "twelve", &value), result);
  expectEqlInt(result.code, ERROR_CODE_REFERENCE_SYMBOL_ALREADY_DEFINED,
               "prevents redefining symbol");

  environment_t *child;
  tryAssert(environmentCreate(machine->global), child);
  const value_t *child_value = environmentResolveSymbol(child, "twelve");
  expectNotNull(child_value, "resolves value form parent");
  expectEqlUint(child_value->type, VALUE_TYPE_NUMBER, "with correct type");
  expectEqlDouble(child_value->as.number, 12.0, "with correct value");

  vmDestroy(&machine);
}

int main(void) {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);

  suite(createDestroy);
  suite(environmentCreateDestroy);
  suite(resolutions);

  arenaDestroy(&test_arena);
  return report();
}
