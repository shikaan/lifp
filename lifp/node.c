#include "node.h"
#include "position.h"
#include <stddef.h>
#include <string.h>

static constexpr size_t INITIAL_SIZE = 8;

result_ref_t nodeCreate(arena_t *arena, node_type_t type) {
  node_t *node = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(node_t)), node);
  node->type = type;
  return ok(result_ref_t, node);
}

result_void_t nodeInit(node_t *self, arena_t *arena) {
  if (self->type == NODE_TYPE_LIST) {
    node_list_t *list = nullptr;
    try(result_void_t, listCreate(node_t, arena, INITIAL_SIZE), list);
    memcpy(&self->value.list, list, sizeof(node_list_t));
  }
  return ok(result_void_t);
}

result_ref_t nodeCopy(const node_t *self) {
  node_t *destination;
  try(result_ref_t, allocSafe(sizeof(node_t)), destination);

  destination->type = self->type;
  destination->position.column = self->position.column;
  destination->position.line = self->position.line;

  switch (self->type) {
  case NODE_TYPE_LIST: {
    destination->value.list.arena = nullptr;
    destination->value.list.capacity = self->value.list.count;
    destination->value.list.count = self->value.list.count;
    destination->value.list.item_size = sizeof(node_t);

    try(result_ref_t, allocSafe(sizeof(node_t) * self->value.list.count),
        destination->value.list.data);

    for (size_t i = 0; i < self->value.list.count; i++) {
      node_t *copy = nullptr;
      try(result_ref_t, nodeCopy(&self->value.list.data[i]), copy);
      destination->value.list.data[i] = *copy;
      deallocSafe(&copy);
    }
    break;
  }
  case NODE_TYPE_SYMBOL: {
    string_t dest_str = strdup(self->value.symbol);
    destination->value.symbol = dest_str;
    break;
  }
  case NODE_TYPE_STRING: {
    string_t dest_str = strdup(self->value.string);
    destination->value.string = dest_str;
    break;
  }
  case NODE_TYPE_NUMBER:
  case NODE_TYPE_BOOLEAN:
  case NODE_TYPE_NIL:
  default:
    destination->value = self->value;
    break;
  }

  return ok(result_ref_t, destination);
}

static void nodeDestroyInner(node_t *self) {
  if (!self)
    return;

  switch (self->type) {
  case NODE_TYPE_LIST: {
    for (size_t i = 0; i < self->value.list.count; i++) {
      node_t node = listGet(node_t, &self->value.list, i);
      nodeDestroyInner(&node);
    }
    deallocSafe(&self->value.list.data);
    break;
  }
  case NODE_TYPE_SYMBOL:
    deallocSafe(&self->value.symbol);
    break;
  case NODE_TYPE_STRING:
    deallocSafe(&self->value.string);
    break;
  case NODE_TYPE_NUMBER:
  case NODE_TYPE_BOOLEAN:
  case NODE_TYPE_NIL:
  default:
    break;
  }
}

void nodeDestroy(node_t **self) {
  if (!self || !(*self))
    return;

  node_t *node = *self;
  nodeDestroyInner(node);
  deallocSafe(self);
}
