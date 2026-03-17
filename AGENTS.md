# Wren Development Guide

This is the Wren programming language - a small, fast, class-based concurrent scripting language implemented in C99/C++98.

## Build Commands

### Building the Project

```bash
# Build release 64-bit (default)
make -C projects/make

# Build debug 64-bit
make -C projects/make config=debug_64bit

# Build release 32-bit
make -C projects/make config=release_32bit

# Build with debug symbols
make -C projects/make config=debug_64bit

# Build without nan-tagging (for testing)
make -C projects/make config=release_64bit-no-nan-tagging
```

Available configurations: `release_64bit`, `release_32bit`, `release_64bit-no-nan-tagging`, `debug_64bit`, `debug_32bit`, `debug_64bit-no-nan-tagging`

### Running Tests

```bash
# Build the test binary first
make -C projects/make config=release_64bit wren_test

# Run all tests
python3 ./util/test.py

# Run tests with debug build
python3 ./util/test.py --suffix=_d

# Run a specific test suite
python3 ./util/test.py <suite_name>

# Available test suites (in test/):
# - api/       - C API tests
# - benchmark/ - Performance benchmarks
# - core/      - Core library tests
# - language/  - Language semantics tests
# - limit/     - Limit tests
# - meta/      - Meta programming tests
# - random/    - Random module tests
# - regression/ - Bug regression tests
# - unit/      - Unit tests
```

### Running a Single Test

```bash
# Build test binary first
make -C projects/make config=release_64bit wren_test

# Run a specific .wren test file directly
./bin/wren_test test/language/class/method.wren

# Or use the test runner for a specific file
python3 -c "
import subprocess
import sys
result = subprocess.run(['./bin/wren_test', 'test/language/class/method.wren'], 
                       capture_output=True, text=True)
print(result.stdout)
print(result.stderr)
sys.exit(result.returncode)
"
```

### Cleaning Builds

```bash
make -C projects/make clean
```

## Code Style Guidelines

### General Style

- **Language**: C99 compatible, C++98 compatible
- **Indentation**: 2 spaces (not tabs)
- **Line length**: Keep lines under 80-100 characters when practical
- **Braces**: Allman style (opening brace on new line)
  ```c
  if (condition)
  {
    // code
  }
  ```

### Naming Conventions

- **Functions**: `snake_case` (e.g., `wrenNewVM`, `wrenGetVersionNumber`)
- **Variables**: `snake_case` (e.g., `user_data`, `gray_count`)
- **Constants/Macros**: `SCREAMING_SNAKE_CASE` (e.g., `WREN_VERSION_NUMBER`, `MAX_LOCALS`)
- **Enums**: `SCREAMING_SNAKE_CASE` for values, prefix with type (e.g., `TOKEN_LEFT_PAREN`)
- **Types/Structs**: `snake_case_t` or `CamelCase` for public API types (e.g., `WrenVM`, `Obj`)
- **Files**: `snake_case.c` / `snake_case.h`

### Imports and Headers

- Group includes by: standard library, external, internal
- Use angle brackets for external, quotes for internal
- Order: stdlib, language-specific headers, project headers

```c
#include <stdarg.h>
#include <string.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
```

### Error Handling

- Use error functions like `wrenError()` that take format strings
- Compile errors include `[module line X] Error: message`
- Runtime errors include stack traces with file/line info
- Always check return values and handle NULL pointers

```c
if (ptr == NULL)
{
  wrenError(vm, "Failed to allocate memory\n");
  return NULL;
}
```

### Code Organization

- Source in `src/vm/` (core VM), `src/optional/` (optional modules)
- Headers in `src/include/` (public API), `src/vm/` (internal)
- Tests in `test/` directory, mirroring source structure
- Use forward declarations to avoid ordering requirements

### Documentation

- Comment complex logic and non-obvious behavior
- Use block comments for function documentation in headers
- TODO comments for known issues (e.g., `// TODO: Tune this.`)

### Wren Test Files

- Use `// expect: <output>` to verify stdout
- Use `// expect error` to verify compile errors
- Use `// expect error line N` for specific error line
- Use `// expect runtime error: <message>` for runtime errors
- Use `// skip: <reason>` to skip tests
- Use `// nontest` to mark non-test files

### Best Practices

1. Always initialize all struct fields explicitly
2. Use `memset()` to zero-initialize structures
3. Avoid bare magic numbers - use named constants
4. Check array bounds before indexing
5. Free resources in reverse order of allocation
6. Use `static` for internal functions
7. Use descriptive variable names, even for loop counters in complex code

## Project Structure

```
src/
  include/      # Public C API headers
  vm/           # Core VM implementation
  optional/     # Optional modules (meta, random)
test/
  api/          # C API tests
  benchmark/    # Performance benchmarks
  core/         # Core library tests
  language/     # Language tests
  limit/        # Limit tests
  regression/   # Bug regression tests
projects/       # Build system files
util/           # Build and test utilities
```

## Common Tasks

### Adding a New Core Method

1. Add method to class definition in `src/vm/wren_core.c`
2. Implement method in appropriate file (often `wren_primitive.c`)
3. Add tests in `test/core/`
4. Build and test: `make -C projects/make && python3 ./util/test.py`

### Adding a New Test

1. Create `.wren` file in appropriate test directory
2. Add expected output with `// expect:` comments
3. Run specific test: `./bin/wren_test test/path/to/test.wren`

### Debugging

- Enable debug tracing in header defines (e.g., `WREN_DEBUG_TRACE_MEMORY`)
- Use debug build: `make -C projects/make config=debug_64bit`
- Check test runner output for line numbers in errors

### Adding a New Foreign Class Test (Example: Vec4)

This example demonstrates how to create a foreign class test that connects C code with Wren.

#### 1. Create the C Header (test/api/vec4.h)

```c
#ifndef vec4_h
#define vec4_h

#include <stdbool.h>
#include <wren.h>

WrenForeignMethodFn vec4BindMethod(const char* signature);
void vec4BindClass(const char* className, WrenForeignClassMethods* methods);

#endif
```

#### 2. Create the C Implementation (test/api/vec4.c)

Key points:
- Use `wrenSetSlotNewForeign()` to allocate foreign object data
- Use `wrenGetSlotForeign()` to retrieve foreign object data
- For static methods returning new objects, read from slot 1+ (receiver is not in slot 0)
- For instance methods, receiver is in slot 0

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec4.h"

static void vec4Allocate(WrenVM* vm)
{
    double* v = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);

    if (wrenGetSlotCount(vm) == 1)  // Default constructor
    {
        v[0] = v[1] = v[2] = v[3] = 0;
    }
    else  // Constructor with parameters
    {
        v[0] = wrenGetSlotDouble(vm, 1);
        v[1] = wrenGetSlotDouble(vm, 2);
        v[2] = wrenGetSlotDouble(vm, 3);
        v[3] = wrenGetSlotDouble(vm, 4);
    }
}

static void vec4GetX(WrenVM* vm)
{
    double* v = (double*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, v[0]);
}

// Static factory method - reads from slot 1+ since there's no receiver
static void vec4Add(WrenVM* vm)
{
    double* a = (double*)wrenGetSlotForeign(vm, 1);
    double* b = (double*)wrenGetSlotForeign(vm, 2);

    double* result = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double) * 4);
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

WrenForeignMethodFn vec4BindMethod(const char* signature)
{
    if (strcmp(signature, "static Vec4.add(_,_)") == 0) return vec4Add;
    if (strcmp(signature, "Vec4.x") == 0) return vec4GetX;
    // ... more methods
    return NULL;
}

void vec4BindClass(const char* className, WrenForeignClassMethods* methods)
{
    if (strcmp(className, "Vec4") == 0)
    {
        methods->allocate = vec4Allocate;
    }
}
```

#### 3. Create the Wren Test (test/api/vec4.wren)

```wren
foreign class Vec4 {
  construct new(x, y, z, w) {}

  foreign x
  foreign toString
  
  foreign static add(a, b)
}

var t = Vec4.new(1, 2, 3, 4)
System.print(t.toString)   // expect: (1, 2, 3, 4)
System.print(t.x)          // expect: 1

var v = Vec4.add(t, t)
System.print(v.toString)   // expect: (2, 4, 6, 8)
```

#### 4. Register in api_tests.c

Add to `APITest_bindForeignMethod`:
```c
method = vec4BindMethod(fullName);
if (method != NULL) return method;
```

Add to `APITest_bindForeignClass`:
```c
vec4BindClass(className, &methods);
if (methods.allocate != NULL) return methods;
```

#### 5. Add to Build System

Edit `projects/make/wren_test.make` and add `vec4.o` to the OBJECTS list.

#### 6. Build and Test

```bash
make -C projects/make config=release_64bit wren_test
python3 ./util/test.py api/vec4
```

#### Important Notes

1. **Slot numbering**: For static methods, arguments start at slot 1. For instance methods, the receiver is at slot 0 and arguments start at slot 1.

2. **Returning new foreign objects**: Use `wrenSetSlotNewForeign(vm, 0, 0, size)` where the second parameter is the slot to write to, and the third is the class slot (0 = same class).

3. **Finalize functions**: Can cause issues with the test runner - test carefully.

4. **Method signature format**: `ClassName.methodName(argTypes)` - use `_` for each argument, `__` for two arguments, etc.
