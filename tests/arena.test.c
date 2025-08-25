#include "../lib/arena.h"
#include "test.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void basic() {
  result_ref_t creation = arenaCreate(1024);
  expectTrue(creation.code == 0, "succeeds arena creation");

  arena_t *arena = creation.value;
  expectNotNull(arena, "arena is not null");

  result_ref_t allocation = arenaAllocate(arena, 100);
  expectTrue(allocation.code == 0, "succeeds arena creation");
  expectNotNull(allocation.value, "allocated memory is not be null");

  case("reset");
  arenaReset(arena);
  
  result_ref_t reallocation = arenaAllocate(arena, 1024);
  expectTrue(reallocation.code == 0, "can reallocate memory after a reset");
  
  arenaDestroy(&arena);
}

void overflow() {
  result_ref_t creation = arenaCreate(100);
  expectTrue(creation.code == 0, "succeeds arena creation");
  
  arena_t *arena = creation.value;
  
  result_ref_t allocation = arenaAllocate(arena, 200);
  expectFalse(allocation.code == 0, "fails oversized allocation");
  expectTrue(strstr(allocation.message, "Available 100, requested 200, total 100") != NULL, "throws correct exception");
  
  arenaDestroy(&arena);
}

void alignment() {
  result_ref_t creation = arenaCreate(1024);
  assert(creation.code == 0);
  arena_t *arena = creation.value;

  for (size_t i = 0; i < 16; i++) {
    result_ref_t allocation = arenaAllocate(arena, i);
    assert(allocation.code == 0);
    uintptr_t pointer = (uintptr_t)allocation.value;
    expectEqlUint(pointer % 8, 0, "address is 8-aligned");
  }

  arenaDestroy(&arena);
}

int main() {
  suite(basic);
  suite(overflow);
  suite(alignment);
  return report();
}
