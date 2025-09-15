#include "map.h"
#include "arena.h"
#include "result.h"
#include "string.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

uint64_t hash(size_t len, const char key[static len]) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash;
}

size_t makeKey(generic_map_t *self, const char *key) {
  uint64_t hashed_key = hash(strlen(key), key);
  return hashed_key % self->capacity;
}

result_ref_t genericMapCreate(arena_t *arena, size_t capacity,
                              size_t item_size) {
  assert(arena);
  assert(capacity > 0);
  assert(item_size > 0);

  generic_map_t *map = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(generic_map_t)), map);

  map->capacity = capacity;
  map->item_size = item_size;
  map->arena = arena;

  try(result_ref_t, arenaAllocate(arena, sizeof(bool) * capacity), map->used);
  try(result_ref_t,
      arenaAllocate(arena, sizeof(char) * MAX_KEY_LENGTH * capacity),
      map->keys);
  try(result_ref_t, arenaAllocate(arena, item_size * capacity), map->values);

  return ok(result_ref_t, map);
}

void rawValueSet(generic_map_t *self, size_t index, const void *value) {
  assert(self);
  byte_t *destination = (byte_t *)self->values + (index * self->item_size);
  // memcpy on nullptr causes undefined behaviour
  assert(destination != nullptr);
  memcpy(destination, value, self->item_size);
}

result_void_t genericMapSet(generic_map_t *self, const char *key, void *value) {
  assert(self);
  size_t key_length = strlen(key);
  if (key_length >= MAX_KEY_LENGTH) {
    throw(result_void_t, MAP_ERROR_INVALID_KEY, nullptr,
          "Map key too long. Expected <= %lu, got %lu", MAX_KEY_LENGTH,
          key_length);
  }

  if (key_length == 0) {
    throw(result_void_t, MAP_ERROR_INVALID_KEY, nullptr,
          "Map key cannot be empy");
  }

  size_t index = makeKey(self, key);
  size_t count = 0;

  while (self->used[index]) {
    const bool is_same_key =
        strncmp(self->keys[index], key, MAX_KEY_LENGTH) == 0;

    if (is_same_key) {
      rawValueSet(self, index, value);
      return ok(result_void_t);
    }

    index = (index + 1) % self->capacity;
    count++;

    if (count == self->capacity) {
      auto used = self->used;
      auto keys = self->keys;
      auto values = self->values;
      size_t capacity = self->capacity * 2;

      try(result_void_t, arenaAllocate(self->arena, sizeof(bool) * capacity),
          self->used);
      try(result_void_t,
          arenaAllocate(self->arena, sizeof(char) * MAX_KEY_LENGTH * capacity),
          self->keys);
      try(result_void_t, arenaAllocate(self->arena, self->item_size * capacity),
          self->values);

      // This needs to happen _before_ we start setting again, else the hashed
      // value may not match
      self->capacity = capacity;
      for (size_t i = 0; i < self->capacity; i++) {
        if (used[i]) {
          byte_t *destination = (byte_t *)values + (i * self->item_size);
          try(result_void_t, genericMapSet(self, keys[i], destination));
        }
      }
      return genericMapSet(self, key, value);
    }
  }

  self->used[index] = true;
  stringCopy(self->keys[index], key, MAX_KEY_LENGTH);
  rawValueSet(self, index, value);

  return ok(result_void_t);
}

void *genericMapGet(generic_map_t *self, const char *key) {
  assert(self);

  size_t index = makeKey(self, key);
  size_t count = 0;

  while ((int)self->used[index] && count < self->capacity) {
    if (strncmp(self->keys[index], key, MAX_KEY_LENGTH) == 0) {
      return (byte_t *)self->values + (index * self->item_size);
    }

    index = (index + 1) % self->capacity;
    count++;
  }

  return nullptr;
}
