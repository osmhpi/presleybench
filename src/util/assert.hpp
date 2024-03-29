#ifndef ASSERT_H
#define ASSERT_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdarg>

extern "C" {
extern char *program_invocation_name;
}

#define presley_runtime_assert(COND, ...) \
  do { if (!(COND)) assert_error(__VA_ARGS__); } while (0)

#define presley_runtime_error(...) presley_runtime_assert(0, __VA_ARGS__)

#define guard(COND) \
  if ((COND) || (DEBUG && assert_error_at_line(__FILE__, __LINE__, "assertion failed: %s", # COND) && 0)) { /* empty */ }

#define debug_assert(COND, ...) \
  do { if (!(COND) && DEBUG) assert_error_at_line(__FILE__, __LINE__, "assertion failed: " __VA_ARGS__); } while (0)

#define debug_error(...) debug_assert(0, __VA_ARGS__)

#define debug_guard(COND) \
  do { debug_assert((COND), "assertion failed: %s", # COND); } while (0)

#if HAVE_FUNC_ATTRIBUTE_WARN_UNUSED_RESULT
#  define _warn_unused_result __attribute__((warn_unused_result))
#else
#  define _warn_unused_result
#endif

#if HAVE_FUNC_ATTRIBUTE_FORMAT
#  define _format(...) __attribute__((format(__VA_ARGS__)))
#else
#  define _format(...)
#endif

#if HAVE_FUNC_ATTRIBUTE_UNUSED
#  define _presley_unused __attribute__((unused))
#else
#  define _presley_unused
#endif

int assert_error_at_line(const char*, unsigned int, const char*, ...) _format(printf, 3, 4);

int assert_error(const char*, ...) _format(printf, 1, 2);

#endif
