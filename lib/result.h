#pragma once

#include <stdio.h>
typedef char message_t[128];

constexpr int RESULT_OK = 0;

#define _metaType(type, ...) type meta
#define Result(ValueType, ...)                                                 \
  struct {                                                                     \
    int code;                                                                  \
    _metaType(__VA_OPT__(__VA_ARGS__, ) void *);                               \
    union {                                                                    \
      ValueType value;                                                         \
      message_t message;                                                       \
    };                                                                         \
  }

#define ResultVoid(...)                                                        \
  struct {                                                                     \
    int code;                                                                  \
    _metaType(__VA_OPT__(__VA_ARGS__, ) void *);                               \
    message_t message;                                                         \
  }

typedef ResultVoid() result_void_t;

typedef Result(int) test_t;

#define _concat_detail(x, y) x##y
#define _concat(x, y) _concat_detail(x, y)
#define _result(ResultType) _concat(ResultType, __LINE__)
#define _error(ResultType) _concat(error_, _concat(ResultType, __LINE__))

#define ok(T, ...)                                                             \
  (T) { .code = RESULT_OK __VA_OPT__(, .value =) __VA_ARGS__ }

#define try(ResultType, Action, ...)                                           \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    throw(ResultType, _result(ResultType).code, _result(ResultType).meta,      \
          "%s", _result(ResultType).message);                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define tryWithMeta(ResultType, Action, Meta, ...)                             \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    throw(ResultType, _result(ResultType).code, Meta, "%s",                    \
          _result(ResultType).message);                                        \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define tryCatch(ResultType, Action, Catch, ...)                               \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    Catch;                                                                     \
    throw(ResultType, _result(ResultType).code, _result(ResultType).meta,      \
          "%s", _result(ResultType).message);                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define tryCatchWithMeta(ResultType, Action, Catch, Meta, ...)                 \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    Catch;                                                                     \
    throw(ResultType, _result(ResultType).code, Meta, "%s",                    \
          _result(ResultType).message);                                        \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define tryFinally(ResultType, Action, Finally, ...)                           \
  auto _result(ResultType) = Action;                                           \
  Finally;                                                                     \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    throw(ResultType, _result(ResultType).code, _result(ResultType).meta,      \
          "%s", _result(ResultType).message);                                  \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define tryFinallyWithMeta(ResultType, Action, Finally, Meta, ...)             \
  auto _result(ResultType) = Action;                                           \
  Finally;                                                                     \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    throw(ResultType, _result(ResultType).code, Meta, "%s",                    \
          _result(ResultType).message);                                        \
  }                                                                            \
  __VA_OPT__(__VA_ARGS__ = (_result(ResultType).value));

#define throw(ResultType, Code, Meta, Fmt, ...)                                \
  {                                                                            \
    ResultType _error(ResultType);                                             \
    _error(ResultType).code = Code;                                            \
    _error(ResultType).meta = Meta;                                            \
    snprintf(_error(ResultType).message, sizeof(message_t),                    \
             Fmt __VA_OPT__(, ) __VA_ARGS__);                                  \
    return _error(ResultType);                                                 \
  }
