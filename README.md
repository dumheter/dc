# (D)umheters (C)ollection
Collection of libraries and tools that I tend to use in projects.

## Features
What does DC include?
### dc/list
An array list class, same as std::vector. (Can have) an internal buffer to avoid allocations when used for a few objects.
### dc/string
A string class, uses `dc/list`, therefore has an internal buffer to avoid allocations for small content. 24 bytes internal buffer by default.
``` cpp
dc::String s;
s = "The quick brown fox"; // Will not allocate.
s += ", jumped over the fence."; // Can't fit in internal buffer, will allocate.
```
It also comes with a `StringView` and a `Utf8Iterator` to iterate each codepoint, instead of byte.
### dc/callstack
Builds a callstack from the frame you are at. Used by `dc/assert`.
```cpp
int main(int, char**) {
  const Result<Callstack, CallstackErr> result = buildCallstack();
  const String& callstack =
    result
    .match([](const Callstack& cs) { return cs.toString(); },
           [](const CallstackErr& err) { return err.toString(); });
  LOG_INFO("{}", callstack);

  return 0;
}
```
### dc/assert
Provides three functions. `dcAssert` will assert and print a message, but continue running. `dcFatalAssert` will assert, print a message, then end the program. Both will call the last function, `debugBraek`, which will trap any attached debugger.
### dc/traits
Provides SFINAE traits.
### dc/log
Log library. Will offload all the blocking code such as printing to stdout and writing to files will entail. The callee code will do minimal work, then put the log work on a queue. Then the log thread will do the actual heavy work.

``` cpp
// basic usage
#include <dc/log.hpp>
int main(int, char**) {
  dc::log::init();
  LOG_INFO("Hello from {}!", "DC");
  const bool logDeinitOk = dc::log::deinit();
  return !logDeinitOk;
}
```

```cpp
// Change default sink:
#include <dc/log.hpp>
int main(int, char**) {
  dc::log::init();
  dc::log::getGlobalLogger()
    .detachLogger("default")
    .attachLogger(ColoredConsoleLogger, "colored logger");
  LOG_INFO("Hello from {}!", "DC");
  const bool logDeinitOk = dc::log::deinit();
  return !logDeinitOk;
}
```

### dc/fmt
Format library.

### dc/result (and optional)
Result is a suprisingly pleasant way to handle functions that may return a result, or an error. Optional is similarly suprisingly pleasant to use.

### dc/time
Provides some time utilities. Such as a `Stopwatch`, and a `Timestamp`.

### dc/pointer_int_pair
Store data in the pointer, since x86 doesnt use the last few bits.

### dc/types
Provides more sane naming to the common types.

### dc/allocator
Allocator interface, used throughout DC.

### dc/math
A few math related functions.

### dc/dtest
A lightweight test library, uses dc/log.

## TODO
[ ] String with boyer-moore search algo  
[ ] Make a hash map  
[ ] Profiler, maybe new git repo.  
[ ] go c++ 20  
