/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <dc/callstack.hpp>
#include <dc/fmt.hpp>
#include <format>

namespace dc {

const char8* toString(FormatErr::Kind kind) {
  switch (kind) {
    case FormatErr::Kind::InvalidSpecification:
      return "Parsed invalid format specification.";
    case FormatErr::Kind::CannotFormatType:
      return "Cannot format type. Specialize the struct Formatter<T> for your "
             "type.";
    case FormatErr::Kind::CannotWriteToFile:
      return "CannotWriteToFile.";
    case FormatErr::Kind::OutOfMemory:
      return "Supplied buffer too small, or memory allocation failed.";
    case FormatErr::Kind::ParseReturnedBadIterator:
      return "Parse returned bad iterator, past end or before begin.";
    default:
      return "Internal error.";
  }
}

String toString(const FormatErr& err, StringView pattern) {
  return format("Format error: \"{}\" at pos {}\nPattern: {}",
                toString(err.kind), err.pos, pattern);
}

template <typename T>
Result<StringView, s64> toStringImpl(T value, char8* buf, s64 bufSize,
                                     Presentation p) {
  std::format_to_n_result<char*> res;
  switch (p) {
    case Presentation::Decimal:
      res =
          std::format_to_n(reinterpret_cast<char*>(buf), bufSize, "{}", value);
      break;
    case Presentation::Hex:
      res = std::format_to_n(reinterpret_cast<char*>(buf), bufSize, "{:x}",
                             value);
      break;
    case Presentation::Binary:
      res = std::format_to_n(reinterpret_cast<char*>(buf), bufSize, "{:b}",
                             value);
      break;
  }

  if (static_cast<s64>(res.size) > bufSize) {
    return Err(static_cast<s64>(res.size));
  }
  return Ok(StringView(reinterpret_cast<const char8*>(buf),
                       static_cast<u64>(res.size)));
}

Result<StringView, s64> toString(s64 value, char8* buf, s64 bufSize,
                                 Presentation p) {
  return toStringImpl(value, buf, bufSize, p);
}

Result<StringView, s64> toString(u64 value, char8* buf, s64 bufSize,
                                 Presentation p) {
  return toStringImpl(value, buf, bufSize, p);
}

namespace detail {

Result<NoneType, FormatErr> rawPrint(FILE* f, StringView str) {
  if (std::fwrite(str.c_str(), 1, str.getSize(), f) != str.getSize()) {
    return Err(FormatErr{FormatErr::Kind::CannotWriteToFile, 0});
  }
  return Ok(None);
}

void printCallstack() {
  auto res = buildCallstack();
  if (res.isOk()) {
    detail::rawPrint(stdout, "Callstack:\n");
    detail::rawPrint(stdout, res->callstack.toView());
    detail::rawPrint(stdout, "\n");
  }
}

}  // namespace detail

}  // namespace dc
