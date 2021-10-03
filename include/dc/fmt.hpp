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

#include <cstdio>  // for FILE
#include <dc/list.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>
#include <dc/types.hpp>
#include <dc/utf.hpp>

namespace dc {

///////////////////////////////////////////////////////////////////////////////
// Format Util
//

struct ParseContext {
  StringView pattern;
};

struct FormatContext {
  List<char8>& out;
  StringView pattern;
};

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
///
/// Example
///    100 -> "100"
///   -100 -> "-100"
///
/// @return View of the resulting string on Ok. Required bytes to format the int
/// on Err.
Result<StringView, s64> toString(s64 value, char8* buf, s64 bufSize,
                                 Presentation);
Result<StringView, s64> toString(u64 value, char8* buf, s64 bufSize,
                                 Presentation);

/// Specialize this struct and its two functions to format your type!
///
/// See Formatter<String> for an example of how to use existing formatters
/// for another type.
template <typename T>
struct Formatter {
  Result<const char8*, FormatErr> parse(ParseContext&) {
    static_assert(false, "You need to specialize Formatter for type 'T'.");
    return Err(FormatErr{FormatErr::Kind::CannotFormatType, 0});
  }

  Result<NoneType, FormatErr> format(const T&, FormatContext&) {
    static_assert(false, "You need to specialize Formatter for type 'T'.");
    return Err(FormatErr{FormatErr::Kind::CannotFormatType, 0});
  }
};

/// Example
///   "123" -> 123
///   "123}abc" -> 123
///   "abc123}" -> Err
/// @param it Pointer to start of string, is also output param, will point to
/// after the end of the number.
/// @param end, will stop at end. Will not read end.
Result<u32, FormatErr> parseInteger(const char8*& it, const char8* end);

/// Example:
/// Fill 7, <03 -> 007
/// Fill wow, ^-7 -> --wow--
struct FormatFill {
  /// If you Formatter::parse should handle "fill", then just pass this when
  /// you iterate on the format specification. And it will only start "activate"
  /// when it sees the '<', '>' or '^' characters.
  /// @retval Some(Ok) when it parsed a "fill" successfully.
  /// @retval None() when it didnt wasnt a "fill".
  /// @retval Some(Err) when it was a "fill" but failed to parse.
  static Option<Result<FormatFill, FormatErr>> parse(const char8*& it,
                                                     const char8* end);

  static Result<NoneType, FormatErr> format(const Option<FormatFill>& fill,
                                            StringView str, FormatContext& ctx);

  enum class Align {
    Left,
    Right,
    Center,
  } align;
  utf8::CodePoint sign;
  u32 space;
};

/// Specialization for Formatter for StringView. Can be used for other
/// string-like types. Look at Formatter<String> for a good example.
template <>
struct Formatter<StringView> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx);

  Result<NoneType, FormatErr> format(const StringView& str, FormatContext& ctx);

  u64 precision = ~0llu;
};

/// Specialize Formatter for String. Inherit from StringView to reuse its
/// formatter.
template <>
struct Formatter<String> : Formatter<StringView> {
  Result<NoneType, FormatErr> format(const String& str, FormatContext& ctx) {
    return Formatter<StringView>::format(str.toView(), ctx);
  }
};

template <>
struct Formatter<const char8*> : Formatter<StringView> {
  Result<NoneType, FormatErr> format(const char8* str, FormatContext& ctx) {
    return Formatter<StringView>::format(StringView{str}, ctx);
  }
};

template <>
struct Formatter<u64> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx);

  Result<NoneType, FormatErr> format(u64 value, FormatContext& ctx);

  Result<char8, FormatErr::Kind> getPresentationChar() const;

  Presentation presentation = Presentation::Decimal;

  /// On hex, prefix with '0x', on binary, prefix with '0b', error with other
  /// presentation types.
  bool prefix = false;

  /// Write '-' in front of the number.
  bool negative = false;

  Option<FormatFill> fill;
};

template <>
struct Formatter<s64> : Formatter<u64> {
  Result<NoneType, FormatErr> format(s64 value, FormatContext& ctx);
};

/// Specialize the formatter for a type that can be cast to another.
#define DC_FORMAT_AS(Type, Base)                                     \
  template <>                                                        \
  struct Formatter<Type> : Formatter<Base> {                         \
    Result<NoneType, FormatErr> format(const Type& value,            \
                                       FormatContext& ctx) {         \
      return Formatter<Base>::format(static_cast<Base>(value), ctx); \
    }                                                                \
  };

DC_FORMAT_AS(u8, u64);
DC_FORMAT_AS(s8, s64);
DC_FORMAT_AS(u16, u64);
DC_FORMAT_AS(s16, s64);
DC_FORMAT_AS(u32, u64);
DC_FORMAT_AS(s32, s64);

///////////////////////////////////////////////////////////////////////////////

struct CustomValue {
  const void* value;
  Result<const char8*, FormatErr> (*format)(const void* value,
                                            ParseContext& parseCtx,
                                            FormatContext& formatCtx);
};

///////////////////////////////////////////////////////////////////////////////

struct FormatArg {
  enum class Types {
    NoneType,
    S32Type,
    S64Type,
    LastSignedIntegerType,
    U32Type,
    U64Type,
    LastIntegerType,

    F32Type,
    F64Type,
    LastFloatType,

    BoolType,

    CStringType,
    StringViewType,
    PointerType,

    CustomType,
  };

  union {
    s32 s32Value;
    s64 s64Value;
    u32 u32Value;
    u64 u64Value;

    f32 f32Value;
    f64 f64Value;

    bool boolValue;

    // TODO cgustafsson: add char8 type?

    const char8* cstringValue;
    StringView stringViewValue;
    const void* pointerValue;

    CustomValue customValue;
  };

  constexpr FormatArg() : boolValue(false), type(Types::NoneType) {}
  constexpr FormatArg(s8 value) : s32Value(value), type(Types::S32Type) {}
  constexpr FormatArg(s16 value) : s32Value(value), type(Types::S32Type) {}
  constexpr FormatArg(s32 value) : s32Value(value), type(Types::S32Type) {}
  constexpr FormatArg(s64 value) : s64Value(value), type(Types::S64Type) {}
  constexpr FormatArg(u8 value) : u32Value(value), type(Types::U32Type) {}
  constexpr FormatArg(u16 value) : u32Value(value), type(Types::U32Type) {}
  constexpr FormatArg(u32 value) : u32Value(value), type(Types::U32Type) {}
  constexpr FormatArg(u64 value) : u64Value(value), type(Types::U64Type) {}

  // TODO cgustafsson: why is not unsigned long same as u64?
  constexpr FormatArg(unsigned long value)
      : u64Value(value), type(Types::U64Type) {}
  constexpr FormatArg(f32 value) : f32Value(value), type(Types::F32Type) {}
  constexpr FormatArg(f64 value) : f64Value(value), type(Types::F64Type) {}
  constexpr FormatArg(bool value) : boolValue(value), type(Types::BoolType) {}
  constexpr FormatArg(const char8* value)
      : cstringValue(value), type(Types::CStringType) {}
  constexpr FormatArg(char8* value)
      : cstringValue(value), type(Types::CStringType) {}
  constexpr FormatArg(StringView value)
      : stringViewValue(value), type(Types::StringViewType) {}
  constexpr FormatArg(const void* value)
      : pointerValue(value), type(Types::PointerType) {}
  template <typename T>
  constexpr FormatArg(const T& value) : type(Types::CustomType) {
    customValue.value = &value;
    // TODO cgustafsson: fallback formatter
    // custom.format = format_custom_arg<
    // T, conditional_t<has_formatter<T, Context>::value,
    // 		typename Context::template formatter_type<T>,
    // 		fallback_formatter<T, char_type>>>;
    customValue.format = formatCustomArg<T>;
  }

  Types type;

 private:
  template <typename T>
  static Result<const char8*, FormatErr> formatCustomArg(
      const void* value, ParseContext& parseCtx, FormatContext& formatCtx) {
    Formatter<T> f;
    Result<const char8*, FormatErr> res = f.parse(parseCtx);
    if (res.isOk()) {
      Result<NoneType, FormatErr> formatRes =
          f.format(*static_cast<const T*>(value), formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  }
};

///////////////////////////////////////////////////////////////////////////////

template <typename... Args>
struct FormatArgs {
  FormatArgs(const Args&... args) { set(0, args...); }

  void set(u32 index) { args[index] = FormatArg(); }

  template <typename T>
  void set(u32 index, const T& arg) {
    args[index] = FormatArg(arg);
  }

  template <typename T, typename... U>
  void set(u32 index, const T& arg, const U&... otherArgs) {
    args[index] = FormatArg(arg);
    set(index + 1, otherArgs...);
  }

  static constexpr auto kArgCount = sizeof...(Args);
  FormatArg args[kArgCount == 0 ? 1 : kArgCount];
};

///////////////////////////////////////////////////////////////////////////////

Result<const char8*, FormatErr> doFormatArg(ParseContext& parseCtx,
                                            FormatContext& formatCtx,
                                            FormatArg& formatArg);

///////////////////////////////////////////////////////////////////////////////
// formatTo
//

template <typename... Args>
Result<NoneType, FormatErr> formatTo(String& out, const StringView fmt,
                                     Args&&... args) {
  return formatTo(out.getInternalList(), fmt, args...);
}

template <typename... Args>
Result<NoneType, FormatErr> formatTo(List<char8>& out, const StringView fmt,
                                     Args&&... args) {
  FormatArgs formatArgs(args...);

  constexpr u32 kArgCount = FormatArgs<Args...>::kArgCount;

  // If the current buffer of char8's is null terminated, remove it.
  if (!out.isEmpty() && *(out.end() - 1) == '\0') out.remove(out.end() - 1);

  // We ignore utf8 and just scan in ascii, since fmt standard format specifiers
  // are all valid ascii characters.
  const char8* begin = fmt.beginChar8();
  const char8* it = begin;
  const char8* end = fmt.endChar8();
  const char8* a = it;
  const char8* b = it;
  s32 usedArgs = 0;

  // TODO cgustafsson: use hasless trickry to scan 4 bytes at time
  while (it != end) {
    if (*it == '{') {
      // peak next that we don't escape it
      if (it + 1 != end && *(it + 1) == '{') {
        ++it;  // together with loopend, we go forward 2 steps

        // "hello {{world}}!"
        //  ^     ^^
        //  a     b it
        out.addRange(a, it);
        a = it + 1;
      } else {
        // "hello world {xx}"
        //  ^           ^^
        //  a          b parseCtx
        out.addRange(a, b);
        FormatArg& formatArg = formatArgs.args[usedArgs++];
        StringView pattern(it + 1, end - (it + 1));
        ParseContext parseCtx{pattern};
        FormatContext formatCtx{out, pattern};
        auto res = doFormatArg(parseCtx, formatCtx, formatArg);
        if (res.isOk()) {
          // did doFormatArg null terminate the string before we are done?
          if (!out.isEmpty() && *(out.end() - 1) == '\0')
            out.remove(out.end() - 1);
          it = res.value();
        } else {
          res.errValue().pos += it + 1 - fmt.beginChar8();
          return Err(dc::move(res).unwrapErr());
        }

        // protect from the format returning invalid iterator
        // TODO cgustafsson: dont silently fix errors
        // if (it >= end) --it;
        if (it >= end)
          return Err(
              FormatErr{FormatErr::Kind::ParseReturnedBadIterator, it - begin});
        else if (it < begin)
          return Err(
              FormatErr{FormatErr::Kind::ParseReturnedBadIterator, it - begin});

        a = it + 1;
      }
    } else if (*it == '}') {
      if (it + 1 != end && *(it + 1) == '}') {
        ++it;  // together with loopend, we go forward 2 steps

        // "hello {{world}}!"
        //          ^    ^^
        //          a    b it
        out.addRange(a, it);
        a = it + 1;
      }
    }
    b = ++it;
  }

  // add last part of the pattern
  if (a != b) out.addRange(a, b);

  DC_ASSERT(kArgCount == usedArgs,
            "Missmatch in amount of arguments provided vs expected when "
            "formatting string.");

  // add null termination at end
  out.add(0);

  return Ok(NoneType());
}

///////////////////////////////////////////////////////////////////////////////
// format
//

template <typename... Args>
Result<String, FormatErr> format(const StringView fmt, Args&&... args) {
  String out;
  auto res = formatTo(out, fmt, args...);
  if (res.isOk()) return Ok(dc::move(out));
  return Err(dc::move(res).unwrapErr());
}

///////////////////////////////////////////////////////////////////////////////
// print
//

namespace detail {
void printCallstack();
}

/// Print.
/// On error, returns Err.
template <typename... Args>
Result<NoneType, FormatErr> printE(StringView str, Args&&... args) {
  return printTo(stdout, str, dc::forward<Args>(args)...);
}

/// Print.
/// On error, print the Err instead of the requested message.
template <typename... Args>
void print(StringView str, Args&&... args) {
  auto res = printTo(stdout, str, dc::forward<Args>(args)...);
  if (res.isErr()) {
    // If we fail this print, we don't care, user didnt handle error themself.
    auto errMsg = toString(res.errValue(), str);
    if (!errMsg.endsWith('\n')) errMsg += '\n';
    rawPrint(stdout, errMsg.toView());
    // TODO cgustafsson: would make more sense to print callstack in reverse
    // order
    detail::printCallstack();
  }
}

Result<NoneType, FormatErr> rawPrint(FILE* f, StringView str);

template <typename... Args>
Result<NoneType, FormatErr> printTo(FILE* f, StringView str, Args&&... args) {
  auto res = format(str, dc::forward<Args>(args)...);
  if (res.isOk()) return rawPrint(f, res.value().toView());
  return Err(dc::move(res).unwrapErr());
}

}  // namespace dc
