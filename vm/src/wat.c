#include <stdint.h>

#if INTPTR_MAX == INT32_MAX
  #define ENV_32_BIT
#elif INTPTR_MAX == INT64_MAX
  #define ENV_64_BIT
#else
  #error "Environment not 32 or 64-bit."
#endif


// int: 11
// float: 10
// bool: 01
// pointer: 00
