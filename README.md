# `cba.h`

For when you CBA with CMake.

This is a [stb-style header-only library](https://github.com/nothings/stb)
designed for writing build recipes in C, and for useful C utilities.

The idea is chiefly based on tsoding's [nob.h
library](https://github.com/tsoding/nob.h), but with a variety changes that I
either prefer or which I'm experimenting with.

Grab only the header with:

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

    // Recursively create a directory.
    assert(try_mkdir("build/artefacts"), "failed to create build directory");

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
    //   cl.exe /ZI /DDEBUG /W4 /nologo /Fe:build/artefacts/main main.c

    // Run the command, block until it terminates, and assert that it exits normally.
    cmd_run(cmd);

    return 0;
}
```

Compile the above once with, e.g.:
```sh
gcc -o cba cba.c
```

And when the source file is modified, the program will automatically rebuild itself when run.

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

- [ ] Flesh out Windows support
- [ ] Ensure that both path separators are supported
- [ ] Dynamic array (and perhaps a #define for enabling dynamic allocation)
- [ ] Hash table/set
- [ ] Piping mechanism
- [ ] Optional function prefixes (perhaps with `#define CBA_STRIP_PREFIXES`)
