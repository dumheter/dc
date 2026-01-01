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

#pragma once

#include <dc/list.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>
#include <dc/types.hpp>
#include <format>
#include <iterator>
#include <string_view>

// Specializations for std::formatter
template <>
struct std::formatter<dc::StringView> : std::formatter<std::string_view> {
  auto format(const dc::StringView& sv, format_context& ctx) const {
    return std::formatter<std::string_view>::format(
        std::string_view(sv.c_str(), sv.getSize()), ctx);
  }
};

template <>
struct std::formatter<dc::String> : std::formatter<std::string_view> {
  auto format(const dc::String& s, format_context& ctx) const {
    return std::formatter<std::string_view>::format(
        std::string_view(s.c_str(), s.getSize()), ctx);
  }
};

namespace dc {

struct FormatErr {
  enum class Kind {
    InvalidSpecification,
    CannotFormatType,
    CannotWriteToFile,
    OutOfMemory,
    ParseReturnedBadIterator,
  } kind = Kind::InvalidSpecification;  //< What kind of error.

  /// Where in the pattern did we encounter an error.
  s64 pos = 0;
};

const char8* toString(FormatErr::Kind kind);

///@param err
///@param pattern The pattern that was used when the error happened.
String toString(const FormatErr& err, StringView pattern);

enum class Presentation {
  Decimal,
  Binary,
  Hex,
};

/// Convert int to string.
Result<StringView, s64> toString(s64 value, char8* buf, s64 bufSize,
                                 Presentation p = Presentation::Decimal);
Result<StringView, s64> toString(u64 value, char8* buf, s64 bufSize,
                                 Presentation p = Presentation::Decimal);

// Helper for back inserter to dc::List
template <typename Container>
class BackInserter {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = void;
  using container_type = Container;

  explicit BackInserter(Container& c) : m_container(&c) {}

  BackInserter& operator=(char value) {
    m_container->add(value);
    return *this;
  }

  BackInserter& operator*() { return *this; }
  BackInserter& operator++() { return *this; }
  BackInserter& operator++(int) { return *this; }

 private:
  Container* m_container;
};

template <typename... Args>
Result<NoneType, FormatErr> formatTo(List<char8>& out, const StringView fmt,
                                     Args&&... args) {
  try {
    std::string_view sv(fmt.c_str(), fmt.getSize());
    std::vformat_to(BackInserter(out), sv, std::make_format_args(args...));
    out.add('\0');  // Maintain null termination convention of original
    return Ok(None);
  } catch (const std::format_error&) {
    return Err(FormatErr{FormatErr::Kind::InvalidSpecification, 0});
  } catch (const std::bad_alloc&) {
    return Err(FormatErr{FormatErr::Kind::OutOfMemory, 0});
  }
}

template <typename... Args>
Result<NoneType, FormatErr> formatTo(String& out, const StringView fmt,
                                     Args&&... args) {
  List<char8>& list = out.getInternalList();
  // If the current buffer of char8's is null terminated, remove it.
  if (!list.isEmpty() && *(list.end() - 1) == '\0') {
    list.remove(list.end() - 1);
  }
  return formatTo(list, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
Result<String, FormatErr> formatStrict(const StringView fmt, Args&&... args) {
  String out;
  auto res = formatTo(out, fmt, std::forward<Args>(args)...);
  if (res.isOk()) return Ok(dc::move(out));
  return Err(res.errValue());
}

template <typename... Args>
String format(const StringView fmt, Args&&... args) {
  String out;
  auto res = formatTo(out, fmt, std::forward<Args>(args)...);
  if (res.isErr()) {
    out = String("<failed to format data: ");
    out += fmt;
    out += ">";
  }
  return out;
}

namespace detail {
Result<NoneType, FormatErr> rawPrint(FILE* f, StringView str);
void printCallstack();
}  // namespace detail

template <typename... Args>
Result<NoneType, FormatErr> printTo(FILE* f, StringView str, Args&&... args) {
  auto res = formatStrict(str, std::forward<Args>(args)...);
  if (res.isOk()) return detail::rawPrint(f, res.value().toView());
  return Err(res.errValue());
}

template <typename... Args>
Result<NoneType, FormatErr> printE(StringView str, Args&&... args) {
  return printTo(stdout, str, std::forward<Args>(args)...);
}

template <typename... Args>
void print(StringView str, Args&&... args) {
  auto res = printTo(stdout, str, std::forward<Args>(args)...);
  if (res.isErr()) {
    auto errMsg = toString(res.errValue(), str);
    if (!errMsg.endsWith('\n')) errMsg += '\n';
    detail::rawPrint(stdout, errMsg.toView());
    detail::printCallstack();
  }
}

}  // namespace dc
