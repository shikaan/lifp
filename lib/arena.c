#include "arena.h"
#include "alloc.h"
#include "result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MEMORY_PROFILE
arena_metrics_t arena_metrics = {};

#define arenaProfileStart(Arena)                                               \
  arena_metrics.arenas[arena_metrics.arenas_count] = Arena;                    \
  arena_metrics.freed[arena_metrics.arenas_count] = false;                     \
  arena_metrics.arenas_count++;

#define arenaProfileEnd(Arena)                                                 \
  for (size_t i = 0; i < arena_metrics.arenas_count; i++) {                    \
    if (arena_metrics.arenas[i] == Arena) {                                    \
      arena_metrics.freed[i] = true;                                           \
      break;                                                                   \
    }                                                                          \
  }

#else

#define arenaProfileStart(Arena)
#define arenaProfileEnd(Arena)

#endif

result_ref_t arenaCreate(size_t size) {
  arena_t *arena = nullptr;
  try(result_ref_t, allocSafe(sizeof(arena_t) + size), arena);
  arena->size = size;
  arena->offset = 0;
#ifdef DEBUG
  static int id = 0;
  arena->id = id++;
#endif

  arenaProfileStart(arena);

  return ok(result_ref_t, arena);
}

result_ref_t arenaAllocate(arena_t *self, size_t size) {
  const size_t aligned_offset = (self->offset + 7U) & ~7U;
  const size_t aligned_size = (size + 7U) & ~7U;

  if (aligned_offset + aligned_size > self->size) {
#ifdef DEBUG
    throw(result_ref_t, ARENA_ERROR_OUT_OF_SPACE, nullptr,
          "Arena %d out of memory. Available %lu, requested %lu, total %lu",
          self->id, self->size - aligned_offset, aligned_size, self->size);
#else
    throw(result_ref_t, ARENA_ERROR_OUT_OF_SPACE, nullptr,
          "Arena out of memory. Available %lu, requested %lu, total %lu",
          self->size - aligned_offset, aligned_size, self->size);
#endif
  }

  byte_t *pointer = &self->memory[aligned_offset];
  memset(pointer, 0, aligned_size);
  self->offset = aligned_offset + aligned_size;
  return ok(result_ref_t, pointer);
}

void arenaDestroy(arena_t **self) {
  arenaProfileEnd(*self);
  deallocSafe(self);
}

void arenaReset(arena_t *self) { self->offset = 0; }

frame_handle_t arenaAllocationFrameStart(arena_t *self) { return self->offset; }
void arenaAllocationFrameEnd(arena_t *self, frame_handle_t frame_handle) {
  self->offset = frame_handle;
}
