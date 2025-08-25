// Arena (v0.0.1)
// ---
//
// Arena allocator for fast memory allocation and bulk deallocation.
//
// The arena allocator provides a fast allocation strategy where memory is
// allocated from a pre-allocated block in a sequential manner. All allocations
// can be freed at once by resetting the arena, making it ideal for temporary
// allocations during parsing, evaluation, and other operations that need to
// allocate many small objects that have similar lifetimes.
//
// Key features:
// - Fast O(1) allocation with no fragmentation
// - Bulk deallocation by resetting the arena
// - No individual free operations needed
// - Memory-efficient for scenarios with many small allocations
//
// ```c
// result_ref_t result = arenaCreate(1024);
// if (result.ok) {
//     arena_t *arena = result.value;
//
//     // Allocate some memory
//     result_ref_t alloc = arenaAllocate(arena, sizeof(int));
//     if (alloc.ok) {
//         int *ptr = (int*)alloc.value;
//         *ptr = 42;
//     }
//
//     // Reset and reuse
//     arenaReset(arena);
//
//     // Clean up
//     arenaDestroy(arena);
// }
// ```

#pragma once

#include "alloc.h"

typedef unsigned char byte_t;
typedef char message_t[128];
typedef size_t frame_handle_t;

typedef enum {
  ARENA_ERROR_MALLOC_ERROR = 1,
  ARENA_ERROR_OUT_OF_SPACE,
} arena_error_t;

/**
 * Arena allocator structure.
 * @name arena_t
 */
typedef struct {
#ifdef DEBUG
  int id;
#endif
  size_t size;     // Total size of the arena memory buffer
  size_t offset;   // Current allocation offset within the buffer
  byte_t memory[]; // Flexible array member containing the actual memory buffer
} arena_t;

#ifdef MEMORY_PROFILE
constexpr size_t MAX_PROFILED_ARENAS = 128;

typedef struct {
  arena_t *arenas[MAX_PROFILED_ARENAS];
  bool freed[MAX_PROFILED_ARENAS];
  size_t arenas_count;
} arena_metrics_t;

extern arena_metrics_t arena_metrics;
#endif

/**
 * Create a new arena with the specified size.
 * @name arenaCreate
 * @param {size_t} size - The total size in bytes for the arena's memory buffer
 * @returns {result_ref_t} Result containing the arena pointer on success, or
 * an allocation error
 * @example
 *   result_ref_t result = arenaCreate(1024);
 *   if (result.ok) {
 *       arena_t *arena = result.value;
 *       // Use the arena...
 *       arenaDestroy(arena);
 *   }
 */
result_ref_t arenaCreate(size_t size);

/**
 * Allocate memory from the arena.
 * @name arenaAllocate
 * @param {arena_t*} self - Pointer to the arena to allocate from
 * @param {size_t} size - Number of bytes to allocate
 * @returns {result_ref_t} Result containing pointer to allocated memory on
 * success, or allocation error
 * @example
 *   result_ref_t result = arenaAllocate(arena, sizeof(int));
 *   if (result.ok) {
 *       int *ptr = (int*)result.value;
 *       *ptr = 42;
 *   }
 */
result_ref_t arenaAllocate(arena_t *self, size_t size);

/**
 * Marks the start of an allocation frame in the given arena.
 * An allocation frame is a region in an arena which needs recycling before the
 * arena's end of life.
 *
 * Frames can be nested, but ending a frame within another frame will leads to
 * errors.
 *
 * @param {arena_t*} self - Pointer to the arena instance.
 * @returns {frame_handle_t} A handle pointing to the frame, which can be used
 * to restore the arena to this state later.
 * @example
 *    frame_handle_t frame = arenaAllocationFrameStart(arena);
 *    // .. do the thing
 *    arenaAllocationFrameEnd(frame);
 */
frame_handle_t arenaAllocationFrameStart(arena_t *self);

/**
 * Ends the allocation frame for the specified arena.
 * All allocations made since the corresponding frame start may be released
 * or invalidated. See `arenaAllocationFrameStart` for details.
 *
 * @param {arena_t*} self - Pointer to the arena instance.
 * @param {frame_handle_t} frame_handle - A handle to the frame to end.
 * @returns {frame_handle_t} A handle pointing to the frame, which can be used
 * to restore the arena to this state later.
 * @example
 *    frame_handle_t frame = arenaAllocationFrameStart(arena);
 *    // .. do the thing
 *    arenaAllocationFrameEnd(frame);
 */
void arenaAllocationFrameEnd(arena_t *self, frame_handle_t frame_handle);

/**
 * Destroy the arena and free all its memory.
 * @name arenaDestroy
 * @param {arena_t*} self - Pointer to the arena to destroy
 * @example
 *   arenaDestroy(arena);  // arena pointer becomes invalid
 */
void arenaDestroy(arena_t **self);

/**
 * Reset the arena to empty state.
 * @name arenaReset
 * @param {arena_t*} self - Pointer to the arena to reset
 * @example
 *   arenaAllocate(arena, 100);
 *   arenaAllocate(arena, 200);
 *   arenaReset(arena);  // All memory now available again
 */
void arenaReset(arena_t *self);
