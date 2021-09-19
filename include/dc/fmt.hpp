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

namespace dc::xfmt {

struct ParseContext {
  StringView pattern;
};

struct FormatContext {
  List<char8>& out;
};

///////////////////////////////////////////////////////////////////////////////

enum class FormatErr {
  ParseInvalidChar,
  CannotFormatType,
};

const char8* toString(FormatErr err);

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Formatter {
  Result<const char8*, FormatErr> parse(ParseContext& ctx) {
    (void)ctx;
    static_assert(false, "You need to specialize Formatter for type 'T'.");

    return Err(FormatErr::CannotFormatType);
  }

  void format(const T& item, FormatContext& ctx) {
    (void)item;
    (void)ctx;
    static_assert(false, "You need to specialize Formatter for type 'T'.");
  }
};

///////////////////////////////////////////////////////////////////////////////

Result<u32, FormatErr> parseInteger(const char8*& it, const char8* end);

///////////////////////////////////////////////////////////////////////////////

template <>
struct Formatter<String> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx) {
    auto it = ctx.pattern.beginChar8();
    for (; it != ctx.pattern.endChar8(); ++it) {
      if (*it == '}')
        break;
      else if (*it == '.') {
        // TODO cgustafsson: constexpr
        Result<u32, FormatErr> res = parseInteger(++it, ctx.pattern.endChar8());
        if (res.isOk())
          precision = res.value();
        else  // TODO cgustafsson: trivial type should be copyable (lvalue)
          return Err(dc::move(res).unwrapErr());

        --it;  // revert the increase we did
      } else
        return Err(FormatErr::ParseInvalidChar);
    }

    // TODO cgustafsson: dont move primitive types
    return Ok(dc::move(it));
  }

  void format(const String& str, FormatContext& ctx) {
    u64 len = dc::min(str.getSize(), precision);
    ctx.out.addRange(str.c_str(), str.c_str() + len);
  }

  u64 precision = ~0llu;
};

///////////////////////////////////////////////////////////////////////////////

// template <typename ContextT>
// struct ArgMapper {};

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
    /*s8,
    u8,
    s16,
    u16,*/
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
    StringType,
    PointerType,

    CustomType,
  };

  union {
    /*		s8 s8Value;
    u8 u8Value;
    s16 s16Value;
    u16 u16Value;*/
    s32 s32Value;
    s64 s64Value;
    u32 u32Value;
    u64 u64Value;

    f32 f32Value;
    f64 f64Value;

    bool boolValue;

    // TODO cgustafsson: add char8 type?

    const char* cstringValue;
    StringView stringValue;
    const void* pointerValue;

    CustomValue customValue;
  };

  constexpr FormatArg() : boolValue(false), type(Types::NoneType) {}
  constexpr FormatArg(s32 value) : s32Value(value), type(Types::S32Type) {}
  constexpr FormatArg(s64 value) : s64Value(value), type(Types::S64Type) {}
  constexpr FormatArg(u32 value) : u32Value(value), type(Types::U32Type) {}
  constexpr FormatArg(u64 value) : u64Value(value), type(Types::U64Type) {}
  constexpr FormatArg(f32 value) : f32Value(value), type(Types::F32Type) {}
  constexpr FormatArg(f64 value) : f64Value(value), type(Types::F64Type) {}
  constexpr FormatArg(bool value) : boolValue(value), type(Types::BoolType) {}
  constexpr FormatArg(const char* value)
      : cstringValue(value), type(Types::CStringType) {}
  constexpr FormatArg(StringView value)
      : stringValue(value), type(Types::StringType) {}
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
    if (res.isOk()) f.format(*static_cast<const T*>(value), formatCtx);
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

  // we ignore utf8 and just scan in ascii
  const char8* it = fmt.beginChar8();
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
        ParseContext parseCtx{StringView(it + 1, end - it)};
        FormatContext formatCtx{out};
        auto res = doFormatArg(parseCtx, formatCtx, formatArg);
        if (res.isOk())
          it = res.value();
        else  // TODO cgustafsson: dont move primitive types
          return Err(dc::move(res).unwrapErr());
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

template <typename... Args>
Result<NoneType, FormatErr> print(StringView str, Args&&... args) {
  (void)str;
  (void)sizeof...(args);
  // ...
}

}  // namespace dc::xfmt
