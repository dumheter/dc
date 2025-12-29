# DC Coding Guidelines

## Build Commands

### Building the Project
```bash
# Configure build (from project root)
 cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDC_BUILD_TESTS=1 -DCMAKE_BUILD_TYPE=Debug -G "Ninja" -B build

# Build with address sanitizer
cmake -DDC_BUILD_TESTS=1 -DCMAKE_BUILD_TYPE=Debug -DC_SANITIZE_ADDRESS=ON -G "Ninja" -B build

# Build with memory sanitizer (Linux only)
cmake -DDC_BUILD_TESTS=1 -DCMAKE_BUILD_TYPE=Debug -DC_SANITIZE_MEMORY=ON -G "Ninja" -B build

# Build the library
cmake --build build
```

### Testing
```bash
# Run all tests
./build/tests/dc_test.exe
```

### Formatting & Linting
```bash
# Format all C++ files (excludes thirdparty)
./format

# Format specific file
clang-format -style=file -i path/to/file.cpp
```

The project uses clang-format with Google style (see `.clang-format`).

## Code Style Guidelines

### File Organization
- **Header files**: `include/dc/` directory
- **Source files**: `src/` directory
- **Test files**: `tests/` directory with `.test.cpp` suffix
- **License header**: MIT license at top of all files (23 lines)
- **Header guard**: Use `#pragma once` (not `#ifndef` guards)
- **Section comments**: Use `//` comments with `////` separators for major sections

### Imports & Includes
- Use `#pragma once` as first line after license header
- Standard library includes: `#include <cstring>`, `#include <cstdint>`, etc.
- Project includes: `#include <dc/assert.hpp>`, `#include <dc/types.hpp>`, etc.
- No circular dependencies between headers

### Type System
- Use custom types from `include/dc/types.hpp`:
  - Signed integers: `s8`, `s16`, `s32`, `s64`
  - Unsigned integers: `u8`, `u16`, `u32`, `u64`, `usize`
  - Floating point: `f32`, `f64`
  - Char: `char8` (not `char`)
  - Pointers: `uintptr`, `intptr`
- Never use built-in `int`, `unsigned`, `long`, `size_t` directly
- Use `std::uint64_t`, etc. only when absolutely necessary for STL compatibility

### Naming Conventions
- **Classes**: PascalCase (e.g., `StringView`, `Utf8Iterator`, `Stopwatch`)
- **Functions**: camelCase (e.g., `getSize()`, `isEmpty()`, `buildCallstack()`)
- **Variables**: camelCase (e.g., `m_string`, `m_size`, `localVar`)
- **Member variables**: Prefix with `m_` (e.g., `m_string`, `m_size`, `m_offset`)
- **Constants**: Prefix with `k` (e.g., `kLoremIpsum`, `kCachelineMinusListBytes`)
- **Macros**: ALL_CAPS with `DC_` prefix (e.g., `DC_ASSERT`, `DC_INLINE`, `DC_NOINLINE`)
- **Namespaces**: lowercase (e.g., `dc`, `dc::details`, `dc::utf8`)
- **Test cases**: camelCase (e.g., `stringViewRunTime`, `utf8IteratorEndComparison`)

### Formatting (Google Style)
- Indentation: 2 spaces
- Braces: K&R style (opening brace on same line)
- Pointer/reference style: `const char8* str` (space after type, before name)
- Line width: No strict limit, but prefer readability
- Trailing whitespace: Not allowed

### Attributes
- Use `[[nodiscard]]` on functions and classes that return important values
- Use `[[maybe_unused]]` on intentionally unused parameters
- Use `noexcept` on move constructors/assignment operators
- Use `const` when not mutating the variable. Always be const correct.

### Error Handling
- **Result types**: Use `Result<T, E>` for functions that may fail
- **Option types**: Use `Option<T>` for nullable values
- **Assertions**: 
  - `DC_ASSERT(condition, msg)` - Assert and log, continue execution
  - `DC_FATAL_ASSERT(condition, msg)` - Assert, log, then terminate
- **No exceptions**: This codebase does not use C++ exceptions
- Always check Result/Option return values before accessing values
- Use `.isSome()`, `.isNone()`, `.isOk()`, `.isErr()` for checking

### Macros
Use the provided macros from `dc/macros.hpp`:
- `DC_DELETE_COPY(ClassName)` - Delete copy constructor and assignment
- `DC_DEFAULT_COPY(ClassName)` - Default copy constructor and assignment
- `DC_DELETE_MOVE(ClassName)` - Delete move constructor and assignment
- `DC_DEFAULT_MOVE(ClassName)` - Default move constructor and assignment
- `DC_UNUSED(v)` - Mark variable as unused (silences warnings)
- `DC_FILENAME` - Strips path from `__FILE__`, returns just filename

### Compiler Compatibility
The codebase supports MSVC, Clang, and GCC. Use the provided macros:
- `DC_COMPILER_MSVC`, `DC_COMPILER_CLANG`, `DC_COMPILER_GCC`
- `DC_INLINE` - Force inline
- `DC_NOINLINE` - Prevent inlining
- `DC_DISABLE_OPTIMIZATIONS` - Disable optimizations for a block

### Memory & Performance
- Pass large objects by `const&` or by value for move semantics
- Use `dc::move()` (not `std::move`) for move semantics
- Prefer move semantics over copying when possible
- Use the allocator interface (`IAllocator`) for memory management
- Zero-cost abstractions are a priority

### Testing Guidelines
- Test file naming: `<module>.test.cpp` (e.g., `string.test.cpp`)
- Test function naming: `DTEST(testName)` macro
- Use assertions: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, etc.
- Setup: Include `<dc/dtest.hpp>` at top of test files
- Main test entry point: `main()` function calls `DTEST_RUN()`

### Comments & Documentation
- Use Doxygen-style comments for public APIs (`///` or `/** */`)
- Parameter documentation: `@param paramName Description`
- Return documentation: `@return Description`
- Keep comments concise and focused on "why", not "what"
- No TODO comments in production code (or mark them with your name)

### Special Patterns
- **Iterators**: Implement `operator*`, `operator++`, `operator--`, `begin()`, `end()`
- **String handling**: Use `dc::String` for mutable strings, `dc::StringView` for immutable views
- **UTF-8**: Use `Utf8Iterator` for codepoint iteration, not byte iteration
- **Logging**: Use `LOG_INFO`, `LOG_DEBUG`, `LOG_ERROR`, etc. from `dc/log.hpp`
- **Formatting**: Use `dc::format()` for string formatting
- **auto**: Use auto only when the type is written on the right side, as in `const auto testContent = dc::String("Hello, World!");`. Never repeat the type on both left and right side.

### Platform Specifics
- Windows: Use `#ifdef WIN32` for platform-specific code
- Linker options: `-rdynamic` on non-Windows for better callstack support
- Path handling: Use `\\` on Windows, `/` on Unix (see `DC_FILENAME` macro)

## Quick Reference

```cpp
// Header template
/**
 * MIT License
 * [full license text here]
 */

#pragma once

#include <dc/types.hpp>
#include <dc/allocator.hpp>

namespace dc {

class Example {
 public:
  Example() = default;
  
  [[nodiscard]] usize getSize() const { return m_size; }
  
 private:
  usize m_size = 0;
};

}  // namespace dc

// Source template
#include <dc/example.hpp>

namespace dc {

usize Example::getSize() const {
  return m_size;
}

}  // namespace dc
```
