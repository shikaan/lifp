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

static inline bool stringStartsWith(const char *string, const char *prefix) {
  const size_t len = strlen(prefix);
  return strncmp(string, prefix, len) == 0;
}

static inline void stringConcat(size_t dst_len, char dst[static dst_len],
                                size_t one_len, const char *one, size_t two_len,
                                const char *two) {
  size_t min_len = one_len < dst_len ? one_len : dst_len;

  for (size_t i = 0; i < min_len; i++) {
    dst[i] = one[i];
  }

  size_t end_len = min_len + two_len < dst_len ? min_len + two_len : dst_len;
  for (size_t i = min_len; i < end_len; i++) {
    dst[i] = two[i - min_len];
  }

  dst[end_len] = 0;
}
