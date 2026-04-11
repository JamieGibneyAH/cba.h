# `cba.h`

For when you CBA with CMake.

This is a [stb-style header-only library](https://github.com/nothings/stb)
designed for writing build recipes in C, and for useful C utilities.

The idea is mostly based on tsoding's [nob.h
library](https://github.com/tsoding/nob.h), but with many changes that I either
prefer or that I'm experimenting with. If you don't know about tsoding's
content, please do yourself a favour and check it out!

## Usage

The below file is built with any C compiler (e.g. `gcc -o cba cba.c`), and
then run to build a main program. The program will automatically rebuild itself
when modified, so it doesn't need to be recompiled manually.

```c
#define CBA_IMPLEMENTATION
#include "cba.h"

int main(int argc, char** argv) {
    CBA_BOOTSTRAP(argc, argv);

    assert(try_mkdir("build"), "failed to create build directory");

    Command cmd = {0};

    cmd_append(&cmd,
        CBA_C_COMPILER,
        CBA_COMPILER_DEBUG_FLAGS,
        CBA_COMPILER_COMMON_FLAGS,
        CBA_COMPILER_OUTPUT("build/main"),
        CBA_COMPILER_INPUTS("main.c"),
    );

    // with clang, the above creates:
    // "clang -Wall -Wextra -glldb -DDEBUG -o build/main main.c"
    // or with MSVC:
    // "cl.exe /W4 /nologo /ZI /DDEBUG /Fe:build/main.c main.c"

    cmd_run(cmd);

    return 0;
}
```

A more thorough example:

```c
#define CBA_IMPLEMENTATION
#include "cba.h"

int main(int argc, char** argv) {
    // Allow the program to rebuild itself when modified
    CBA_BOOTSTRAP(argc, argv);


    // Create a command for appending arguments to
    Command cmd = {0};

    // You can manually append arguments
    cmd_append(&cmd, "gcc", "-o", "main", "main.c");

    // Or you can use compiler-specific macros
    cmd_append(&cmd,
        // e.g. "gcc" or "cl.exe"
        CBA_C_COMPILER,
        // e.g. "-o main", or "/Fe:main"
        CBA_COMPILER_OUTPUT("main"),
        CBA_COMPILER_INPUTS("main.c")
    );


    // Execute the command, block until it terminates, and assert that it exits normally
    cmd_run(cmd);

    // Or without the assertion
    bool success = cmd_try_run(cmd);

    // Or without blocking...
    ProcessID pid = cmd_run(cmd, .async = true);

    // ..which you can later wait for
    proc_wait(pid);


    // You can reset commands to reuse them
    cmd_reset(&cmd);


    // And for convenience, you can also run whole commands in one call
    cmd_run_direct("gcc -o main main.c");


    return 0;
}
```

## Extras

`cba.h` also provides various utilties for C programs, including an arena
allocator, file operations, string operations, and more.

- See the `alloc`, `alloc_bytes`, and `alloc_array` macros for allocating via the arena.
- See `begin_temp_memory()` and `end_temp_memory()` for temporary arena allocations.
- See the `str_*` functions for string operations.
- See the `str_arr_*` functions for string arrays.
- See the `file_*` functions for file operations.
- See the `print`, `assert`, and `panic` macros for printing/assertions/panics.
- See the `info`, `warn`, and `error` macros for user-facing logs.

<!--
## API

### Files

```c
b32 file_move(const char* path, const char* new_path);
```

Moves (renames) a file to a new path.



```c
b32 file_copy(const char* path, const char* new_path, b32 symbolic_link);
```

Copies a file to a new path, optionally creating a symbolic link. On Windows,
trying to create a symbolic link will have no effect.



```c
b32 file_delete(const char* path);
```

Deletes a file. If the file is a directory, it will be removed recursively.



```c
b32 file_exists(const char* path);
```

Whether a file exists.



```c
FileType file_get_type(const char* path);
```

Returns the type of a file:
- `FILE_TYPE_UNKNOWN`: the file type could not be detected.
- `FILE_TYPE_FILE`: the file is a regular file.
- `FILE_TYPE_DIRECTORY`: the file is a directory.
- `FILE_TYPE_SYMLINK`: the file is a symbolic link.
- `FILE_TYPE_OTHER`: the file type was detected, but is not covered.



```c
b32 try_mkdir(const char* path);
```

Attempts to create a directory, returning `true` if successful or if the
directory already exists. If `path` describes nested directories which don't
exist, it will create all of them.



```c
i32 files_need_rebuild(String output_path, StringArray input_paths);
i32 file_need_rebuild(String output_path, String input_path);
```

Whether the input file(s) have been modified sooner than the output file.
Returns `1` if true, `0` otherwise, and `-1` in case of an error.

-->
