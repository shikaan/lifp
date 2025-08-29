#include "list.h"
#include "arena.h"
#include "result.h"
#include <assert.h>
#include <string.h>

result_ref_t genericListCreate(arena_t *arena, size_t capacity,
                               size_t list_size, size_t item_size) {
  assert(arena);
  generic_list_t *list = nullptr;
  try(result_ref_t, arenaAllocate(arena, list_size), list);

  list->count = 0;
  list->capacity = capacity;
  list->item_size = item_size;
  list->arena = arena;
  try(result_ref_t, arenaAllocate(arena, item_size * list->capacity),
      list->data);

  return ok(result_ref_t, list);
}

result_void_t genericListAppend(generic_list_t *self, const void *item) {
  assert(self);
  if (!item) {
    return ok(result_void_t);
  }

  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity * 2;

    void *new_data = nullptr;
    try(result_void_t,
        arenaAllocate(self->arena, self->item_size * new_capacity), new_data);

    memmove(new_data, self->data, self->item_size * self->count);

    self->data = new_data;
    self->capacity = new_capacity;
  }

  void *destination = (byte_t *)self->data + (self->item_size * self->count);
  memmove(destination, item, self->item_size);
  self->count++;

  return ok(result_void_t);
}

void *genericListGet(const generic_list_t *self, size_t index) {
  assert(self);
  if (index >= self->count)
    return nullptr;

  return (byte_t *)self->data + (self->item_size * index);
}

result_void_t genericListCopy(const generic_list_t *source,
                              generic_list_t *destination) {
  assert(source);
  assert(destination);

  if (destination->item_size != source->item_size) {
    throw(result_void_t, LIST_ERROR_INCOMPATIBLE_LISTS, nullptr,
          "Lists have incompatible item sizes");
  }

  destination->item_size = source->item_size;

  for (size_t i = 0; i < source->count; i++) {
    void *source_node = genericListGet(source, i);
    try(result_void_t, genericListAppend(destination, source_node));
  }

  destination->count = source->count;
  return ok(result_void_t);
}

result_void_t genericListUnshift(generic_list_t *self) {
  assert(self);

  if (self->count <= 0) {
    throw(result_void_t, LIST_ERROR_EMPTY_LIST, nullptr,
          "Cannot unshift empty list");
  }

  memmove(self->data, (char *)self->data + self->item_size,
          self->item_size * (self->count - 1));
  self->count--;

  return ok(result_void_t);
}

void genericListClear(generic_list_t *self) { self->count = 0; }