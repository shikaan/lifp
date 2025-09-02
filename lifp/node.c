#include "node.h"
#include "../lib/string.h"
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

result_void_t nodeCopy(const node_t *source, node_t *destination,
                       arena_t *destination_arena) {
  destination->type = source->type;
  destination->position.column = source->position.column;
  destination->position.line = source->position.line;

  if (source->type == NODE_TYPE_LIST) {
    for (size_t i = 0; i < source->value.list.count; i++) {
      node_t value = source->value.list.data[i];
      node_t *dupe;
      try(result_void_t, nodeCreate(destination_arena, value.type), dupe);
      try(result_void_t, nodeCopy(&value, dupe, destination_arena));
      try(result_void_t, listAppend(node_t, &destination->value.list, dupe));
    }
  } else if (source->type == NODE_TYPE_SYMBOL ||
             source->type == NODE_TYPE_STRING) {
    const string_t src_str = (source->type == NODE_TYPE_SYMBOL)
                                 ? source->value.symbol
                                 : source->value.string;
    size_t len = strlen(src_str) + 1;
    string_t dest_str;
    try(result_void_t, arenaAllocate(destination_arena, len), dest_str);
    stringCopy(dest_str, src_str, len);
    if (source->type == NODE_TYPE_SYMBOL) {
      destination->value.symbol = dest_str;
    } else {
      destination->value.string = dest_str;
    }
  } else {
    destination->value = source->value;
  }

  return ok(result_void_t);
}

result_ref_t nodeClone(arena_t *arena, const node_t *source) {
  node_t *destination = nullptr;
  try(result_ref_t, nodeCreate(arena, source->type), destination);
  try(result_ref_t, nodeCopy(source, destination, arena));
  return ok(result_ref_t, destination);
}
