#pragma once

#include "./result.h"
#include <stdlib.h>

#ifdef MEMORY_PROFILE
constexpr long MAX_SEGMENTS = 1024;

typedef struct {
  unsigned long bytes;
  unsigned long segments_count;
  void *pointers[MAX_SEGMENTS];
  size_t sizes[MAX_SEGMENTS];
  bool freed[MAX_SEGMENTS];
} safe_alloc_metrics_t;

extern safe_alloc_metrics_t safe_alloc_metrics;

#define allocProfileStart(Pointer, Size)                                       \
  if (safe_alloc_metrics.segments_count < MAX_SEGMENTS) {                      \
    safe_alloc_metrics.pointers[safe_alloc_metrics.segments_count] = Pointer;  \
    safe_alloc_metrics.sizes[safe_alloc_metrics.segments_count] = Size;        \
    safe_alloc_metrics.freed[safe_alloc_metrics.segments_count] = false;       \
    safe_alloc_metrics.segments_count++;                                       \
    safe_alloc_metrics.bytes += (Size);                                        \
  }

#define allocProfileEnd(DoublePointer)                                         \
  for (unsigned long i = 0; i < safe_alloc_metrics.segments_count; i++) {      \
    if (*(DoublePointer) == safe_alloc_metrics.pointers[i] &&                  \
        !safe_alloc_metrics.freed[i]) {                                        \
      safe_alloc_metrics.bytes -= safe_alloc_metrics.sizes[i];                 \
      safe_alloc_metrics.freed[i] = true;                                      \
    }                                                                          \
  }

#define allocGetMetrics() safe_alloc_metrics

#define allocMetricsInit() safe_alloc_metrics_t safe_alloc_metrics = {}

#else

#define allocProfileStart(Pointer, Size)
#define allocProfileEnd(DoublePointer)
#define allocGetMetrics()
#define allocMetricsInit()

#endif

typedef enum {
  ALLOC_ERROR_MALLOC_ERROR = 1,
} alloc_error_t;

/**
 * The result of an allocation. It returns a void* to be cast by the caller.
 * @name result_ref_t
 */
typedef Result(void *) result_ref_t;

/**
 * Copy bytes from source to destination memory locations.
 * @name bytewiseCopy
 * @param {void*} dest - Pointer to the destination memory location
 * @param {const void*} src - Pointer to the source memory location to copy from
 * @param {size_t} size - Number of bytes to copy
 * @example
 *   char source[] = "Hello";
 *   char dest[6];
 *   bytewiseCopy(dest, source, 6);  // dest now contains "Hello"
 */
static inline void bytewiseCopy(void *dest, const void *src, size_t size) {
  const auto dest_bytes = (unsigned char *)dest;
  const auto src_bytes = (const unsigned char *)src;
  for (size_t i = 0; i < size; i++) {
    dest_bytes[i] = src_bytes[i];
  }
}

/**
 * Safely allocate memory with error handling.
 * @name allocSafe
 * @param {size_t} size - Number of bytes to allocate
 * @returns {result_ref_t} Result containing allocated memory pointer on
 * success, or error on failure
 * @example
 *   result_ref_t result = allocSafe(100);
 *   if (isOk(result)) {
 *     my_type_t *ptr = result.value;
 *     // Use allocated memory
 *   }
 */
static inline result_ref_t allocSafe(size_t size) {
  void *ptr = malloc(size);

  if (ptr == nullptr) {
    throw(result_ref_t, ALLOC_ERROR_MALLOC_ERROR, nullptr,
          "Unable to allocate size: %lu", size);
  }

  allocProfileStart(ptr, size);

  return ok(result_ref_t, ptr);
}

/**
 * Safely deallocate memory and set pointer to nullptr.
 * @name deallocSafe
 * @param {void**} DoublePointer - Pointer to the pointer that should be freed
 * @example
 *   char *ptr = malloc(100);
 *   deallocSafe(&ptr);  // ptr is now nullptr
 */
#define deallocSafe(DoublePointer)                                             \
  {                                                                            \
    if (*(DoublePointer) != nullptr) {                                         \
      allocProfileEnd(DoublePointer);                                          \
      free((void *)*(DoublePointer));                                          \
      *(DoublePointer) = nullptr;                                              \
    }                                                                          \
  }
