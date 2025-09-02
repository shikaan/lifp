#include "value.h"
#include "../lib/string.h"
#include "node.h"
#include <string.h>

result_ref_t valueCreate(arena_t *arena) {
  value_t *value = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  return ok(result_ref_t, value);
}

result_void_t valueInit(value_t *value, arena_t *arena, value_type_t type,
                        node_type_t form_type) {
  value->type = type;

  if (value->type == VALUE_TYPE_LIST) {
    value_list_t *list = nullptr;
    try(result_void_t, listCreate(value_t, arena, VALUE_LIST_INITIAL_SIZE),
        list);
    memcpy(&value->value.list, list, sizeof(value_list_t));
  }

  if (value->type == VALUE_TYPE_CLOSURE) {
    value_list_t *arguments = nullptr;
    try(result_void_t, listCreate(node_t, arena, VALUE_LIST_INITIAL_SIZE),
        arguments);
    memcpy(&value->value.closure.arguments, arguments, sizeof(value_list_t));

    node_t *form = nullptr;
    try(result_void_t, nodeCreate(arena, form_type), form);
    memcpy(&value->value.closure.form, form, sizeof(node_t));
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
  case VALUE_TYPE_NUMBER:
    destination->value.number = source->value.number;
    break;
  case VALUE_TYPE_BUILTIN:
    destination->value.builtin = source->value.builtin;
    break;
  case VALUE_TYPE_SPECIAL:
    destination->value.special = source->value.special;
    break;
  case VALUE_TYPE_NIL:
    destination->value.nil = source->value.nil;
    break;
  case VALUE_TYPE_STRING:
    size_t len = strlen(source->value.string);
    string_t string;
    try(result_void_t,
        arenaAllocate(destination_arena, (len + 1) * sizeof(char)), string);
    stringCopy(string, source->value.string, len + 1);
    destination->value.string = string;
    break;
  case VALUE_TYPE_CLOSURE:
    try(result_void_t,
        nodeCopy(&source->value.closure.form, &destination->value.closure.form,
                 destination_arena));
    for (size_t i = 0; i < source->value.closure.arguments.count; i++) {
      node_t value = source->value.closure.arguments.data[i];
      node_t duplicated;
      try(result_void_t, nodeCopy(&value, &duplicated, destination_arena));
      try(result_void_t,
          listAppend(node_t, &destination->value.closure.arguments,
                     &duplicated));
    }
    break;
  case VALUE_TYPE_LIST: {
    for (size_t i = 0; i < source->value.list.count; i++) {
      value_t value = listGet(value_t, &source->value.list, i);
      value_t duplicated;
      try(result_void_t, valueCopy(&value, &duplicated, destination_arena));
      try(result_void_t,
          listAppend(value_t, &destination->value.list, &duplicated));
    }
    break;
  }
  default:
    unreachable();
  }

  return ok(result_void_t);
}
