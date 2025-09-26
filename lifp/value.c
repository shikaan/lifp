#include "value.h"
#include "position.h"
#include "types.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

result_value_ref_t valueCreate(value_type_t type, value_as_t as,
                               position_t pos) {
  value_t *value = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(sizeof(value_t)), pos, value);
  value->type = type;
  value->as = as;
  value->position = pos;
  return ok(result_value_ref_t, value);
}

result_value_ref_t valueDeepCopy(const value_t *self) {
  value_t *destination = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(sizeof(value_t)), self->position,
              destination);
  destination->type = self->type;
  destination->position = self->position;

  switch (destination->type) {
  case VALUE_TYPE_BOOLEAN:
  case VALUE_TYPE_NUMBER:
  case VALUE_TYPE_BUILTIN:
  case VALUE_TYPE_SPECIAL:
  case VALUE_TYPE_NIL:
    destination->as = self->as;
    break;
  case VALUE_TYPE_CLOSURE:
    // TODO
    destination->as = self->as;
    break;
  case VALUE_TYPE_LIST: {
    size_t count = self->as.list->count;
    tryWithMeta(result_value_ref_t, valueArrayCreate(count), self->position,
                destination->as.list);

    for (size_t i = 0; i < count; i++) {
      value_t *copied;
      tryWithMeta(result_value_ref_t, valueDeepCopy(&self->as.list->data[i]),
                  self->position, copied);
      destination->as.list->data[i] = *copied;
    }
    break;
  }
  case VALUE_TYPE_STRING: {
    destination->as.string = strdup(self->as.string);
    break;
  }
  default:
    unreachable();
  }

  return ok(result_value_ref_t, destination);
}

result_ref_t argumentsCreate(size_t count) {
  arguments_t *args = nullptr;
  try(result_ref_t, allocSafe(sizeof(arguments_t)), args);
  try(result_ref_t, allocSafe(sizeof(string_t *) * count), args->data);
  args->count = count;
  return ok(result_ref_t, args);
}

result_ref_t valueArrayCreate(size_t count) {
  value_array_t *array = nullptr;
  try(result_ref_t, allocSafe(sizeof(value_array_t)), array);
  array->count = count;
  try(result_ref_t, allocSafe(sizeof(value_t) * count), array->data);
  return ok(result_ref_t, array);
}

static void valueDestroyInner(value_t *self) {
  if (!self)
    return;

  switch (self->type) {
  case VALUE_TYPE_BOOLEAN:
  case VALUE_TYPE_NUMBER:
  case VALUE_TYPE_NIL:
  case VALUE_TYPE_BUILTIN:
  case VALUE_TYPE_SPECIAL:
    // These types don't have nested allocations to clean up
    break;
  case VALUE_TYPE_CLOSURE:
    // TODO
    break;
  case VALUE_TYPE_LIST: {
    valueArrayDestroy(&self->as.list);
    break;
  }
  case VALUE_TYPE_STRING:
    if (self->as.string) {
      deallocSafe(&self->as.string);
    }
    break;
  default:
    unreachable();
  }
}

void valueDestroy(value_t **self) {
  if (!self || !*self)
    return;

  // Clean up nested structures first
  valueDestroyInner(*self);

  // Then free the main value structure
  deallocSafe(self);
}

void valueArrayDestroy(value_array_t **self) {
  value_array_t *array = (*self);
  for (size_t i = 0; i < array->count; i++) {
    valueDestroyInner(&array->data[i]);
  }
  deallocSafe(&array->data);
  deallocSafe(self);
}
