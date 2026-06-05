/*  
    cba.h | v1.4.1 | https://github.com/jamiegibney/cba.h
  
    Single-header library for build recipes and general utilities in C.


  
    # Usage

    Add #define CBA_IMPLEMENTATION before including this file in ONE C/C++ file to
    generate the implementation:

    #include ...
    #include ...
    #define CBA_IMPLEMENTATION
    #include "cba.h"

    All functions in this header are documented via comments above their definitions.
  


    # Example
  
    #define CBA_IMPLEMENTATION
    #include "cba.h"
  
    int main(int argc, char** argv) {
        // Allow the program to rebuild itself when modified.
        CBA_REBUILD(argc, argv);

        // Create a directory (also works recursively).
        file_try_create_directory("build");

        // An array of arguments which can be run like a shell command.
        Command cmd = {0};

        // Use the CBA_COMPILER_* macros for compiler-specific flags.
        cmd_append(&cmd,
            CBA_COMPILER_C,
            CBA_COMPILER_DEBUG_FLAGS,
            CBA_COMPILER_COMMON_FLAGS,
            CBA_COMPILER_OUTPUT("build/main"),
            CBA_COMPILER_INPUTS("main.c"),
        );

        // With GCC, the above forms:
        //   gcc -ggdb -DDEBUG -Wall -Wextra -o build/main main.c
        //
        // And with MSVC:
        //   cl.exe /ZI /DDEBUG /W4 /nologo /D_CRT_SECURE_NO_WARNINGS /Fe:build/main main.c

        // Run the command, block until it terminates, and assert that it exits normally.
        cmd_run(cmd);

        return 0;
    }



    # Other options

    Before including this file, #define any of the below options to override them:
  
    - CBA_VERBOSE                     to see internal logging (e.g. for file errors)
    - CBA_NO_COLOR_OUTPUT             to prevent coloured output via ANSI escape codes
    - CBA_PRINT_ON_REBUILD            to see messages when the program rebuilds itself
    - CBA_REBUILD_COMMAND             the command to use for rebuilding
    - CBA_REBUILD_FAILED_MESSAGE      formatted message printed when a rebuild fails
    - CBA_REBUILD_COMPLETED_MESSAGE   formatted message printed when a rebuild succeeds
    - CBA_[INFO/WARN/ERROR]_PREFIX    prefix to use for info/warn/error macros
    - CBA_MEMORY_BLOCK_SIZE           number of bytes to allocate for arena memory blocks
    - CBA_ALIGNMENT                   number of bytes to align allocations to
    - CBA_MIN_STRING_CAPACITY         minimum capacity for strings
    - CBA_MIN_ARRAY_CAPACITY          minimum capacity for string arrays and commands


    
    # Notes

    - You should prefer to use '/' characters to separate paths as it's easier to convert
      these on Windows compared to converting '\' separators on Unix.

    - You can print Strings with `print(stok, sfmt(s));`
        - stok      expands to "`%.*s`"
        - sfmt(s)   expands to (int)s.len, (const char*)s.data

    - The String type is always null-terminated UNLESS you take a "slice" of another string.
      You can use `str_to_cstr` or `str_copy` to always allocate a null-terminated version.
    
    - All allocations are done via a single arena allocator (global_arena). This
      partitions a large memory block into smaller regions, which you can do via the
      alloc, alloc_bytes, and alloc_array macros. The arena will dynamically allocate new
      memory blocks if it exceeds its current capacity.
    
    - "Dynamically allocated" types and the da_append macro don't use actual dynamic
      allocation (i.e. via realloc()), but will simply allocate new space in the global
      arena. This can be inefficient in terms of memory utilisation, but keeps all of the
      memory within large blocks and avoids individual calls to malloc, etc.



    For version history and a copy of the license, see the bottom of the file.
*/


#ifndef CBA_HEADER_GUARD
#define CBA_HEADER_GUARD

// @mark: overrideable defines

/// Maximum capacity for arrays.
#ifndef CBA_MIN_ARRAY_CAPACITY
    #define CBA_MIN_ARRAY_CAPACITY (256)
#elif CBA_MIN_ARRAY_CAPACITY == 0
    #error array count must be greater than 0
#endif

/// Number of bytes to allocate to each of the global arena's memory blocks.
#ifndef CBA_MEMORY_BLOCK_SIZE
    #define CBA_MEMORY_BLOCK_SIZE (8 << 20) // 8 MB
#elif CBA_MEMORY_BLOCK_SIZE == 0
    #error memory block size must be greater than 0
#endif

/// Number of bytes to align arena allocations to.
#ifndef CBA_ALIGNMENT
    #define CBA_ALIGNMENT (32)
#elif CBA_ALIGNMENT == 0
    #error memory alignment must be greater than 0
#endif

/// Minumum capacity for strings.
#ifndef CBA_MIN_STRING_CAPACITY
    #define CBA_MIN_STRING_CAPACITY (512)
#elif CBA_MIN_STRING_CAPACITY == 0
    #error min string capacity must be greater than 0
#endif

#define CBA_GCC   0
#define CBA_CLANG 0
#define CBA_MSVC  0

#define CBA_WINDOWS 0
#define CBA_MACOS   0
#define CBA_LINUX   0

#define CBA_64_BIT 0
#define CBA_32_BIT 0

#define CBA_X86 0
#define CBA_ARM 0

#if defined(__clang__)
    #undef CBA_CLANG
    #define CBA_CLANG 1
#elif defined(__GNUC__)
    #undef CBA_GCC
    #define CBA_GCC 1
#elif defined(_MSC_VER)
    #undef CBA_MSVC
    #define CBA_MSVC 1

    #if _MSC_VER < 1900
        #error MSVC versions below 19.0 are not supported
    #endif
#else
    #error unsupported complier
#endif

#if defined(_WIN32)
    #undef CBA_WINDOWS
    #define CBA_WINDOWS 1
#elif defined(__APPLE__)
    #undef CBA_MACOS
    #define CBA_MACOS 1
#elif defined(__linux) 
    #undef CBA_LINUX
    #define CBA_LINUX 1
#else
    #error unsupported platform
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__) || defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
    #undef CBA_64_BIT
    #define CBA_64_BIT 1
#else
    #undef CBA_32_BIT
    #define CBA_32_BIT 1
#endif

#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
    #undef CBA_ARM
    #define CBA_ARM 1
#elif defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
    #undef CBA_X86
    #define CBA_X86 1
#else
    #error unsupported architecture
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <math.h>

#if CBA_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS (1)
    #endif
    #define _WINUSER_
    #define _WINGDI_
    #define _WINCON_
    #define _IMM_
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #include <synchapi.h>
    #include <shellapi.h>
#else
    #if CBA_MACOS
        #include <mach-o/dyld.h>
        #include <sys/sysctl.h>
        #include <copyfile.h>
    #elif CBA_LINUX
		#include <sys/sendfile.h>
    #endif

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <signal.h>
    #include <fcntl.h>
    #include <dirent.h>
    #include <ftw.h>
#endif

#if CBA_MSVC
    #define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)

    #define CBA_DLL_EXPORT extern "C" __declspec(dllexport)
    #define CBA_INLINE __forceinline

    #if _MSC_VER < 1740
        #define CBA_FALLTHROUGH
    #else
        #define CBA_FALLTHROUGH [[fallthrough]]
    #endif

    #define CBA_UNREACHABLE

    #if _MSC_VER < 1300
        #define CBA_TRAP __asm int 3
    #else
        #define CBA_TRAP __debugbreak()
    #endif
#else
    #if defined(__MINGW_PRINTF_FORMAT)
        #define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
    #else
        #define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
    #endif

    #define CBA_DLL_EXPORT extern "C" __attribute__((visibility("default")))
    #define CBA_INLINE __attribute__((__always_inline__)) inline
    #define CBA_FALLTHROUGH [[fallthrough]]
    #define CBA_TRAP __builtin_trap()
#endif

#define CBA_UNUSED(x) (void)(x)

#if CBA_MSVC
    #ifndef __FILE_NAME__
        #define __FILE_NAME__ __FILE__
    #endif
    #ifndef __PRETTY_FUNCTION__
        #define __PRETTY_FUNCTION__ __FUNCSIG__
    #endif
#endif

/// Prefix to use for `info` logging calls.
#ifndef CBA_INFO_PREFIX
    #define CBA_INFO_PREFIX  "info"
#endif
/// Prefix to use for `warn` logging calls.
#ifndef CBA_WARN_PREFIX
    #define CBA_WARN_PREFIX  "warn"
#endif
/// Prefix to use for `error` logging calls.
#ifndef CBA_ERROR_PREFIX
    #define CBA_ERROR_PREFIX "error"
#endif

#ifndef CBA_NO_COLOR_OUTPUT
    #define ANSI_BOLD(s)               "\x1b[1m"    s "\x1b[0m"
    #define ANSI_RED(s)                "\x1b[31m"   s "\x1b[0m"
    #define ANSI_GREEN(s)              "\x1b[32m"   s "\x1b[0m"
    #define ANSI_YELLOW(s)             "\x1b[33m"   s "\x1b[0m"
    #define ANSI_BLUE(s)               "\x1b[34m"   s "\x1b[0m"
    #define ANSI_PURPLE(s)             "\x1b[35m"   s "\x1b[0m"
    #define ANSI_CYAN(s)               "\x1b[36m"   s "\x1b[0m"
    #define ANSI_GRAY(s)               "\x1b[37m"   s "\x1b[0m"
    #define ANSI_BRIGHT_RED(s)         "\x1b[91m"   s "\x1b[0m"
    #define ANSI_BRIGHT_GREEN(s)       "\x1b[92m"   s "\x1b[0m"
    #define ANSI_BRIGHT_YELLOW(s)      "\x1b[93m"   s "\x1b[0m"
    #define ANSI_BRIGHT_BLUE(s)        "\x1b[94m"   s "\x1b[0m"
    #define ANSI_BRIGHT_PURPLE(s)      "\x1b[95m"   s "\x1b[0m"
    #define ANSI_BRIGHT_CYAN(s)        "\x1b[96m"   s "\x1b[0m"
    #define ANSI_BRIGHT_GRAY(s)        "\x1b[97m"   s "\x1b[0m"
    #define ANSI_BOLD_RED(s)           "\x1b[1;31m" s "\x1b[0m"
    #define ANSI_BOLD_GREEN(s)         "\x1b[1;32m" s "\x1b[0m"
    #define ANSI_BOLD_YELLOW(s)        "\x1b[1;33m" s "\x1b[0m"
    #define ANSI_BOLD_BLUE(s)          "\x1b[1;34m" s "\x1b[0m"
    #define ANSI_BOLD_PURPLE(s)        "\x1b[1;35m" s "\x1b[0m"
    #define ANSI_BOLD_CYAN(s)          "\x1b[1;36m" s "\x1b[0m"
    #define ANSI_BOLD_GRAY(s)          "\x1b[1;37m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_RED(s)    "\x1b[1;91m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_GREEN(s)  "\x1b[1;92m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_YELLOW(s) "\x1b[1;93m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_BLUE(s)   "\x1b[1;94m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_PURPLE(s) "\x1b[1;95m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_CYAN(s)   "\x1b[1;96m" s "\x1b[0m"
    #define ANSI_BOLD_BRIGHT_GRAY(s)   "\x1b[1;97m" s "\x1b[0m"
#else
    #define ANSI_BOLD(s)               s
    #define ANSI_RED(s)                s
    #define ANSI_GREEN(s)              s
    #define ANSI_YELLOW(s)             s
    #define ANSI_BLUE(s)               s
    #define ANSI_PURPLE(s)             s
    #define ANSI_CYAN(s)               s
    #define ANSI_GRAY(s)               s
    #define ANSI_BRIGHT_RED(s)         s
    #define ANSI_BRIGHT_GREEN(s)       s
    #define ANSI_BRIGHT_YELLOW(s)      s
    #define ANSI_BRIGHT_BLUE(s)        s
    #define ANSI_BRIGHT_PURPLE(s)      s
    #define ANSI_BRIGHT_CYAN(s)        s
    #define ANSI_BRIGHT_GRAY(s)        s
    #define ANSI_BOLD_RED(s)           s
    #define ANSI_BOLD_GREEN(s)         s
    #define ANSI_BOLD_YELLOW(s)        s
    #define ANSI_BOLD_BLUE(s)          s
    #define ANSI_BOLD_PURPLE(s)        s
    #define ANSI_BOLD_CYAN(s)          s
    #define ANSI_BOLD_GRAY(s)          s
    #define ANSI_BOLD_BRIGHT_RED(s)    s
    #define ANSI_BOLD_BRIGHT_GREEN(s)  s
    #define ANSI_BOLD_BRIGHT_YELLOW(s) s
    #define ANSI_BOLD_BRIGHT_BLUE(s)   s
    #define ANSI_BOLD_BRIGHT_PURPLE(s) s
    #define ANSI_BOLD_BRIGHT_CYAN(s)   s
    #define ANSI_BOLD_BRIGHT_GRAY(s)   s
#endif

#define print(s, ...)                                                                    \
    printf(ANSI_BOLD("%s:%04i") ": " s "\n", __FILE_NAME__, __LINE__, ## __VA_ARGS__)

#define cba_assert(cond, s, ...)                                                         \
    if (!(cond)) {                                                                       \
        fprintf(stderr,                                                                  \
                ANSI_BOLD("%s:%04i") ANSI_BOLD_RED(" failed assertion") " in %s: \"" s "\"\n",     \
                __FILE_NAME__,                                                           \
                __LINE__,                                                                \
                __FUNCTION__,                                                            \
                ## __VA_ARGS__);                                                         \
        CBA_TRAP;                                                                        \
    } (void)(0)

#define cba_panic(s, ...)                                                       \
    fprintf(stderr,                                                             \
            ANSI_BOLD("%s:%04i") ANSI_BOLD_RED(" panic") " in %s: \"" s "\"\n", \
            __FILE_NAME__,                                                      \
            __LINE__,                                                           \
            __FUNCTION__,                                                       \
            ## __VA_ARGS__);                                                    \
    CBA_TRAP

#define info(s, ...)  printf("[" ANSI_BOLD_GREEN(CBA_INFO_PREFIX)  "] " s "\n", ## __VA_ARGS__)
#define warn(s, ...)  printf("[" ANSI_BOLD_YELLOW(CBA_WARN_PREFIX) "] " s "\n", ## __VA_ARGS__)
#define error(s, ...) fprintf(stderr, "[" ANSI_BOLD_RED(CBA_ERROR_PREFIX) "] " s "\n", ## __VA_ARGS__)
#define ping printf(ANSI_BOLD_GREEN("PING") " @ %s in %s:" ANSI_BOLD("%04i") "\n", __FILE_NAME__, __FUNCTION__, __LINE__)

#ifdef CBA_VERBOSE
    #define verbose_print(s, ...) print(s, ## __VA_ARGS__)
#else
    #define verbose_print(s, ...)
#endif

#define todo() cba_panic("TODO: %s", __PRETTY_FUNCTION__)
#define unreachable() cba_panic("unreachable code path was hit")

#define CBA_DEF static inline

#if defined(__cplusplus)
    #define CBA_LITERAL(type) type
#else
    #define CBA_LITERAL(type) (type)
#endif

#if CBA_WINDOWS
    typedef HANDLE ProcessID;
    typedef HANDLE FileDescriptor;
    #define INVALID_HANDLE INVALID_HANDLE_VALUE
    #define CBA_MAX_PATH MAX_PATH
    #define CBA_PATH_SEPARATOR '\\'
    #define CBA_LINE_ENDING "\r\n"
#else
    typedef int ProcessID;
    typedef int FileDescriptor;
    #define INVALID_HANDLE (-1)
    #define CBA_MAX_PATH PATH_MAX
    #define CBA_PATH_SEPARATOR '/'
    #define CBA_LINE_ENDING "\n"
#endif

#define CBA_WHITESPACE_CHARS " \t\n\r\v\f"

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef size_t    usize;
typedef ptrdiff_t isize;

typedef int32_t   b32;

typedef float     f32;
typedef double    f64;

#define ___CBA_STATIC_ASSERT(cond, msg) typedef char static_assert_##msg[(!!(cond))*2-1]
#define __CBA_STATIC_ASSERT(cond, line) ___CBA_STATIC_ASSERT(cond, at_line_##line)
#define _CBA_STATIC_ASSERT(cond, line)  __CBA_STATIC_ASSERT(cond, line)
#define CBA_STATIC_ASSERT(cond)         _CBA_STATIC_ASSERT(cond, __LINE__)

CBA_STATIC_ASSERT(sizeof(u8)  == sizeof(i8));
CBA_STATIC_ASSERT(sizeof(u16) == sizeof(i16));
CBA_STATIC_ASSERT(sizeof(u32) == sizeof(i32));
CBA_STATIC_ASSERT(sizeof(u64) == sizeof(i64));

CBA_STATIC_ASSERT(sizeof(u8)  == 1);
CBA_STATIC_ASSERT(sizeof(u16) == 2);
CBA_STATIC_ASSERT(sizeof(u32) == 4);
CBA_STATIC_ASSERT(sizeof(u64) == 8);

CBA_STATIC_ASSERT(sizeof(usize) == sizeof(isize));

CBA_STATIC_ASSERT(sizeof(f32) == 4);
CBA_STATIC_ASSERT(sizeof(f64) == 8);

#define U8_MIN (0x00u)
#define U8_MAX (0xffu)
#define I8_MIN (-0x7f - 1)
#define I8_MAX (0x7f)

#define U16_MIN (0x0000u)
#define U16_MAX (0xffffu)
#define I16_MIN (-0x7fff - 1)
#define I16_MAX (0x7fff)

#define U32_MIN (0x00000000u)
#define U32_MAX (0xffffffffu)
#define I32_MIN (-0x7fffffff - 1)
#define I32_MAX (0x7fffffff)

#define U64_MIN (0x0000000000000000ull)
#define U64_MAX (0xffffffffffffffffull)
#define I64_MIN (-0x7fffffffffffffffll - 1)
#define I64_MAX (0x7fffffffffffffffll)

#if CBA_64_BIT
    #define USIZE_MIN U64_MIN
    #define USIZE_MAX U64_MAX
    #define ISIZE_MIN I64_MIN
    #define ISIZE_MAX I64_MAX
#else
    #define USIZE_MIN U32_MIN
    #define USIZE_MAX U32_MAX
    #define ISIZE_MIN I32_MIN
    #define ISIZE_MAX I32_MAX
#endif

#define F32_MIN     (1.17549435e-38f)
#define F32_MAX     (3.40282347e+38f)
#define F32_EPSILON (1.19209290e-7f)
#define F64_MIN     (2.2250738585072014e-308)
#define F64_MAX     (1.7976931348623157e+308)
#define F64_EPSILON (2.2204460492503131e-16)

// @mark: bits & useful masks

#define BIT1  (1llu <<  0)
#define BIT2  (1llu <<  1)
#define BIT3  (1llu <<  2)
#define BIT4  (1llu <<  3)
#define BIT5  (1llu <<  4)
#define BIT6  (1llu <<  5)
#define BIT7  (1llu <<  6)
#define BIT8  (1llu <<  7)
#define BIT9  (1llu <<  8)
#define BIT10 (1llu <<  9)
#define BIT11 (1llu << 10)
#define BIT12 (1llu << 11)
#define BIT13 (1llu << 12)
#define BIT14 (1llu << 13)
#define BIT15 (1llu << 14)
#define BIT16 (1llu << 15)
#define BIT17 (1llu << 16)
#define BIT18 (1llu << 17)
#define BIT19 (1llu << 18)
#define BIT20 (1llu << 19)
#define BIT21 (1llu << 20)
#define BIT22 (1llu << 21)
#define BIT23 (1llu << 22)
#define BIT24 (1llu << 23)
#define BIT25 (1llu << 24)
#define BIT26 (1llu << 25)
#define BIT27 (1llu << 26)
#define BIT28 (1llu << 27)
#define BIT29 (1llu << 28)
#define BIT30 (1llu << 29)
#define BIT31 (1llu << 30)
#define BIT32 (1llu << 31)
#define BIT33 (1llu << 32)
#define BIT34 (1llu << 33)
#define BIT35 (1llu << 34)
#define BIT36 (1llu << 35)
#define BIT37 (1llu << 36)
#define BIT38 (1llu << 37)
#define BIT39 (1llu << 38)
#define BIT40 (1llu << 39)
#define BIT41 (1llu << 40)
#define BIT42 (1llu << 41)
#define BIT43 (1llu << 42)
#define BIT44 (1llu << 43)
#define BIT45 (1llu << 44)
#define BIT46 (1llu << 45)
#define BIT47 (1llu << 46)
#define BIT48 (1llu << 47)
#define BIT49 (1llu << 48)
#define BIT50 (1llu << 49)
#define BIT51 (1llu << 50)
#define BIT52 (1llu << 51)
#define BIT53 (1llu << 52)
#define BIT54 (1llu << 53)
#define BIT55 (1llu << 54)
#define BIT56 (1llu << 55)
#define BIT57 (1llu << 56)
#define BIT58 (1llu << 57)
#define BIT59 (1llu << 58)
#define BIT60 (1llu << 59)
#define BIT61 (1llu << 60)
#define BIT62 (1llu << 61)
#define BIT63 (1llu << 62)
#define BIT64 (1llu << 63)

#define LOW_1_BITS   (0x0000000000000001llu)
#define LOW_2_BITS   (0x0000000000000003llu)
#define LOW_3_BITS   (0x0000000000000007llu)
#define LOW_4_BITS   (0x000000000000000fllu)
#define LOW_5_BITS   (0x000000000000001fllu)
#define LOW_6_BITS   (0x000000000000003fllu)
#define LOW_7_BITS   (0x000000000000007fllu)
#define LOW_8_BITS   (0x00000000000000ffllu)
#define LOW_9_BITS   (0x00000000000001ffllu)
#define LOW_10_BITS  (0x00000000000003ffllu)
#define LOW_11_BITS  (0x00000000000007ffllu)
#define LOW_12_BITS  (0x0000000000000fffllu)
#define LOW_13_BITS  (0x0000000000001fffllu)
#define LOW_14_BITS  (0x0000000000003fffllu)
#define LOW_15_BITS  (0x0000000000007fffllu)
#define LOW_16_BITS  (0x000000000000ffffllu)
#define LOW_17_BITS  (0x000000000001ffffllu)
#define LOW_18_BITS  (0x000000000003ffffllu)
#define LOW_19_BITS  (0x000000000007ffffllu)
#define LOW_20_BITS  (0x00000000000fffffllu)
#define LOW_21_BITS  (0x00000000001fffffllu)
#define LOW_22_BITS  (0x00000000003fffffllu)
#define LOW_23_BITS  (0x00000000007fffffllu)
#define LOW_24_BITS  (0x0000000000ffffffllu)
#define LOW_25_BITS  (0x0000000001ffffffllu)
#define LOW_26_BITS  (0x0000000003ffffffllu)
#define LOW_27_BITS  (0x0000000007ffffffllu)
#define LOW_28_BITS  (0x000000000fffffffllu)
#define LOW_29_BITS  (0x000000001fffffffllu)
#define LOW_30_BITS  (0x000000003fffffffllu)
#define LOW_31_BITS  (0x000000007fffffffllu)
#define LOW_32_BITS  (0x00000000ffffffffllu)
#define LOW_33_BITS  (0x00000001ffffffffllu)
#define LOW_34_BITS  (0x00000003ffffffffllu)
#define LOW_35_BITS  (0x00000007ffffffffllu)
#define LOW_36_BITS  (0x0000000fffffffffllu)
#define LOW_37_BITS  (0x0000001fffffffffllu)
#define LOW_38_BITS  (0x0000003fffffffffllu)
#define LOW_39_BITS  (0x0000007fffffffffllu)
#define LOW_40_BITS  (0x000000ffffffffffllu)
#define LOW_41_BITS  (0x000001ffffffffffllu)
#define LOW_42_BITS  (0x000003ffffffffffllu)
#define LOW_43_BITS  (0x000007ffffffffffllu)
#define LOW_44_BITS  (0x00000fffffffffffllu)
#define LOW_45_BITS  (0x00001fffffffffffllu)
#define LOW_46_BITS  (0x00003fffffffffffllu)
#define LOW_47_BITS  (0x00007fffffffffffllu)
#define LOW_48_BITS  (0x0000ffffffffffffllu)
#define LOW_49_BITS  (0x0001ffffffffffffllu)
#define LOW_50_BITS  (0x0003ffffffffffffllu)
#define LOW_51_BITS  (0x0007ffffffffffffllu)
#define LOW_52_BITS  (0x000fffffffffffffllu)
#define LOW_53_BITS  (0x001fffffffffffffllu)
#define LOW_54_BITS  (0x003fffffffffffffllu)
#define LOW_55_BITS  (0x007fffffffffffffllu)
#define LOW_56_BITS  (0x00ffffffffffffffllu)
#define LOW_57_BITS  (0x01ffffffffffffffllu)
#define LOW_58_BITS  (0x03ffffffffffffffllu)
#define LOW_59_BITS  (0x07ffffffffffffffllu)
#define LOW_60_BITS  (0x0fffffffffffffffllu)
#define LOW_61_BITS  (0x1fffffffffffffffllu)
#define LOW_62_BITS  (0x3fffffffffffffffllu)
#define LOW_63_BITS  (0x7fffffffffffffffllu)

#define HIGH_1_BITS  (0x8000000000000000llu)
#define HIGH_2_BITS  (0xc000000000000000llu)
#define HIGH_3_BITS  (0xe000000000000000llu)
#define HIGH_4_BITS  (0xf000000000000000llu)
#define HIGH_5_BITS  (0xf800000000000000llu)
#define HIGH_6_BITS  (0xfc00000000000000llu)
#define HIGH_7_BITS  (0xfe00000000000000llu)
#define HIGH_8_BITS  (0xff00000000000000llu)
#define HIGH_9_BITS  (0xff80000000000000llu)
#define HIGH_10_BITS (0xffc0000000000000llu)
#define HIGH_11_BITS (0xffe0000000000000llu)
#define HIGH_12_BITS (0xfff0000000000000llu)
#define HIGH_13_BITS (0xfff8000000000000llu)
#define HIGH_14_BITS (0xfffc000000000000llu)
#define HIGH_15_BITS (0xfffe000000000000llu)
#define HIGH_16_BITS (0xffff000000000000llu)
#define HIGH_17_BITS (0xffff800000000000llu)
#define HIGH_18_BITS (0xffffc00000000000llu)
#define HIGH_19_BITS (0xffffe00000000000llu)
#define HIGH_20_BITS (0xfffff00000000000llu)
#define HIGH_21_BITS (0xfffff80000000000llu)
#define HIGH_22_BITS (0xfffffc0000000000llu)
#define HIGH_23_BITS (0xfffffe0000000000llu)
#define HIGH_24_BITS (0xffffff0000000000llu)
#define HIGH_25_BITS (0xffffff8000000000llu)
#define HIGH_26_BITS (0xffffffc000000000llu)
#define HIGH_27_BITS (0xffffffe000000000llu)
#define HIGH_28_BITS (0xfffffff000000000llu)
#define HIGH_29_BITS (0xfffffff800000000llu)
#define HIGH_30_BITS (0xfffffffc00000000llu)
#define HIGH_31_BITS (0xfffffffe00000000llu)
#define HIGH_32_BITS (0xffffffff00000000llu)
#define HIGH_33_BITS (0xffffffff80000000llu)
#define HIGH_34_BITS (0xffffffffc0000000llu)
#define HIGH_35_BITS (0xffffffffe0000000llu)
#define HIGH_36_BITS (0xfffffffff0000000llu)
#define HIGH_37_BITS (0xfffffffff8000000llu)
#define HIGH_38_BITS (0xfffffffffc000000llu)
#define HIGH_39_BITS (0xfffffffffe000000llu)
#define HIGH_40_BITS (0xffffffffff000000llu)
#define HIGH_41_BITS (0xffffffffff800000llu)
#define HIGH_42_BITS (0xffffffffffc00000llu)
#define HIGH_43_BITS (0xffffffffffe00000llu)
#define HIGH_44_BITS (0xfffffffffff00000llu)
#define HIGH_45_BITS (0xfffffffffff80000llu)
#define HIGH_46_BITS (0xfffffffffffc0000llu)
#define HIGH_47_BITS (0xfffffffffffe0000llu)
#define HIGH_48_BITS (0xffffffffffff0000llu)
#define HIGH_49_BITS (0xffffffffffff8000llu)
#define HIGH_50_BITS (0xffffffffffffc000llu)
#define HIGH_51_BITS (0xffffffffffffe000llu)
#define HIGH_52_BITS (0xfffffffffffff000llu)
#define HIGH_53_BITS (0xfffffffffffff800llu)
#define HIGH_54_BITS (0xfffffffffffffc00llu)
#define HIGH_55_BITS (0xfffffffffffffe00llu)
#define HIGH_56_BITS (0xffffffffffffff00llu)
#define HIGH_57_BITS (0xffffffffffffff80llu)
#define HIGH_58_BITS (0xffffffffffffffc0llu)
#define HIGH_59_BITS (0xffffffffffffffe0llu)
#define HIGH_60_BITS (0xfffffffffffffff0llu)
#define HIGH_61_BITS (0xfffffffffffffff8llu)
#define HIGH_62_BITS (0xfffffffffffffffcllu)
#define HIGH_63_BITS (0xfffffffffffffffellu)

#define ALL_64_BITS  (0xffffffffffffffffllu)

// @mark: version

/// 64-bit packed representation of a version.
///
/// See `version_pack` and `version_unpack`.
typedef u64 Version;

/// Packs a `major`, `minor`, and `patch` value into a `Version`.
///
/// - The `major` value must be less than 2 ^ 8  (0 - 255)
/// - The `minor` value must be less than 2 ^ 16 (0 - 65535)
/// - The `patch` value must be less than 2 ^ 40 (0 - 1099511627775)
#define version_pack(major, minor, patch) \
  (Version)(((patch) & 0xFFFFFFFFFFllu) | (((minor) & 0xFFFFllu) << 40) | (((major) & 0xFFllu) << 56))

/// Unpacks a `Version` into its `major`, `minor`, and `patch` components.
///
/// `patch` should be a 64-bit value.
#define version_unpack(version, major, minor, patch)                           \
  do {                                                                         \
    *(major) = ((version) >> 56) & 0xFFllu;                                    \
    *(minor) = ((version) >> 40) & 0xFFFFllu;                                  \
    *(patch) = ((version)      ) & 0xFFFFFFFFFFllu;                            \
  } while (0)

#define version_get_maj(version)   (((version) >> 56) & 0xFFllu)
#define version_get_min(version)   (((version) >> 40) & 0xFFFFllu)
#define version_get_patch(version) (((version)      ) & 0xFFFFFFFFFFllu)

// @mark: types

/// A kind of file type.
enum FileKind {
    /// The file's type could not be detected.
    FILE_KIND_UNKNOWN = 0,
    /// The file is a regular file.
    FILE_KIND_REGULAR,
    /// The file is a directory.
    FILE_KIND_DIRECTORY,
    /// The file is a symbolic link.
    FILE_KIND_SYMLINK,
    /// The file's type was detected, but is not covered.
    FILE_KIND_OTHER,
};
typedef enum FileKind FileKind;

struct ArenaBlockFooter {
    u8* base;
    usize used;
    usize capacity;
};
typedef struct ArenaBlockFooter ArenaBlockFooter;

#define get_arena_footer(arena) ((ArenaBlockFooter*)((arena)->base + (arena)->capacity))

/// An arena allocator, used for linearly partitioning a memory block into smaller
/// regions.
struct Arena {
    /// Base pointer of the arena's memory block.
    u8* base;
    /// The number of bytes which the arena has allocated.
    usize used;
    /// The total number of bytes in the arena's memory block.
    usize capacity;

    /// Minimum block size to use for allocating memory blocks.
    usize min_block_size;
};
typedef struct Arena Arena;

/// UTF-8 encoded string type.
struct String {
    /// Pointer to the string's data.
    char* data;
    /// Number of bytes used in the string.
    usize len;
    /// Total number of bytes allocated to the string.
    usize cap;
};
typedef struct String String;

/// Array of `String` elements.
struct StringArray {
    /// Pointer to the array's data.
    String* items;
    /// Number of items in the array.
    usize count;
    /// Number of items allocated to the array.
    usize cap;
};
typedef struct StringArray StringArray;

/// Options to provide when running a command.
struct CommandOptions {
    /// Optional pointer to a `String` to use for capturing the command's output.
    ///
    /// @important: this option cannot be paired with a non-null `async_pid`.
    String* output_string;
    /// Optional pointer to a `ProcessID` to set when the command shouldn't block
    /// immediately. The `ProcessID` value can later be waited on via `proc_wait`.
    ///
    /// @important: this option cannot be paired with either a non-null `output_string` or
    /// a true `silence_output` value.
    ProcessID* async_pid;
    /// Whether to consume and "silence" the standard output and error streams.
    ///
    /// @important: this option cannot be paired with a non-null `async_pid`.
    b32 silence_output;
    /// Optional pointer to an `int` to set to the process' exit code. If the process
    /// failed to spawn, this will be set to `-1`.
    ///
    /// @important: this option cannot be paired with a non-null `async_pid`.
    int* exit_code;
};
typedef struct CommandOptions CommandOptions;

/// A specialised `StringArray` designed to represent a sequence of arguments which can be
/// run as a shell command.
struct Command {
    /// Pointer to the array of arguments in the command.
    String* items;
    /// Number of arguments in the command.
    usize count;
    /// Number of arguments allocated to the command.
    usize cap;
};
typedef struct Command Command;

// @mark: definitions

// @jcg: simply used as a marker.
#define uninit

#ifdef abs
    #undef abs
#endif

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

#define abs(x)           ((x) < 0 ? -(x) : (x))
#define min(a, b)        ((a) < (b) ? (a) : (b))
#define max(a, b)        ((a) > (b) ? (a) : (b))
#define clamp(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define clamp01(x)       clamp((x), 0, 1)
#define is_pow2(x)       ((x) && (((x) & ((x) - 1)) == 0))
#define eps_eq32(a, b)   (abs((a) - (b)) <= F32_EPSILON)
#define eps_eq64(a, b)   (abs((a) - (b)) <= F64_EPSILON)
#define eq032(x)         (abs(x) <= F32_EPSILON)
#define eq064(x)         (abs(x) <= F32_EPSILON)

#define is_lower(ch)        ('a' <= (ch) && (ch) <= 'z')
#define is_upper(ch)        ('A' <= (ch) && (ch) <= 'Z')
#define is_alpha(ch)        (is_lower(ch) || is_upper(ch))
#define is_numeric(ch)      ('0' <= (ch) && (ch) <= '9')
#define is_alphanumeric(ch) (is_alpha(ch) || is_numeric(ch))

#define is_whitespace(ch) ((ch) == ' ' || (ch) == '\n' || (ch) == '\r' || (ch) == '\t')
#define is_decimal(ch)    ((ch) == '.' || (ch) == ',')
#define is_separator(ch)  ((ch) == '\\' || (ch) == '/')

#define kilobytes(num) ((u64)(num) << 10)
#define megabytes(num) ((u64)(num) << 20)
#define gigabytes(num) ((u64)(num) << 30)
#define terabytes(num) ((u64)(num) << 40)

#define countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define btos(boolean) ((boolean) ? "yes" : "no")

#define memz(ptr, bytes) memset((ptr), 0, (bytes))
#define memz_array(ptr, count) memz((ptr), (count) * sizeof((ptr)[0]))

#define streq(a, b)  (strcmp(a, b) == 0)
#define strneq(a, b) (strcmp(a, b) != 0)

#define endian_swap_16(x) ((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
#define endian_swap_32(x) (((x) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | ((x) << 24))
#define endian_swap_64(x)                                                                \
    ((((x) >> 56) & 0x00000000000000ff) | (((x) >> 40) & 0x000000000000ff00) |           \
     (((x) >> 24) & 0x0000000000ff0000) | (((x) >> 8)  & 0x00000000ff000000) |           \
     (((x) << 8)  & 0x000000ff00000000) | (((x) << 24) & 0x0000ff0000000000) |           \
     (((x) << 40) & 0x00ff000000000000) | (((x) << 56) & 0xff00000000000000))

#if CBA_64_BIT
    #define endian_swap_usize(x) endian_swap_64(x)
#else
    #define endian_swap_usize(x) endian_swap_32(x)
#endif

#if CBA_MSVC
    #define CBA_COMPILER_C "cl.exe"
    #define CBA_COMPILER_CPP "cl.exe"
    #define CBA_COMPILER_OUTPUT(output) alloc_sprintf("/Fe:%s", output)
    #define CBA_COMPILER_COMMON_FLAGS "/W4", "/nologo", "/D_CRT_SECURE_NO_WARNINGS"
    #define CBA_COMPILER_DEBUG_FLAGS "/ZI", "/DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "/O3", "/DNDEBUG"
#elif CBA_GCC
    #define CBA_COMPILER_C "gcc"
    #define CBA_COMPILER_CPP "g++"
    #define CBA_COMPILER_OUTPUT(output) "-o", output
    #define CBA_COMPILER_COMMON_FLAGS "-Wall", "-Wextra"
    #define CBA_COMPILER_DEBUG_FLAGS "-ggdb", "-DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "-O3", "-DNDEBUG"
#elif CBA_CLANG
    #define CBA_COMPILER_C "clang"
    #define CBA_COMPILER_CPP "clang++"
    #define CBA_COMPILER_OUTPUT(output) "-o", output
    #define CBA_COMPILER_COMMON_FLAGS "-Wall", "-Wextra"
    #define CBA_COMPILER_DEBUG_FLAGS "-glldb", "-DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "-O3", "-DNDEBUG"
#endif

#define CBA_COMPILER_INPUTS(...) __VA_ARGS__

#ifndef CBA_REBUILD_FAILED_MESSAGE
    #define CBA_REBUILD_FAILED_MESSAGE(binary_name) alloc_sprintf("Failed to rebuild \"%s\"", (binary_name))
#endif
#ifndef CBA_REBUILD_COMPLETED_MESSAGE
    #define CBA_REBUILD_COMPLETED_MESSAGE(binary_name, elapsed_ns) alloc_sprintf("Rebuilt \"%s\" in %s", (binary_name), fmt_time((elapsed_ns), 0))
#endif

#ifndef CBA_REBUILD_COMMAND
    #if defined(__cplusplus)
        #define CBA_REBUILD_COMMAND(output_path, source_path)                      \
            CBA_COMPILER_CPP, CBA_COMPILER_COMMON_FLAGS, CBA_COMPILER_OUTPUT(output_path), CBA_COMPILER_INPUTS(source_path)
    #else
        #define CBA_REBUILD_COMMAND(output_path, source_path)                    \
            CBA_COMPILER_C, CBA_COMPILER_COMMON_FLAGS, CBA_COMPILER_OUTPUT(output_path), CBA_COMPILER_INPUTS(source_path)
    #endif
#endif

CBA_DEF void __cba_rebuild(int argc, char** argv, const char* source_path, ...);

/// Allow the program to rebuild itself when modified.
#define CBA_REBUILD(argc, argv) __cba_rebuild((argc), (argv), __FILE__, NULL)

/// Allow the program to rebuild itself when it or any additional files are modified.
#define CBA_REBUILD_WITH(argc, argv, ...) __cba_rebuild((argc), (argv), __FILE__, __VA_ARGS__, NULL)

// @mark: general

/// Returns the current time in nanoseconds.
///
/// You shouldn't expect values returned from this function to be relative to any time
/// point in particular, but they are always relative to each other.
CBA_DEF u64 nanos_now(void);

/// Sleeps the current thread for `ms` milliseconds.
CBA_DEF void wait_ms(u64 ms);

/// Swaps `len_bytes` bytes between the memory at `a` and `b`.
CBA_DEF void mem_swap(void* a, void* b, usize len_bytes);

/// Whether the current system is little-endian or not.
CBA_DEF b32 is_little_endian();

/// Returns the next power of two value for `x`.
CBA_DEF usize next_pow2(usize x);

/// Whether the provided executable name could be found in the system's PATH.
///
/// On Unix systems this uses `which`, and on Windows it uses `where.exe`.
CBA_DEF b32 has_exe_in_path(const char* exe_name);

/// Whether the current git repository is checked out on a "main" or "master" branch.
CBA_DEF b32 is_main_git_branch();
/// Returns the (short) hash of the currently checked out git commit, if found.
CBA_DEF String git_commit_hash();
/// Returns the full hash of the currently checked out git commit, if found.
CBA_DEF String git_full_commit_hash();
/// Returns the name of the currently checked out git branch, if found.
CBA_DEF String git_branch_name();
/// Returns the name of the committer for the currently checked out git commit, if found.
CBA_DEF String git_committer_name();

// @mark: arena

/// Global arena, used for all cba allocations.
extern Arena global_arena;

/// Allocates at least `size` bytes via the provided arena, returning an address to the
/// resulting memory. Allocations are aligned to the system's cache line size, and are
/// always zeroed by this function.
///
/// If the arena does not have the capacity to allocate `size` bytes, an assertion will
/// fail.
CBA_DEF void* arena_alloc(Arena* arena, usize size);
/// Frees all of the arena's memory blocks and zeroes the arena.
CBA_DEF void arena_free(Arena* arena);

/// Allocates a single instance of a `type`.
#define alloc(type) (type*)arena_alloc(&global_arena, sizeof(type))
/// Allocates a `count` of bytes.
#define alloc_bytes(count) (u8*)arena_alloc(&global_arena, (count))
/// Allocates a `count` of elements of a `type`.
#define alloc_array(count, type) (type*)arena_alloc(&global_arena, (count) * sizeof(type))

/// Returns a pointer to a null-terminated C-string created via a formatted string and
/// optional arguments. The string is allocated via the global arena.
CBA_DEF char* alloc_sprintf(const char* fmt, ...) PRINTF_FORMAT(1, 2);

/// Returns a new copy of the provided `cstr` which is surrounded with double-quotes.
///
/// For example:
/// ```
/// const char* cstr = "hello world";
/// surround_dq(cstr); // -> "hello world"
/// ```
CBA_DEF char* surround_dq(const char* cstr);
/// Returns a new copy of the provided `cstr` which is surrounded with single-quotes.
///
/// For example:
/// ```
/// const char* cstr = "hello world";
/// surround_dq(cstr); // -> 'hello world'
/// ```
CBA_DEF char* surround_sq(const char* cstr);
/// Returns a new copy of the provided `cstr` which is surrounded with back-quotes.
///
/// For example:
/// ```
/// const char* cstr = "hello world";
/// surround_dq(cstr); // -> `hello world`
/// ```
CBA_DEF char* surround_bq(const char* cstr);

// @mark: files

/// If any files in the `input_paths` array have been modified since the file at
/// `output_path`, `1` is returned and `0` otherwise. If an error occurs, `-1` is
/// returned.
CBA_DEF i32 files_need_rebuild(String output_path, StringArray input_paths);
/// If the file at `input_path` has been modified since the file at `output_path`, `1` is
/// returned and `0` otherwise. If an error occurs (i.e. a filesystem error), `-1` is
/// returned.
CBA_DEF i32 file_needs_rebuild(String output_path, String input_path);

/// Creates a file at `path`, returning true if the operation succeeded.
CBA_DEF b32 file_create(const char* path);
/// Moves the file at `path` to `new_path`, returning true if the operation succeeded.
///
/// If a file exists at the `new_path`, the file will be overwritten.
CBA_DEF b32 file_move(const char* path, const char* new_path);
/// Copies the file at `path` to `new_path`, returning true if the operation succeeded.
///
/// If a file exists at the `new_path`, the file will be overwritten.
///
/// On Unix systems, `symbolic_link` will make `new_path` a symbolic link to the file at
/// `path`. On Windows, setting `symbolic_link` to `true` will have no effect.
///
/// You can use `#if CBA_WINDOWS` or `#ifdef _WIN32` to check the platform.
CBA_DEF b32 file_copy(const char* path, const char* new_path, b32 symbolic_link);
/// Deletes the file at `path`, returning true if the operation succeeded or if the file
/// did not exist. If the file is a directory, its contents will be deleted recursively.
CBA_DEF b32 file_delete(const char* path);
/// Whether a file at `path` exists.
CBA_DEF b32 file_exists(const char* path);
/// Returns the type of the file at `path`.
CBA_DEF FileKind file_get_kind(const char* path);
/// Returns the length of the file in bytes.
CBA_DEF usize file_length(const char* path);
/// Reads a number of `bytes` from the file into the `dest` memory.
CBA_DEF b32 file_read(const char* path, void* dest, usize bytes);
/// Writes a number of `bytes` from the provided `memory` to the file, optionally
/// appending to the file.
CBA_DEF b32 file_write(const char* path, void* memory, usize bytes, b32 append);
/// Attempts to make a directory at `path` if the directory does not already exist.
///
/// If the directory was successfully created or already exists, this function returns
/// `true`. If an error occurred, it returns `false`.
///
/// The operation is recursive, so you can provide nested directories and they will
/// all be created. For example:
///
/// `file_try_create_directory("a/b/c/d");`
///
/// Will create all non-existing directories.
CBA_DEF b32 file_try_create_directory(const char* path);
/// Attempts to return the file names of all entries within a directory at `path`. If this
/// fails, the resulting array will be zeroed.
///
/// If `include_directory_path` is `true`, the resulting strings will include the
/// directory path. 
///
/// For example, for a directory `/a/b` containing files `c.txt` and `d.txt`:
/// `str_to_directory_entries(path, true); // -> { "/a/b/c.txt", "/a/b/d.txt" }`
CBA_DEF StringArray file_get_directory_entries(const char* path, b32 include_directory_path);

// @mark: processes

/// Attempts to spawn a new process with the provided `cmd`, which will be invoked by the
/// system's shell.
///
/// If the process failed to spawn, the resulting `ProcessID` will be `INVALID_HANDLE`.
///
/// This function needs to permanently allocate memory - be sure not to wrap it in a
/// temporary memory block!
CBA_DEF ProcessID proc_start(Command cmd, FileDescriptor output_fd);

/// Blocks the current thread until the provided process has terminated, returning the
/// result:
/// - `-1`: the process could not be waited on
/// - `0`: the process returned a non-zero exit code, or was terminated by a signal
/// - `1`: the process exited normally
///
/// You may optionally pass an `int` pointer to be set to the process' exit code, or `1`
/// if the process was signalled. If the process did not return (i.e. another error
/// occurred and this function returns -1), the exit code is not set.
///
/// The provided `ProcessID` cannot be `INVALID_HANDLE`.
CBA_DEF i32 proc_wait(ProcessID proc, int* exit_code);

CBA_DEF i32 __proc_wait_va(usize n, ...);

/// Waits on any number of `ProcessID` values.
#define procs_wait(...) \
    __proc_wait_va((sizeof((ProcessID[]) { __VA_ARGS__ }) / sizeof(ProcessID)), __VA_ARGS__)

// @mark: string

/// Wraps a string literal with a `String`.
#define strl(literal) ((String) { .data = (char*)(literal), .len = sizeof(literal) - 1, .cap = sizeof(literal) })
/// Creates the printf-style formatting arguments for the provided `String`.
#define sfmt(s) (int)((s).len), (const char*)((s).data)
/// Expands to a printf-style formatting sequence for printing a `String`.
#define stok "`%.*s`"

/// Clears the string (sets its length to 0), and zeroes its memory.
CBA_DEF void str_clear(String* str);

/// Allocates an empty string with a capacity of `CBA_MIN_STRING_CAPACITY` bytes.
CBA_DEF String str_alloc(void);
/// Allocates an empty string with `cap` bytes.
CBA_DEF String str_alloc_with_cap(usize cap);
/// Allocates a formatted string based on the provided format string and arguments.
CBA_DEF String str_sprintf(const char* fmt, ...) PRINTF_FORMAT(1, 2);
/// Allocates a string from the provided null-terminated C-string. The resulting string's
/// data is null-terminated, but is not included in its length.
CBA_DEF String str_from_cstr(const char* cstr);
/// Allocates a string from the provided character buffer.
CBA_DEF String str_from_chars(char* buffer, usize count);
/// Allocates a string containing the contents of the file at the provided `file_path`. If
/// the file couldn't be read, the returned string will be zeroed.
CBA_DEF String str_from_file(const char* file_path);
/// Returns an absolute path to the current working directory (i.e., wherever the program
/// was run from).
CBA_DEF String str_from_cwd(void);

/// Writes the contents of `s` to a file at `path`, optionally appending the data to the
/// file.
CBA_DEF b32 str_write_to_file(String s, const char* path, b32 append);

/// Creates a "slice" of the provided `String`, with the provided `start` position and `len`.
CBA_DEF String str_slice(String str, usize start, usize len);
/// "Shrinks" the provided `String` by `shift` elements from the left.
///
/// For example:
/// ```
/// String s = strl("abcde");
/// str_shrink_left(&s, 2); // -> becomes "cde"
/// ```
CBA_DEF void str_shrink_left(String* str, usize shift);
/// "Shrinks" the provided `String` by `shift` elements from the right.
///
/// For example:
/// ```
/// String s = strl("abcde");
/// str_shrink_right(&s, 2); // -> becomes "abc"
/// ```
CBA_DEF void str_shrink_right(String* str, usize shift);

/// Returns a slice of the provided `str` which includes only the file name of a full file
/// path and optionally its extension. If there is no root path or extension, the original
/// string is returned.
CBA_DEF String str_path_file_name(String str, b32 include_extension);
/// Returns a slice of the provided `str` which includes only the file extension of a full
/// file path. If an extension couldn't be found, the returned string will be zeroed.
CBA_DEF String str_path_file_extension(String str);
/// Returns a slice of the provided `str` which includes only the parent path of a full
/// file path. If a root path couldn't be found, the returned string will be zeroed.
CBA_DEF String str_path_pwd(String str);
/// Returns a string containing an absolute path obtained from `str`.
///
/// This will expand relative paths such as `"../file"` or `"."`.
CBA_DEF String str_path_to_absolute(String str);
/// Attempts to split the provided file `path` string into all of its parent paths,
/// starting with the root directory and ending with the parent directory of the file (if
/// any). If this fails, the resulting array will be zeroed.
///
/// For example, `/a/b/c/d/file.txt` would be split into:
/// `{ "/a", "/a/b", "/a/b/c", "/a/b/c/d" }`
CBA_DEF StringArray str_to_parent_paths(String path);
/// Returns a full copy of the provided `str` which includes only the file name of a full
/// file path and optionally can `include_extension`. If this fails, the resulting string
/// will be zeroed.
CBA_DEF String str_path_copy_file_name(String str, b32 include_extension);
/// Returns a full copy of the provided `str` which includes only the file extension of a
/// full file path. If this fails, the resulting string will be zeroed.
CBA_DEF String str_path_copy_file_extension(String str);
/// Returns a full copy of the provided `str` which includes only the parent path of a
/// full file path. If this fails, the resulting string will be zeroed.
CBA_DEF String str_path_copy_pwd(String str);

/// Creates a deep copy of the provided `str`: new memory is allocated for the resulting
/// string.
CBA_DEF String str_copy(String str);
/// Copies the contents and length of `source` into `dest`.
CBA_DEF void str_copy_into(String* dest, String source);

/// Appends a null character to the provided string.
CBA_DEF void str_append_null(String* str);
/// Appends the platform's line ending to the provided string. On Windows this is `\r\n`,
/// otherwise it's `\n`.
CBA_DEF void str_append_line_ending(String* str);
/// Appends the provided character to the provided string.
CBA_DEF void str_append_char(String* str, char ch);
/// Appends the provided null-terminated C-string to the provided string. The
/// null-terminatoris not included.
CBA_DEF void str_append_cstr(String* str, const char* cstr);
/// Appends the provided character buffer to the provided string.
CBA_DEF void str_append_chars(String* str, char* buffer, usize count);
/// Appends the contents of `other` to the provided string.
CBA_DEF void str_append_other(String* str, String other);
/// Appends formatted string to the provided string.
CBA_DEF void str_appendf(String* str, const char* fmt, ...) PRINTF_FORMAT(2, 3);

/// Sets the provided string's characters to lowercase.
CBA_DEF void str_to_lower(String* str);
/// Sets the provided string's characters to uppercase.
CBA_DEF void str_to_upper(String* str);

/// Shifts the provided string's contents by `shift` elements to the left, beginning at
/// the `start` index. The `start` index is included in the shift, and the shifted region
/// extends until the end of the string.
///
/// Note that if the shift would underflow the beginning of the string, this function will
/// panic.
CBA_DEF void str_lshift(String* str, usize start, usize shift);
/// Shifts the provided string's contents by `shift` elements to the right, beginning at
/// the `start` index. The `start` index is included in the shift, and the shifted region
/// extends until the end of the string. Elements which precede the shifted region are set
/// to zero.
///
/// Note that if the shift would overflow the end of the string, this function will panic.
CBA_DEF void str_rshift(String* str, usize start, usize shift);

/// Inserts the provided `ch` character to `at` in the provided string.
CBA_DEF void str_insert_char(String* str, usize at, char ch);
/// Inserts the provided `other` string to `at` in the provided string.
CBA_DEF void str_insert_other(String* str, usize at, String other);
/// Inserts the null-terminated C-string to `at` in the provided string.
CBA_DEF void str_insert_cstr(String* str, usize at, const char* cstr);

/// Removes the character at index `at` from the provided string.
CBA_DEF void str_remove(String* str, usize at);
/// Removes the provided range of characters from the provided string. The range starts at
/// and includes `start` and extends up to, but does not include, `end`.
CBA_DEF void str_remove_range(String* str, usize start, usize end);

/// Replaces all instances of the `from` character with the `to` character.
CBA_DEF void str_replace_chars(String* str, char from, char to);
/// Replaces all instances of the `from` string with the `to` string.
CBA_DEF void str_replace_others(String* str, String from, String to);
/// Replaces all instances of the null-terminated `from` C-string with the null-terminated
/// `to` C-string.
CBA_DEF void str_replace_cstrs(String* str, const char* from, const char* to);

/// Trims all characters in the null-terminated `delims` C-string from the start and end
/// of the provided `string`, returning `true` if any characters were trimmed.
CBA_DEF b32 str_trim_chars(String* str, const char* delims);
/// Trims all whitespace characters from the start and end of the provided `string`. This
/// includes: ' ', '\n', '\r', '\t', '\v', '\f', returning `true` if any characters were
/// trimmed.
CBA_DEF b32 str_trim_whitespace(String* str);
/// Trims all null characters (`'\0'`) from the start and end of the provided `string`. If
/// any characters were trimmed, `true` is returned.
CBA_DEF b32 str_trim_null(String* str);

/// Splits the provided `String` by `delim`, returning an array of the separated strings.
/// The `delim` character will not be included in any of the resulting strings.
///
/// If no delimiters are found, the resulting array will simply contain the original
/// string.
CBA_DEF StringArray str_split_by(String str, char delim);
/// Splits the provided `String` by newline characters, returning an array of the
/// separated lines. No newline characters will not be included in the resulting strings.
///
/// This function considers `\r\n` sequences to be newlines, and will omit them from the
/// results.
///
/// If no newlines are found, the resulting array will simply contain the original
/// string.
CBA_DEF StringArray str_split_lines(String str);

// @todo: case-insensitive versions of below?

/// Whether `a` is equivalent to `b`.
CBA_DEF b32 str_eq(String a, String b);
/// Whether `a` is equivalent to the null-terminated `b` C-string.
CBA_DEF b32 str_eq_cstr(String str, const char* cstr);

/// Whether `a` is equivalent to `b`, regardless of case.
CBA_DEF b32 str_eq_ignoring_case(String a, String b);
/// Whether `a` is equivalent to the null-terminated `b` C-string, regardless of case.
CBA_DEF b32 str_eq_cstr_ignoring_case(String str, const char* cstr);

/// Whether `str` starts with `cstr` (excluding a null-terminator).
CBA_DEF b32 str_starts_with(String str, const char* cstr);
/// Whether `str` ends with `cstr` (excluding a null-terminator).
CBA_DEF b32 str_ends_with(String str, const char* cstr);

/// Whether any characters in the `needles` C-string could be found in `haystack`. When
/// `case_sensitive` is false, case is ignored for alphabetic characters. When `where` is
/// non-NULL, it is set to the index of the first matching character, if found.
CBA_DEF b32 str_find_first_of_any_in_cstr(String haystack, const char* needles, b32 case_sensitive, usize* where);
/// Whether any characters in the `needles` array of `count` elements could be found in
/// `haystack`. When `case_sensitive` is false, case is ignored for alphabetic characters.
/// When `where` is non-NULL, it is set to the index of the first matching character, if
/// found.
CBA_DEF b32 str_find_first_of_any(String haystack, const char* needles, usize count, b32 case_sensitive, usize* where);

/// Whether any characters in the `needles` C-string could be found in `haystack`. When
/// `case_sensitive` is false, case is ignored for alphabetic characters. When `where` is
/// non-NULL, it is set to the index of the last matching character, if found.
CBA_DEF b32 str_find_last_of_any_in_cstr(String haystack, const char* needles, b32 case_sensitive, usize* where);
/// Whether any characters in the `needles` array of `count` elements could be found in
/// `haystack`. When `case_sensitive` is false, case is ignored for alphabetic characters.
/// When `where` is non-NULL, it is set to the index of the last matching character, if
/// found.
CBA_DEF b32 str_find_last_of_any(String haystack, const char* needles, usize count, b32 case_sensitive, usize* where);

/// Whether `needle` could be found in `haystack`. When `where` is non-NULL, it is set to
/// the index of the first matching character, if found.
CBA_DEF b32 str_find_first_char(String haystack, char needle, usize* where);
/// Whether `needle` could be found in `haystack`. When `where` is non-NULL, it is set to
/// the index of the last matching character, if found.
CBA_DEF b32 str_find_last_char(String haystack, char needle, usize* where);
/// Whether `needle` could be found in `haystack`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters. When `where` is non-NULL, it is set to the index of
/// the first matching string, if found.
CBA_DEF b32 str_find_first_other(String haystack, String needle, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters. When `where` is non-NULL, it is set to the index of
/// the last matching string, if found.
CBA_DEF b32 str_find_last_other(String haystack, String needle, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters. When `where` is non-NULL, it is set to the index of
/// the first matching string, if found.
CBA_DEF b32 str_find_first_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters. When `where` is non-NULL, it is set to the index of
/// the last matching string, if found.
CBA_DEF b32 str_find_last_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where);

/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing forwards. When `where` is non-NULL, it is set to the index of the
/// first matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_first_char_from(String haystack, char needle, usize from, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing backwards. When `where` is non-NULL, it is set to the index of the
/// last matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_last_char_from(String haystack, char needle, usize from, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing forwards. When `where` is non-NULL, it is set to the index of the
/// first matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_first_other_from(String haystack, String needle, usize from, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing backwards. When `where` is non-NULL, it is set to the index of the
/// last matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_last_other_from(String haystack, String needle, usize from, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing forwards. When `where` is non-NULL, it is set to the index of the
/// first matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_first_cstr_from(String haystack, const char* needle, usize from, b32 case_sensitive, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing backwards. When `where` is non-NULL, it is set to the index of the
/// last matching character, if found. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
CBA_DEF b32 str_find_last_cstr_from(String haystack, const char* needle, usize from, b32 case_sensitive, usize* where);

/// Returns the number of characters matching the `needle` char in the provided string.
CBA_DEF u64 str_count_chars(String haystack, char needle);
/// Returns the number of matches with the `needle` C-string in the provided string.
/// Null-terminators are not considered.
CBA_DEF u64 str_count_cstrs(String haystack, const char* needle, b32 case_sensitive);
/// Returns the number of matches with the `needle` string in the provided string.
CBA_DEF u64 str_count_others(String haystack, String needle, b32 case_sensitive);

/// Whether `haystack`, contains `needle`.
///
/// This function searches forwards: use `str_find_last_char` if you need to search from
/// the end of the string.
CBA_DEF b32 str_contains_char(String haystack, char needle);
/// Whether `haystack`, contains `needle`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
///
/// This function searches forwards: use `str_find_last_cstr` if you need to search from
/// the end of the string.
CBA_DEF b32 str_contains_cstr(String haystack, const char* needle, b32 case_sensitive);
/// Whether `haystack`, contains `needle`. When `case_sensitive` is false, case is
/// ignored for alphabetic characters.
///
/// This function searches forwards: use `str_find_last_other` if you need to search from
/// the end of the string.
CBA_DEF b32 str_contains_other(String haystack, String needle, b32 case_sensitive);

/// Attempts to parse the string `str` to a `i64` value, returning `true` if successful.
CBA_DEF b32 str_parse_to_i64(String str, i64* dest);
/// Attempts to parse the string `str` to a `f64` value, returning `true` if successful.
CBA_DEF b32 str_parse_to_f64(String str, f64* dest);

/// Chops any characters in `src` which precede the first occurence of `ch`, and places
/// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
CBA_DEF b32 str_chop_up_to_char(String* src, String* dest, char ch);
/// Chops any characters in `src` which precede the first occurence of `cstr`, and places
/// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
CBA_DEF b32 str_chop_up_to_cstr(String* src, String* dest, const char* cstr, b32 case_sensitive);
/// Chops any characters in `src` which precede the first occurence of `cstr`, and places
/// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
CBA_DEF b32 str_chop_up_to_other(String* src, String* dest, String other, b32 case_sensitive);

// /// Chops any characters in `src` which precede the first occurence of `ch`, and places
// /// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
// CBA_DEF b32 str_chop_back_to_char(String* src, String* dest, char ch);
// /// Chops any characters in `src` which precede the first occurence of `cstr`, and places
// /// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
// CBA_DEF b32 str_chop_back_to_cstr(String* src, String* dest, const char* cstr, b32 case_sensitive);
// /// Chops any characters in `src` which precede the first occurence of `cstr`, and places
// /// the "chopped" characters into `dest`. If `ch` was found, this function returns true.
// CBA_DEF b32 str_chop_back_to_other(String* src, String* dest, String other, b32 case_sensitive);

/// Returns a string of the current time, formatted as "HH:MM:SS" (24-hour time).
CBA_DEF String str_from_current_time();
/// Returns a string of the current date, formatted as "DD/MM/YYYY".
CBA_DEF String str_from_current_date();

/// Returns the Levenshtein distance between strings `a` and `b`, which is the number of
/// changes required to convert one string into the other.
CBA_DEF usize str_levenshtein_distance(String a, String b);
/// Returns a similarity from `0.0` to `1.0` of the strings `a` and `b` using a
/// Levenshtein distance algorithm.
CBA_DEF f32 str_levenshtein_similarity(String a, String b);

/// Allocates and returns a null-terminated string containing the data of the provided string.
CBA_DEF char* str_to_cstr(String str);

/// Returns a formatted pretty string of the amount of memory represented by `num_bytes`.
///
/// For example:
/// `1234     -> "1.205 KB"`
/// `1234567  -> "1.177 MB"`
/// `56324857 -> "53.716 MB"`
CBA_DEF char* fmt_bytes(usize num_bytes);

/// Creates a formatted string of the provided `u8` as binary.
///
/// The result is always in big-endian.
CBA_DEF char* fmt_binary8(u8 b);
/// Creates a formatted string of the provided `u16` as binary.
///
/// The result is always in big-endian.
CBA_DEF char* fmt_binary16(u16 b);
/// Creates a formatted string of the provided `u32` as binary.
///
/// The result is always in big-endian.
CBA_DEF char* fmt_binary32(u32 b);
/// Creates a formatted string of the provided `u64` as binary.
///
/// The result is always in big-endian.
CBA_DEF char* fmt_binary64(u64 b);

/// Returns a formatted pretty string of the amount of time represented by `nanos`.
///
/// `verbosity` describes how verbose units are:
/// - `0`: "ns",          "ms",           "s",       etc.
/// - `1`: "nanos",       "millis",       "secs",    etc.
/// - `2`: "nanoseconds", "milliseconds", "seconds", etc.
CBA_DEF char* fmt_time(u64 nanos, u8 verbosity);

/// Returns a formatted pretty string of a `Version`.
///
/// For example:
/// Version v = version_pack(1, 2, 345);
/// print("%s", fmt_version(v)); // "1.2.345"
CBA_DEF const char* fmt_version(Version v);

// @mark: string array

/// Appends a single `String` to the provided `StringArray`.
CBA_DEF void str_arr_append_str(StringArray* arr, String str);

CBA_DEF void __str_arr_append_va(StringArray* arr, usize n, ...);

/// Appends any number of `String`s to the provided `StringArray`.
#define str_arr_append(arr, ...)                                             \
    __str_arr_append_va((arr), (sizeof((String[]){__VA_ARGS__}) / sizeof(String)), __VA_ARGS__)

CBA_DEF void __str_arr_append_cstrs_va(StringArray* arr, usize n, ...);

/// Appends any number of C-strings to the provided `StringArray`.
#define str_arr_append_cstrs(arr, ...) \
    __str_arr_append_cstrs_va((arr), (sizeof((const char*[]){__VA_ARGS__}) / sizeof(const char*)), __VA_ARGS__)

/// Converts an array of C-strings to a `StringArray`, allocating space for each new
/// string.
CBA_DEF StringArray str_arr_from_cstr_arr(char** arr, usize count);

/// Appends the contents of the `other` array to the `arr` array.
CBA_DEF void str_arr_concat(StringArray* arr, StringArray other);

/// Concatenates each element from a `StringArray` to a single `String`, separating each
/// element by the provided `separator` C-string.
CBA_DEF String str_arr_flatten_to_str(StringArray arr, const char* separator);

// @mark: command

/// Appends a `String` to the provided `Command`.
CBA_DEF void cmd_append_str(Command* cmd, String str);
/// Appends a `StringArray` to the provided `Command`.
CBA_DEF void cmd_append_str_arr(Command* cmd, StringArray arr);

CBA_DEF void __cmd_append_va(Command* cmd, usize n, ...);
/// Appends any number of C-strings to the provided `Command`.
#define cmd_append(cmd, ...)                                                   \
    __cmd_append_va((cmd), (sizeof((const char*[]) { __VA_ARGS__ }) / sizeof(const char*)), __VA_ARGS__)

/// Appends the contents of the `other` command to the `cmd` command.
CBA_DEF void cmd_concat(Command* cmd, Command other);
/// "Resets" the command (sets its count to 0 and clears its strings).
CBA_DEF void cmd_reset(Command* cmd);

/// Appends an entire C-string to a command, splitting by spaces. If arguments are
/// surrounded by either `'` or `"` characters, they are appended as whole arguments.
///
/// For example:
///
/// `cmd_append_split(&cmd, "'hello there' from the \"split command\"");`
/// 
/// Produces `{ "hello there", "from", "the", "split command" }`.
CBA_DEF void cmd_append_split(Command* cmd, const char* args);

/// Runs the provided command with the provided options.
///
/// If the command is asynchronous, this returns `true` if the process was started.
/// Otherwise, this returns `true` if the processed exited successfully.
///
/// See also `cmd_try_run`.
CBA_DEF b32 cmd_try_run_with_opts(Command cmd, CommandOptions opts);

/// Runs the provided command with default options.
///
/// If the command is asynchronous, this returns `true` if the process was started.
/// Otherwise, this returns `true` if the processed exited successfully.
#define cmd_try_run(cmd, ...) \
    cmd_try_run_with_opts((cmd), CBA_LITERAL(CommandOptions) { __VA_ARGS__ })

/// Runs the provided command with default options, and asserts that the command succeeds.
#define cmd_run(cmd, ...)                                                                 \
    cba_assert(cmd_try_run_with_opts((cmd), CBA_LITERAL(CommandOptions) { __VA_ARGS__ }), \
               "failed to run command `%.*s`",                                            \
               sfmt(cmd_flatten(cmd)))

/// Runs the whole `command` with the provided options.
///
/// Arguments surrounded by either `'` or `"` characters are treated as whole arguments.
///
/// If the command is asynchronous, this returns `true` if the process was started.
/// Otherwise, this returns `true` if the processed exited successfully.
CBA_DEF b32 cmd_try_run_direct_with_opts(const char* command, CommandOptions opts);

/// Runs the whole `command` with default options.
///
/// Arguments surrounded by either `'` or `"` characters are treated as whole arguments.
///
/// If the command is asynchronous, this returns `true` if the process was started.
/// Otherwise, this returns `true` if the processed exited successfully.
#define cmd_try_run_direct(cmd, ...) \
    cmd_try_run_direct_with_opts((cmd), CBA_LITERAL(CommandOptions) { __VA_ARGS__ })

/// Runs the whole `command` with default options, and asserts that the commands suceeds.
///
/// Arguments surrounded by either `'` or `"` characters are treated as whole arguments.
#define cmd_run_direct(cmd, ...)                                                                 \
    cba_assert(cmd_try_run_direct_with_opts((cmd), CBA_LITERAL(CommandOptions) { __VA_ARGS__ }), \
               "failed to run command `%s`",                                                     \
               cmd)

/// Concatenates all arguments in a command into a string using spaces.
///
/// If a singular argument in the command contains a space, then the string will be
/// surrounded by `"`.
CBA_DEF String cmd_flatten(Command cmd);
/// Concatenates all arguments in a command into a string using spaces.
///
/// If a singular argument in the command contains a space, then the string will be
/// surrounded by `delim`.
CBA_DEF String cmd_flatten_with_delims(Command cmd, char delim);
/// Concatenates all arguments in a command into a C-string using spaces.
///
/// If a singular argument in the command contains a space, then the string will be
/// surrounded by `"`.
CBA_DEF char* cmd_flatten_to_cstr(Command cmd);
/// Concatenates all arguments in a command into a C-string using spaces.
///
/// If a singular argument in the command contains a space, then the string will be
/// surrounded by `delim`.
CBA_DEF char* cmd_flatten_to_cstr_with_delims(Command cmd, char delim);

#ifdef __cplusplus
    #define _DECLTYPE_CAST(x) (decltype(x))
#else
    #define _DECLTYPE_CAST(x)
#endif

#define da_reserve(arr, new_cap)                                                                         \
    do {                                                                                                 \
        if ((new_cap) > (arr)->cap) {                                                                    \
            if (!(arr)->cap) {                                                                           \
                (arr)->cap = CBA_MIN_ARRAY_CAPACITY;                                                     \
            }                                                                                            \
            (arr)->cap = next_pow2(new_cap);                                                             \
            void* new_data = arena_alloc(&global_arena, (arr)->cap * sizeof(*(arr)->items));             \
            memcpy(new_data, (arr)->items, (arr)->count * sizeof(*(arr)->items));                        \
            (arr)->items = _DECLTYPE_CAST((arr)->items)new_data;                                         \
            cba_assert((arr)->items, "failed to reallocate %zu elements for dynamic array", (arr)->cap); \
        }                                                                                                \
    } while (0)

#define da_append(arr, element)                   \
    do {                                          \
        da_reserve((arr), (arr)->count + 1);      \
        (arr)->items[(arr)->count++] = (element); \
    } while (0)
#define da_append_many(arr, elements, element_count)                                              \
    do {                                                                                          \
        da_reserve((arr), (arr)->count + (element_count));                                        \
        memcpy((arr)->items + (arr)->count, (elements), (element_count) * sizeof(*(arr)->items)); \
        (arr)->count += (element_count);                                                          \
    } while (0)





// @mark: implementation

#ifdef CBA_IMPLEMENTATION

Arena global_arena = {0};

#if CBA_WINDOWS
#define CBA_WIN32_ERR_MSG_SIZE (4 << 10) // 4 KB

CBA_DEF char* win32_err_message(DWORD err) {
    char* result = NULL;

    static char buffer[CBA_WIN32_ERR_MSG_SIZE] = {0};
    DWORD msg_size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
        LANG_USER_DEFAULT, buffer, CBA_WIN32_ERR_MSG_SIZE, NULL);

    if (msg_size == 0) {
        if (GetLastError() != ERROR_MR_MID_NOT_FOUND) {
            if (sprintf(buffer, "Could not get error message for 0x%lX", err) > 0) {
                result = (char*)buffer;
            }
        }
        else if (sprintf(buffer, "Invalid Windows error code: 0x%lX", err) > 0) {
            result = (char*)buffer;
        }
    }
    else {
        // trim trailing whitespace.
        while (msg_size > 1 && is_whitespace(buffer[msg_size - 1])) {
            msg_size -= 1;
            buffer[msg_size] = '\0';
        }
    }

    return result;
}


static inline const char* _os_error() {
    return win32_err_message(GetLastError());
}

#else
static inline const char* _os_error() {
    return strerror(errno);
}

#endif


CBA_DEF void __cba_rebuild(int argc, char** argv, const char* source_path, ...) {
    u64 start_ns = nanos_now();
    CBA_UNUSED(start_ns);

    int exit_code = 0;

    String binary_path = str_from_cstr(argv[0]);

#if CBA_WINDOWS
    if (str_starts_with(binary_path, ".\\")) {
        str_lshift(&binary_path, 2, 2);
    }
#endif

#if CBA_WINDOWS
    if (!str_ends_with(binary_path, ".exe")) {
        str_append_cstr(&binary_path, ".exe");
    }
#endif

    String old_binary_path = str_copy(binary_path);
    str_append_cstr(&old_binary_path, ".bak");
    
    const char* binary_path_cstr = binary_path.data;
    const char* old_binary_path_cstr = old_binary_path.data;

    // @jcg: try to remove a previously backed-up executable. It doesn't really matter if
    // it fails - this just helps to keep the file tree a little cleaner.
    if (file_exists(old_binary_path_cstr)) {
        file_delete(old_binary_path_cstr);
    }

    StringArray source_paths = {0};
    str_arr_append_cstrs(&source_paths, source_path);

    // @jcg: if this header is found in the root directory then it too can be watched,
    // which is particularly useful when developing cba in its own repository.
    if (file_exists("cba.h")) {
        str_arr_append_cstrs(&source_paths, "cba.h");
    }

    uninit va_list args;
    va_start(args, source_path);
    for (;;) {
        const char* path = va_arg(args, const char*);
        if (!path) break;

        str_arr_append_cstrs(&source_paths, path);
    }
    va_end(args);

    i32 rebuild_needed = files_need_rebuild(binary_path, source_paths);

    if (rebuild_needed == -1) {
        exit_code = 1;
    }
    else if (rebuild_needed == 1) {
        Command cmd = {0};

        // @jcg: a backup of the previous executable has to be created in case the rebuild
        // fails. Ideally it'd be removed after a successful rebuild, but the file can't
        // be deleted while mapped to memory on all operating systems. This function tries
        // to remove the backed up file when the program is next run, but it can't remove
        // it before then.

        if (file_move(binary_path_cstr, old_binary_path_cstr)) {
            cmd_append(&cmd, CBA_REBUILD_COMMAND(argv[0], source_path));

            b32 success = cmd_try_run(cmd);

            if (success) {
#if defined(CBA_PRINT_ON_REBUILD) || defined(CBA_VERBOSE)
                if (exit_code == 0) {
                    info("%s", CBA_REBUILD_COMPLETED_MESSAGE(binary_path_cstr, nanos_now() - start_ns));
                }
#endif

                // re-run the previous command with the new binary.
                StringArray cmd_args = str_arr_from_cstr_arr(argv + 1, (usize)(argc - 1));

                cmd_reset(&cmd);
                cmd_append_str(&cmd, binary_path);
                cmd_append_str_arr(&cmd, cmd_args);

                success = cmd_try_run(cmd);

                if (!success) {
                    exit_code = 1;
                }
            }
            else {
#if defined(CBA_PRINT_ON_REBUILD) || defined(CBA_VERBOSE)
                error("%s", CBA_REBUILD_FAILED_MESSAGE(binary_path_cstr));
#endif
                cba_assert(file_move(old_binary_path_cstr, binary_path_cstr),
                           "failed to move old binary back to the current one");
                exit_code = 1;
            }
        }
        else {
            exit_code = 1;
        }
    }

    if (rebuild_needed != 0) {
        exit(exit_code);
    }
}


CBA_DEF u64 nanos_now(void) {
    u64 result = 0;

#if CBA_WINDOWS
    uninit LARGE_INTEGER freq, counter;
    cba_assert(QueryPerformanceFrequency(&freq), "failed to obtain performance counter frequency");
    cba_assert(QueryPerformanceCounter(&counter), "failed to obtain performance counter");

    result = counter.QuadPart * (1000000000 / freq.QuadPart);
#else
    result = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
#endif

    return result;
}


CBA_DEF void wait_ms(u64 ms) {
#if CBA_WINDOWS
    Sleep((DWORD)ms);
#else
    u64 secs = ms / 1000;
    u64 nanos = (ms - (secs * 1000)) * 1000000;

    struct timespec duration = {
        .tv_sec  = (long)secs,
        .tv_nsec = (long)nanos,
    };

    nanosleep(&duration, NULL);
#endif
}


CBA_DEF void mem_swap(void* a, void* b, usize len_bytes) {
    u8* lhs = (u8*)a;
    u8* rhs = (u8*)b;

    while (len_bytes--) {
        u8 tmp = *lhs;
        *lhs = *rhs;
        *rhs = tmp;
        lhs += 1;
        rhs += 1;
    }
}


CBA_DEF b32 is_little_endian() {
    u16 x = 1;
    return *((u8*)&x);
}


CBA_DEF usize next_pow2(usize x) {
    uninit usize result;

    if (x == 0) {
        result = 1;
    }
    else {
        if (is_pow2(x)) {
            x += 1;
        }

        x -= 1;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        x += 1;

        result = x;
    }

    return result;
}


CBA_DEF b32 has_exe_in_path(const char* exe_name) {
    b32 result = false;

#if CBA_WINDOWS
    result = cmd_try_run_direct(alloc_sprintf("where.exe /q '%s'", exe_name));
#else
    result = cmd_try_run_direct(alloc_sprintf("which -s '%s'", exe_name));
#endif

    return result;
}


CBA_DEF b32 is_main_git_branch() {
    b32 result = false;

    String capture = {0};
    if (cmd_try_run_direct("git branch", .output_string = &capture) &&
        (str_contains_cstr(capture, "main", false) ||
         str_contains_cstr(capture, "master", false))) {
        result = true;
    }

    return result;
}


CBA_DEF String git_commit_hash() {
    String result = str_from_cstr("[UNKNOWN COMMIT HASH]");

    if (cmd_try_run_direct("git log --pretty=format:%h -n 1", .output_string = &result)) {
        str_trim_whitespace(&result);
    }

    return result;
}


CBA_DEF String git_full_commit_hash() {
    String result = str_from_cstr("[UNKNOWN COMMIT HASH]");

    if (cmd_try_run_direct("git log --pretty=format:%H -n 1", .output_string = &result)) {
        str_trim_whitespace(&result);
    }

    return result;
}


CBA_DEF String git_branch_name() {
    String result = str_from_cstr("[UNKNOWN BRANCH]");

    if (cmd_try_run_direct("git branch --show-current", .output_string = &result)) {
        str_trim_whitespace(&result);
    }

    return result;
}


CBA_DEF String git_committer_name() {
    String result = str_from_cstr("[UNKNOWN COMMITTER]");

    if (cmd_try_run_direct("git log --pretty=format:%an -n 1", .output_string = &result)) {
        str_trim_whitespace(&result);
    }

    return result;
}


CBA_DEF void* arena_alloc(Arena* arena, usize size) {
    void* result = NULL;

    usize alignment_offset = 0;
    isize curr = (isize)(arena->base + arena->used);
    isize mask = (isize)CBA_ALIGNMENT - 1;

    if (curr & mask) {
        alignment_offset = (usize)((isize)CBA_ALIGNMENT - (curr & mask));
    }

    usize effective_size = size + alignment_offset;

    if ((arena->used + effective_size) > arena->capacity) {
        if (!arena->min_block_size) {
            arena->min_block_size = CBA_MEMORY_BLOCK_SIZE;
            cba_assert(is_pow2(CBA_ALIGNMENT), "CBA_ALIGNMENT is not a power-of-two value");
        }

        ArenaBlockFooter new_footer = {
            .base     = arena->base,
            .used     = arena->used,
            .capacity = arena->capacity,
        };

        uninit usize block_size;
        if (effective_size > arena->min_block_size) {
            block_size = effective_size + sizeof(ArenaBlockFooter);
        }
        else {
            block_size = arena->min_block_size + sizeof(ArenaBlockFooter);
        }

        arena->base     = (u8*)calloc(block_size, 1);
        arena->used     = 0;
        arena->capacity = block_size - sizeof(ArenaBlockFooter);

        *get_arena_footer(arena) = new_footer;
    }

    result = arena->base + arena->used + alignment_offset;
    arena->used += effective_size;
    memz(result, size);

    return result;
}


CBA_DEF void arena_free(Arena* arena) {
    while (arena->base) {
        ArenaBlockFooter footer = *get_arena_footer(arena);

        free(arena->base);

        arena->base     = footer.base;
        arena->used     = footer.used;
        arena->capacity = footer.capacity;
    }

    memz(arena, sizeof(Arena));
}


CBA_DEF char* alloc_sprintf(const char* fmt, ...) {
    char* result = NULL;

    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    cba_assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    result = alloc_array(len + 1, char);
    vsnprintf(result, len + 1, fmt, args);
    cba_assert(result[len] == '\0', "null-terminator was not appended");

    va_end(args);

    return result;
}


CBA_DEF char* surround_dq(const char* cstr) {
    return alloc_sprintf("\"%s\"", cstr);
}


CBA_DEF char* surround_sq(const char* cstr) {
    return alloc_sprintf("'%s'", cstr);
}


CBA_DEF char* surround_bq(const char* cstr) {
    return alloc_sprintf("`%s`", cstr);
}





// @mark: files

static inline FileDescriptor _open_fd_for_read_write(const char* path) {
    FileDescriptor result = INVALID_HANDLE;

#if CBA_WINDOWS
    SECURITY_ATTRIBUTES attr = {0};
    attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    attr.bInheritHandle = TRUE;

    HANDLE fd = CreateFileA(
        path,
        GENERIC_WRITE | GENERIC_READ,
        0,
        &attr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fd != INVALID_HANDLE) {
        result = (FileDescriptor)fd;
    }
    else {
        verbose_print("failed to open file descriptor for \"%s\": %s", path, _os_error());
    }
#else
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd >= 0) {
        result = (FileDescriptor)fd;
    }
    else {
        verbose_print("failed to open file descriptor for \"%s\": %s", path, _os_error());
    }
#endif

    return result;
}


static inline void _close_fd(FileDescriptor fd) {
#if CBA_WINDOWS
    CloseHandle(fd);
#else
    close(fd);
#endif
}


static usize _seek_fd(FileDescriptor fd, b32 end) {
    usize result = 0;

    cba_assert(fd != INVALID_HANDLE, "cannot seek with an invalid file descriptor");

#if CBA_WINDOWS
    DWORD pos = SetFilePointer(fd, 0, NULL, end ? FILE_END : FILE_BEGIN);
    cba_assert(pos != INVALID_SET_FILE_POINTER, "failed to seek file: %s", _os_error());
    result = (usize)pos;
#else
    result = (usize)lseek(fd, 0, end ? SEEK_END : SEEK_SET);
#endif

    return result;
}


static isize _read_fd(FileDescriptor fd, void* memory, usize bytes) {
    isize result = 0;

#if CBA_WINDOWS
    // @fixme: this doesn't always seem write to `memory`.
    uninit DWORD bytes_read;
    b32 success = ReadFile(fd, memory, (DWORD)bytes, &bytes_read, NULL);

    if (success) {
        result = (isize)bytes_read;
    }
    else {
        result = -1;
    }
#else
    result = (isize)read(fd, memory, bytes);
#endif

    return result;
}


CBA_DEF i32 files_need_rebuild(String output_path, StringArray input_paths) {
    i32 result = 0;

#if CBA_WINDOWS
    char* output_path_cstr = str_to_cstr(output_path);

    HANDLE output_path_fd = CreateFileA(output_path_cstr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (output_path_fd != INVALID_HANDLE_VALUE) {
        uninit FILETIME output_path_time;
        BOOL got_output_file_time = GetFileTime(output_path_fd, NULL, NULL, &output_path_time);
        CloseHandle(output_path_fd);

        if (got_output_file_time) {
            for (usize i = 0; i < input_paths.count; ++i) {
                char* input_path_cstr = str_to_cstr(input_paths.items[i]);

                HANDLE input_path_fd = CreateFileA(input_path_cstr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

                if (input_path_fd != INVALID_HANDLE_VALUE) {
                    uninit FILETIME input_path_time;
                    BOOL found_input_file_time = GetFileTime(input_path_fd, NULL, NULL, &input_path_time);
                    CloseHandle(input_path_fd);

                    if (found_input_file_time) {
                        if (CompareFileTime(&input_path_time, &output_path_time) == 1) {
                            result = 1;
                            break;
                        }
                    }
                    else {
                        verbose_print("failed to stat input file \"%s\": %s", input_path_cstr, _os_error());
                        result = -1;
                    }
                }
                else {
                    verbose_print("failed to open input file \"%s\": %s", input_path_cstr, _os_error());
                    result = -1;
                }
            }
        }
        else {
            verbose_print("failed to stat output file \"%s\": %s", output_path_cstr, _os_error());
            result = -1;
        }
    }
    else {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            result = 1;
        }
        else {
            verbose_print("failed to open output file \"%s\": %s", output_path_cstr, _os_error());
            result = -1;
        }
    }
#else
    char* output_path_cstr = str_to_cstr(output_path);

    uninit struct stat statbuf;

    if (stat(output_path_cstr, &statbuf) >= 0) {
        time_t output_path_time = statbuf.st_mtime;

        for (usize i = 0; i < input_paths.count; ++i) {
            char* input_path_cstr = str_to_cstr(input_paths.items[i]);

            if (stat(input_path_cstr, &statbuf) >= 0) {
                time_t input_path_time = statbuf.st_mtime;
                if (input_path_time > output_path_time) {
                    result = 1;
                    break;
                }
            }
            else {
                verbose_print("failed to stat input file \"%s\": %s", input_path_cstr, _os_error());
                result = -1;
            }
        }
    }
    else {
        if (errno == ENOENT) {
            result = 1;
        }
        else {
            verbose_print("failed to stat output file \"%s\": %s", output_path_cstr, _os_error());
            result = -1;
        }
    }
#endif

    return result;
}


CBA_DEF i32 file_needs_rebuild(String output_path, String input_path) {
    i32 result = false;

    StringArray arr = {0};
    str_arr_append_str(&arr, input_path);

    result = files_need_rebuild(output_path, arr);

    return result;
}


CBA_DEF b32 file_create(const char* path) {
    b32 result = false;

    FILE* f = fopen(path, "w+");

    if (f) {
        result = true;
        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, _os_error());
    }

    return result;
}


CBA_DEF b32 file_move(const char* path, const char* new_path) {
    b32 result = false;

#if CBA_WINDOWS
    result = MoveFileExA(path, new_path, MOVEFILE_REPLACE_EXISTING);
#else
    result = rename(path, new_path) == 0;
#endif

    if (!result) {
        verbose_print("failed to rename \"%s\" to \"%s\": %s", path, new_path, _os_error());
    }

    return result;
}


CBA_DEF b32 file_copy(const char* path, const char* new_path, b32 symbolic_link) {
    b32 result = false;

#if CBA_WINDOWS
    result = CopyFileA(path, new_path, FALSE);

    if (symbolic_link) {
        verbose_print("warning: cannot create symbolic links on windows!");
    }
#elif CBA_MACOS
    if (symbolic_link) {
        result = symlink(path, new_path) == 0;
    }
    else {
        result = copyfile(path, new_path, NULL, COPYFILE_DATA) == 0;
    }
#else
    if (symbolic_link) {
        result = symlink(path, new_path) == 0;
    }
    else {
        uninit isize size;
        FileDescriptor existing_fd = open(path, O_RDONLY, 0);
        FileDescriptor new_fd      = open(new_path, O_WRONLY | O_CREAT, 0666);

        uninit struct stat stat_existing;
        fstat(existing_fd, &stat_existing);
        size = sendfile(new_fd, existing_fd, 0, stat_existing.st_size);

        int i = ftruncate(new_fd, size);
        cba_assert(i == 0, "failed to truncate new file during copy");

        close(new_fd);
        close(existing_fd);

        result = size == stat_existing.st_size;
    }
#endif

    if (!result) {
        verbose_print("failed to copy file \"%s\" to \"%s\": %s", path, new_path, _os_error());
    }

    return result;
}

#if !CBA_WINDOWS
static inline int _rment(const char* path, const struct stat* st, int flags, struct FTW* ftwp) {
    CBA_UNUSED(st); CBA_UNUSED(flags); CBA_UNUSED(ftwp);

    int result = remove(path);
        
    if (result != 0) {
        verbose_print("failed to remove directory entry \"%s\": %s", path, _os_error());
    }

    return result;
}
#endif

CBA_DEF b32 file_delete(const char* path) {
    b32 result = false;

#if CBA_WINDOWS
    if (file_exists(path)) {
        FileKind kind = file_get_kind(path);

        switch (kind) {
            case FILE_KIND_DIRECTORY: {
                // @todo: does this work recursively?
                result = RemoveDirectoryA(path);

                if (!result) {
                    verbose_print("failed to delete directory \"%s\": %s", path, _os_error());
                }
            } break;

            case FILE_KIND_REGULAR:
            case FILE_KIND_SYMLINK:
            case FILE_KIND_OTHER: {
                result = DeleteFileA(path);

                if (!result) {
                    verbose_print("failed to delete file \"%s\": %s", path, _os_error());
                }
            } break;

            default: break;
        }
    }
#else
    if (file_exists(path)) {
        FileKind ft = file_get_kind(path);

        cba_assert(ft != FILE_KIND_UNKNOWN, "the file exists, so its type should have been recognised");

        if (ft == FILE_KIND_DIRECTORY) {
            int r = nftw(path, _rment, 512, FTW_PHYS | FTW_DEPTH);

            if (r == 0) {
                result = true;
            }
            else {
                verbose_print("failed to recursively delete \"%s\": %s", path, _os_error());
            }
        } 
        else {
            int r = remove(path);

            if (r == 0) {
                result = true;
            }
            else {
                verbose_print("failed to delete \"%s\": %s", path, _os_error());
            }
        }
    }
    else {
        result = true;
    }
#endif

    return result;
}


CBA_DEF b32 file_exists(const char* path) {
    b32 result = false;

#if CBA_WINDOWS
    result = GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
    result = access(path, F_OK) == 0;
#endif

    return result;
}


CBA_DEF FileKind file_get_kind(const char* path) {
    FileKind result = FILE_KIND_UNKNOWN;

#if CBA_WINDOWS
    DWORD attributes = GetFileAttributesA(path);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        result = (attributes & FILE_ATTRIBUTE_DIRECTORY) ? FILE_KIND_DIRECTORY : FILE_KIND_REGULAR;
    }
    else {
        verbose_print("failed to get file attributes for \"%s\": %s", path, _os_error());
    }
#else
    uninit struct stat statbuf;
    if (lstat(path, &statbuf) >= 0) {
        if (S_ISREG(statbuf.st_mode)) {
            result = FILE_KIND_REGULAR;
        }
        else if (S_ISDIR(statbuf.st_mode)) {
            result = FILE_KIND_DIRECTORY;
        }
        else if (S_ISLNK(statbuf.st_mode)) {
            result = FILE_KIND_SYMLINK;
        }
        else {
            result = FILE_KIND_OTHER;
        }
    }
    else {
        verbose_print("failed to stat file \"%s\": %s", path, _os_error());
    }
#endif

    return result;
}


CBA_DEF usize file_length(const char* path) {
    usize result = 0;

#if CBA_WINDOWS
    uninit WIN32_FILE_ATTRIBUTE_DATA attribute_data;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attribute_data)) {
#if CBA_64_BIT
        result = ((usize)attribute_data.nFileSizeHigh << 32) | attribute_data.nFileSizeLow;
#else
        result = attribute_data.nFileSizeLow;
#endif
    }
    else {
        verbose_print("failed to get file attributes for \"%s\": %s", path, _os_error());
    }
#else
    uninit struct stat statbuf;
    if (lstat(path, &statbuf) >= 0) {
        result = (usize)statbuf.st_size;
    }
    else {
        verbose_print("failed to stat file \"%s\": %s", path, _os_error());
    }
#endif

    return result;
}


CBA_DEF b32 file_read(const char* path, void* dest, usize bytes) {
    b32 result = false;

    cba_assert(dest, "cannot read to NULL memory");
    cba_assert(bytes, "cannot read zero bytes");

    FILE* f = fopen(path, "rb");

    if (f) {
        usize bytes_read = fread(dest, 1, bytes, f);

        if (bytes_read == 0) {
            verbose_print("failed to read any memory from \"%s\": %s", path, _os_error());
        }
        else {
            result = true;
        }

        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, _os_error());
    }

    return result;
}


CBA_DEF b32 file_write(const char* path, void* memory, usize bytes, b32 append) {
    b32 result = false;

    cba_assert(memory, "cannot write from NULL memory");
    cba_assert(bytes, "cannot write zero bytes");

    uninit FILE* f;
    if (append) {
        f = fopen(path, "a+b");
    }
    else {
        f = fopen(path, "w+b");
    }

    if (f) {
        usize bytes_written = fwrite(memory, 1, bytes, f);

        if (!bytes_written && !feof(f)) {
            verbose_print("failed to write memory to \"%s\": %s", path, _os_error());
        }
        else {
            result = true;
        }

        fflush(f);
        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, _os_error());
    }

    return result;
}

#if CBA_WINDOWS
CBA_INLINE b32 _create_dir(const char* path) {
    b32 result = true;

    if (!file_exists(path)) {
        result = _mkdir(path) == 0;

        if (!result) {
            verbose_print("failed to create directory \"%s\": %s", path, _os_error());
        }
    }

    return result;
}
#else
CBA_INLINE b32 _create_dir(const char* path) {
    b32 result = true;

    if (!file_exists(path)) {
        int res = mkdir(path, 0755);

        if (res < 0) {
            cba_assert(errno != EEXIST, "the file should not exist, because it has already been checked");
            verbose_print("failed to create directory \"%s\": %s", path, _os_error());
            result = false;
        }
    }

    return result;
}
#endif

CBA_DEF b32 file_try_create_directory(const char* path) {
    b32 result = true;

    String path_str = str_from_cstr(path);

    if (!file_exists(path)) {
        StringArray parent_paths = str_to_parent_paths(path_str);

        if (parent_paths.items) {
            for (usize i = 0; i < parent_paths.count; ++i) {
                char* path_cstr = (char*)str_to_cstr(parent_paths.items[i]);
                if (!_create_dir(path_cstr)) {
                    result = false;
                    break;
                }
            }

            // @jcg: the above only creates parent paths: the top-level dir still needs to
            // be created.
            if (result && !_create_dir((char*)path)) {
                result = false;
            }
        }
    }
    else {
        verbose_print("directory \"%s\" already exists", path);
        result = true;
    }

    return result;
}


CBA_DEF StringArray file_get_directory_entries(const char* path, b32 include_directory_path) {
    StringArray result = {0};

    String path_str = str_from_cstr(path);

    cba_assert(path_str.data[path_str.len] == '\0', "path string is not null-terminated");

    FileKind ft = file_get_kind(path);
    cba_assert(ft == FILE_KIND_DIRECTORY, "the path \"%s\" is not a directory", path);

#if CBA_WINDOWS
    uninit WIN32_FIND_DATA find_data;
    uninit HANDLE file;
    String tmp = str_copy(path_str);

    if (tmp.data[tmp.len - 1] != '\\') {
        str_append_char(&tmp, '\\');
        str_append_null(&tmp);
    }

    file = FindFirstFileA(tmp.data, &find_data);

    if (file != INVALID_HANDLE) {
        do {
            String entry = str_from_cstr((const char*)find_data.cFileName);

            print("next file in dir: " stok, sfmt(entry));

            if (include_directory_path) {
                str_append_other(&entry, path_str);

                if (!is_separator(path_str.data[path_str.len - 1])) {
                    str_append_char(&entry, CBA_PATH_SEPARATOR);
                }
            }

            str_arr_append_str(&result, entry);
        } while (FindNextFileA(file, &find_data));

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            verbose_print("error getting next directory entry: %s", _os_error());
        }

        FindClose(file);
    }
    else {
        verbose_print("failed to open directory \"%.*s\": %s", sfmt(path_str), _os_error());
    }
#else
    DIR* d = opendir(path);
    cba_assert(d, "failed to open dir \"%s\": %s", path, _os_error());

    struct dirent* dent = NULL;

    while ((dent = readdir(d))) {
        if ((dent->d_type == DT_LNK) || (dent->d_type == DT_DIR) || (dent->d_type == DT_REG)) {
            String entry = str_alloc_with_cap(CBA_MAX_PATH);

            if (include_directory_path) {
                str_append_other(&entry, path_str);

                if (!is_separator(path_str.data[path_str.len - 1])) {
                    str_append_char(&entry, CBA_PATH_SEPARATOR);
                }
            }

            str_append_chars(&entry, (char*)dent->d_name, (usize)dent->d_namlen);

            if (!str_ends_with(entry, ".") && !str_ends_with(entry, "..")) {
                str_arr_append_str(&result, entry);
            }
        }
    }

    closedir(d);
#endif

    return result;
}





// @mark: procs

#if CBA_WINDOWS
// @jcg: windows needs some specific escaping of backslashes and quotes:
// https://learn.microsoft.com/en-gb/archive/blogs/twistylittlepassagesallalike/everyone-quotes-command-line-arguments-the-wrong-way
static inline String _cmd_flatten_win32(Command cmd) {
    String result = {0};

    usize capacity = 1;

    for (usize i = 0; i < cmd.count; ++i) {
        capacity += cmd.items[i].cap; 
    }

    // @todo: no longer needed?
    result = str_alloc_with_cap(capacity * 2);

    for (usize i = 0; i < cmd.count; ++i) {
        String* arg = &cmd.items[i];
        cba_assert(arg->len, "argument should not be empty");

        if (i != 0) {
            str_append_char(&result, ' ');
        }

        if (str_find_first_of_any_in_cstr(*arg, " \t\n\v\"", true, NULL)) {
            usize backslashes = 0;
            str_append_char(&result, '\"');

            for (usize ii = 0; ii < arg->len; ++ii) {
                char ch = arg->data[ii];

                if (ch == '\\') {
                    backslashes += 1;
                }
                else {
                    if (ch == '\"') {
                        for (usize iii = 0; iii < (backslashes + 1); ++iii) {
                            str_append_char(&result, '\\');
                        }
                    }

                    backslashes = 0;
                }

                str_append_char(&result, ch);
            }

            for (usize ii = 0; ii < backslashes; ++ii) {
                str_append_char(&result, '\\');
            }

            str_append_char(&result, '\"');
        }
        else {
            str_append_other(&result, *arg);
        }
    }

    return result;
}
#endif

CBA_DEF ProcessID proc_start(Command cmd, FileDescriptor output_fd) {
    ProcessID result = INVALID_HANDLE;

#if CBA_WINDOWS
    STARTUPINFO startup_info = {0};
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    startup_info.hStdOutput = (output_fd != INVALID_HANDLE)
                                  ? output_fd
                                  : GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = (output_fd != INVALID_HANDLE)
                                 ? output_fd
                                 : GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION process_info = {0};
    String flattened = _cmd_flatten_win32(cmd);
    verbose_print("win32 flattened command: " stok, sfmt(flattened));
    b32 success = CreateProcessA(
        NULL,
        flattened.data,
        NULL,
        NULL,
        true,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );

    if (success) {
        result = process_info.hProcess;
        CloseHandle(process_info.hThread);
    }
    else {
        verbose_print("failed to create process: %s", _os_error());
    }
#else
    if (cmd.count >= 1) {
        pid_t cpid = fork();

        if (cpid < 0) {
            verbose_print("failed to fork child process: %s", _os_error());
        }
        else if (cpid == 0) {
            b32 streams_valid = true;

            if (output_fd != INVALID_HANDLE) {
                if (dup2(output_fd, STDERR_FILENO) < 0) {
                    verbose_print("failed to create stderr for child process: %s", _os_error());
                    streams_valid = false;
                }
                if (streams_valid && dup2(output_fd, STDOUT_FILENO) < 0) {
                    verbose_print("failed to create stdout for child process: %s", _os_error());
                    streams_valid = false;
                }
            }

            if (streams_valid) {
                // @jcg: this is the memory allocated to the child process' arguments, so
                // it needs to be allocated permanently to outlive the child process.
                char** arr = alloc_array(cmd.count + 1, char*);
                for (usize i = 0; i < cmd.count; ++i) {
                    arr[i] = alloc_array(cmd.items[i].len + 1, char);
                    memcpy(arr[i], cmd.items[i].data, cmd.items[i].len);
                }

                int exec_result = execvp(arr[0], arr);

                if (exec_result >= 0) {
                    result = (ProcessID)cpid;
                    verbose_print("spawned process from \"%s\"", cmd_flatten_to_cstr(cmd));
                }
                else {
                    verbose_print("failed to exec child process for \"%s\": %s", arr[0], _os_error());
                }
            }
            else {
                kill(cpid, SIGKILL);
            }
        }
        else {
            result = cpid;
        }
    }
#endif

    return result;
}


CBA_DEF i32 proc_wait(ProcessID proc, int* exit_code) {
    i32 result = 1;

    cba_assert(proc != INVALID_HANDLE, "cannot wait on invalid process");

#if CBA_WINDOWS
    DWORD res = WaitForSingleObject(proc, INFINITE);

    if (res != WAIT_FAILED) {
        uninit DWORD exit_status;
        if (GetExitCodeProcess(proc, &exit_status)) {
            if (exit_status != 0) {
                result = 0;
            }

            if (exit_code) {
                *exit_code = (int)exit_status;
            }

            CloseHandle(proc);
        }
        else {
            verbose_print("failed to get child process exit status: %s", _os_error());
        }
    }
    else {
        verbose_print("failed to wait on child process: %s", _os_error());
        result = -1;
    }
#else
    for (;;) {
        int wstatus = 0;

        if (waitpid(proc, &wstatus, 0) < 0) {
            verbose_print("failed to wait on command with PID %d: %s", proc, _os_error());
            result = -1;
            break;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);

            if (exit_status != 0) {
                result = 0;
            }

            if (exit_code) {
                *exit_code = exit_status;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            verbose_print("process with PID %d was terminated by signal %d", proc, WTERMSIG(wstatus));
            result = 0;

            if (exit_code) {
                *exit_code = 1;
            }

            break;
        }

        wait_ms(1);
    }
#endif

    return result;
}


CBA_DEF i32 __proc_wait_va(usize n, ...) {
    i32 result = 1;

    uninit va_list args;
    va_start(args, n);

    for (usize i = 0; i < n; ++i) {
        ProcessID arg = va_arg(args, ProcessID);
        i32 r = proc_wait(arg, NULL);

        if (r != 1) {
            result = r;
            break;
        }
    }

    va_end(args);

    return result;
}





// @mark: strings

CBA_DEF void _str_resize(String* str, usize new_len) {
    new_len = max(new_len, CBA_MIN_STRING_CAPACITY);

    if (!str->cap) {
        str->data = alloc_array(new_len + 1, char);
        str->cap = new_len;
    }

    if (new_len > str->cap) {
        usize new_cap = next_pow2(new_len) + 1;

        char* new_data = alloc_array(new_cap, char);
        memcpy(new_data, str->data, str->len);
        str->data = new_data;
        str->cap = new_cap;
    }
}

CBA_DEF void str_clear(String* str) {
    str->len = 0;
    memz(str->data, str->cap);
}


CBA_DEF String str_alloc(void) {
    String result = {0};
    _str_resize(&result, CBA_MIN_STRING_CAPACITY);

    return result;
}


CBA_DEF String str_alloc_with_cap(usize cap) {
    String result = {0};
    _str_resize(&result, cap);

    return result;
}


CBA_DEF String str_sprintf(const char* fmt, ...) {
    String result = {0};

    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    cba_assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    // @jcg: in a nutshell, vsnprintf returns the length minus a null-terminator when used
    // as above, but will append a null-terminator anyway when used as below - hence the
    // cap + 1 for the allocation.
    usize cap = max(len, CBA_MIN_STRING_CAPACITY);
    _str_resize(&result, cap);
    result.len = len;
    vsnprintf(result.data, len + 1, fmt, args);

    va_end(args);

    return result;
}


CBA_DEF String str_from_cstr(const char* cstr) {
    usize len = (usize)strlen(cstr);

    String result = {0};
    _str_resize(&result, len);

    memcpy(result.data, cstr, len);
    result.len = len;

    return result;
}


CBA_DEF String str_from_chars(char* buffer, usize count) {
    String result = {0};
    _str_resize(&result, count);

    memcpy(result.data, buffer, count);
    result.len = count;

    return result;
}


CBA_DEF String str_from_file(const char* file_path) {
    String result = {0};

    FILE* f = fopen(file_path, "rb");
    cba_assert(f, "failed to open file \"%s\" for reading", file_path);

    fseek(f, 0, SEEK_END);
    usize len = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len) {
        _str_resize(&result, len);
        result.len = len;

        usize bytes_read = (usize)fread(result.data, 1, len, f);
        cba_assert(bytes_read > 0, "no bytes were read from \"%s\"", file_path);
    }

    fclose(f);

    return result;
}


CBA_DEF String str_from_cwd(void) {
    String result = str_alloc_with_cap(CBA_MAX_PATH);

#if CBA_WINDOWS
    char* cwd = _getcwd(result.data, CBA_MAX_PATH);
#else
    char* cwd = getcwd(result.data, CBA_MAX_PATH);
#endif

    cba_assert(cwd, "failed to obtain current working directory");

    result.len = (usize)strlen(cwd);

    return result;
}


CBA_DEF b32 str_write_to_file(String s, const char* path, b32 append) {
    b32 result = false;

    FILE* f = fopen(path, append ? "ab" : "wb");

    if (f) {
        usize bytes_written = fwrite(s.data, 1, s.len, f);

        if (!bytes_written && !feof(f)) {
            verbose_print("failed to write string to file \"%s\": %s", path, _os_error());
        }
        else {
            result = true;
        }

        fflush(f);
        fclose(f);
    }
    else {
        verbose_print("failed to write string to file \"%s\": %s", path, _os_error());
    }

    return result;
}


CBA_DEF String str_slice(String str, usize start, usize len) {
    cba_assert((start + len) <= str.len,
               "string slice exceeds the string's length (start: %zu, len: %zu, string len: %zu)",
               start, len, str.len);

    String result = {
        .data = str.data + start,
        .len = len,
        .cap = str.cap - start,
    };

    return result;
}


CBA_DEF void str_shrink_left(String* str, usize shift) {
    cba_assert(str->len >= shift, "shift of %zu exceeds string's length of %zu", shift, str->len);

    str->data += shift;
    str->len -= shift;
    str->cap -= shift;
}


CBA_DEF void str_shrink_right(String* str, usize shift) {
    cba_assert(str->len >= shift, "shift of %zu exceeds string's length of %zu", shift, str->len);

    str->len -= shift;
    str->cap -= shift;
}


CBA_DEF String str_path_file_name(String str, b32 include_extension) {
    String result = str;

    uninit usize separator_pos;
    b32 found_separator = str_find_last_char(str, '/',  &separator_pos) ||
                          str_find_last_char(str, '\\', &separator_pos);

    if (found_separator) {
        // @jcg: +1 because the separator shouldn't be included.
        result.data = str.data + (separator_pos + 1);
        result.len  -= separator_pos + 1;
        result.cap  -= separator_pos + 1;
    }

    if (!include_extension) {
        uninit usize dot_pos;
        if (str_find_last_char(str, '.', &dot_pos)) {
            result.len -= str.len - dot_pos;
            result.cap -= str.len - dot_pos;
        }
    }

    return result;
}


CBA_DEF String str_path_file_extension(String str) {
    String result = {0};

    uninit usize dot_pos;
    if (str_find_last_char(str, '.', &dot_pos)) {
        result.data = str.data + dot_pos;
        result.len = str.len - dot_pos;
        result.cap = str.cap - dot_pos;
    }

    return result;
}


CBA_DEF String str_path_pwd(String str) {
    String result = {0};

    uninit usize separator_pos;
    b32 found_separator = str_find_last_char(str, '/', &separator_pos) ||
                          str_find_last_char(str, '\\', &separator_pos);

    if (found_separator) {
        result.data = str.data;
        result.len = separator_pos;
        result.cap = str.cap - separator_pos - 1;
    }

    return result;
}


CBA_DEF String str_path_to_absolute(String str) {
    String result = str_alloc_with_cap(CBA_MAX_PATH);

    cba_assert(str.data[str.len] == '\0', "string is not null-terminated");

#if CBA_WINDOWS
    DWORD bytes = GetFullPathNameA(str.data, CBA_MAX_PATH, result.data, NULL);

    if (!bytes) {
        verbose_print("failed to get absolute path name from " stok ": %s", sfmt(str), _os_error());
        str_clear(&result);
        str_copy_into(&result, str);
    }
    else {
        result.len = bytes;
    }
#else
    cba_assert(str.len < CBA_MAX_PATH, "input path length exceeds PATH_MAX");

    char* p = realpath(str.data, result.data);

    if (!p) {
        if ((str.len > 0) && (str.data[0] == CBA_PATH_SEPARATOR)) {
            // the path appears absolute anyway.
            str_copy_into(&result, str);
        }
        else {
            // the path appears relative, so prepending the cwd should work.
            String cwd = str_from_cwd();
            str_append_other(&result, cwd);

            str_append_char(&result, CBA_PATH_SEPARATOR);
            str_append_other(&result, str);
        }
    }
    else {
        result.len = strlen(result.data);
    }
#endif

    return result;
}


CBA_DEF StringArray str_to_parent_paths(String path) {
    StringArray result = {0};

    // "/a/b/c/d/file.txt" -> { "/a", "/a/b", "/a/b/c", "/a/b/c/d" }
    //
    // "some_dir_name/subdir/file.txt" -> { "some_dir_name", "some_dir_name/subdir" }

    cba_assert(path.len > 0, "cannot convert an empty path to its parents");

    b32 last_was_separator = false;

    for (usize i = 0; i < path.len; ++i) {
        if (is_separator(path.data[i])) {
            if (!last_was_separator) {
                last_was_separator = true;

                if (i != 0) {
                    String substr = str_copy(str_slice(path, 0, i));
                    str_arr_append_str(&result, substr);
                }
            }
        }
        else {
            last_was_separator = false;
        }
    }

    return result;
}


CBA_DEF String str_path_copy_file_name(String str, b32 include_extension) {
    return str_copy(str_path_file_name(str, include_extension));
}


CBA_DEF String str_path_copy_file_extension(String str) {
    String result = {0};
    String slice = str_path_file_extension(str);

    if (slice.data) {
        result = str_copy(slice);
    }

    return result;
}


CBA_DEF String str_path_copy_pwd(String str) {
    String result = {0};
    String slice = str_path_pwd(str);

    if (slice.data) {
        result = str_copy(slice);
    }

    return result;
}


CBA_DEF String str_copy(String str) {
    String result = {0};
    _str_resize(&result, str.cap);
    result.len = str.len;

    memcpy(result.data, str.data, str.len);

    return result;
}


CBA_DEF void str_copy_into(String* dest, String source) {
    _str_resize(dest, source.len);

    memcpy(dest->data, source.data, source.len);
    dest->len = source.len;
}


CBA_DEF void str_append_null(String* str) {
    _str_resize(str, str->len + 1);
    str->data[str->len] = 0;
    str->len += 1;
}


CBA_DEF void str_append_line_ending(String* str) {
#if CBA_WINDOWS
    char buf[] = { '\r', '\n' };
    str_append_chars(str, buf, sizeof(buf));
#else
    str_append_char(str, '\n');
#endif
}


CBA_DEF void str_append_char(String* str, char ch) {
    _str_resize(str, str->len + 1);
    str->data[str->len] = ch;
    str->len += 1;
}


CBA_DEF void str_append_cstr(String* str, const char* cstr) {
    usize len = (usize)strlen(cstr);

    _str_resize(str, str->len + len);
    memcpy(str->data + str->len, cstr, len);
    str->len += len;
}


CBA_DEF void str_append_chars(String* str, char* buffer, usize count) {
    _str_resize(str, str->len + count);
    memcpy(str->data + str->len, buffer, count);
    str->len += count;
}


CBA_DEF void str_append_other(String* str, String other) {
    _str_resize(str, str->len + other.len);
    memcpy(str->data + str->len, other.data, other.len);
    str->len += other.len;
}


CBA_DEF void str_appendf(String* str, const char* fmt, ...) {
    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    cba_assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    _str_resize(str, str->len + len + 1);
    vsnprintf(str->data + str->len, len + 1, fmt, args);
    str->len += len;

    va_end(args);
}


CBA_DEF void str_to_lower(String* str) {
    for (usize i = 0; i < str->len; ++i) {
        if (is_upper(str->data[i])) {
            str->data[i] ^= 0x20;
        }
    }
}


CBA_DEF void str_to_upper(String* str) {
    for (usize i = 0; i < str->len; ++i) {
        if (is_lower(str->data[i])) {
            str->data[i] ^= 0x20;
        }
    }
}


CBA_DEF void str_lshift(String* str, usize start, usize shift) {
    if (!shift) return;

    cba_assert(start <= str->len,
               "shift start is outside of the string (start: %zu, len: %zu)",
               start, str->len);
    cba_assert(0 < start && shift <= start,
               "string should would underflow (start: %zu, shift: %zu)",
               start, shift);

    for (usize i = start; i < str->len; ++i) {
        str->data[i - shift] = str->data[i];
    }

    str->len -= shift;
    // @jcg: required to retain a null-terminator after the string contents.
    memz(str->data + str->len, shift);
}


CBA_DEF void str_rshift(String* str, usize start, usize shift) {
    if (!shift) return;

    cba_assert(start <= str->len,
               "shift start is outside of the string (start: %zu, len: %zu)",
               start, str->len);

    _str_resize(str, str->len + shift);

    usize new_len = str->len + shift;
    usize end = start + shift;

    for (usize i = (new_len - 1); i >= end; --i) {
        str->data[i] = str->data[i - shift];
    }

    memz(str->data + start, shift);
    str->len = new_len;
}


CBA_DEF void str_insert_char(String* str, usize at, char ch) {
    str_rshift(str, at, 1);
    str->data[at] = ch;
}


CBA_DEF void str_insert_other(String* str, usize at, String other) {
    str_rshift(str, at, other.len);
    memcpy(str->data + at, other.data, other.len);
}


CBA_DEF void str_insert_cstr(String* str, usize at, const char* cstr) {
    usize len = (usize)strlen(cstr);
    str_rshift(str, at, len);
    memcpy(str->data + at, cstr, len);
}


CBA_DEF void str_remove(String* str, usize at) {
    cba_assert(str->len > 0, "tried to remove from an empty string (zero length)");
    str_lshift(str, at + 1, 1);
}


CBA_DEF void str_remove_range(String* str, usize start, usize end) {
    cba_assert(str->len > 0, "tried to remove from an empty string (zero length)");
    cba_assert(end >= start, "incorrect removal range (start: %zu, end: %zu)", start, end);

    usize count = end - start;
    if (count > end) {
        count = end;
    }

    str_lshift(str, end, count);
}


CBA_DEF void str_replace_chars(String* str, char from, char to) {
    for (usize i = 0; i < str->len; ++i) {
        if (str->data[i] == from) {
            str->data[i] = to;
        }
    }
}


CBA_DEF void str_replace_others(String* str, String from, String to) {
    if (!from.len || (str->len < from.len)) return;

    b32 left_shift = to.len < from.len;
    usize shift_amount = left_shift
                         ? (from.len - to.len)
                         : (to.len - from.len);

    usize pos = 0;

    while (pos <= (str->len - from.len)) {
        b32 matched = true;

        for (usize i = 0; i < from.len; ++i) {
            if (str->data[pos + i] != from.data[i]) {
                matched = false;
                break;
            }
        }

        if (matched) {
            usize shift_start = pos + from.len;

            if (left_shift) {
                str_lshift(str, shift_start, shift_amount);
            }
            else {
                str_rshift(str, shift_start, shift_amount);
            }

            memcpy(str->data + pos, to.data, to.len);
            pos += to.len;
        }
        else {
            pos += 1;
        }
    }
}


CBA_DEF void str_replace_cstrs(String* str, const char* from, const char* to) {
    usize from_len = (usize)strlen(from);
    usize to_len = (usize)strlen(to);

    if (!from_len || (str->len < from_len)) return;

    b32 left_shift = to_len < from_len;
    usize shift_amount = left_shift
                             ? (from_len - to_len)
                             : (to_len - from_len);

    usize pos = 0;

    while (pos <= (str->len - from_len)) {
        b32 matched = true;

        for (usize i = 0; i < from_len; ++i) {
            if (str->data[pos + i] != from[i]) {
                matched = false;
                break;
            }
        }

        if (matched) {
            usize shift_start = pos + from_len;

            if (left_shift) {
                str_lshift(str, shift_start, shift_amount);
            }
            else {
                str_rshift(str, shift_start, shift_amount);
            }

            memcpy(str->data + pos, to, to_len);
            pos += to_len;
        }
        else {
            pos += 1;
        }
    }
}


CBA_DEF b32 str_trim_chars(String* str, const char* delims) {
    b32 result = false;

    if (!str->len) return false;

    usize num_delims = (usize)strlen(delims);

    isize start = 0;
    isize end = str->len - 1;

    for (; start < (isize)str->len; ++start) {
        b32 found = false;

        for (usize c = 0; c < num_delims; ++c) {
            if (str->data[start] == delims[c]) {
                found = true;
                result = true;
                break;
            }
        }

        if (!found) break;
    }

    for (; end > start; --end) {
        b32 found = false;

        for (usize c = 0; c < num_delims; ++c) {
            if (str->data[end] == delims[c]) {
                found = true;
                result = true;
                break;
            }
        }

        if (!found) break;
    }

    str->data += start;
    str->len = end - start + 1;

    return result;
}


CBA_DEF b32 str_trim_whitespace(String* str) {
    return str_trim_chars(str, CBA_WHITESPACE_CHARS);
}


CBA_DEF b32 str_trim_null(String* str) {
    b32 result = false;

    if (str->len) {
        isize start = 0;
        isize end = str->len - 1;

        for (; start < (isize)str->len; ++start) {
            if (str->data[start] != '\0') {
                break;
            }
        }

        for (; end > start; --end) {
            if (str->data[end] != '\0') {
                break;
            }
        }

        str->data += start;
        str->len = end - start + 1;
    }

    return result;
}


CBA_DEF StringArray str_split_by(String str, char delim) {
    StringArray result = {0};

    CBA_UNUSED(str); CBA_UNUSED(delim);

    todo();

    return result;
}


CBA_DEF StringArray str_split_lines(String str) {
    StringArray result = {0};

    String tmp = {0};

    while (str_chop_up_to_char(&str, &tmp, '\n')) {
        String s = str_copy(tmp);

        if ((s.len > 1) && (s.data[s.len - 2] == '\r')) {
            s.len -= 1;
        }

        str_arr_append_str(&result, s);

        usize shift = 0;

        while ((shift < str.len) && ((str.data[shift] == '\r') || (str.data[shift] == '\n'))) {
            shift += 1;
        }

        str_shrink_left(&str, shift);
    }

    if (!result.count) {
        str_arr_append_str(&result, str);
    }

    return result;
}


CBA_DEF b32 str_eq(String a, String b) {
    return (a.len == b.len) && (memcmp(a.data, b.data, a.len) == 0);
}


CBA_DEF b32 str_eq_cstr(String str, const char* cstr) {
    return (str.len == (usize)strlen(cstr)) && (memcmp(str.data, cstr, str.len) == 0);
}


CBA_DEF b32 str_eq_ignoring_case(String a, String b) {
    b32 result = true;

    if (a.len == b.len) {
        for (usize i = 0; i < a.len; ++i) {
            char cha = a.data[i];
            char chb = b.data[i];

            b32 matches = (cha == chb) ||
                          (is_alpha(cha) && is_alpha(chb) && ((cha ^ 0x20) == chb));

            if (!matches) {
                result = false;
                break;
            }
        }
    }
    else {
        result = false;
    }

    return result;
}


CBA_DEF b32 str_eq_cstr_ignoring_case(String str, const char* cstr) {
    b32 result = true;

    usize b_len = (usize)strlen(cstr);

    if (str.len == b_len) {
        for (usize i = 0; i < str.len; ++i) {
            char cha = str.data[i];
            char chb = cstr[i];

            b32 matches = (cha == chb) ||
                          (is_alpha(cha) && is_alpha(chb) && ((cha ^ 0x20) == chb));

            if (!matches) {
                result = false;
                break;
            }
        }
    }
    else {
        result = false;
    }

    return result;
}


CBA_DEF b32 str_starts_with(String str, const char* cstr) {
    usize len = (usize)strlen(cstr);
    return (str.len >= len) && (memcmp(str.data, cstr, len) == 0);
}


CBA_DEF b32 str_ends_with(String str, const char* cstr) {
    b32 result = false;

    usize len = (usize)strlen(cstr);

    if (len <= str.len) {
        u8* ptr = (u8*)(str.data + (str.len - len));
        result = memcmp(ptr, cstr, len) == 0;
    }

    return result;
}


CBA_DEF b32 str_find_first_of_any_in_cstr(String haystack, const char* needles, b32 case_sensitive, usize* where) {
    return str_find_first_of_any(haystack, needles, (usize)strlen(needles), case_sensitive, where);
}


CBA_DEF b32 str_find_first_of_any(String haystack, const char* needles, usize count, b32 case_sensitive, usize* where) {
    b32 result = false;

    for (usize i = 0; i < haystack.len; ++i) {
        for (usize ii = 0; ii < count; ++ii) {
            char a = haystack.data[i];
            char b = needles[ii];

            if ((a == b) || (!case_sensitive && is_alpha(a) && is_alpha(b) && ((a ^ 0x20) == b))) {
                result = true;

                if (where) {
                    *where = i;
                }

                goto outer;
            }
        }
    }

outer:

    return result;
}


CBA_DEF b32 str_find_last_of_any_in_cstr(String haystack, const char* needles, b32 case_sensitive, usize* where) {
    return str_find_last_of_any(haystack, needles, (usize)strlen(needles), case_sensitive, where);
}


CBA_DEF b32 str_find_last_of_any(String haystack, const char* needles, usize count, b32 case_sensitive, usize* where) {
    b32 result = false;

    for (usize i = 0; i < haystack.len; ++i) {
        usize idx = haystack.len - i - 1;

        for (usize ii = 0; ii < count; ++ii) {
            char a = haystack.data[idx];
            char b = needles[ii];

            if ((a == b) || (!case_sensitive && is_alpha(a) && is_alpha(b) && ((a ^ 0x20) == b))) {
                result = true;

                if (where) {
                    *where = idx;
                }

                goto outer;
            }
        }
    }

outer:

    return result;
}


CBA_DEF b32 str_find_first_char(String haystack, char needle, usize* where) {
    return str_find_first_char_from(haystack, needle, 0, where);
}


CBA_DEF b32 str_find_last_char(String haystack, char needle, usize* where) {
    return haystack.len && str_find_last_char_from(haystack, needle, haystack.len - 1, where);
}


CBA_DEF b32 str_find_first_other(String haystack, String needle, b32 case_sensitive, usize* where) {
    // @todo: could be implemented in terms of str_find_first_other_from?
    b32 result = false;

    if (haystack.len && needle.len && (haystack.len > needle.len)) {
        usize iters = haystack.len - needle.len + 1;
        usize off = 0;

        do {
            b32 mismatch = false;

            for (usize i = 0; i < needle.len; ++i) {
                char a = haystack.data[off + i];
                char b = needle.data[i];

                if (case_sensitive || !is_alpha(a) || !is_alpha(b)) {
                    mismatch = a != b;
                }
                else {
                    // @jcg: xor-ing an alphabetic ascii character with 32 (0x20) flips its case.
                    mismatch = (a != b) && ((a ^ 0x20) != b);
                }

                if (mismatch) break;
            }

            if (!mismatch) {
                result = true;

                if (where) {
                    *where = off;
                }

                break;
            }

            off += 1;
        } while (off < iters);
    }

    return result;
}


CBA_DEF b32 str_find_last_other(String haystack, String needle, b32 case_sensitive, usize* where) {
    // @todo: could be implemented in terms of str_find_last_other_from?
    b32 result = false;

    if (haystack.len && needle.len && (haystack.len > needle.len)) {
        // @todo: this might be off by one
        isize off = haystack.len - needle.len;

        do {
            b32 mismatch = false;

            for (usize i = 0; i < needle.len; ++i) {
                char a = haystack.data[off + i];
                char b = needle.data[i];

                if (case_sensitive || !is_alpha(a) || !is_alpha(b)) {
                    mismatch = a != b;
                }
                else {
                    // @jcg: xor-ing an alphabetic ascii character with 32 (0x20) flips its case.
                    mismatch = (a != b) && ((a ^ 0x20) != b);
                }

                if (mismatch) {
                    break;
                }
            }

            if (!mismatch) {
                result = true;

                if (where) {
                    *where = off;
                }

                break;
            }

            off -= 1;
        } while (off >= 0);
    }

    return result;
}


CBA_DEF b32 str_find_first_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where) {
    uninit b32 result;

    String needle_str = str_from_cstr(needle);
    result = str_find_first_other(haystack, needle_str, case_sensitive, where);

    return result;
}


CBA_DEF b32 str_find_last_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where) {
    uninit b32 result;

    String needle_str = str_from_cstr(needle);
    result = str_find_last_other(haystack, needle_str, case_sensitive, where);

    return result;
}


CBA_DEF b32 str_find_first_char_from(String haystack, char needle, usize from, usize* where) {
    cba_assert(from < haystack.len,
               "cannot find outside of string's bounds (len: %zu, from: %zu)",
               haystack.len, from);

    b32 result = false;

    for (usize i = from; i < haystack.len; ++i) {
        if (haystack.data[i] == needle) {
            if (where) {
                *where = i;
            }

            result = true;
            break;
        }
    }

    return result;
}


CBA_DEF b32 str_find_last_char_from(String haystack, char needle, usize from, usize* where) {
    cba_assert(from < haystack.len,
               "cannot find outside of string's bounds (len: %zu, from: %zu)",
               haystack.len, from);

    b32 result = false;

    for (usize i = 0; i <= from; ++i) {
        usize idx = haystack.len - i - 1;

        if (haystack.data[idx] == needle) {
            if (where) {
                *where = idx;
            }

            result = true;
            break;
        }
    }

    return result;
}


CBA_DEF u64 str_count_chars(String haystack, char needle) {
    u64 result = 0;

    for (usize i = 0; i < haystack.len; ++i) {
        if (haystack.data[i] == needle) {
            result += 1;
        }
    }

    return result;
}


CBA_DEF b32 str_contains_char(String haystack, char needle) {
    return str_find_first_char(haystack, needle, NULL);
}


CBA_DEF b32 str_contains_cstr(String haystack, const char* needle, b32 case_sensitive) {
    return str_find_first_cstr(haystack, needle, case_sensitive, NULL);
}


CBA_DEF b32 str_contains_other(String haystack, String needle, b32 case_sensitive) {
    return str_find_first_other(haystack, needle, case_sensitive, NULL);
}


CBA_DEF b32 str_find_first_other_from(String haystack, String needle, usize from, b32 case_sensitive, usize* where) {
    cba_assert(from < haystack.len, "cannot start out of the bounds of the string (from %zu, len %zu)", haystack.len, from);

    b32 result = false;

    if (haystack.len && needle.len && (haystack.len > needle.len)) {
        usize iters = haystack.len - needle.len - from + 1;
        usize off = from;

        do {
            b32 mismatch = false;

            for (usize i = 0; i < needle.len; ++i) {
                char a = haystack.data[off + i];
                char b = needle.data[i];

                if (case_sensitive || !is_alpha(a) || !is_alpha(b)) {
                    mismatch = a != b;
                }
                else {
                    // @jcg: xor-ing an alphabetic ascii character with 32 (0x20) flips its case.
                    mismatch = (a != b) && ((a ^ 0x20) != b);
                }

                if (mismatch) break;
            }

            if (!mismatch) {
                result = true;

                if (where) {
                    *where = off;
                }

                break;
            }

            off += 1;
        } while (off < iters);
    }

    return result;
}


CBA_DEF b32 str_find_last_other_from(String haystack, String needle, usize from, b32 case_sensitive, usize* where) {
    cba_assert(from < haystack.len, "cannot start out of the bounds of the string (from %zu, len %zu)", haystack.len, from);

    b32 result = false;

    if (haystack.len && needle.len && (haystack.len > needle.len)) {
        isize off = haystack.len - needle.len;

        do {
            b32 mismatch = false;

            for (usize i = from; i < needle.len; ++i) {
                char a = haystack.data[off + i];
                char b = needle.data[i];

                if (case_sensitive || !is_alpha(a) || !is_alpha(b)) {
                    mismatch = a != b;
                }
                else {
                    // @jcg: xor-ing an alphabetic ascii character with 32 (0x20) flips its case.
                    mismatch = (a != b) && ((a ^ 0x20) != b);
                }

                if (mismatch) {
                    break;
                }
            }

            if (!mismatch) {
                result = true;

                if (where) {
                    *where = off;
                }

                break;
            }

            off -= 1;
        } while (off >= 0);
    }

    return result;
}


CBA_DEF b32 str_find_first_cstr_from(String haystack, const char* needle, usize from, b32 case_sensitive, usize* where) {
    b32 result = false;

    String needle_str = str_from_cstr(needle);
    result = str_find_first_other_from(haystack, needle_str, from, case_sensitive, where);

    return result;
}


CBA_DEF b32 str_find_last_cstr_from(String haystack, const char* needle, usize from, b32 case_sensitive, usize* where) {
    b32 result = false;

    String needle_str = str_from_cstr(needle);
    result = str_find_last_other_from(haystack, needle_str, from, case_sensitive, where);

    return result;
}


CBA_DEF u64 str_count_cstrs(String haystack, const char* needle, b32 case_sensitive) {
    String needle_str = str_from_cstr(needle);
    u64 result = str_count_others(haystack, needle_str, case_sensitive);

    return result;
}


CBA_DEF u64 str_count_others(String haystack, String needle, b32 case_sensitive) {
    u64 result = 0;

    if (haystack.len > needle.len) {
        usize max_iters = haystack.len - needle.len;
        usize off = 0;

        do {
            b32 mismatch = false;

            for (usize i = 0; i < needle.len; ++i) {
                char a = haystack.data[off + i];
                char b = needle.data[i];

                if (case_sensitive || !is_alpha(a) || !is_alpha(b)) {
                    mismatch = a != b;
                }
                else {
                    // @jcg: xor-ing an alphabetic ascii character with 32 (0x20) flips its case.
                    mismatch = (a != b) && ((a ^ 0x20) != b);
                }

                if (mismatch) break;
            }

            if (mismatch) {
                off += 1;
            }
            else {
                result += 1;
                off += needle.len;
            }
        } while (off < max_iters);
    }

    return result;
}


CBA_DEF b32 str_parse_to_i64(String str, i64* dest) {
    b32 result = true;

    i64 sign = 1;
    usize pos = 0;
    b32 truncated = false;
    b32 found_digit = false;

    for (usize i = 0; i < str.len; ++i) {
        b32 is_digit = is_numeric(str.data[i]);
        b32 is_decimal = is_decimal(str.data[i]);

        b32 is_pos = str.data[i] == '+';
        b32 is_min = str.data[i] == '-';
        b32 is_sign = is_pos || is_min;

        if (is_sign) {
            if (i > 0) {
                result = false;
                break;
            }

            if (is_min) {
                sign = -1;
            }

            pos = 1;
        }
        else {
            if (is_decimal) {
                if (!truncated) {
                    str.len = i;
                    truncated = true;
                }
            }
            else if (!is_digit) {
                result = false;
                break;
            }

            if (!found_digit) {
                if (str.data[i] == '0') {
                    pos = i;
                }
                else {
                    found_digit = true;
                }
            }
        }
    }

    if (result) {
        usize start = (sign == -1) ? pos : (pos + 1);

        i64 factor = 1;
        for (usize i = start; i < (str.len - pos); ++i) {
            factor *= 10;
        }

        i64 value = 0;

        while (pos < str.len) {
            i64 digit = (i64)(str.data[pos] - '0');
            value += digit * factor;

            pos += 1;
            factor /= 10;
        }

        *dest = sign * value;
    }

    return result;
}


CBA_DEF b32 str_parse_to_f64(String str, f64* dest) {
    // @todo: parsing for inf/nan
    b32 result = true;

    f64 sign = 1.0;
    usize pos = 0;
    i64 decimal_idx = -1;
    b32 found_digit = false;

    for (usize i = 0; i < str.len; ++i) {
        b32 is_digit = is_numeric(str.data[i]);
        b32 is_decimal = is_decimal(str.data[i]);

        b32 is_pos = str.data[i] == '+';
        b32 is_min = str.data[i] == '-';
        b32 is_sign = is_pos || is_min;

        if (is_sign) {
            if (i > 0) {
                result = false;
                break;
            }

            if (is_min) sign = -1.0;
            pos = 1;
        }
        else if (is_decimal) {
            if (decimal_idx == -1) {
                decimal_idx = (i64)i;
            }
            else {
                result = false;
                break;
            }
        }
        else if (!is_digit) {
            result = false;
            break;
        }

        if (!found_digit) {
            if (str.data[i] == '0') {
                pos = i;
            }
            else {
                found_digit = true;
            }
        }
    }

    if (result) {
        if (!found_digit) {
            *dest = 0.0 * sign;
        }
        else {
            f64 value = 0.0;
            f64 factor = 1.0;

            if (decimal_idx == -1) {
                for (usize i = 1; i < (str.len - pos); ++i) {
                    factor *= 10.0;
                }
            }
            else if (decimal_idx == (i64)pos) {
                factor = 0.1;
            }
            else {
                for (i64 i = pos + 1; i < decimal_idx; ++i) {
                    factor *= 10.0;
                }
            }

            while (pos < str.len) {
                if ((i64)pos != decimal_idx) {
                    f64 digit = (f64)((u32)(str.data[pos] - '0'));
                    value += digit * factor;

                    factor *= 0.1;
                }
                
                pos += 1;
            }

            *dest = sign * value;
        }
    }

    return result;
}


CBA_DEF b32 str_chop_up_to_char(String* src, String* dest, char ch) {
    b32 result = false;

    for (usize i = 0; i < src->len; ++i) {
        if (src->data[i] == ch) {
            dest->data = src->data;
            dest->len = i;
            dest->cap = i;

            src->data += i + 1;
            src->len -= i + 1;
            src->cap -= i + 1;

            result = true;
            break;
        }
    }

    return result;
}


CBA_DEF b32 str_chop_up_to_cstr(String* src, String* dest, const char* cstr, b32 case_sensitive) {
    uninit b32 result;

    String str = str_from_cstr(cstr);
    result = str_chop_up_to_other(src, dest, str, case_sensitive);

    return result;
}


CBA_DEF b32 str_chop_up_to_other(String* src, String* dest, String other, b32 case_sensitive) {
    b32 result = false;

    cba_assert(src->len >= other.len,
               "other is too large for the source string (src len: %zu | other len: %zu)",
               src->len, other.len);

    // @todo: case sensitive
    CBA_UNUSED(case_sensitive);

    for (usize i = 0; i < src->len; ++i) {
        b32 matches = true;

        for (usize ii = 0; ii < other.len; ++ii) {
            if (src->data[i + ii] != other.data[ii]) {
                matches = false;
                break;
            }
        }

        if (matches) {
            dest->data = src->data;
            dest->len = i;
            dest->cap = i;

            src->data += i + 1;
            src->len -= i + 1;
            src->cap -= i + 1;

            result = true;
            break;
        }
    }

    return result;
}


CBA_DEF String str_from_current_time() {
    String result = str_alloc_with_cap(32);

#if CBA_WINDOWS
    SYSTEMTIME t = {0};
    GetLocalTime(&t);
    str_appendf(&result, "%02u:%02u:%02u", t.wHour, t.wMinute, t.wSecond);
#else
    time_t now = time(0);
    struct tm* t = localtime(&now);
    result.len = (usize)strftime(result.data, 32, "%X", t);
#endif

    return result;
}


CBA_DEF String str_from_current_date() {
    String result = str_alloc_with_cap(32);

#if CBA_WINDOWS
    SYSTEMTIME t = {0};
    GetLocalTime(&t);
    str_appendf(&result, "%02u:%02u:%04u", t.wDay, t.wMonth, t.wYear);
#else
    time_t now = time(0);
    struct tm* t = localtime(&now);
    result.len = (usize)strftime(result.data, 32, "%d/%m/%Y", t);
#endif

    return result;
}


CBA_DEF usize str_levenshtein_distance(String a, String b) {
    usize result = 0;

    if (!a.len) {
        result = b.len;
    }
    else if (!b.len) {
        result = a.len;
    }
    else {
        usize* v0 = alloc_array(b.len + 1, usize);
        usize* v1 = alloc_array(b.len + 1, usize);

        for (usize i = 0; i < (b.len + 1); ++i) {
            v0[i] = i;
        }

        for (usize i = 0; i < a.len; ++i) {
            v1[0] = i + 1;

            for (usize ii = 0; ii < b.len; ++ii) {
                usize cost = (a.data[i] == b.data[ii]) ? 0 : 1;
                v1[ii + 1] = min(v1[ii] + 1, min(v0[ii + 1] + 1, v0[ii] + cost));
            }

            memcpy(v0, v1, (b.len + 1) * sizeof(usize));
        }

        result = v1[b.len];
    }

    return result;
}


CBA_DEF f32 str_levenshtein_similarity(String a, String b) {
    f32 result = 0.0f;

    if (a.len && b.len) {
        usize distance = str_levenshtein_distance(a, b);
        usize max_len = max(a.len, b.len);

        result = 1.0f - ((f32)(distance) / (f32)(max_len));
    }

    return result;
}


CBA_DEF char* str_to_cstr(String str) {
    char* result = alloc_array(str.len + 1, char);
    memcpy(result, str.data, str.len);
    return result;
}


CBA_DEF char* fmt_bytes(usize num_bytes) {
    const usize KB = 1llu << 10;
    const usize MB = 1llu << 20;
    const usize GB = 1llu << 30;
    const usize TB = 1llu << 40;

    uninit char* result;

    if ((num_bytes / TB) > 0) {
        result = alloc_sprintf("%.3lf TB", (f64)(num_bytes) * 1e-12);
    }
    else if ((num_bytes / GB) > 0) {
        result = alloc_sprintf("%.3lf GB", (f64)(num_bytes) * 1e-9);
    }
    else if ((num_bytes / MB) > 0) {
        result = alloc_sprintf("%.3lf MB", (f64)(num_bytes) * 1e-6);
    }
    else if ((num_bytes / KB) > 0) {
        result = alloc_sprintf("%.3lf KB", (f64)(num_bytes) * 1e-3);
    }
    else {
        result = alloc_sprintf("%zu bytes", num_bytes);
    }

    return result;
}


CBA_DEF char* _fmt_binary(u64 value, usize width) {
    char* result = alloc_array(width + 1 + (width / 8), char);

    usize pos = 0;

    for (usize i = 0; i < width; ++i) {
        if ((i % 8 == 0) && (i != width - 1)) {
            result[pos] = ' ';
            pos += 1;
        }

        result[pos] = ((value >> ((width - 1) - i)) & BIT1) + '0';

        pos += 1;
    }

    return result;
}


CBA_DEF char* fmt_binary8(u8 b) {
    return _fmt_binary(b, 8);
}


CBA_DEF char* fmt_binary16(u16 b) {
    return _fmt_binary(b, 16);
}


CBA_DEF char* fmt_binary32(u32 b) {
    return _fmt_binary(b, 32);
}


CBA_DEF char* fmt_binary64(u64 b) {
    return _fmt_binary(b, 64);
}


CBA_DEF char* fmt_time(u64 nanos, u8 verbosity) {
    uninit char* result;

    const u64 MICRO = 1000;
    const u64 MILLI = 1000000;
    const u64 SEC   = 1000000000;
    const u64 MIN   = 60000000000;
    const u64 HOUR  = 3600000000000;

    if (nanos < MICRO) {
        const char* unit = verbosity == 0 ? "ns" : (verbosity == 1 ? "nanos" : "nanoseconds");
        result = alloc_sprintf("%llu %s", nanos, unit);
    }
    else if (nanos < MILLI) {
        const char* unit = verbosity == 0 ? "µs" : (verbosity == 1 ? "micros" : "microseconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-3, unit);
    }
    else if (nanos < SEC) {
        const char* unit = verbosity == 0 ? "ms" : (verbosity == 1 ? "millis" : "milliseconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-6, unit);
    }
    else if (nanos < MIN) {
        const char* unit = verbosity == 0 ? "s" : (verbosity == 1 ? "secs" : "seconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-9, unit);
    }
    else if (nanos < HOUR) {
        const char* unit = verbosity == 0 ? "m" : (verbosity == 1 ? "mins" : "minutes");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * (1e-9 / 60.0), unit);
    }
    else {
        const char* unit = verbosity == 0 ? "h" : (verbosity == 1 ? "hrs" : "hours");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * (1e-9 / 3600.0), unit);
    }

    return result;
}


CBA_DEF const char* fmt_version(Version v) {
  uninit u64 major, minor, patch;
  version_unpack(v, &major, &minor, &patch);

  return alloc_sprintf("%llu.%llu.%llu", major, minor, patch);
}





// @mark: string array

CBA_DEF void _str_arr_resize(StringArray* arr, usize new_len) {
    new_len = max(new_len, CBA_MIN_ARRAY_CAPACITY);
        
    if (!arr->cap) {
        arr->items = alloc_array(new_len, String);
        arr->cap = new_len;
    }

    if (new_len > arr->cap) {
        usize new_cap = next_pow2(new_len);

        print("reallocating string array to %zu elements", new_cap);
        String* new_data = alloc_array(new_cap, String);
        memcpy(new_data, arr->items, arr->count * sizeof(String));
        arr->items = new_data;
        arr->cap = new_cap;
    }
}

CBA_DEF void str_arr_append_str(StringArray* arr, String str) {
    if (!arr->items) {
        arr->count = 0;
    }

    _str_arr_resize(arr, arr->count + 1);

    arr->items[arr->count] = str;
    arr->count += 1;
}


CBA_DEF void __str_arr_append_va(StringArray* arr, usize n, ...) {
    uninit va_list args;
    va_start(args, n);

    for (usize i = 0; i < n; ++i) {
        String arg = va_arg(args, String);
        str_arr_append_str(arr, arg);
    }

    va_end(args);
}


CBA_DEF void __str_arr_append_cstrs_va(StringArray* arr, usize n, ...) {
    uninit va_list args;
    va_start(args, n);

    for (usize i = 0; i < n; ++i) {
        const char* arg = va_arg(args, const char*);
        str_arr_append_str(arr, str_from_cstr(arg));
    }

    va_end(args);
}


CBA_DEF StringArray str_arr_from_cstr_arr(char** arr, usize count) {
    StringArray result = {0};

    for (usize i = 0; i < count; ++i) {
        String s = str_from_cstr(arr[i]);
        str_arr_append_str(&result, s);
    }

    return result;
}


CBA_DEF void str_arr_concat(StringArray* arr, StringArray other) {
    for (usize i = 0; i < other.count; ++i) {
        str_arr_append_str(arr, other.items[i]);
    }
}


CBA_DEF String str_arr_flatten_to_str(StringArray arr, const char* separator) {
    String result = {0};
    String sep = str_from_cstr(separator);

    // @jcg: starts with 1 to keep space for a null-terminator.
    usize cap = 1;

    for (usize i = 0; i < arr.count; ++i) {
        cap += arr.items[i].len;
        cap += sep.len;
    }

    result = str_alloc_with_cap(cap);

    for (usize i = 0; i < arr.count; ++i) {
        str_append_other(&result, arr.items[i]);

        if (i < (arr.count - 1)) {
            str_append_other(&result, sep);
        }
    }

    return result;
}





// @mark: commands

CBA_DEF void _cmd_resize(Command* cmd, usize new_len) {
    new_len = max(new_len, CBA_MIN_ARRAY_CAPACITY);

    if (!cmd->cap) {
        cmd->items = alloc_array(new_len, String);
        cmd->cap = new_len;
    }

    if (new_len > cmd->cap) {
        usize new_cap = next_pow2(new_len);

        print("reallocating command to %zu elements", new_cap);
        String* new_data = alloc_array(new_cap, String);
        memcpy(new_data, cmd->items, cmd->count * sizeof(String));
        cmd->cap = new_cap;
    }
}

CBA_DEF void cmd_append_str(Command* cmd, String str) {
    if (!cmd->items) {
        cmd->count = 0;
    }

    if (str.len) {
        _cmd_resize(cmd, cmd->count + 1);

        cmd->items[cmd->count] = str;
        cmd->count += 1;
    }
}


CBA_DEF void cmd_append_str_arr(Command* cmd, StringArray arr) {
    for (usize i = 0; i < arr.count; ++i) {
        cba_assert(arr.items[i].len, "cannot append empty string to command (element %zu of string array)", i);
        cmd_append_str(cmd, arr.items[i]);
    }
}


CBA_DEF void __cmd_append_va(Command* cmd, usize n, ...) {
    uninit va_list args;
    va_start(args, n);

    for (usize i = 0; i < n; ++i) {
        const char* arg = va_arg(args, const char*);
        cmd_append_str(cmd, str_from_cstr(arg));
    }

    va_end(args);
}


CBA_DEF void cmd_concat(Command* cmd, Command other) {
    for (usize i = 0; i < other.count; ++i) {
        cmd_append_str(cmd, other.items[i]);
    }
}


CBA_DEF void cmd_reset(Command* cmd) {
    for (usize i = 0; i < cmd->count; ++i) {
        str_clear(&cmd->items[i]);
    }

    cmd->count = 0;
}


CBA_DEF void cmd_append_split(Command* cmd, const char* args) {
    String args_str = str_from_cstr(args);
    cba_assert(args_str.len > 0, "cannot split empty command");

    String concat_arg = {0};

    i32 double_quote_pos = -1;
    i32 single_quote_pos = -1;
    b32 last_char_was_space = false;
    usize next_append_pos = 0;

    for (usize i = 0; i < args_str.len; ++i) {
        if (args_str.data[i] == '\"') {
            if (double_quote_pos != -1) {
                usize len = i - (usize)double_quote_pos;
                cba_assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, (usize)double_quote_pos + 1, len - 1);

                if (concat_arg.len) {
                    arg = str_sprintf("%.*s%.*s", sfmt(concat_arg), sfmt(arg));
                    concat_arg.len = 0;
                }

                cmd_append_str(cmd, str_copy(arg));
                next_append_pos = i + 1;

                single_quote_pos = -1;
                double_quote_pos = -1;
            }
            else {
                double_quote_pos = (i32)i;

                if (!last_char_was_space) {
                    usize len = i - next_append_pos;
                    cba_assert(len != (usize)(-1), "incorrect length");
                    concat_arg = str_slice(args_str, next_append_pos, len);

                    next_append_pos = i + 1;
                }
            }
        }
        else if (args_str.data[i] == '\'') {
            if (single_quote_pos != -1) {
                usize len = i - (usize)single_quote_pos;
                cba_assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, (usize)single_quote_pos + 1, len - 1);

                if (concat_arg.len) {
                    arg = str_sprintf("%.*s%.*s", sfmt(concat_arg), sfmt(arg));
                    concat_arg.len = 0;
                }

                cmd_append_str(cmd, str_copy(arg));
                next_append_pos = i + 1;

                single_quote_pos = -1;
                double_quote_pos = -1;
            }
            else {
                single_quote_pos = (i32)i;

                if (!last_char_was_space) {
                    usize len = i - next_append_pos;
                    cba_assert(len != (usize)(-1), "incorrect length");
                    concat_arg = str_slice(args_str, next_append_pos, len);

                    next_append_pos = i + 1;
                }
            }
        }
        else if (args_str.data[i] == ' ') {
            if ((single_quote_pos == -1) && (double_quote_pos == -1) && (i != next_append_pos)) {
                usize len = i - next_append_pos;
                cba_assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, next_append_pos, len);

                cmd_append_str(cmd, str_copy(arg));
            }

            next_append_pos = i + 1;
            last_char_was_space = true;
        }

        if (args_str.data[i] != ' ') {
            last_char_was_space = false;
        }
    }

    usize final_len = args_str.len - next_append_pos;

    if (final_len) {
        char final = args_str.data[args_str.len - 1];
        if (final == '\'' || final == '\"') {
            final_len -= 1;
        }

        String arg = str_slice(args_str, next_append_pos, final_len);
        cmd_append_str(cmd, str_copy(arg));
    }
}


CBA_DEF b32 cmd_try_run_with_opts(Command cmd, CommandOptions opts) {
    b32 result = false;

    verbose_print("running `%s`", cmd_flatten_to_cstr(cmd));

    FileDescriptor output_fd = INVALID_HANDLE;
    char* output_file_path = NULL;

    if (opts.silence_output || opts.output_string) {
        cba_assert(!opts.async_pid, "cannot silence and/or read command output to string when running as async");

        static u64 output_file_id = 0;

        output_file_path = alloc_sprintf("cba_output_file_%llu", output_file_id);
        output_file_id += nanos_now();

        while (file_exists(output_file_path)) {
            output_file_path = alloc_sprintf("cba_output_file_%llu", output_file_id);
            output_file_id += nanos_now();
        }

        cba_assert(file_create(output_file_path), "failed to create output file \"%s\"", output_file_path);

        output_fd = _open_fd_for_read_write(output_file_path);
        cba_assert(output_fd != INVALID_HANDLE, "failed to open file descriptor for output file \"%s\"", output_file_path);
    }

    ProcessID pid = proc_start(cmd, output_fd);

    if (opts.async_pid && (pid != INVALID_HANDLE)) {
        *opts.async_pid = pid;
        result = true;
    }
    else {
        i32 proc_result = proc_wait(pid, opts.exit_code);

        if (proc_result == 1) {
            result = true;

            if (opts.output_string) {
                usize bytes = _seek_fd(output_fd, true);

                *opts.output_string = str_alloc_with_cap(bytes);

                if (bytes > 0) {
                    cba_assert(_seek_fd(output_fd, false) == 0, "incorrect seek position");

                    isize bytes_read = _read_fd(output_fd, opts.output_string->data, bytes);
                    cba_assert(bytes_read != -1, "failed to read from output file \"%s\": %s", output_file_path, _os_error());

                    opts.output_string->len = bytes_read;
                }
            }
        }
        else if (opts.exit_code && proc_result == -1) {
            *opts.exit_code = -1;
        }
    }

    if (output_fd != INVALID_HANDLE) {
        _close_fd(output_fd);
    }

    if (output_file_path && !file_delete(output_file_path)) {
        verbose_print("failed to remove output file \"%s\": %s", output_file_path, _os_error());
    }

    return result;
}


CBA_DEF b32 cmd_try_run_direct_with_opts(const char* command, CommandOptions opts) {
    b32 result = false;

    Command cmd = {0};
    cmd_append_split(&cmd, command);

    result = cmd_try_run_with_opts(cmd, opts);

    return result;
}


CBA_DEF String cmd_flatten(Command cmd) {
    return cmd_flatten_with_delims(cmd, '"');
}


CBA_DEF String cmd_flatten_with_delims(Command cmd, char delim) {
    String result = {0};

    // // @jcg: starts with 1 to keep space for a null-terminator.
    // usize capacity = 1;
    //
    // for (usize i = 0; i < cmd.count; ++i) {
    //     // + 3 for space character and (possible) quotes.
    //     capacity += cmd.items[i].cap + 3; 
    // }
    //
    // result = str_alloc_with_cap(capacity);

    for (usize i = 0; i < cmd.count; ++i) {
        String arg = cmd.items[i];
        cba_assert(arg.len, "argument should not be empty");

        if (i != 0) {
            str_append_char(&result, ' ');
        }

        if (!str_contains_char(arg, ' ')) {
            str_append_other(&result, arg);
        }
        else {
            // @todo: bit of a hack to avoid surrounding arguments which already contain
            // the delimiter with more of that delimiter. Might want to implement
            // something that checks for balanced delimiters, and perhaps whether the
            // first delimiter is adjacent to a space or something...
            if (delim == '"' && str_contains_char(result, '"')) {
                str_append_char(&result, '\'');
                str_append_other(&result, arg);
                str_append_char(&result, '\'');
            }
            else if (delim == '\'' && str_contains_char(result, '\'')) {
                str_append_char(&result, '"');
                str_append_other(&result, arg);
                str_append_char(&result, '"');
            }
            else {
                str_append_char(&result, delim);
                str_append_other(&result, arg);
                str_append_char(&result, delim);
            }
        }
    }

    return result;
}


CBA_DEF char* cmd_flatten_to_cstr(Command cmd) {
    String result = cmd_flatten(cmd);
    return result.data;
}


CBA_DEF char* cmd_flatten_to_cstr_with_delims(Command cmd, char delim) {
    String result = cmd_flatten_with_delims(cmd, delim);
    return result.data;
}

#endif // CBA_IMPLEMENTATION

#endif // CBA_HEADER_GUARD

/*
    # Version history

    - v1.4.1 (13 May 2026) (by @jamiegibney)
        - Fixed an issue where exit code values might be overwritten or not set
    - v1.4.0 (13 May 2026) (by @jamiegibney)
        - Added an optional exit_code pointer to command options
        - Arenas now support dynamic allocation of their memory blocks
        - Updated the default alignment to 32 bytes
    - v1.3.0 (13 May 2026) (by @jamiegibney)
        - Reverted dynamic allocations to favour (inefficiently) reallocating via the global arena
        - Added new string functions
        - Added surround_ functions for creating C-strings surrounded in quotes
        - Added has_exe_in_path (i.e. "which" or "where.exe" commands)
        - Added functions for obtaining information from git
        - Added Levenshtein distance functions
        - Added new endianness functions
        - Added fmt_binary functions for formatting binary strings
        - Added ANSI_ macros for ANSI escape codes
        - Added version_get_ macros for getting singular version fields
        - Added cba_ prefix to the assert and panic macros
        - Updated printing macros
        - Updated high/low bitmasks
        - Files are now always flushed directly after writing to them
        - Removed begin/end temporary memory macros and calls to avoid some bugs
        - CBA_REBUILD_COMMAND is now implemented in terms of CBA_COMPIER_ macros
        - Fixed cmd_try_run_direct
        - Fixed fmt_time function
        - Various bug fixes
    - v1.2.0 (01 May 2026) (by @jamiegibney)
        - Strings, StringArrays, and Commands now support dynamic allocation by default
        - Added CBA_NO_DYNAMIC_ALLOCATION to opt out of dynamically-allocated strings and arrays
        - Added da_append macros for dynamic arrays
        - Added str split/chop/shrink functions
        - Added next_pow2
        - Added BIT_ and FIRST/LAST_BITS macros
        - Added line ending macros
        - Fixed the _seek_fd function on Windows
        - Updated documentation
        - Renamed CBA_DEFAULT_STRING_CAPACITY to CBA_MIN_STRING_CAPACITY
        - Renamed CBA_ARRAY_CAPACITY to CBA_MIN_ARRAY_CAPACITY
    - v1.1.0 (13 Apr 2026) (by @jamiegibney)
        - Strings now store their data as char* for convenience
        - Updated directory entry function (now a "file" function)
    - v1.0.2 (13 Apr 2026) (by @jamiegibney)
        - Minor refactors/renaming/documentation updates
    - v1.0.1 (13 Apr 2026) (by @jamiegibney)
        - macOS/Linux fixes
        - New string functions
        - Updated version history formatting
    - v1.0.0 (12 Apr 2026) (by @jamiegibney)
        - Windows support
        - MSVC support (19.0+ only)
        - Added an option for silencing command outputs
        - Updated string and command functions
        - Arena allocations now use CBA_ALIGNMENT for alignment
        - Updated ANSI escape codes with `\x*` prefixes
        - Fixed CBA_UNUSED and static assertion macros
        - Implemented CBA_LITERAL calls
        - Updated function documentation
        - Replaced internal strerror and GetLastError calls with _os_error
    - v0.1.2 (11 Apr 2026) (by @jamiegibney)
        - Added CBA_NO_COLOR_OUTPUT
        - Assert and panic now use traps
        - Global arena/memory block are extern, and declared in CBA_IMPLEMENTATION block
        - Updated rebuild commands
        - Improved documentation
        - Minor fixes
    - v0.1.1 (11 Apr 2026) (by @jamiegibney)
        - Implemented str_to_directory_entries
        - Added recursive deletion to file_delete
        - File_length now only stats
        - Rebuild now checks for cba.h in cwd
        - Fixed get_cwd
        - Fixed str_ends_with
        - Renamed FILE_TYPE_FILE to FILE_TYPE_REGULAR
        - Renamed CBA_PATH_SEP to CBA_PATH_SEPARATOR
    - v0.1.0 (11 Apr 2026) (by @jamiegibney)
        - Initial release



    # License

    ------------------------------------------------------------------------------
    MIT License

    Copyright (c) 2026 Jamie Gibney

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    ------------------------------------------------------------------------------
*/
