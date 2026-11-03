// Precondition checks.
//
// Following the Google C++ Style Guide this code base does not use
// exceptions. Programming errors (violated preconditions such as a negative
// price or mismatched vector sizes) abort with a message, like absl's CHECK.
// Operations that can legitimately fail on valid input (a root that is not
// bracketed, a singular matrix) return std::optional instead.

#ifndef CPP_CORE_CHECK_H_
#define CPP_CORE_CHECK_H_

#include <cstdio>
#include <cstdlib>

#define INVESTM_CHECK(condition, message)                              \
  do {                                                                 \
    if (!(condition)) {                                                \
      std::fprintf(stderr, "%s:%d: check failed: %s (%s)\n", __FILE__, \
                   __LINE__, #condition, message);                     \
      std::abort();                                                    \
    }                                                                  \
  } while (false)

#endif  // CPP_CORE_CHECK_H_
