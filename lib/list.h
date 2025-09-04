// List (v0.0.1)
// ---
//
// Generic list implementation with arena allocation.
//
// The list provides a type-safe list implementation using C macros for
// generic programming. It colocates all the elements in the list in the
// same memory chunk, allocated on the provided arena.
//
// ```c
// result_ref_t arena_result = arenaCreate(1024);
// if (arena_result.ok) {
//     arena_t *arena = arena_result.value;
//
//     // Create a map for integers
//     result_ref_t list_result = listCreate(int, arena, 16);
//     if (list_result.ok) {
//         List(int) *list = list_result.value;
//
//         // Append values
//         int value = 42;
//         listAppend(int, list, &value);
//
//         // Get values
//         int *retrieved = listGet(int, list, 0);
//         if (retrieved) {
//             printf("Value: %d\n", *retrieved); // Prints: Value: 42
//         }
//     }
//
//     arenaDestroy(&arena);
// }
// ```

#pragma once

#include "arena.h"
#include "result.h"

typedef enum {
  LIST_ERROR_ALLOCATION = ARENA_ERROR_OUT_OF_SPACE,
  LIST_ERROR_INCOMPATIBLE_LISTS,
  LIST_ERROR_EMPTY_LIST,
} list_error_t;

/**
 * Generic List structure macro.
 * Creates a type-safe list structure for the specified value type.
 * @name List
 * @param {Type} Type - The value type to store in the map
 * @example
 *   List(int) *int_map;
 *   List(char*) *string_map;
 */
// NOLINTBEGIN - Type cannot be put in parentheses
#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    arena_t *arena;                                                            \
    Type *data;                                                                \
  }
// NOLINTEND

typedef List(void) generic_list_t;

/**
 * Create a new list with the specified capacity.
 * @name listCreate
 * @param {Type} ItemType - The type of values to store in the list
 * @param {size_t} Capacity - Initial capacity of the map (should be power of 2)
 * @returns {result_ref_t} List pointer on success, or allocation error
 * @example
 *   result_ref_t result = listCreate(int, arena, 16);
 *   if (result.ok) {
 *       List(int) *list = result.value;
 *       // Use the map...
 *   }
 */
#define listCreate(ItemType, Arena, Capacity)                                  \
  genericListCreate(Arena, (Capacity), sizeof(List(ItemType)), sizeof(ItemType))

/**
 * Appends a value to the list.
 * @name listAppend
 * @param {Type} ItemType - The type of values to append in the list
 * @param {List(Type)*} List - Pointer to the list to modify
 * @param {Type*} Item - Pointer to the value to store
 * @returns {result_ref_t} Success, allocation error
 * @example
 *   int value = 42;
 *   result_void_t result = listAppend(int, list, &value);
 *   if (!result.ok) {
 *       // Handle error
 *   }
 */
// TODO: can this be made typesafe with static asserts?
#define listAppend(ItemType, List, Item)                                       \
  genericListAppend((generic_list_t *)(List), Item)

/**
 * Get a value from the list by index.
 * @name listGet
 * @param {Type} ItemType - The type of values stored in the list
 * @param {List(Type)*} List - Pointer to the list to search
 * @returns {Type*} Pointer to the value if found, nullptr if not found
 * @example
 *   int *value = listGet(int, list, "key");
 *   if (value) {
 *       printf("Found: %d\n", *value);
 *   } else {
 *       printf("Key not found\n");
 *   }
 */
// TODO: provide bound checks
#define listGet(ItemType, List, Index) (ItemType)(List)->data[Index]

/**
 * Remove the first element from the list (unshift operation).
 * @name listUnshift
 * @param {Type} ItemType - The type of values stored in the list
 * @param {List(Type)*} List - Pointer to the list
 * @returns {result_void_t} Success or error if the list is empty
 * @example
 *   result_void_t result = listUnshift(int, list);
 *   if (!result.ok) {
 *       // Handle error (e.g., empty list)
 *   }
 */
#define listUnshift(ItemType, List) genericListUnshift((generic_list_t *)(List))

#define listClear(ItemType, List) genericListClear((generic_list_t *)(List))

result_ref_t genericListCreate(arena_t *arena, size_t capacity,
                               size_t list_size, size_t item_size);
result_void_t genericListAppend(generic_list_t *self, const void *item);

void *genericListGet(const generic_list_t *self, size_t index);

result_void_t genericListUnshift(generic_list_t *self);

void genericListClear(generic_list_t *self);
