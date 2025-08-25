#include "node.h"
#include <string.h>

static constexpr size_t INITIAL_SIZE = 8;

result_ref_t nodeCreate(arena_t *arena, node_type_t type) {
  node_t *node = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(node_t)), node);

  if (type == NODE_TYPE_LIST) {
    node_list_t *list = nullptr;
    try(result_ref_t, listCreate(node_t, arena, INITIAL_SIZE), list);
    memcpy(&node->value.list, list, sizeof(node_list_t));
  }

  node->type = type;
  return ok(result_ref_t, node);
}

result_void_t nodeCopy(const node_t *source, node_t *destination) {
  destination->type = source->type;
  destination->position.column = source->position.column;
  destination->position.line = source->position.line;

  if (source->type == NODE_TYPE_LIST) {
    try(result_void_t,
        listCopy(node_t, &source->value.list, &destination->value.list));
  } else {
    destination->value = source->value;
  }

  return ok(result_void_t);
}

result_ref_t nodeClone(arena_t *arena, const node_t *source) {
  node_t *destination = nullptr;
  try(result_ref_t, nodeCreate(arena, source->type), destination);
  try(result_ref_t, nodeCopy(source, destination));
  return ok(result_ref_t, destination);
}
