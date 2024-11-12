#if defined _WIN32

#elif defined(__linux__) || defined(__linux)
#include <pwd.h>
#if defined(__i386__)
    #define OS_X86
#elif defined(__arm__) || defined(__aarch64__)
    #define OS_ARM
#elif defined(__mips__)
    #define OS_MIPS
#elif defined(__sw_64__)
    #define OS_SW64
#elif defined(__loongarch64)
    #define OS_LOONGARCH64
#else
    #define OS_X86_64
#endif
#elif defined(__APPLE__) || defined(__MACH__)
    #define OS_MAC
#else
    #error "Unknown compiler"
#endif