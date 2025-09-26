#include "environment.h"
#include "../lib/string.h"
#include "value.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
    throw(result_void_t, MAP_ERROR_INVALID_KEY, nullptr,
          "Map key cannot be empy");
  }

  size_t index = makeKey(self, key);
  size_t count = 0;

  while (self->used[index]) {
    const bool is_same_key = strncmp(self->keys[index], key, key_len) == 0;

    if (is_same_key) {
      self->data[index] = *value;
      return ok(result_void_t);
    }

    index = (index + 1) % self->capacity;
    count++;

    if (count == self->capacity) {
      self->capacity *= 2;
      try(result_void_t, reallocSafe(self->used, sizeof(bool) * self->capacity),
          self->used);
      try(result_void_t,
          reallocSafe(self->keys, sizeof(char *) * self->capacity), self->keys);
      try(result_void_t,
          reallocSafe(self->data, sizeof(value_t) * self->capacity),
          self->data);

      // retry after expansion
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

static void valueMapDestroyInner(value_map_t *self) {
  for (size_t i = 0; i < self->capacity; i++) {
    if (self->used[i]) {
      deallocSafe(&self->keys[i]);
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

result_environment_ref_t environmentCreate(environment_t *parent) {
  environment_t *environment = nullptr;
  try(result_environment_ref_t, allocSafe(sizeof(environment_t)), environment);

  value_map_t *values = nullptr;
  try(result_environment_ref_t, valueMapCreate(8), values);

  environment->values = *values;
  environment->parent = parent;

  return ok(result_environment_ref_t, environment);
}

void environmentDestroy(environment_t **self) {
  valueMapDestroyInner(&(*self)->values);
  deallocSafe(self);
}