#pragma once

#include <stddef.h>
#include <string.h>

// This is a portable re-implementation of strlcpy
static inline size_t stringCopy(char *dst, const char *src, size_t size) {
  size_t src_length = strlen(src);

  if (size != 0) {
    size_t dst_length = (src_length >= size) ? size - 1 : src_length;
    if (dst_length > 0) {
      memcpy(dst, src, dst_length);
    }
    dst[dst_length] = '\0';
  }
  return src_length;
}