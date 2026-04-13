# `cba.h`

This is a [stb-style](https://github.com/nothings/stb) header-only library
designed for writing build recipes in C, and for useful C utilities. It's
largely based on tsoding's [nob.h library](https://github.com/tsoding/nob.h) —
a great alternative for the 'use C to build C' ("no-build") approach.

Grab `cba.h` with:

```sh
wget https://raw.githubusercontent.com/jamiegibney/cba.h/refs/heads/master/cba.h
```


## Usage

Add `#define CBA_IMPLEMENTATION` before including `cba.h` in ONE C/C++ file to
generate the implementation:

```c
#include ...
#include ...
#define CBA_IMPLEMENTATION
#include "cba.h"
```

And otherwise just `#include "cba.h` for its definitions.

### Simple example

```c
// File: cba.c

#define CBA_VERBOSE          // See internal logging.
#define CBA_PRINT_ON_REBUILD // Print a message when rebuilding.
#define CBA_IMPLEMENTATION   // Generate implementations.
#include "cba.h"

int main(int argc, char** argv) {
    // Allow the program to rebuild itself when modified.
    CBA_REBUILD(argc, argv);

    // Create a directory (also works recursively).
    assert(file_try_create_directory("build"), "failed to create build directory");

    // A command is an array of arguments which can be run via the shell.
    Command cmd = {0};

    // Use the CBA_COMPILER_* macros for compiler-specific flags.
    cmd_append(&cmd,
        CBA_COMPILER_C,
        CBA_COMPILER_DEBUG_FLAGS,
        CBA_COMPILER_COMMON_FLAGS,
        CBA_COMPILER_OUTPUT("build/artefacts/main"),
        CBA_COMPILER_INPUTS("main.c"),
    );

    // With GCC, the above forms:
    //   gcc -ggdb -DDEBUG -Wall -Wextra -o build/artefacts/main main.c
    //
    // And with MSVC:
    //   cl.exe /D_CRT_SECURE_NO_WARNINGS /DDEBUG /W4 /nologo /Fe:build/artefacts/main main.c

    // Run the command, block until it terminates, and assert that it exits normally.
    cmd_run(cmd);

    return 0;
}
```

Compile the above once with e.g.:
```sh
gcc -o cba cba.c
```

And when the source file is modified, the program will automatically rebuild itself when run.

> [!NOTE]
> Only MSVC versions 19.0 and above (i.e. Visual Studio 2026) are supported due
> to its compliance with certain C99 features.

## Other utilities

`cba.h` also provides various utilties for C programs, including an arena
allocator, file operations, string operations, and more.

- See the `alloc`, `alloc_bytes`, and `alloc_array` macros for allocating via the arena.
- See `begin_temp_memory()` and `end_temp_memory()` for temporary arena allocations.
- See the `str_*` functions for string operations.
- See the `str_arr_*` functions for string arrays.
- See the `file_*` functions for file operations.
- See the `print`, `assert`, and `panic` macros for printing/assertions/panics.
- See the `info`, `warn`, and `error` macros for user-facing logs.

## Todo

- [x] Flesh out Windows support
- [ ] API document (perhaps with a tool to extract doc comments)
- [ ] Ensure that either path separator is supported on all platforms?
- [ ] Dynamic array (and perhaps a #define for enabling dynamic allocation)
    - Have a look at the `da_append` implementation in `nob.h`
- [ ] Hash table/set
- [ ] Automatic `compile_commands.json` generation?
- [ ] Piping mechanism
- [ ] Optional function prefixes (perhaps `#define CBA_STRIP_PREFIXES`)
