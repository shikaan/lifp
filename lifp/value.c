#include "value.h"
#include "../lib/string.h"
#include "node.h"
#include "types.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

result_ref_t valueCreate(arena_t *arena, value_type_t type) {
  value_t *value = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  value->type = type;
  value->position.column = 0;
  value->position.line = 0;
  return ok(result_ref_t, value);
}

result_void_t valueInitList(value_t *self, arena_t *arena, size_t size) {
  self->type = VALUE_TYPE_LIST;
  value_list_t *list = nullptr;
  try(result_void_t, listCreate(value_t, arena, size), list);
  memcpy(&self->value.list, list, sizeof(value_list_t));
  return ok(result_void_t);
}

result_void_t valueInitClosure(value_t *self, arena_t *arena,
                               node_type_t form_type) {
  self->type = VALUE_TYPE_CLOSURE;
  node_list_t *arguments = nullptr;
  try(result_void_t, listCreate(node_t, arena, 2), arguments);
  self->value.closure.arguments = *arguments;

  node_t *form = nullptr;
  try(result_void_t, nodeCreate(arena, form_type), form);
  try(result_void_t, nodeInit(form, arena));
  self->value.closure.form = *form;

  self->value.closure.captured_environment = nullptr;

  return ok(result_void_t);
}

result_void_t valueInitBoolean(value_t *self, arena_t *arena, bool value) {
  (void)arena;
  self->type = VALUE_TYPE_BOOLEAN;
  self->value.boolean = value;
  return ok(result_void_t);
}

result_void_t valueInitNumber(value_t *self, arena_t *arena, number_t value) {
  (void)arena;
  self->type = VALUE_TYPE_NUMBER;
  self->value.number = value;
  return ok(result_void_t);
}

result_void_t valueInitNil(value_t *self, arena_t *arena, nullptr_t value) {
  (void)arena;
  self->type = VALUE_TYPE_NIL;
  self->value.nil = value;
  return ok(result_void_t);
}

result_void_t valueInitBuiltin(value_t *self, arena_t *arena, builtin_t value) {
  (void)arena;
  self->type = VALUE_TYPE_BUILTIN;
  self->value.builtin = value;
  return ok(result_void_t);
}

result_void_t valueInitSpecial(value_t *self, arena_t *arena,
                               special_form_t value) {
  (void)arena;
  self->type = VALUE_TYPE_SPECIAL;
  self->value.special = value;
  return ok(result_void_t);
}

result_void_t valueInitString(value_t *self, arena_t *arena,
                              const char *value) {
  self->type = VALUE_TYPE_STRING;
  size_t len = strlen(value);
  string_t string;
  try(result_void_t, arenaAllocate(arena, (len + 1) * sizeof(char)), string);
  stringCopy(string, value, len + 1);
  self->value.string = string;
  return ok(result_void_t);
}

result_void_t valueCopy(const value_t *source, value_t *destination,
                        arena_t *destination_arena) {
  destination->position.line = source->position.line;
  destination->position.column = source->position.column;

  switch (source->type) {
  case VALUE_TYPE_BOOLEAN:
    try(result_void_t, valueInitBoolean(destination, destination_arena,
                                        source->value.boolean));
    break;
  case VALUE_TYPE_NUMBER:
    try(result_void_t,
        valueInitNumber(destination, destination_arena, source->value.number));
    break;
  case VALUE_TYPE_BUILTIN:
    try(result_void_t, valueInitBuiltin(destination, destination_arena,
                                        source->value.builtin));
    break;
  case VALUE_TYPE_SPECIAL:
    try(result_void_t, valueInitSpecial(destination, destination_arena,
                                        source->value.special));
    break;
  case VALUE_TYPE_NIL:
    try(result_void_t,
        valueInitNil(destination, destination_arena, source->value.nil));
    break;
  case VALUE_TYPE_STRING:
    try(result_void_t,
        valueInitString(destination, destination_arena, source->value.string));
    break;
  case VALUE_TYPE_CLOSURE:
    try(result_void_t, valueInitClosure(destination, destination_arena,
                                        source->value.closure.form.type));
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

    try(result_void_t,
        environmentClone(source->value.closure.captured_environment,
                         destination_arena),
        destination->value.closure.captured_environment);
    break;
  case VALUE_TYPE_LIST: {
    try(result_void_t, valueInitList(destination, destination_arena,
                                     source->value.list.count));
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
