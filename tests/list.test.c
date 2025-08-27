#include "../lib/list.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

typedef List(int) int_list_t;
static arena_t *test_arena;

int main(void) {
  result_ref_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.code == 0);
  test_arena = allocation.value;

  allocation = listCreate(int, test_arena, 1);
  assert(allocation.code == 0);
  int_list_t *list = allocation.value;

  case("empty");
  expectEqlSize(list->count, 0, "is empty");
  expectEqlSize(list->capacity, 1, "has correct capacity");
  
  case("after push");
  int item = 12;
  tryAssert(listAppend(int, list, &item));
  expectEqlSize(list->count, 1, "is not empty");
  expectEqlSize(list->capacity, 1, "has same capacity");
  expectEqlInt(listGet(int, list, list->count - 1), item, "has correct item");
  
  case("with resize");
  int item_2 = 2;
  tryAssert(listAppend(int, list, &item_2));
  expectEqlSize(list->count, 2, "has correct count");
  expectEqlSize(list->capacity, 2, "has updated capacity");
  
  case("with another resize");
  for (size_t i = 0; i < 4; i++) {
    tryAssert(listAppend(int, list, &item_2));
  }
  expectEqlSize(list->capacity, 8, "has updated capacity again");

  case("copy");
  int_list_t *another = nullptr;
  tryAssert(listCreate(int, test_arena, 1), another);
  tryAssert(listAppend(int, another, &item));
  result_void_t result = listCopy(int, list, another);
  expectEqlInt(result.code, RESULT_OK, "copies the list");
  expectEqlSize(list->count, 6, "updates count");
 
  List(char) *different = nullptr;
  tryAssert(listCreate(char, test_arena, 1), different); 
  result = listCopy(int, list, different);
  expectEqlInt(result.code, LIST_ERROR_INCOMPATIBLE_LISTS, "throws error");

  arenaDestroy(&test_arena);
  return report();
}
