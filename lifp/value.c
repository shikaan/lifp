#include "value.h"
#include "node.h"
#include <string.h>

result_ref_t valueCreateInit(arena_t *arena, value_type_t type,
                             node_type_t form_type) {
  value_t *value = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  try(result_ref_t, valueInit(value, arena, type, form_type));
  return ok(result_ref_t, value);
}

result_ref_t valueCreate(arena_t *arena) {
  value_t *value = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  return ok(result_ref_t, value);
}

result_void_t valueInit(value_t *value, arena_t *arena, value_type_t type,
                        node_type_t form_type) {
  value->type = type;

  if (value->type == VALUE_TYPE_LIST) {
    frame_handle_t frame = arenaAllocationFrameStart(arena);
    value_list_t *list = nullptr;
    try(result_void_t, listCreate(value_t, arena, VALUE_LIST_INITIAL_SIZE),
        list);
    memcpy(&value->value.list, list, sizeof(value_list_t));
    arenaAllocationFrameEnd(arena, frame);
  }

  if (value->type == VALUE_TYPE_CLOSURE) {
    frame_handle_t frame = arenaAllocationFrameStart(arena);
    value_list_t *list = nullptr;
    try(result_void_t, listCreate(node_t, arena, VALUE_LIST_INITIAL_SIZE),
        list);
    memcpy(&value->value.closure.arguments, list, sizeof(value_list_t));

    node_t *form = nullptr;
    try(result_void_t, nodeCreate(arena, form_type), form);
    memcpy(&value->value.closure.form, form, sizeof(node_t));
    arenaAllocationFrameEnd(arena, frame);
  }

  return ok(result_void_t);
}

result_void_t valueCopy(value_t *source, value_t *destination,
                        arena_t *destination_arena) {
  node_type_t form_type = 0;
  if (source->type == VALUE_TYPE_CLOSURE) {
    form_type = source->value.closure.form.type;
  }

  try(result_void_t,
      valueInit(destination, destination_arena, source->type, form_type));
  destination->position.line = source->position.line;
  destination->position.column = source->position.column;

  switch (source->type) {
  case VALUE_TYPE_BOOLEAN:
    destination->value.boolean = source->value.boolean;
    break;
  case VALUE_TYPE_INTEGER:
    destination->value.integer = source->value.integer;
    break;
  case VALUE_TYPE_BUILTIN:
    destination->value.builtin = source->value.builtin;
    break;
  case VALUE_TYPE_NIL:
    destination->value.nil = source->value.nil;
    break;
  case VALUE_TYPE_CLOSURE:
    nodeCopy(&source->value.closure.form, &destination->value.closure.form);
    try(result_void_t, listCopy(value_t, &source->value.closure.arguments,
                                &destination->value.closure.arguments));
    break;
  case VALUE_TYPE_LIST:
    try(result_void_t,
        listCopy(value_t, &source->value.list, &destination->value.list));
    break;
  default:
    unreachable();
  }

  return ok(result_void_t);
}
