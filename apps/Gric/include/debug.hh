#ifndef DEBUGP_HH
#define DEBUGP_HH

#include <stdint.h>
#include <string>

#define INFO_PRINT_ENABLED 1
#if DEBUG_PRINT_ENABLED
#define INFO_PRINT_ENABLED 1
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(format, args...) ((void)0)
#endif
#if INFO_PRINT_ENABLED
#define INFO_PRINT printf
#else
#define INFO_PRINT(format, args...) ((void)0)
#endif
#define STEP printf("%s %d\n",__FUNCTION__,__LINE__)
#endif
