#include "value.h"
#include "../lib/string.h"
#include "node.h"
#include "position.h"
#include "types.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
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
  case VALUE_TYPE_CLOSURE: {
    destination->as.closure.environment = self->as.closure.environment;
    self->as.closure.environment->refcount++;

    size_t count = self->as.closure.arguments->count;
    arguments_t *closure_arguments = nullptr;
    tryWithMeta(result_value_ref_t, argumentsCreate(count), self->position,
                closure_arguments);

    for (size_t i = 0; i < count; i++) {
      closure_arguments->data[i] = strdup(self->as.closure.arguments->data[i]);
    }

    destination->as.closure.arguments = closure_arguments;

    node_t *closure_form = nullptr;
    tryWithMeta(result_value_ref_t, nodeCopy(self->as.closure.form),
                self->position, closure_form);
    destination->as.closure.form = closure_form;

    break;
  }
  case VALUE_TYPE_LIST: {
    size_t count = self->as.list->count;
    tryWithMeta(result_value_ref_t, valueArrayCreate(count), self->position,
                destination->as.list);

    for (size_t i = 0; i < count; i++) {
      value_t *copied;
      tryWithMeta(result_value_ref_t, valueDeepCopy(&self->as.list->data[i]),
                  self->position, copied);
      destination->as.list->data[i] = *copied;
      deallocSafe(&copied);
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

void argumentsDestroy(arguments_t **self) {
  if (!self || !(*self))
    return;
  arguments_t *array = (*self);
  for (size_t i = 0; i < array->count; i++) {
    deallocSafe(&array->data[i]);
  }
  deallocSafe(&array->data);
  deallocSafe(self);
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
  case VALUE_TYPE_CLOSURE:
    argumentsDestroy(&self->as.closure.arguments);
    environmentDestroy(&self->as.closure.environment);
    nodeDestroy(&self->as.closure.form);
    break;
  case VALUE_TYPE_LIST: {
    valueArrayDestroy(&self->as.list);
    break;
  }
  case VALUE_TYPE_STRING:
    deallocSafe(&self->as.string);
    break;
  case VALUE_TYPE_SPECIAL:
  case VALUE_TYPE_BOOLEAN:
  case VALUE_TYPE_NUMBER:
  case VALUE_TYPE_NIL:
  case VALUE_TYPE_BUILTIN:
  default:
    break;
  }
}

void valueDestroy(value_t **self) {
  if (!self || !*self)
    return;
  valueDestroyInner(*self);
  deallocSafe(self);
}

void valueArrayDestroy(value_array_t **self) {
  if (!self || !*self)
    return;
  value_array_t *array = (*self);
  for (size_t i = 0; i < array->count; i++) {
    valueDestroyInner(&array->data[i]);
  }
  deallocSafe(&array->data);
  deallocSafe(self);
}

static uint64_t hash(size_t len, const char key[static len]) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash;
}

static size_t makeKey(const value_map_t *self, const char *key) {
  uint64_t hashed_key = hash(strlen(key), key);
  return hashed_key % self->capacity;
}

result_value_map_ref_t valueMapCreate(size_t capacity) {
  assert(capacity > 0);

  value_map_t *map = nullptr;
  try(result_value_map_ref_t, allocSafe(sizeof(value_map_t)), map);
  try(result_value_map_ref_t, allocSafe(sizeof(bool) * capacity), map->used);
  try(result_value_map_ref_t, allocSafe(sizeof(char *) * capacity), map->keys);
  try(result_value_map_ref_t, allocSafe(sizeof(value_t) * capacity), map->data);
  map->capacity = capacity;

  return ok(result_value_map_ref_t, map);
}

result_void_t valueMapSet(value_map_t *self, const char *key,
                          const value_t *value) {
  assert(self);
  size_t key_len = strlen(key);
  if (key_len == 0) {
    throw(result_void_t, VALUE_MAP_ERROR_INVALID_KEY, nullptr,
          "Map key cannot be empy");
  }

  size_t index = makeKey(self, key);
  size_t count = 0;

  while (self->used[index]) {
    const bool is_same_key = strcmp(self->keys[index], key) == 0;

    if (is_same_key) {
      self->data[index] = *value;
      return ok(result_void_t);
    }

    index = (index + 1) % self->capacity;
    count++;

    if (count == self->capacity) {
      size_t old_capacity = self->capacity;
      bool *old_used = self->used;
      char **old_keys = self->keys;
      value_t *old_data = self->data;

      self->capacity *= 2;
      try(result_void_t, allocSafe(sizeof(bool) * self->capacity), self->used);
      try(result_void_t, allocSafe(sizeof(char *) * self->capacity),
          self->keys);
      try(result_void_t, allocSafe(sizeof(value_t) * self->capacity),
          self->data);

      // rehash
      for (size_t i = 0; i < old_capacity; i++) {
        if (old_used[i]) {
          valueMapSet(self, old_keys[i], &old_data[i]);

          // This is not part of rehashing, but we'd need to walk the list again
          // to cleanup keys a few lines down, might as well doing it here.
          deallocSafe(&old_keys[i]);
        }
      }

      deallocSafe(&old_used);
      deallocSafe(&old_keys);
      deallocSafe(&old_data);

      return valueMapSet(self, key, value);
    }
  }

  char *label;
  try(result_void_t, allocSafe(sizeof(char) * (key_len + 1)), label);
  stringCopy(label, key, key_len + 1);

  self->used[index] = true;
  self->keys[index] = label;
  self->data[index] = *value;

  return ok(result_void_t);
}

void *valueMapGet(const value_map_t *self, const char *key) {
  assert(self);

  size_t index = makeKey(self, key);
  size_t count = 0;

  while ((int)self->used[index] && count < self->capacity) {
    if (strcmp(self->keys[index], key) == 0) {
      return &self->data[index];
    }

    index = (index + 1) % self->capacity;
    count++;
  }

  return nullptr;
}

void valueMapDestroyInner(value_map_t *self) {
  if (!self)
    return;

  for (size_t i = 0; i < self->capacity; i++) {
    if (self->used[i]) {
      deallocSafe(&self->keys[i]);
      valueDestroyInner(&self->data[i]);
    }
  }
  deallocSafe(&self->keys);
  deallocSafe(&self->data);
  deallocSafe(&self->used);
}

void valueMapDestroy(value_map_t **self) {
  valueMapDestroyInner((*self));
  deallocSafe(self);
}
