/*
 * cba.h
 * https://github.com/jamiegibney/cba.h
 *
 * STB-style header library for building C/C++ projects and general C utilities.
 *
 *
 *
 * # Usage
 *
 * In ONE place in your project, define `CBA_IMPLEMENTATION` before including this header:
 *
 * #define CBA_IMPLEMENTATION
 * #include "cba.h"
 *
 * Otherwise, including this header will only include its definitions.
 *
 * You can also define `CBA_VERBOSE` to get logging when, e.g., internal commands fail:
 *
 * #define CBA_VERBOSE
 * #define CBA_IMPLEMENTATION
 * #include "cba.h"
 *
 * And, to see a message when bootstrapping, you can define `CBA_PRINT_ON_BOOTSTRAP`:
 *
 * #define CBA_PRINT_ON_BOOTSTRAP
 * #define CBA_IMPLEMENATION
 * #include "cba.h"
 *
 *
 *
 * # Example
 *
 * #define CBA_IMPLEMENTATION
 * #include "cba.h"
 *
 * int main(int argc, char** argv) {
 *     CBA_BOOTSTRAP(argc, argv);
 *
 *     assert(try_mkdir("build"), "failed to create build directory");
 *
 *     Command cmd = {0};
 *     cmd_append(&cmd,
 *         CBA_C_COMPILER,
 *         CBA_COMPILER_DEBUG_FLAGS,
 *         CBA_COMPILER_COMMON_FLAGS,
 *         CBA_COMPILER_OUTPUT("build/main"),
 *         CBA_COMPILER_INPUTS("main.c"),
 *     );
 *
 *     cmd_run(cmd);
 *
 *     return 0;
 * }
 *
 *
 *
 * # Overrideable defines
 *
 * - `CBA_BOOTSTRAP_COMMAND`: command to use for bootstrapping.
 * - `CBA_BOOTSTRAP_MESSAGE`: formatting for bootstrap message.
 * - `CBA_[INFO/WARN/ERROR]_PREFIX`: prefix to use for a log macro.
 * - `CBA_MEMORY_BLOCK_SIZE`: number of bytes to allocate to the global arena.
 * - `CBA_STRING_MIN_CAPACITY`: minimum number of bytes to allocate to strings.
 * - `CBA_ARRAY_COUNT`: number of elements allocated to array types, like `StringArray` and `Command`.
 *
 *
 *
 * # License
 * ------------------------------------------------------------------------------
 * MIT License
 * 
 * Copyright (c) 2026 Jamie Gibney
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ------------------------------------------------------------------------------
 */

// @todo: prefixes

#ifndef CBA_ARRAY_COUNT
    #define CBA_ARRAY_COUNT (256)
#elif CBA_ARRAY_COUNT == 0
    #error array count must be greater than 0
#endif

#ifndef CBA_MEMORY_BLOCK_SIZE
    #define CBA_MEMORY_BLOCK_SIZE (64 << 20) // 64 MB
#elif CBA_MEMORY_BLOCK_SIZE == 0
    #error memory block size must be greater than 0
#endif

#ifndef CBA_STRING_MIN_CAPACITY
    #define CBA_STRING_MIN_CAPACITY (512)
#elif CBA_STRING_MIN_CAPACITY == 0
    #error min string capacity must be greater than 0
#endif

#define CBA_GCC   0
#define CBA_CLANG 0
#define CBA_MSVC  0
#define CBA_WINDOWS 0
#define CBA_MACOS 0
#define CBA_LINUX 0
#define CBA_64_BIT 0
#define CBA_32_BIT 0
#define CBA_X86 0
#define CBA_ARM 0

#if defined(__GNUC__)
    #undef CBA_GCC
    #define CBA_GCC 1
#elif defined(__clang__)
    #undef CBA_CLANG
    #define CBA_CLANG 1
#elif defined(_MSC_VER)
    #undef CBA_MSVC
    #define CBA_MSVC 1
#else
    #error unsupported complier
#endif

#define CBA_ANSI !CBA_MSVC

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
    #define CBA_UNREACHABLE __builtin_unreachable()
    #define CBA_TRAP __builtin_trap()
#endif

#if CBA_MSVC
    #ifndef __FILE_NAME__
        #define __FILE_NAME__ __FILE__
    #endif
    #ifndef __PRETTY_FUNCTION__
        #define __PRETTY_FUNCTION__ __FUNCSIG__
    #endif
#endif

#ifndef CBA_INFO_PREFIX
    #define CBA_INFO_PREFIX  "info"
#endif
#ifndef CBA_WARN_PREFIX
    #define CBA_WARN_PREFIX  "warn"
#endif
#ifndef CBA_ERROR_PREFIX
    #define CBA_ERROR_PREFIX "error"
#endif

#ifdef assert
    #undef assert
#endif
#ifdef error
    #undef error
#endif

#if CBA_ANSI
    #define print(s, ...)                                                                    \
        printf("\e[1m%s:%04i\e[0m: " s "\n", __FILE_NAME__, __LINE__, ## __VA_ARGS__)

    #define assert(cond, s, ...)                                                             \
        if (!(cond)) {                                                                       \
            fprintf(stderr,                                                                  \
                    "\e[1m%s:%04i \e[31mfailed assertion\e[0m: \"" s "\"\n",                 \
                    __FILE_NAME__,                                                           \
                    __LINE__,                                                                \
                    ## __VA_ARGS__);                                                         \
            exit(1);                                                                         \
        } (void)(0)
    
    #define panic(s, ...)                                                                    \
        fprintf(stderr,                                                                      \
                "\e[1m%s:%04i \e[31mpanic\e[0m: \"" s "\"\n",                                \
                __FILE_NAME__,                                                               \
                __LINE__,                                                                    \
                ## __VA_ARGS__);                                                             \
        exit(1)

    #define info(s, ...)  printf("[\e[1;32m" CBA_INFO_PREFIX "\e[0m] " s "\n", ## __VA_ARGS__)
    #define warn(s, ...)  printf("[\e[1;33m" CBA_WARN_PREFIX "\e[0m] " s "\n", ## __VA_ARGS__)
    #define error(s, ...) fprintf(stderr, "[\e[1;31m" CBA_ERROR_PREFIX "\e[0m] " s "\n", ## __VA_ARGS__)
#else
    #define print(s, ...)                                                                    \
        printf("%s:%04i: " s "\n", __FILE_NAME__, __LINE__, ## __VA_ARGS__)

    #define assert(cond, s, ...)                                                             \
        if (!(cond)) {                                                                       \
            fprintf(stderr,                                                                  \
                    "%s:%04i failed assertion: \"" s "\"\n",                                 \
                    __FILE_NAME__,                                                           \
                    __LINE__,                                                                \
                    ## __VA_ARGS__);                                                         \
            exit(1);                                                                         \
        } (void)(0)
    
    #define panic(s, ...)                                                                    \
        fprintf(stderr,                                                                      \
                "%s:%04i panic: \"" s "\"\n",                                                \
                __FILE_NAME__,                                                               \
                __LINE__,                                                                    \
                ## __VA_ARGS__);                                                             \
        exit(1)

    #define info(s, ...)  printf("[" CBA_INFO_PREFIX "] " s "\n", ## __VA_ARGS__)
    #define warn(s, ...)  printf("[" CBA_WARN_PREFIX "] " s "\n", ## __VA_ARGS__)
    #define error(s, ...) fprintf(stderr, "[" CBA_ERROR_PREFIX "] " s "\n", ## __VA_ARGS__)
#endif

#ifdef CBA_VERBOSE
    #define verbose_print(s, ...) print(s, ## __VA_ARGS__)
#else
    #define verbose_print(s, ...)
#endif

#define ping printf("\e[1;32mPING\e[0m @ %s:\e[1m%04i\e[0m\n", __FILE_NAME__, __LINE__)
#define todo() panic("TODO: %s", __PRETTY_FUNCTION__)
#define unreachable() CBA_UNREACHABLE; panic("unreachable code path was hit")

#define CBA_DEF inline static

#ifndef CBA_BOOTSTRAP_MESSAGE
    #define CBA_BOOTSTRAP_MESSAGE(binary_name) alloc_sprintf("Bootstrapping \"%s\"", (binary_name))
#endif

#ifndef CBA_BOOTSTRAP_COMMAND
    #if CBA_CLANG
        #if defined(__cplusplus)
            #define CBA_BOOTSTRAP_COMMAND(output_path, source_path) "clang++", "-Wall", "-Wextra", "-o", output_path, source_path
        #else
            #define CBA_BOOTSTRAP_COMMAND(output_path, source_path) "clang", "-Wall", "-Wextra", "-o", output_path, source_path
        #endif
    #elif CBA_GCC
        #if defined(__cplusplus)
            #define CBA_BOOTSTRAP_COMMAND(output_path, source_path) "g++", "-Wall", "-Wextra", "-o", output_path, source_path
        #else
            #define CBA_BOOTSTRAP_COMMAND(output_path, source_path) "gcc", "-Wall", "-Wextra", "-o", output_path, source_path
        #endif
    #elif CBA_MSVC
        #define CBA_BOOTSTRAP_COMMAND(output_path, source_path) "cl.exe", alloc_sprintf("/Fe:%s", (output_path)), source_path
    #else
    #endif
#endif // CBA_BOOTSTRAP_COMMAND

#define unused [[maybe_unused]]

// @mark: types

#if CBA_WINDOWS
    typedef HANDLE ProcessID;
    typedef HANDLE FileDescriptor;
    #define INVALID_HANDLE INVALID_HANDLE_VALUE
    #define CBA_MAX_PATH MAX_PATH
    #define CBA_PATH_SEP '\\'
#else
    typedef int ProcessID;
    typedef int FileDescriptor;
    #define INVALID_HANDLE (-1)
    #define CBA_MAX_PATH PATH_MAX
    #define CBA_PATH_SEP '/'
#endif

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

#define __STATIC_ASSERT(cond, line) typedef char static_assert_l##line_[(!!(cond))*2-1]
#define _STATIC_ASSERT(cond, line) __STATIC_ASSERT(cond, line)
#define STATIC_ASSERT(cond) _STATIC_ASSERT(cond, __LINE__)

STATIC_ASSERT(sizeof(u8)  == sizeof(i8));
STATIC_ASSERT(sizeof(u16) == sizeof(i16));
STATIC_ASSERT(sizeof(u32) == sizeof(i32));
STATIC_ASSERT(sizeof(u64) == sizeof(i64));

STATIC_ASSERT(sizeof(u8)  == 1);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(u64) == 8);

STATIC_ASSERT(sizeof(usize) == sizeof(isize));

STATIC_ASSERT(sizeof(f32) == 4);
STATIC_ASSERT(sizeof(f64) == 8);

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

enum FileType {
    /// The file type could not be detected.
    FILE_TYPE_UNKNOWN = 0,
    /// The file is a regular file.
    FILE_TYPE_FILE,
    /// The file is a directory.
    FILE_TYPE_DIRECTORY,
    /// The file is a symbolic link.
    FILE_TYPE_SYMLINK,
    /// The file type was detected, but is not covered.
    FILE_TYPE_OTHER,
};
typedef enum FileType FileType;

struct Arena {
    /// Base pointer of the arena's memory block.
    u8* base;
    /// The number of bytes which the arena has allocated.
    usize used;
    /// The total number of bytes in the arena's memory block.
    usize capacity;
    /// Number of bytes to align allocations to. This is set during the first call to
    /// `arena_alloc` based on the system's cache line size.
    usize alignment;
    /// How many temporary memory blocks the arena is currently within, used for
    /// debugging.
    i32 temp_memory_pos;
};
typedef struct Arena Arena;

/// The approach with strings here is:
/// - Preallocate their memory, and do not allow dynamic resizing.
/// - Retain space for a null-terminator at the end of the string.
/// - Prefer to use this string type over `const char*`, unless it's particularly annoying
///   (e.g. for `file_*` functions, like `file_exists`).
struct String {
    /// Pointer to the string's data.
    u8* data;
    /// Number of bytes used in the string.
    usize len;
    /// Total number of bytes allocated to the string.
    usize cap;
};
typedef struct String String;

struct StringArray {
    /// Pointer to the array's data.
    String* items;
    /// Number of items in the array.
    usize count;
};
typedef struct StringArray StringArray;

struct CommandOptions {
    String* output_string;
    ProcessID* async_pid;
};
typedef struct CommandOptions CommandOptions;

struct Command {
    /// Pointer to the array of arguments in the command.
    String* items;
    /// Number of arguments in the command.
    usize count;
};
typedef struct Command Command;

// @mark: definitions

/// @jcg: simply used as a marker.
#define uninit

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

#define is_upper(ch)   ('A' <= (ch) && (ch) <= 'Z')
#define is_lower(ch)   ('a' <= (ch) && (ch) <= 'z')
#define is_alpha(ch)   (is_upper(ch) || is_lower(ch))
#define is_numeric(ch) ('0' <= (ch) && (ch) <= '9')
#define is_decimal(ch) ((ch) == '.' || (ch) == ',')
#define is_alphanumeric(ch) (is_alpha(ch) || is_number(ch))
#define is_whitespace(ch) ((ch) == ' ' || (ch) == '\n' || (ch) == '\r' || (ch) == '\t')
#define is_separator(ch) ((ch) == '\\' || (ch) == '/')

#define kilobytes(num) ((u64)(num) << 10)
#define megabytes(num) ((u64)(num) << 20)
#define gigabytes(num) ((u64)(num) << 30)
#define terabytes(num) ((u64)(num) << 40)

#define countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define mem_zero(ptr, bytes) memset((ptr), 0, (bytes))
#define mem_zero_array(ptr, count) mem_zero((ptr), (count) * sizeof((ptr)[0]))

#define streq(a, b)  (strcmp(a, b) == 0)
#define strneq(a, b) (strcmp(a, b) != 0)

#define endian_swap_32(x) (((x) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | ((x) << 24))
#define endian_swap_64(x)                                                                \
    ((((x) >> 56) & 0x00000000000000ff) | (((x) >> 40) & 0x000000000000ff00) |           \
     (((x) >> 24) & 0x0000000000ff0000) | (((x) >> 8)  & 0x00000000ff000000) |           \
     (((x) << 8)  & 0x000000ff00000000) | (((x) << 24) & 0x0000ff0000000000) |           \
     (((x) << 40) & 0x00ff000000000000) | (((x) << 56) & 0xff00000000000000))

#if CBA_MSVC
    #define CBA_C_COMPILER "gcc"
    #define CBA_CPP_COMPILER "g++"
    #define CBA_COMPILER_OUTPUT(output) "-o", output
    #define CBA_COMPILER_COMMON_FLAGS "-Wall", "-Wextra"
    #define CBA_COMPILER_DEBUG_FLAGS "-ggdb", "-DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "-O3", "-DNDEBUG"
#elif CBA_GCC
    #define CBA_C_COMPILER "clang"
    #define CBA_CPP_COMPILER "clang++"
    #define CBA_COMPILER_OUTPUT(output) "-o", output
    #define CBA_COMPILER_COMMON_FLAGS "-Wall", "-Wextra"
    #define CBA_COMPILER_DEBUG_FLAGS "-glldb", "-DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "-O3", "-DNDEBUG"
#elif CBA_CLANG
    #define CBA_C_COMPILER "cl.exe"
    #define CBA_CPP_COMPILER "cl.exe"
    #define CBA_COMPILER_OUTPUT(output) alloc_sprintf("/Fe:%s", output)
    #define CBA_COMPILER_COMMON_FLAGS "/W4", "/nologo"
    #define CBA_COMPILER_DEBUG_FLAGS "/ZI", "/DDEBUG"
    #define CBA_COMPILER_RELEASE_FLAGS "/O3", "/DNDEBUG"
#endif

#define CBA_COMPILER_INPUTS(...) __VA_ARGS__

CBA_DEF void __cba_bootstrap_build(int argc, char** argv, const char* source_path, ...);
#define CBA_BOOTSTRAP(argc, argv) __cba_bootstrap_build((argc), (argv), __FILE__, NULL)
#define CBA_BOOTSTRAP_WITH(argc, argv, ...) __cba_bootstrap_build((argc), (argv), __FILE__, __VA_ARGS__, NULL)

/// Returns an absolute path to the current working directory (i.e., wherever the program
/// was run from).
CBA_DEF String get_cwd(void);

/// Returns the size of the processor's cache lines in bytes.
CBA_DEF usize cache_line_size(void);

/// Swaps `len_bytes` bytes between the memory at `a` and `b`.
CBA_DEF void mem_swap(void* a, void* b, usize len_bytes);

// @mark: arena

/// Statically-allocated arena memory block, assigned to the global arena.
static u8 arena_memory_block[CBA_MEMORY_BLOCK_SIZE] = {0};

/// Global arena, used for all cba allocations.
static Arena global_arena = {
    .base = arena_memory_block,
    .used = 0,
    .capacity = CBA_MEMORY_BLOCK_SIZE,
    .alignment = 0,
    .temp_memory_pos = 0,
};

/// Allocates at least `size` bytes via the global arena, returning an address to the
/// resulting memory. Allocations are aligned to the system's cache line size, and are
/// always zeroed by this function.
///
/// If the arena does not have the capacity to allocate `size` bytes, an assertion will
/// fail.
///
/// See also the convenience macros:
/// - `alloc`: allocates an instance of a type
/// - `alloc_bytes`: allocates a number of bytes
/// - `alloc_array`: allocates a number of typed elements
CBA_DEF void* arena_alloc(Arena* arena, usize size);

#define alloc(type) (type*)arena_alloc(&global_arena, sizeof(type))
#define alloc_bytes(count) (u8*)arena_alloc(&global_arena, (count))
#define alloc_array(count, type) (type*)arena_alloc(&global_arena, (count) * sizeof(type))

#define begin_temp_memory() global_arena.temp_memory_pos += 1; usize __savepoint = global_arena.used
#define end_temp_memory()   global_arena.temp_memory_pos -= 1; global_arena.used = __savepoint

// @mark: files

/// If any files in the `input_paths` array have been modified since the file at
/// `output_path`, `1` is returned and `0` otherwise. If an error occurs (i.e. a
/// filesystem error), `-1` is returned.
CBA_DEF i32 files_needs_rebuild(String output_path, StringArray input_paths);
/// If the file at `input_path` has been modified since the file at `output_path`, `1` is
/// returned and `0` otherwise. If an error occurs (i.e. a filesystem error), `-1` is
/// returned.
CBA_DEF i32 file_needs_rebuild(String output_path, String input_path);

/// Creates a file at `path`.
CBA_DEF b32 file_create(const char* path);
/// Moves the file at `path` to `new_path`, returning true if the operation succeeded.
CBA_DEF b32 file_move(const char* path, const char* new_path);
/// Copies the file at `path` to `new_path`, returning true if the operation succeeded.
///
/// On Unix systems, `symbolic_link` will make `new_path` a symbolic link to the file at
/// `path`. On Windows, setting `symbolic_link` to `true` will have no effect.
///
/// You can use `#if CBA_WINDOWS` or `#ifdef _WIN32` to check the platform.
CBA_DEF b32 file_copy(const char* path, const char* new_path, b32 symbolic_link);
/// Deletes the file at `path`, returning true if the operation succeeded.
CBA_DEF b32 file_delete(const char* path);
/// Whether a file at `path` exists.
CBA_DEF b32 file_exists(const char* path);
/// Returns the type of the file at `path`.
CBA_DEF FileType file_get_type(const char* path);
/// Returns the length of the file in bytes.
CBA_DEF usize file_length(const char* path);
/// Reads a number of `bytes` from the file into the `dest` memory.
CBA_DEF b32 file_read(const char* path, void* dest, usize bytes);
/// Writes a number of `bytes` from the provided `memory` to the file, optionally
/// appending to the file.
CBA_DEF b32 file_write(const char* path, void* memory, usize bytes, b32 append);
/// Attempts to make a directory at `path` if the directory does not already exist. The
/// operation is recursive, so you can provide new nested directories, and they will all
/// be created (if possible).
///
/// If the directory was successfully created or already exists, this function returns
/// `true`. If an error occurred, it returns `false`.
CBA_DEF b32 try_mkdir(const char* path);

// @mark: processes

/// Sleeps the current thread for `ms` milliseconds.
CBA_DEF void wait_ms(u64 ms);

/// Returns the current time in nanoseconds.
CBA_DEF u64 nanos_now(void);

/// Attempts to spawn a new process with the provided `cmd`, which will be invoked by the
/// system's shell.
///
/// If the process failed to spawn, the resulting `ProcessID` will be `INVALID_HANDLE`.
///
/// This process needs to permanently allocate memory - be sure not to wrap it in a
/// temporary memory block!
CBA_DEF ProcessID proc_start(Command cmd, FileDescriptor output_fd);

/// Blocks the current thread until the provided process has terminated, returning the
/// result:
/// - `-1`: the process could not be waited on
/// - `0`: the process returned a non-zero exit code, or was terminated by a signal
/// - `1`: the process exited normally
CBA_DEF i32 proc_wait(ProcessID proc);

CBA_DEF i32 __proc_wait_va(usize n, ...);
#define procs_wait(...) \
    __proc_wait_va((sizeof((ProcessID[]) { __VA_ARGS__ }) / sizeof(ProcessID)), __VA_ARGS__)

// @mark: string

#define strl(literal) ((String) { .data = (u8*)(literal), .len = sizeof(literal) - 1, .cap = sizeof(literal) })
#define sfmt(s) (int)((s).len), (char*)((s).data)
#define stok "`%.*s`"

/// Clears the string (sets its length to 0), and zeroes its memory.
CBA_DEF void str_clear(String* str);

/// Allocates an empty string with a capacity of `CBA_STRING_MIN_CAPACITY` bytes.
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

/// Writes the contents of `s` to a file at `path`, optionally appending the data to the
/// file.
CBA_DEF b32 str_write_to_file(String s, const char* path, b32 append);

/// Creates a "slice" of the provided `str`, with the provided `start` position and `len`.
CBA_DEF String str_slice(String str, usize start, usize len);

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
CBA_DEF String str_path_to_absolute(String str);

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
/// Attempts to split the provided file `path` string into all of its parent paths,
/// starting with the root directory and ending with the parent directory of the file (if
/// any). If this fails, the resulting array will be zeroed.
///
/// For example, `/a/b/c/d/file.txt` would be split into:
/// `{ "/a", "/a/b", "/a/b/c", "/a/b/c/d" }`
CBA_DEF StringArray str_to_parent_paths(String path);

/// Attempts to return the file names of all entries within a directory at `path`. If this
/// fails, the resulting array will be zeroed.
///
/// If `include_directory_path` is `true`, the resulting strings will include the
/// directory path. 
///
/// For example, for a directory `/a/b` containing files `c.txt` and `d.txt`:
/// `str_to_directory_entries(path, true); // -> { "/a/b/c.txt", "/a/b/d.txt" }`
CBA_DEF StringArray str_to_directory_entries(String path, b32 include_directory_path);

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
/// of the provided `string`.
CBA_DEF void str_trim_chars(String* str, const char* delims);
/// Trims all whitespace characters from the start and end of the provided `string`. This
/// includes: ' ', '\n', '\r', '\t'.
CBA_DEF void str_trim_whitespace(String* str);

/// Whether `a` is equivalent to `b`.
CBA_DEF b32 str_eq(String a, String b);
/// Whether `a` is equivalent to the null-terminated `b` C-string.
CBA_DEF b32 str_eq_cstr(String str, const char* cstr);

/// Whether `str` starts with `cstr` (excluding a null-terminator).
CBA_DEF b32 str_starts_with(String str, const char* cstr);
/// Whether `str` ends with `cstr` (excluding a null-terminator).
CBA_DEF b32 str_ends_with(String str, const char* cstr);

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
CBA_DEF b32 str_find_last_other(String haystack, String needle, usize* where);
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
/// first matching character, if found.
CBA_DEF b32 str_find_first_char_from(String haystack, char needle, usize from, usize* where);
/// Whether `needle` could be found in `haystack`, starting the search at the `from` index
/// and progressing backwards. When `where` is non-NULL, it is set to the index of the
/// last matching character, if found.
CBA_DEF b32 str_find_last_char_from(String haystack, char needle, usize from, usize* where);

// @todo: other/cstr variants of above.

/// Returns the number of characters matching the `needle` char in the provided string.
CBA_DEF u64 str_count_chars(String haystack, char needle);
/// Returns the number of matches with the `needle` C-string in the provided string.
/// Null-terminators are not considered.
CBA_DEF u64 str_count_cstrs(String haystack, const char* needle, b32 case_sensitive);
/// Returns the number of matches with the `needle` string in the provided string.
CBA_DEF u64 str_count_others(String haystack, String needle, b32 case_sensitive);

/// Attempts to parse the string `str` to a `i64` value, returning `true` if successful.
CBA_DEF b32 str_parse_to_i64(String str, i64* dest);
/// Attempts to parse the string `str` to a `f64` value, returning `true` if successful.
CBA_DEF b32 str_parse_to_f64(String str, f64* dest);

/// Allocates and returns a null-terminated string containing the data of the provided string.
CBA_DEF char* str_to_cstr(String str);

/// Returns a pointer to a null-terminated C-string created via a formatted string and
/// optional arguments. The string is allocated via the global arena.
CBA_DEF char* alloc_sprintf(const char* fmt, ...) PRINTF_FORMAT(1, 2);

/// Returns a formatted pretty string of the amount of memory represented by `num_bytes`.
CBA_DEF char* fmt_bytes(usize num_bytes);

/// Returns a formatted pretty string of the amount of time represented by `nanos`.
///
/// `unit_verbosity` describes how verbose units are:
/// - `0`: "ns",          "ms",           "s",       etc.
/// - `1`: "nanos",       "millis",       "secs",    etc.
/// - `2`: "nanoseconds", "milliseconds", "seconds", etc.
CBA_DEF char* fmt_time(u64 nanos, u8 unit_verbosity);

// @mark: string array

/// Appends a single `String` to the provided `StringArray`.
CBA_DEF void str_arr_append_str(StringArray* arr, String str);

CBA_DEF void __str_arr_append_va(StringArray* arr, usize n, ...);
#define str_arr_append(arr, ...)                                             \
    __str_arr_append_va((arr), (sizeof((String[]){__VA_ARGS__}) / sizeof(String)), __VA_ARGS__)

CBA_DEF void __str_arr_append_cstrs_va(StringArray* arr, usize n, ...);
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

/// Run the provided command with the provided options.
///
/// See also `cmd_try_run`.
CBA_DEF b32 cmd_try_run_with_opts(Command cmd, CommandOptions opts);
#define cmd_try_run(cmd, ...) \
    cmd_try_run_with_opts((cmd), (CommandOptions) { __VA_ARGS__ })

#define cmd_run(cmd, ...)                                                                \
    assert(cmd_try_run_with_opts((cmd), (CommandOptions) { __VA_ARGS__ }),               \
           "failed to run command `%.*s`",                                               \
           sfmt(cmd_flatten(cmd)))

CBA_DEF b32 cmd_try_run_direct_with_opts(const char* command, CommandOptions opts);
#define cmd_try_run_direct(cmd, ...) \
    cmd_try_run_with_opts((cmd), ((CommandOptions) { __VA_ARGS__ }))

#define cmd_run_direct(cmd, ...)                                                         \
    assert(cmd_try_run_direct_with_opts((cmd), (CommandOptions) { __VA_ARGS__ }),        \
           "failed to run command `%s`",                                                 \
           cmd)

// CBA_DEF b32 __cmd_run_direct_va(usize n, ...);
// #define cmd_run_direct(...) \
//     __cmd_run_direct_va((sizeof((const char*[]) { __VA_ARGS__ }) / sizeof(const char*)), __VA_ARGS__)

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





// @mark: implementation

#ifdef CBA_IMPLEMENTATION

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
#endif

CBA_DEF void __cba_bootstrap_build(int argc, char** argv, const char* source_path, ...) {
    int exit_code = 0;

    String binary_path = str_from_cstr(argv[0]);

#if CBA_WINDOWS
    if (!str_ends_with(binary_path, ".exe")) {
        str_append_cstr(&binary_path, ".exe");
    }
#endif

    StringArray source_paths = {0};
    str_arr_append(&source_paths, str_from_cstr(source_path));

    uninit va_list args;
    va_start(args, source_path);
    for (;;) {
        const char* path = va_arg(args, const char*);
        if (!path) break;

        str_arr_append(&source_paths, str_from_cstr(path));
    }
    va_end(args);

    i32 rebuild_needed = files_needs_rebuild(binary_path, source_paths);
    if (rebuild_needed == -1) {
        exit_code = 1;
    }
    else if (rebuild_needed == 1) {
        Command cmd = {0};
        String old_binary_path = str_sprintf("%.*s.bak", sfmt(binary_path));

        const char* binary_path_cstr = (char*)binary_path.data;
        const char* old_binary_path_cstr = (char*)old_binary_path.data;

#if defined(CBA_PRINT_ON_BOOTSTRAP) || defined(CBA_VERBOSE)
        info("%s", CBA_BOOTSTRAP_MESSAGE(binary_path_cstr));
#endif

        if (file_move(binary_path_cstr, old_binary_path_cstr)) {
            cmd_append(&cmd, CBA_BOOTSTRAP_COMMAND(argv[0], source_path));

            b32 success = cmd_try_run(cmd);
            cmd_reset(&cmd);

            if (success) {
                // re-run the previous command with the new binary.
                StringArray args = str_arr_from_cstr_arr(argv + 1, (usize)(argc - 1));
                cmd_append_str(&cmd, binary_path);
                cmd_append_str_arr(&cmd, args);

                b32 success = cmd_try_run(cmd);
                cmd_reset(&cmd);

                if (!success) {
                    exit_code = 1;
                }
            }
            else {
                assert(file_move(old_binary_path_cstr, binary_path_cstr),
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

CBA_DEF String get_cwd(void) {
    String result = str_alloc_with_cap(CBA_MAX_PATH);

#if CBA_WINDOWS
    char* cwd = _getcwd((char*)result.data, CBA_MAX_PATH);
#else
    char* cwd = getcwd((char*)result.data, CBA_MAX_PATH);
#endif

    assert(cwd, "failed to obtain current working directory");

    return result;
}

CBA_DEF usize cache_line_size(void) {
    usize result = 0;

#if CBA_WINDOWS
    DWORD buffer_size = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = NULL;

    begin_temp_memory();
    {
        GetLogicalProcessorInformation(0, &buffer_size);
        buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)arena_alloc(&global_arena, buffer_size);
        GetLogicalProcessorInformation(&buffer, &buffer_size);

        u32 count = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        for (u32 i = 0; i < count; ++i) {
            if ((buffer[i].Relationship == RelationCache) && (buffer[i].Cache.Level == 1)) {
                result = buffer[i].Cache.LineSize;
                break;
            }
        }
    }
    end_temp_memory();
#elif CBA_MACOS
    usize usize_size = sizeof(usize);
    sysctlbyname("hw.cachelinesize", &result, &usize_size, 0, 0);
#else
    FILE* f = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");

    if (f) {
        u32 i = 0;

        // @jcg: no need to check result because if it's 0 then it'd be returned anyway.
        fscanf(f, "%u", &i);
        result = (usize)i;

        fclose(f);
    }
#endif

    return result;
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

CBA_DEF void* arena_alloc(Arena* arena, usize size) {
    void* result = NULL;

    if (!arena->alignment) {
        arena->alignment = cache_line_size();
    }

    usize alignment_offset = 0;
    isize curr = (isize)(arena->base + arena->used);
    isize mask = (isize)arena->alignment - 1;

    if (curr & mask) {
        alignment_offset = (usize)((isize)arena->alignment - (curr & mask));
    }

    usize effective_size = size + alignment_offset;

    assert((arena->used + effective_size) <= arena->capacity,
           "arena overflowed its memory block (capacity: %zu, used: %zu, allocating: %zu)",
           arena->capacity, arena->used, effective_size);

    result = arena->base + arena->used + alignment_offset;
    arena->used += effective_size;
    mem_zero(result, size);

    return result;
}





// @mark: files

static inline FileDescriptor _open_fd_for_read_write(const char* path) {
    FileDescriptor result = INVALID_HANDLE;

#if CBA_WINDOWS
    SECURITY_ATTRIBUTES attr = {0};
    attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    attr.bInheritHandle = TRUE;

    HANDLE fd = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, &attr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fd != INVALID_HANDLE) {
        result = (FileDescriptor)fd;
    }
    else {
        verbose_print("failed to open file descriptor for \"%s\": %s", path, win32_err_message(GetLastError()));
    }
#else
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd >= 0) {
        result = (FileDescriptor)fd;
    }
    else {
        verbose_print("failed to open file descriptor for \"%s\": %s", path, strerror(errno));
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

CBA_DEF i32 files_needs_rebuild(String output_path, StringArray input_paths) {
    i32 result = 0;

#if CBA_WINDOWS
    begin_temp_memory();

    char* output_path_cstr = str_to_cstr(output_path);

    HANDLE output_path_fd = CreateFile(output_path_cstr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (output_path_fd != INVALID_HANDLE_VALUE) {
        uninit FILETIME output_path_time;
        BOOL got_output_file_time = GetFileTime(output_path_fd, NULL, NULL, &output_path_time);
        CloseHandle(output_path_fd);

        if (got_output_file_time) {
            for (usize i = 0; i < input_paths.count; ++i) {
                // @jcg: bit of a dodgy hack, but this is ok because the arena does not
                // perform other allocations while the copied string data is in used, and so
                // its memory is not touched. it isn't necessary to use temporary memory here
                // anyway, but it just helps to keep the memory usage down in the case that
                // there are a lot of input file paths which need to be copied.
                begin_temp_memory();
                char* input_path_cstr = str_to_cstr(input_paths.items[i]);
                end_temp_memory();

                HANDLE input_path_fd = CreateFile(input_path_cstr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

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
                        verbose_print("failed to stat input file \"%s\": %s", input_path_cstr, win32_err_message(GetLastError()));
                        result = -1;
                    }
                }
                else {
                    verbose_print("failed to open input file \"%s\": %s", input_path_cstr, win32_err_message(GetLastError()));
                    result = -1;
                }
            }
        }
        else {
            verbose_print("failed to stat output file \"%s\": %s", output_path_cstr, win32_err_message(GetLastError()));
            result = -1;
        }
    }
    else {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            result = 1;
        }
        else {
            verbose_print("failed to open output file \"%s\": %s", output_path_cstr, win32_err_message(GetLastError()));
            result = -1;
        }
    }

    end_temp_memory();
#else
    begin_temp_memory();

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
                verbose_print("failed to stat input file \"%s\": %s", input_path_cstr, strerror(errno));
                result = -1;
            }
        }
    }
    else {
        if (errno == ENOENT) {
            result = 1;
        }
        else {
            verbose_print("failed to stat output file \"%s\": %s", output_path_cstr, strerror(errno));
            result = -1;
        }
    }

    end_temp_memory();
#endif

    return result;
}

CBA_DEF i32 file_needs_rebuild(String output_path, String input_path) {
    i32 result = false;

    begin_temp_memory();

    StringArray arr = {0};
    str_arr_append(&arr, input_path);

    result = files_needs_rebuild(output_path, arr);

    end_temp_memory();

    return result;
}

CBA_DEF b32 file_create(const char* path) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#else
    FILE* f = fopen(path, "w+");

    if (f) {
        result = true;
        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF b32 file_move(const char* path, const char* new_path) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#else
    result = rename(path, new_path) == 0;

    if (!result) {
        verbose_print("failed to rename \"%s\" to \"%s\": %s", path, new_path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF b32 file_copy(const char* path, const char* new_path, b32 symbolic_link) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#elif CBA_MACOS
    if (symbolic_link) {
        result = symlink(path, new_path) == 0;
    }
    else {
        result = copyfile(path, new_path, NULL, COPYFILE_DATA) == 0;
    }
#else
    begin_temp_memory();
    {
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
            assert(i == 0, "failed to truncate new file during copy");

            close(new_fd);
            close(existing_fd);

            result = size == stat_existing.st_size;
        }
    }
    end_temp_memory();
#endif

    return result;
}

CBA_DEF b32 file_delete(const char* path) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#else
    // @todo: recursive dir deletion? i.e. if rmdir returns ENOTEMPTY. there's ntfw on
    // posix which might be useful.

    result = remove(path) == 0;

    if (!result) {
        verbose_print("failed to delete \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF b32 file_exists(const char* path) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#else
    result = access(path, F_OK) == 0;
#endif

    return result;
}

CBA_DEF FileType file_get_type(const char* path) {
    FileType result = FILE_TYPE_UNKNOWN;

#if CBA_WINDOWS
    todo();
#else
    uninit struct stat statbuf;
    if (lstat(path, &statbuf) >= 0) {
        if (S_ISREG(statbuf.st_mode)) {
            result = FILE_TYPE_FILE;
        }
        else if (S_ISDIR(statbuf.st_mode)) {
            result = FILE_TYPE_DIRECTORY;
        }
        else if (S_ISLNK(statbuf.st_mode)) {
            result = FILE_TYPE_SYMLINK;
        }
        else {
            result = FILE_TYPE_OTHER;
        }
    }
    else {
        verbose_print("failed to stat file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF usize file_length(const char* path) {
    usize result = 0;

#if CBA_WINDOWS
    todo();
#else
    FILE* f = fopen(path, "rb");

    if (f) {
        int seek_result = fseek(f, 0, SEEK_END);

        if (seek_result != 0) {
            verbose_print("failed to seek to end of file \"%s\": %s", path, strerror(errno));
        }
        else {
            result = (usize)ftell(f);
        }

        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF b32 file_read(const char* path, void* dest, usize bytes) {
    b32 result = false;

    assert(dest, "cannot read to NULL memory");
    assert(bytes, "cannot read zero bytes");

#if CBA_WINDOWS
    todo();
#else
    FILE* f = fopen(path, "rb");

    if (f) {
        usize bytes_read = fread(dest, 1, bytes, f);

        if (bytes_read == 0) {
            verbose_print("failed to read any memory from \"%s\": %s", path, strerror(errno));
        }
        else {
            result = true;
        }

        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF b32 file_write(const char* path, void* memory, usize bytes, b32 append) {
    b32 result = false;

    assert(memory, "cannot write from NULL memory");
    assert(bytes, "cannot write zero bytes");

#if CBA_WINDOWS
    todo();
#else
    uninit FILE* f;
    if (append) {
        f = fopen(path, "wb");
    }
    else {
        f = fopen(path, "ab");
    }

    if (f) {
        usize bytes_written = fwrite(memory, 1, bytes, f);

        if (!bytes_written && !feof(f)) {
            verbose_print("failed to write memory to \"%s\": %s", path, strerror(errno));
        }
        else {
            result = true;
        }

        fclose(f);
    }
    else {
        verbose_print("failed to open file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

#if !CBA_WINDOWS
CBA_INLINE b32 _mkdir(const char* path) {
    b32 result = true;

    if (!file_exists(path)) {
        int res = mkdir(path, 0755);

        if (res < 0) {
            assert(errno != EEXIST, "the file should not exist, because it has already been checked");
            verbose_print("failed to create directory \"%s\": %s", path, strerror(errno));
            result = false;
        }
    }

    return result;
}
#endif

CBA_DEF b32 try_mkdir(const char* path) {
    b32 result = false;

    // @todo: assert path is a directory?

#if CBA_WINDOWS
    todo();
#else
    begin_temp_memory();
    {
        String path_str = str_from_cstr(path);

        if (!file_exists(path)) {
            StringArray parent_paths = str_to_parent_paths(path_str);

            if (parent_paths.items) {
                result = true;

                for (usize i = 0; i < parent_paths.count; ++i) {
                    char* path_cstr = (char*)str_to_cstr(parent_paths.items[i]);
                    _mkdir(path_cstr);
                }

                // @jcg: the above only creates parent paths: the top-level dir still needs to
                // be created.
                _mkdir((char*)path);
            }
        }
        else {
            verbose_print("directory \"%s\" already exists", path);
            result = true;
        }
    }
    end_temp_memory();
#endif

    return result;
}





// @mark: procs

CBA_DEF void wait_ms(u64 ms) {
#if CBA_WINDOWS
    Sleep(ms);
#else
    u64 secs = ms / 1000;
    u64 nanos = (ms - (secs * 1000)) * 1000000;

    print("%llu | %llu | %llu", ms, secs, nanos);

    struct timespec duration = {
        .tv_sec  = (long)secs,
        .tv_nsec = (long)nanos,
    };

    nanosleep(&duration, NULL);
#endif
}

CBA_DEF u64 nanos_now(void) {
    u64 result = 0;

#if CBA_WINDOWS
    todo();
#else
    result = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
#endif

    return result;
}

CBA_DEF ProcessID proc_start(Command cmd, FileDescriptor output_fd) {
    ProcessID result = INVALID_HANDLE;

#if CBA_WINDOWS
    todo();
#else
    assert(global_arena.temp_memory_pos == 0, "cannot spawn new process in a temporary memory block");

    if (cmd.count >= 1) {
        pid_t cpid = fork();

        if (cpid < 0) {
            verbose_print("failed to fork child process: \"%s\"", strerror(errno));
        }
        else if (cpid == 0) {
            b32 streams_valid = true;

            if (output_fd != INVALID_HANDLE) {
                if (dup2(output_fd, STDERR_FILENO) < 0) {
                    verbose_print("failed to create stderr for child process: %s", strerror(errno));
                    streams_valid = false;
                }
                if (streams_valid && dup2(output_fd, STDOUT_FILENO) < 0) {
                    verbose_print("failed to create stdout for child process: %s", strerror(errno));
                    streams_valid = false;
                }
            }

            if (streams_valid) {
                // @jcg: this is the memory allocated to the child process' arguments, so
                // it needs to be allocated permanently to outlive the child process.
                char** arr = alloc_array(cmd.count + 1, char*);
                for (usize i = 0; i < cmd.count; ++i) {
                    arr[i] = alloc_array(cmd.items[i].len, char);
                    memcpy(arr[i], cmd.items[i].data, cmd.items[i].len);
                }

                int exec_result = execvp(arr[0], arr);

                if (exec_result >= 0) {
                    result = (ProcessID)cpid;

                    begin_temp_memory();
                    {
                        verbose_print("spawned process from \"%s\"", cmd_flatten_to_cstr(cmd));
                    }
                    end_temp_memory();
                }
                else {
                    verbose_print("failed to exec child process for \"%s\": %s", arr[0], strerror(errno));
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

CBA_DEF i32 proc_wait(ProcessID proc) {
    i32 result = 1;

#if CBA_WINDOWS
    todo();
#else
    for (;;) {
        int wstatus = 0;

        if (waitpid(proc, &wstatus, 0) < 0) {
            verbose_print("failed to wait on command with PID %d: %s", proc, strerror(errno));
            result = -1;
            break;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);

            if (exit_status != 0) {
                result = 0;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            verbose_print("process with PID %d was terminated by signal %d", proc, WTERMSIG(wstatus));
            result = 0;
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
        i32 r = proc_wait(arg);

        if (r != 1) {
            result = r;
            break;
        }
    }

    va_end(args);

    return result;
}





// @mark: strings

CBA_DEF void str_clear(String* str) {
    str->len = 0;
    mem_zero(str->data, str->cap);
}

CBA_DEF String str_alloc(void) {
    String result = {
        .data = alloc_bytes(CBA_STRING_MIN_CAPACITY),
        .len = 0,
        .cap = CBA_STRING_MIN_CAPACITY - 1,
    };

    return result;
}

CBA_DEF String str_alloc_with_cap(usize cap) {
    String result = {
        .data = alloc_bytes(cap),
        .len = 0,
        .cap = cap - 1,
    };

    return result;
}

CBA_DEF String str_sprintf(const char* fmt, ...) {
    String result = {0};

    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    // @jcg: in a nutshell, vsnprintf returns the length minus a null-terminator when used
    // as above, but will append a null-terminator anyway when used as below - hence the
    // cap + 1 for the allocation.
    usize cap = max(len, CBA_STRING_MIN_CAPACITY);
    result.data = alloc_bytes(cap + 1);
    result.len = len;
    result.cap = cap;
    vsnprintf((char*)result.data, len + 1, fmt, args);

    va_end(args);

    return result;
}

CBA_DEF String str_from_cstr(const char* cstr) {
    usize len = (usize)strlen(cstr);
    usize cap = max(len + 1, CBA_STRING_MIN_CAPACITY);

    String result = {
        // @jcg: one extra byte is allocated so that the string is compatible with
        // c-strings, as it'll technically have a null-terminator. This avoids some
        // annoying issues when dealing with APIs that expect null-terminated strings.
        .data = alloc_bytes(cap),
        .len = len,
        .cap = cap - 1,
    };

    memcpy(result.data, cstr, len);

    return result;
}

CBA_DEF String str_from_chars(char* buffer, usize count) {
    usize cap = max(count, CBA_STRING_MIN_CAPACITY);

    String result = {
        .data = alloc_bytes(cap + 1),
        .len = count,
        .cap = cap,
    };

    memcpy(result.data, buffer, count);

    return result;
}

CBA_DEF String str_from_file(const char* file_path) {
    String result = {0};

    FILE* f = fopen(file_path, "rb");
    assert(f, "failed to open file \"%s\" for reading", file_path);

    fseek(f, 0, SEEK_END);
    usize len = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len) {
        result.data = alloc_bytes(len + 1);
        result.len = len;
        result.cap = len;

        usize bytes_read = (usize)fread(result.data, 1, len, f);
        assert(bytes_read > 0, "no bytes were read from \"%s\"", file_path);
    }

    fclose(f);

    return result;
}

CBA_DEF b32 str_write_to_file(String s, const char* path, b32 append) {
    b32 result = false;

#if CBA_WINDOWS
    todo();
#else
    FILE* f = fopen(path, append ? "ab" : "wb");

    if (f) {
        usize bytes_written = fwrite(s.data, 1, s.len, f);

        if (!bytes_written && !feof(f)) {
            verbose_print("failed to write string to file \"%s\": %s", path, strerror(errno));
        }
        else {
            result = true;
        }

        fclose(f);
    }
    else {
        verbose_print("failed to write string to file \"%s\": %s", path, strerror(errno));
    }
#endif

    return result;
}

CBA_DEF String str_slice(String str, usize start, usize len) {
    assert((start + len) <= str.len,
           "string slice exceeds the string's length (start: %zu, len: %zu, string len: %zu)",
           start, len, str.len);

    String result = {
        .data = str.data + start,
        .len = len,
        .cap = str.cap - len,
    };

    return result;
}

CBA_DEF String str_path_file_name(String str, b32 include_extension) {
    String result = str;

    uninit usize separator_pos;
    b32 found_separator = str_find_last_char(str, '/',  &separator_pos) ||
                          str_find_last_char(str, '\\', &separator_pos);

    if (found_separator) {
        // @jcg: +1 because the separator shouldn't be included.
        result.data += separator_pos + 1;
        result.len  -= separator_pos + 1;
    }

    if (!include_extension) {
        uninit usize dot_pos;
        if (str_find_last_char(str, '.', &dot_pos)) {
            result.len -= str.len - dot_pos;
        }
    }

    return result;
}

CBA_DEF String str_path_file_extension(String str) {
    String result = {0};

    uninit usize dot_pos;
    if (str_find_last_char(str, '.', &dot_pos)) {
        result.data = str.data;
        result.len = str.len - dot_pos;
    }

    return result;
}

CBA_DEF String str_path_pwd(String str) {
    String result = {0};

    uninit usize separator_pos;
    b32 found_separator = str_find_last_char(str, '/', &separator_pos) ||
                          str_find_last_char(str, '\\', &separator_pos);

    if (found_separator) {
        result.data += separator_pos - 1;
        result.len = separator_pos;
    }

    return result;
}

CBA_DEF String str_path_to_absolute(String str) {
    String result = str_alloc_with_cap(CBA_MAX_PATH);

#if CBA_WINDOWS
    todo();
#else
    assert(str.len < CBA_MAX_PATH, "input path length exceeds PATH_MAX");

    char* p = realpath((char*)str.data, (char*)result.data);

    if (!p) {
        if ((str.len > 0) && (str.data[0] == CBA_PATH_SEP)) {
            // the path appears absolute anyway.
            str_copy_into(&result, str);
        }
        else {
            // the path appears relative, so prepending the cwd should work.
            begin_temp_memory();
            {
                String cwd = get_cwd();
                str_append_other(&result, cwd);
            }
            end_temp_memory();

            str_append_char(&result, CBA_PATH_SEP);
            str_append_other(&result, str);
        }
    }
    else {
        result.len = strlen((char*)result.data);
    }
#endif

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

CBA_DEF StringArray str_to_parent_paths(String path) {
    StringArray result = {0};

    // "/a/b/c/d/file.txt" -> { "/a", "/a/b", "/a/b/c", "/a/b/c/d" }
    //
    // "some_dir_name/subdir/file.txt" -> { "some_dir_name", "some_dir_name/subdir" }

    assert(path.len > 0, "cannot convert an empty path to its parents");

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

CBA_DEF StringArray str_to_directory_entries(String path, b32 include_directory_path) {
    (void)(path);
    (void)(include_directory_path);

    StringArray result = {0};

    todo();

    return result;
}

CBA_DEF String str_copy(String str) {
    String result = {
        .data = alloc_bytes(str.cap + 1),
        .len = str.len,
        .cap = str.cap,
    };

    memcpy(result.data, str.data, str.len);

    return result;
}

CBA_DEF void str_copy_into(String* dest, String source) {
    assert(source.len <= dest->cap,
           "insufficient capacity for string copy (cap: %zu, source len: %zu)",
           dest->cap, source.len);

    memcpy(dest->data, source.data, source.len);
    dest->len = source.len;
}

CBA_DEF void str_append_null(String* str) {
    assert(str->len < str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu)",
           str->cap, str->len);

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
    assert(str->len < str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu)",
           str->cap, str->len);

    str->data[str->len] = (u8)ch;
    str->len += 1;
}

CBA_DEF void str_append_cstr(String* str, const char* cstr) {
    usize len = (usize)strlen(cstr);

    assert((str->len + len) <= str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu, appending: %zu)",
           str->cap, str->len, len);

    memcpy(str->data + str->len, cstr, len);
    str->len += len;
}

CBA_DEF void str_append_chars(String* str, char* buffer, usize count) {
    assert((str->len + count) <= str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu, appending: %zu)",
           str->cap, str->len, count);

    memcpy(str->data + str->len, buffer, count);
    str->len += count;
}

CBA_DEF void str_append_other(String* str, String other) {
    assert((str->len + other.len) <= str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu, appending: %zu)",
           str->cap, str->len, other.len);

    memcpy(str->data + str->len, other.data, other.len);
    str->len += other.len;
}

CBA_DEF void str_appendf(String* str, const char* fmt, ...) {
    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    // @jcg: see the str_sprintf implementation for an explanation of the + 1s here.
    assert((str->len + len + 1) <= str->cap,
           "tried to exceed string capacity (cap: %zu, len: %zu, appending: %i)",
           str->cap, str->len, len);

    vsnprintf((char*)(str->data + str->len), len + 1, fmt, args);

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

    assert(start <= str->len,
           "shift start is outside of the string (start: %zu, len: %zu)",
           start, str->len);
    assert(0 < start && shift <= start,
           "string should would underflow (start: %zu, shift: %zu)",
           start, shift);

    for (usize i = start; i < str->len; ++i) {
        str->data[i - shift] = str->data[i];
    }

    str->len -= shift;
    // @jcg: required to retain a null-terminator after the string contents.
    mem_zero(str->data + str->len, shift);
}

CBA_DEF void str_rshift(String* str, usize start, usize shift) {
    if (!shift) return;

    assert(start <= str->len,
           "shift start is outside of the string (start: %zu, len: %zu)",
           start, str->len);
    assert((str->len + shift) <= str->cap,
           "string shift would overflow (len: %zu, shift: %zu, cap: %zu)",
           str->len, shift, str->cap);

    usize new_len = str->len + shift;
    usize end = start + shift;

    for (usize i = (new_len - 1); i >= end; --i) {
        str->data[i] = str->data[i - shift];
    }

    mem_zero(str->data + start, shift);
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
    assert(str->len > 0, "tried to remove from an empty string (zero length)");
    str_lshift(str, at + 1, 1);
}

CBA_DEF void str_remove_range(String* str, usize start, usize end) {
    assert(str->len > 0, "tried to remove from an empty string (zero length)");
    assert(end >= start, "incorrect removal range (start: %zu, end: %zu)", start, end);

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

CBA_DEF void str_trim_chars(String* str, const char* delims) {
    if (!str->len) return;

    usize start = 0;
    usize end = str->len - 1;
    usize num_delims = (usize)strlen(delims);

    while (start < str->len) {
        b32 found = false;

        for (usize d = 0; d < num_delims; ++d) {
            if ((char)str->data[start] == delims[d]) {
                found = true;
                break;
            }
        }

        if (!found) break;
        start += 1;
    }

    while (end > start) {
        b32 found = false;

        for (usize d = 0; d < num_delims; ++d) {
            if ((char)str->data[end] == delims[d]) {
                found = true;
                break;
            }

            if (!found) break;
            end -= 1;
        }
    }

    str->data += start;
    str->len = end - start + 1;
}

CBA_DEF void str_trim_whitespace(String* str) {
    str_trim_chars(str, " \n\r\t\v\f");
}

CBA_DEF b32 str_eq(String a, String b) {
    return (a.len == b.len) && (memcmp(a.data, b.data, a.len) == 0);
}

CBA_DEF b32 str_eq_cstr(String str, const char* cstr) {
    return (str.len == (usize)strlen(cstr)) && (memcmp(str.data, cstr, str.len) == 0);
}

CBA_DEF b32 str_starts_with(String str, const char* cstr) {
    usize len = (usize)strlen(cstr);
    return (str.len >= len) && (memcmp(str.data, cstr, len) == 0);
}

CBA_DEF b32 str_ends_with(String str, const char* cstr) {
    usize len = (usize)strlen(cstr);
    return (str.len >= len) && (memcmp(str.data + len, cstr, len) == 0);
}

CBA_DEF b32 str_find_first_char(String haystack, char needle, usize* where) {
    return str_find_first_char_from(haystack, needle, 0, where);
}

CBA_DEF b32 str_find_last_char(String haystack, char needle, usize* where) {
    return haystack.len && str_find_last_char_from(haystack, needle, haystack.len - 1, where);
}

CBA_DEF b32 str_find_first_other(String haystack, String needle, b32 case_sensitive, usize* where) {
    if (!haystack.len || !needle.len) return false;

    b32 result = false;

    if (haystack.len > needle.len) {
        usize iters = haystack.len - needle.len;
        usize off = 0;

        do {
            b32 mismatch = false;

            for (usize i = 0; i < needle.len; ++i) {
                u8 a = haystack.data[off + i];
                u8 b = needle.data[off];

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

CBA_DEF b32 str_find_last_other(String haystack, String needle, usize* where) {
    (void)haystack;
    (void)needle;
    (void)where;

    b32 result = false;
    // @todo
    todo();
    return result;
}

CBA_DEF b32 str_find_first_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where) {
    begin_temp_memory();

    String needle_str = str_from_cstr(needle);
    b32 result = str_find_first_other(haystack, needle_str, case_sensitive, where);

    end_temp_memory();
    return result;
}

CBA_DEF b32 str_find_last_cstr(String haystack, const char* needle, b32 case_sensitive, usize* where) {
    (void)haystack;
    (void)needle;
    (void)case_sensitive;
    (void)where;

    b32 result = false;
    // @todo
    todo();
    return result;
}

CBA_DEF b32 str_find_first_char_from(String haystack, char needle, usize from, usize* where) {
    assert(from < haystack.len,
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
    assert(from < haystack.len,
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

CBA_DEF u64 str_count_cstrs(String haystack, const char* needle, b32 case_sensitive) {
    begin_temp_memory();

    String needle_str = str_from_cstr(needle);
    u64 result = str_count_others(haystack, needle_str, case_sensitive);

    end_temp_memory();
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
                u8 a = haystack.data[off + i];
                u8 b = needle.data[i];

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
    // @todo: parsing for inf/NaN?
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

CBA_DEF char* str_to_cstr(String str) {
    assert(str.data[str.len] == 0, "string data is not null-terminated");

    // u8* result = alloc_bytes(str.len + 1);
    // memcpy(result, str.data, str.len);

    return (char*)str.data;
}

CBA_DEF char* alloc_sprintf(const char* fmt, ...) {
    char* result = NULL;

    uninit va_list args;
    va_start(args, fmt);

    int len = vsnprintf(NULL, 0, fmt, args);
    assert(len > 0, "failed to construct format string from \"%s\"", fmt);

    result = alloc_array(len + 1, char);
    vsnprintf(result, len + 1, fmt, args);
    assert(result[len] == '\0', "null-terminator was not appended");

    va_end(args);

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

CBA_DEF char* fmt_time(u64 nanos, u8 unit_verbosity) {
    uninit char* result;

    const u64 MICRO = 1000;
    const u64 MILLI = 1000000;
    const u64 SEC   = 1000000000;
    const u64 MIN   = 60000000000;

    if (nanos < MICRO) {
        const char* unit = unit_verbosity == 0 ? "ns" : (unit_verbosity == 1 ? "nanos" : "nanoseconds");
        result = alloc_sprintf("%llu %s", nanos, unit);
    }
    else if (nanos < MILLI) {
        const char* unit = unit_verbosity == 0 ? "µs" : (unit_verbosity == 1 ? "micros" : "microseconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-3, unit);
    }
    else if (nanos < SEC) {
        const char* unit = unit_verbosity == 0 ? "ms" : (unit_verbosity == 1 ? "millis" : "milliseconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-6, unit);
    }
    else if (nanos < MIN) {
        const char* unit = unit_verbosity == 0 ? "s" : (unit_verbosity == 1 ? "secs" : "seconds");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * 1e-9, unit);
    }
    else {
        const char* unit = unit_verbosity == 0 ? "m" : (unit_verbosity == 1 ? "mins" : "minutes");
        result = alloc_sprintf("%.3lf %s", (f64)nanos * (60.0 * 1e-9), unit);
    }

    return result;
}





// @mark: string array

CBA_DEF void str_arr_append_str(StringArray* arr, String str) {
    if (!arr->items) {
        arr->items = alloc_array(CBA_ARRAY_COUNT, String);
        arr->count = 0;
    }

    assert(arr->count < CBA_ARRAY_COUNT,
           "tried to exceed maximum string count (current: %zu)",
           arr->count);

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

CBA_DEF void cmd_append_str(Command* cmd, String str) {
    if (!cmd->items) {
        cmd->items = alloc_array(CBA_ARRAY_COUNT, String);
        cmd->count = 0;
    }

    assert(cmd->count < CBA_ARRAY_COUNT,
           "tried to exceed maximum command count (current: %zu)",
           cmd->count);

    cmd->items[cmd->count] = str;
    cmd->count += 1;
}

CBA_DEF void cmd_append_str_arr(Command* cmd, StringArray arr) {
    for (usize i = 0; i < arr.count; ++i) {
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
    assert(args_str.len > 0, "cannot split empty command");

    i32 double_quote_pos = -1;
    i32 single_quote_pos = -1;
    usize next_append_pos = 0;

    for (usize i = 0; i < args_str.len; ++i) {
        if (args_str.data[i] == '\"') {
            if (double_quote_pos != -1) {
                usize len = i - (usize)double_quote_pos;
                assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, (usize)double_quote_pos + 1, len - 1);

                cmd_append_str(cmd, str_copy(arg));
                next_append_pos = i + 1;

                single_quote_pos = -1;
                double_quote_pos = -1;
            }
            else {
                double_quote_pos = i;
            }
        }
        else if (args_str.data[i] == '\'') {
            if (single_quote_pos != -1) {
                usize len = i - (usize)single_quote_pos;
                assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, (usize)single_quote_pos + 1, len - 1);

                cmd_append_str(cmd, str_copy(arg));
                next_append_pos = i + 1;

                single_quote_pos = -1;
                double_quote_pos = -1;
            }
            else {
                single_quote_pos = i;
            }
        }
        else if (args_str.data[i] == ' ') {
            if ((single_quote_pos == -1) && (double_quote_pos == -1) && (i != next_append_pos)) {
                usize len = i - next_append_pos;
                assert(len != (usize)(-1), "incorrect length");
                String arg = str_slice(args_str, next_append_pos, len);

                cmd_append_str(cmd, str_copy(arg));
            }

            next_append_pos = i + 1;
        }
    }

    usize final_len = args_str.len - next_append_pos;

    if (final_len) {
        u8 final = args_str.data[args_str.len - 1];
        if (final == '\'' || final == '\"') {
            final_len -= 1;
        }

        String arg = str_slice(args_str, next_append_pos, final_len);
        cmd_append_str(cmd, str_copy(arg));
    }
}

CBA_DEF b32 cmd_try_run_with_opts(Command cmd, CommandOptions opts) {
    b32 result = false;

    usize sp = global_arena.used;

    verbose_print("running \"%s\"", cmd_flatten_to_cstr(cmd));

    FileDescriptor output_fd = INVALID_HANDLE;
    char* output_file_path = NULL;

    if (opts.output_string) {
        assert(!opts.async_pid, "cannot read command output to string when running as async");

        static u64 output_file_id = 0;

        output_file_path = alloc_sprintf("cba_output_file_%llu", output_file_id);
        output_file_id += nanos_now();

        while (file_exists(output_file_path)) {
            output_file_path = alloc_sprintf("cba_output_file_%llu", output_file_id);
            output_file_id += nanos_now();
        }

        assert(file_create(output_file_path), "failed to create output file \"%s\"", output_file_path);

        output_fd = _open_fd_for_read_write(output_file_path);
        assert(output_fd != INVALID_HANDLE, "failed to open file descriptor for output file \"%s\"", output_file_path);
    }

    ProcessID pid = proc_start(cmd, output_fd);

    if (opts.async_pid && (pid != INVALID_HANDLE)) {
        *opts.async_pid = pid;
        result = true;
    }
    else {
        if (proc_wait(pid) == 1) {
            result = true;

            if (opts.output_string) {
                usize bytes = (usize)lseek(output_fd, 0, SEEK_END);

                *opts.output_string = str_alloc_with_cap(bytes);

                if (bytes > 0) {
                    assert(lseek(output_fd, 0, SEEK_SET) == 0, "incorrect seek position");

                    isize bytes_read = (isize)read(output_fd, opts.output_string->data, bytes);
                    assert(bytes_read != -1, "failed to read from output file \"%s\": %s", output_file_path, strerror(errno));

                    opts.output_string->len = bytes_read;
                }
            }

            global_arena.used = sp;
        }
    }

    if (output_file_path) {
        file_delete(output_file_path);
    }

    if (output_fd != INVALID_HANDLE) {
        _close_fd(output_fd);
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

    // @jcg: starts with 1 to keep space for a null-terminator.
    usize capacity = 1;

    for (usize i = 0; i < cmd.count; ++i) {
        // + 3 for space character and (possible) quotes.
        capacity += cmd.items[i].cap + 3; 
    }

    result.data = alloc_bytes(capacity);
    result.cap = capacity;

    for (usize i = 0; i < cmd.count; ++i) {
        String* item = &cmd.items[i];

        if (item->len) {
            if (i != 0) {
                str_append_char(&result, ' ');
            }

            if (!str_find_first_char(*item, ' ', NULL)) {
                str_append_other(&result, *item);
            }
            else {
                str_append_char(&result, delim);
                str_append_other(&result, *item);
                str_append_char(&result, delim);
            }
        }
    }

    return result;
}

CBA_DEF char* cmd_flatten_to_cstr(Command cmd) {
    String result = cmd_flatten(cmd);
    return (char*)result.data;
}

CBA_DEF char* cmd_flatten_to_cstr_with_delims(Command cmd, char delim) {
    String result = cmd_flatten_with_delims(cmd, delim);
    return (char*)result.data;
}

#endif // CBA_IMPLEMENTATION
