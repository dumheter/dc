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

#include <cfloat>
#include <cstring>
#include <dc/assert.hpp>
#include <dc/callstack.hpp>
#include <dc/fmt.hpp>

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
  String out;

  Result<NoneType, FormatErr> res = Err(FormatErr{});

  if (!pattern.isEmpty())
    res = formatTo(out,
                   "Format error: \"{}\"\n"
                   "  Location: character {} (zero-indexed)\n"
                   "  Pattern:   {}\n"
                   "            ",
                   toString(err.kind), static_cast<u64>(err.pos), pattern);
  else
    res = formatTo(out, "Format error: \"{}\"", toString(err.kind));

  if (res.isErr()) {
    return String(toString(err.kind));
  }

  constexpr s64 kMaxDrawPos = 256;
  if (err.pos != 0 && !pattern.isEmpty() && err.pos < kMaxDrawPos) {
    for (s64 i = 0; i < err.pos; ++i) out += ' ';
    out += '^';
    out += "\n\n";
  } else {
    out += "\n\n";
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////
// stb_sprintf
//

#define STBSP__SPECIAL 0x7000

static struct {
  short temp;  // force next field to be 2-byte aligned
  char pair[201];
} stbsp__digitpair = {0,
                      "00010203040506070809101112131415161718192021222324"
                      "25262728293031323334353637383940414243444546474849"
                      "50515253545556575859606162636465666768697071727374"
                      "75767778798081828384858687888990919293949596979899"};

// copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
#define STBSP__COPYFP(dest, src)                                       \
  {                                                                    \
    int cn;                                                            \
    for (cn = 0; cn < 8; cn++) ((char*)&dest)[cn] = ((char*)&src)[cn]; \
  }

static double const stbsp__bot[23] = {
    1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007,
    1e+008, 1e+009, 1e+010, 1e+011, 1e+012, 1e+013, 1e+014, 1e+015,
    1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022};
static double const stbsp__negbot[22] = {
    1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008,
    1e-009, 1e-010, 1e-011, 1e-012, 1e-013, 1e-014, 1e-015, 1e-016,
    1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022};
static double const stbsp__negboterr[22] = {
    -5.551115123125783e-018,  -2.0816681711721684e-019,
    -2.0816681711721686e-020, -4.7921736023859299e-021,
    -8.1803053914031305e-022, 4.5251888174113741e-023,
    4.5251888174113739e-024,  -2.0922560830128471e-025,
    -6.2281591457779853e-026, -3.6432197315497743e-027,
    6.0503030718060191e-028,  2.0113352370744385e-029,
    -3.0373745563400371e-030, 1.1806906454401013e-032,
    -7.7705399876661076e-032, 2.0902213275965398e-033,
    -7.1542424054621921e-034, -7.1542424054621926e-035,
    2.4754073164739869e-036,  5.4846728545790429e-037,
    9.2462547772103625e-038,  -4.8596774326570872e-039};
static double const stbsp__top[13] = {1e+023, 1e+046, 1e+069, 1e+092, 1e+115,
                                      1e+138, 1e+161, 1e+184, 1e+207, 1e+230,
                                      1e+253, 1e+276, 1e+299};
static double const stbsp__negtop[13] = {1e-023, 1e-046, 1e-069, 1e-092, 1e-115,
                                         1e-138, 1e-161, 1e-184, 1e-207, 1e-230,
                                         1e-253, 1e-276, 1e-299};
static double const stbsp__toperr[13] = {8388608,
                                         6.8601809640529717e+028,
                                         -7.253143638152921e+052,
                                         -4.3377296974619174e+075,
                                         -1.5559416129466825e+098,
                                         -3.2841562489204913e+121,
                                         -3.7745893248228135e+144,
                                         -1.7356668416969134e+167,
                                         -3.8893577551088374e+190,
                                         -9.9566444326005119e+213,
                                         6.3641293062232429e+236,
                                         -5.2069140800249813e+259,
                                         -5.2504760255204387e+282};
static double const stbsp__negtoperr[13] = {
    3.9565301985100693e-040,  -2.299904345391321e-063,
    3.6506201437945798e-086,  1.1875228833981544e-109,
    -5.0644902316928607e-132, -6.7156837247865426e-155,
    -2.812077463003139e-178,  -5.7778912386589953e-201,
    7.4997100559334532e-224,  -4.6439668915134491e-247,
    -6.3691100762962136e-270, -9.436808465446358e-293,
    8.0970921678014997e-317};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static u64 const stbsp__powten[20] = {1,
                                      10,
                                      100,
                                      1000,
                                      10000,
                                      100000,
                                      1000000,
                                      10000000,
                                      100000000,
                                      1000000000,
                                      10000000000,
                                      100000000000,
                                      1000000000000,
                                      10000000000000,
                                      100000000000000,
                                      1000000000000000,
                                      10000000000000000,
                                      100000000000000000,
                                      1000000000000000000,
                                      10000000000000000000U};
#define stbsp__tento19th ((u64)1000000000000000000)
#else
static u64 const stbsp__powten[20] = {1,
                                      10,
                                      100,
                                      1000,
                                      10000,
                                      100000,
                                      1000000,
                                      10000000,
                                      100000000,
                                      1000000000,
                                      10000000000ULL,
                                      100000000000ULL,
                                      1000000000000ULL,
                                      10000000000000ULL,
                                      100000000000000ULL,
                                      1000000000000000ULL,
                                      10000000000000000ULL,
                                      100000000000000000ULL,
                                      1000000000000000000ULL,
                                      10000000000000000000ULL};
#define stbsp__tento19th (1000000000000000000ULL)
#endif

#define stbsp__ddmulthi(oh, ol, xh, yh)                          \
  {                                                              \
    double ahi = 0, alo, bhi = 0, blo;                           \
    s64 bt;                                                      \
    oh = xh * yh;                                                \
    STBSP__COPYFP(bt, xh);                                       \
    bt &= ((~(u64)0) << 27);                                     \
    STBSP__COPYFP(ahi, bt);                                      \
    alo = xh - ahi;                                              \
    STBSP__COPYFP(bt, yh);                                       \
    bt &= ((~(u64)0) << 27);                                     \
    STBSP__COPYFP(bhi, bt);                                      \
    blo = yh - bhi;                                              \
    ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
  }

#define stbsp__ddtoS64(ob, xh, xl)     \
  {                                    \
    double ahi = 0, alo, vh, t;        \
    ob = (s64)xh;                      \
    vh = (double)ob;                   \
    ahi = (xh - vh);                   \
    t = (ahi - xh);                    \
    alo = (xh - (ahi - t)) - (vh + t); \
    ob += (s64)(ahi + alo + xl);       \
  }

#define stbsp__ddrenorm(oh, ol) \
  {                             \
    double s;                   \
    s = oh + ol;                \
    ol = ol - (s - oh);         \
    oh = s;                     \
  }

#define stbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define stbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void stbsp__raise_to_power10(double* ohi, double* olo, double d,
                                    s32 power)  // power can be -323 to +350
{
  double ph, pl;
  if ((power >= 0) && (power <= 22)) {
    stbsp__ddmulthi(ph, pl, d, stbsp__bot[power]);
  } else {
    s32 e, et, eb;
    double p2h, p2l;

    e = power;
    if (power < 0) e = -e;
    et = (e * 0x2c9) >> 14; /* %23 */
    if (et > 13) et = 13;
    eb = e - (et * 23);

    ph = d;
    pl = 0.0;
    if (power < 0) {
      if (eb) {
        --eb;
        stbsp__ddmulthi(ph, pl, d, stbsp__negbot[eb]);
        stbsp__ddmultlos(ph, pl, d, stbsp__negboterr[eb]);
      }
      if (et) {
        stbsp__ddrenorm(ph, pl);
        --et;
        stbsp__ddmulthi(p2h, p2l, ph, stbsp__negtop[et]);
        stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__negtop[et],
                        stbsp__negtoperr[et]);
        ph = p2h;
        pl = p2l;
      }
    } else {
      if (eb) {
        e = eb;
        if (eb > 22) eb = 22;
        e -= eb;
        stbsp__ddmulthi(ph, pl, d, stbsp__bot[eb]);
        if (e) {
          stbsp__ddrenorm(ph, pl);
          stbsp__ddmulthi(p2h, p2l, ph, stbsp__bot[e]);
          stbsp__ddmultlos(p2h, p2l, stbsp__bot[e], pl);
          ph = p2h;
          pl = p2l;
        }
      }
      if (et) {
        stbsp__ddrenorm(ph, pl);
        --et;
        stbsp__ddmulthi(p2h, p2l, ph, stbsp__top[et]);
        stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__top[et], stbsp__toperr[et]);
        ph = p2h;
        pl = p2l;
      }
    }
  }
  stbsp__ddrenorm(ph, pl);
  *ohi = ph;
  *olo = pl;
}

// given a float value, returns the significant bits in bits, and the position
// of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special
//   values returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant
// digits (got %g and %e), or in 0x80000000
/// @param start Pointer to the start of the string with the translated string.
/// May or may not be the @ref out param.
/// @param len Length of the translated string pointed to by @ref start.
/// @retval 0 if positive @ref value
/// @retval 1 if negative @ref value
static s32 stbsp__real_to_str(char const** start, u32* len, char* out,
                              s32* decimal_pos, double value, u32 frac_digits) {
  double d;
  s64 bits = 0;
  s32 expo, e, ng, tens;

  d = value;
  STBSP__COPYFP(bits, d);
  expo = (s32)((bits >> 52) & 2047);
  ng = (s32)((u64)bits >> 63);
  if (ng) d = -d;

  if (expo == 2047)  // is nan or inf?
  {
    *start = (static_cast<u64>(bits) & ((((u64)1) << 52) - 1)) ? "NaN" : "Inf";
    *decimal_pos = STBSP__SPECIAL;
    *len = 3;
    return ng;
  }

  if (expo == 0)  // is zero or denormal
  {
    if (((u64)bits << 1) == 0)  // do zero
    {
      *decimal_pos = 1;
      *start = out;
      out[0] = '0';
      *len = 1;
      return ng;
    }
    // find the right expo for denormals
    {
      s64 v = ((u64)1) << 51;
      while ((bits & v) == 0) {
        --expo;
        v >>= 1;
      }
    }
  }

  // find the decimal exponent as well as the decimal bits of the value
  {
    double ph, pl;

    // log10 estimate - very specifically tweaked to hit or undershoot by no
    // more than 1 of log10 of all expos 1..2046
    tens = expo - 1023;
    tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

    // move the significant bits into position and stick them into an int
    stbsp__raise_to_power10(&ph, &pl, d, 18 - tens);

    // get full as much precision from double-double as possible
    stbsp__ddtoS64(bits, ph, pl);

    // check if we undershot
    if (((u64)bits) >= stbsp__tento19th) ++tens;
  }

  // now do the rounding in integer land
  frac_digits = (frac_digits & 0x80000000)
                    ? ((frac_digits & 0x7ffffff) + 1)
                    : static_cast<u32>(static_cast<s32>(tens) +
                                       static_cast<s32>(frac_digits));
  if ((frac_digits < 24)) {
    u32 dg = 1;
    if ((u64)bits >= stbsp__powten[9]) dg = 10;
    while ((u64)bits >= stbsp__powten[dg]) {
      ++dg;
      if (dg == 20) goto noround;
    }
    if (frac_digits < dg) {
      u64 r;
      // add 0.5 at the right position and round
      e = static_cast<s32>(dg - frac_digits);
      if ((u32)e >= 24) goto noround;
      r = stbsp__powten[e];
      bits = static_cast<s64>(bits) + static_cast<s64>(r / 2);
      if ((u64)bits >= stbsp__powten[dg]) ++tens;
      bits /= static_cast<s64>(r);
    }
  noround:;
  }

  // kill long trailing runs of zeros
  if (bits) {
    u32 n;
    for (;;) {
      if (bits <= 0xffffffff) break;
      if (bits % 1000) goto donez;
      bits /= 1000;
    }
    n = (u32)bits;
    while ((n % 1000) == 0) n /= 1000;
    bits = n;
  donez:;
  }

  // convert to string
  out += 64;
  e = 0;
  for (;;) {
    u32 n;
    char* o = out - 8;
    // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it,
    // constant denomiators be damned)
    if (bits >= 100000000) {
      n = (u32)(bits % 100000000);
      bits /= 100000000;
    } else {
      n = (u32)bits;
      bits = 0;
    }
    while (n) {
      out -= 2;
      *(u16*)out = *(u16*)&stbsp__digitpair.pair[(n % 100) * 2];
      n /= 100;
      e += 2;
    }
    if (bits == 0) {
      if ((e) && (out[0] == '0')) {
        ++out;
        --e;
      }
      break;
    }
    while (out != o) {
      *--out = '0';
      ++e;
    }
  }

  *decimal_pos = tens;
  *start = out;
  *len = static_cast<u32>(e);
  return ng;
}

///////////////////////////////////////////////////////////////////////////////
// Parse Utils
//

// TODO cgustafsson: constexpr
Result<u32, FormatErr> parseInteger(const char8*& it, const char8* end) {
  const char8* numStart = it;

  while (it != end && *it >= '0' && *it <= '9') ++it;

  if (numStart == it)
    return Err(FormatErr{FormatErr::Kind::InvalidSpecification, 0});

  const char8* numEnd = it;
  u32 out = 0;
  while (numStart != numEnd) {
    const s64 idx = numEnd - numStart - 1;
    if (idx >= 20) {  // powten only goes [0, 20)
      return Err(FormatErr{FormatErr::Kind::InvalidSpecification, 0});
    }
    out += static_cast<u32>(*numStart - '0') *
           stbsp__powten[static_cast<usize>(idx)];
    ++numStart;
  }

  return Ok(out);
}

Option<Result<FormatFill, FormatErr>> FormatFill::parse(const char8*& it,
                                                        const char8* end) {
  if (*it == '<' || *it == '>' || *it == '^') {
    if (it + 2 > end)
      return Some(Result<FormatFill, FormatErr>(
          Err(FormatErr{FormatErr::Kind::InvalidSpecification, 0})));

    Result<FormatFill, FormatErr> out = Ok(FormatFill{});

    if (*it == '<')
      out->align = Align::Left;
    else if (*it == '>')
      out->align = Align::Right;
    else
      out->align = Align::Center;

    const auto begin = it;
    ++it;

    auto size = utf8::validate(it);
    const auto strSize = end - it;

    if (size && (s64)*size <= strSize) {
      auto nextIt = it + utf8::decode(it, 0, out->sign);
      if (nextIt < end && *nextIt >= '0' && *nextIt <= '9') {
        it = nextIt;
        if (it >= end)
          return Some(Result<FormatFill, FormatErr>(Err(
              FormatErr{FormatErr::Kind::InvalidSpecification, it - begin})));

        auto value = parseInteger(it, end);
        if (value.isErr())
          return Some(Result<FormatFill, FormatErr>(Err(value.errValue())));

        out->space = *value;
        return Some(dc::move(out));
      }
    }

    out->sign = ' ';
    if (it >= end)
      return Some(Result<FormatFill, FormatErr>(
          Err(FormatErr{FormatErr::Kind::InvalidSpecification, it - begin})));

    auto value = parseInteger(it, end);
    if (value.isErr())
      return Some(Result<FormatFill, FormatErr>(Err(value.errValue())));

    out->space = *value;

    return Some(dc::move(out));
  }

  return None;
}

Result<NoneType, FormatErr> FormatFill::format(const Option<FormatFill>& fill,
                                               StringView str,
                                               FormatContext& ctx) {
  const auto begin = str.begin();
  const auto end = str.end();
  const auto len = end - begin;

  if (fill && fill->space > len) {
    if (fill->align == FormatFill::Align::Left) {
      ctx.out.addRange(begin, end);

      auto i = fill->space - len;
      while (i-- > 0) {
        ctx.out.add(static_cast<char8>(fill->sign));
      }
    } else if (fill->align == FormatFill::Align::Right) {
      auto i = fill->space - len;
      while (i-- > 0) {
        ctx.out.add(static_cast<char8>(fill->sign));
      }

      ctx.out.addRange(begin, end);
    } else /* if (fill->align == FormatFill::Align::Center) */ {
      auto i = (fill->space - len) / 2;
      while (i-- > 0) {
        ctx.out.add(static_cast<char8>(fill->sign));
      }
      if ((fill->space - len) % 2 == 1) {
        ctx.out.add(static_cast<char8>(fill->sign));
      }

      ctx.out.addRange(begin, end);

      i = (fill->space - len) / 2;
      while (i-- > 0) {
        ctx.out.add(static_cast<char8>(fill->sign));
      }
    }
  } else {
    ctx.out.addRange(begin, end);
  }

  return Ok(None);
}

///////////////////////////////////////////////////////////////////////////////
// Formatters
//

template <>
struct Formatter<f64> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx) {
    auto it = ctx.pattern.begin();
    auto end = ctx.pattern.end();
    for (; it < end; ++it) {
      if (*it == '}')
        break;
      else if (*it == '.') {
        // TODO cgustafsson: constexpr
        Result<u32, FormatErr> res = parseInteger(++it, ctx.pattern.end());
        if (res.isOk())
          decimals = *res;
        else
          return Err(res.errValue());

        --it;  // Adjust for the for loop increasing it once as well.
      } else if (auto res = FormatFill::parse(it, end)) {
        if (res->isOk()) {
          fill = Some(res->value());
          --it;  // Adjust for the for loop increasing it once as well.
        } else
          return Err(res->errValue());
      } else
        return Err(FormatErr{FormatErr::Kind::InvalidSpecification,
                             it - ctx.pattern.begin()});
    }

    return Ok(it);
  }

  Result<NoneType, FormatErr> format(f64 value, FormatContext& ctx) {
    // TODO cgustafsson: harden for large numbers

    // This code is translated from stb_sprintf, if you ever need to look at
    // that code again, here is a variable translation table:
    // clang-format off
	// pr := decimals (precision)
	// fl := (flags, we don't have those)
	// fv := value
	// l  := len
	// dp := decimalPos
	// cs := comma seprator logic, = 0 for comma seperator disabled
	// s  :=
	// sn := outBegin
	// num:= numBuf
	// n  := u32
    // clang-format on

    constexpr u32 kBufSize = 512;
    char8 numBuf[kBufSize];  // big enough for e308 (with commas) or e-307
    char8 const* outBegin;
    u32 len;
    s32 decimalPos;
    const auto isNegative = stbsp__real_to_str(&outBegin, &len, numBuf,
                                               &decimalPos, value, decimals);

    char8* s = numBuf + 64;
    u32 n = 0, trailingZeros = 0;

    if (decimalPos == STBSP__SPECIAL) {
      s = (char*)outBegin;
      // cs = 0;
      // pr = 0;
    } else {
      // pre process the string
      if (decimalPos <= 0) {
        s32 i;
        // handle 0.000*000xxxx
        *s++ = '0';
        if (decimals) *s++ = '.';
        n = static_cast<u32>(-decimalPos);
        if (n > decimals) n = decimals;
        i = static_cast<s32>(n);
        while (i) {
          if ((((uintptr)s) & 3) == 0) break;
          *s++ = '0';
          --i;
        }
        // TODO cgustafsson: why a second pass on i?
        while (i >= 4) {
          *(u32*)s = 0x30303030;
          s += 4;
          i -= 4;
        }
        while (i) {
          *s++ = '0';
          --i;
        }
        if (static_cast<s32>(len) + i > static_cast<s32>(decimals))
          len = static_cast<u32>(static_cast<s32>(decimals) - i);
        i = static_cast<s32>(len);
        while (i) {
          *s++ = *outBegin++;
          --i;
        }
        trailingZeros = decimals - (n + len);
        // cs = 1 + (3 << 24);  // how many tens did we write (for commas below)
      } else {
        if ((u32)decimalPos >= len) {
          // handle xxxx000*000.0
          for (;;) {
            *s++ = outBegin[n];
            ++n;
            if (n >= len) break;
          }

          if (n < (u32)decimalPos) {
            n = static_cast<u32>(decimalPos) - n;
            while (n) {
              if ((((uintptr)s) & 3) == 0) break;
              *s++ = '0';
              --n;
            }
            while (n >= 4) {
              *(u32*)s = 0x30303030;
              s += 4;
              n -= 4;
            }
            while (n) {
              *s++ = '0';
              --n;
            }
          }

          // cs = (int)(s - (num + 64)) + (3 << 24);  // cs is how many tens
          if (decimals) {
            *s++ = '.';
            trailingZeros = decimals;
          }
        } else {
          // handle xxxxx.xxxx000*000
          n = 0;
          for (;;) {
            *s++ = outBegin[n];
            ++n;
            if (n >= (u32)decimalPos) break;
          }
          // cs = (int)(s - (num + 64)) + (3 << 24);  // cs is how many tens
          if (decimals) *s++ = '.';
          if (static_cast<s32>(len) - decimalPos > static_cast<s32>(decimals))
            len = static_cast<u32>(static_cast<s32>(decimals) + decimalPos);
          while (n < len) {
            *s++ = outBegin[n];
            ++n;
          }
          trailingZeros =
              static_cast<u32>(static_cast<s32>(decimals) -
                               (static_cast<s32>(len) - decimalPos));
        }
      }
      // decimals = 0;

      len = (u32)(s - (numBuf + 64));
      s = numBuf + 64;
    }

    // get fw=leading/trailing space, pr=leading zeros
    // if (pr < (stbsp__int32)l) pr = l;
    // n = pr + lead[0] + tail[0] + tz;
    // if (fw < (stbsp__int32)n) fw = n;
    // fw -= n;
    // pr -= l;

    // handle right justify and leading zeros
    // if ((fl & STBSP__LEFTJUST) == 0) {
    // 	if (fl & STBSP__LEADINGZERO)  // if leading zeros, everything is in pr
    // 	{
    // 		pr = (fw > pr) ? fw : pr;
    // 		fw = 0;
    // 	} else {
    // 		fl &= ~STBSP__TRIPLET_COMMA;  // if no leading zeros, then no
    // commas
    // 	}
    // }

    // ignoring lots of lines regarding leading

    // TODO cgustafsson: this extra buffer is terrible and we should
    // write directly to the output buffer
    constexpr u32 kPrefix = 1 + 1;  // '-' and '.'
    char8 strBuf[kBufSize + kPrefix];
    char8* strIt = strBuf;
    // char8* strEnd = strBuf;

    if (isNegative) *strIt++ = '-';

    // copy the string
    n = len;
    while (n) {
      s32 i = static_cast<s32>(n);  // clamp on callback buf size
      // stbsp__cb_buf_clamp(i, n);
      n -= static_cast<u32>(i);
      while (i >= 4) {
        *(u32 volatile*)strIt = *(u32 volatile*)s;
        strIt += 4;
        s += 4;
        i -= 4;
      }
      while (i) {
        *strIt++ = *s++;
        --i;
      }
      // flush our data by calling callback to take buf
      // stbsp__chk_cb_buf(1);
    }

    // copy trailing zeros
    while (trailingZeros) {
      s32 i = static_cast<s32>(trailingZeros);
      // stbsp__cb_buf_clamp(i, trailingZeros);
      trailingZeros -= static_cast<u32>(i);
      while (i) {
        if ((((uintptr)strIt) & 3) == 0) break;
        *strIt++ = '0';
        --i;
      }
      while (i >= 4) {
        *(u32*)strIt = 0x30303030;
        strIt += 4;
        i -= 4;
      }
      while (i) {
        *strIt++ = '0';
        --i;
      }
      // flush our data by calling callback to take buf
      // stbsp__chk_cb_buf(1);
    }

    // copy tail if there is one
    // sn = tail + 1;
    // while (tail[0]) {
    // 	stbsp__int32 i;
    // 	stbsp__cb_buf_clamp(i, tail[0]);
    // 	tail[0] -= (char)i;
    // 	while (i) {
    // 		*bf++ = *sn++;
    // 		--i;
    // 	}
    // 	stbsp__chk_cb_buf(1);
    // }

    return FormatFill::format(fill, StringView{strBuf, strIt}, ctx);
  }

  u32 decimals = DBL_DIG;

  Option<FormatFill> fill;
};

Result<const char8*, FormatErr> Formatter<StringView>::parse(
    ParseContext& ctx) {
  auto it = ctx.pattern.begin();
  const auto end = ctx.pattern.end();
  for (; it != end; ++it) {
    if (*it == '}')
      break;
    else if (*it == '.') {
      // TODO cgustafsson: constexpr
      Result<u32, FormatErr> res = parseInteger(++it, ctx.pattern.end());
      if (res.isOk())
        precision = *res;
      else
        return Err(res.errValue());

      --it;  // Adjust for the for loop increasing it once as well.
    } else if (auto res = FormatFill::parse(it, end)) {
      if (res->isOk()) {
        fill = Some(res->value());
        --it;  // Adjust for the for loop increasing it once as well.
      } else
        return Err(res->errValue());
    } else
      return Err(FormatErr{FormatErr::Kind::InvalidSpecification,
                           it - ctx.pattern.begin()});
  }

  return Ok(it);
}

Result<NoneType, FormatErr> Formatter<StringView>::format(const StringView& str,
                                                          FormatContext& ctx) {
  auto len = dc::min(str.getSize(), precision);
  return FormatFill::format(fill, StringView{str.c_str(), len}, ctx);
}

Result<const char8*, FormatErr> Formatter<u64>::parse(ParseContext& ctx) {
  auto it = ctx.pattern.begin();
  auto end = ctx.pattern.end();
  for (; it != end; ++it) {
    if (*it == '}')
      break;
    else if (*it == 'b')
      presentation = Presentation::Binary;
    else if (*it == 'x')
      presentation = Presentation::Hex;
    else if (*it == '#')
      prefix = true;
    else if (auto res = FormatFill::parse(it, end)) {
      if (res->isOk()) {
        fill = Some(res->value());
        --it;  // Adjust for the for loop increasing it once as well.
      } else
        return Err(res->errValue());
    } else
      return Err(FormatErr{FormatErr::Kind::InvalidSpecification,
                           it - ctx.pattern.begin()});
  }

  return Ok(it);
}

Result<NoneType, FormatErr> Formatter<u64>::format(u64 value,
                                                   FormatContext& ctx) {
  constexpr u32 kPrefix = 1 + 2;          // '-' + '0x'
  constexpr u32 kBufSize = 20 + kPrefix;  // len("18446744073709551616") == 20
  char8 buf[kBufSize];
  char8* bufStart = buf + kPrefix;

  auto viewOrErr = toString(value, bufStart, kBufSize - kPrefix, presentation);
  if (viewOrErr.isErr())
    // TODO cgustafsson: fill in error pos ?
    return Err(FormatErr{FormatErr::Kind::OutOfMemory, 0});

  bufStart = (char8*)viewOrErr->begin();
  const char8* bufEnd = viewOrErr->end();

  if (prefix) {
    auto res = getPresentationChar();
    if (res.isErr()) {
      // TODO cgustafsson: fill in error pos?
      return Err(FormatErr{res.errValue(), 0});
    }
    bufStart -= 2;
    bufStart[0] = '0';
    bufStart[1] = *res;
  }

  if (negative) {
    bufStart -= 1;
    bufStart[0] = '-';
  }

  return FormatFill::format(fill, StringView{bufStart, bufEnd}, ctx);
}

Result<char8, FormatErr::Kind> Formatter<u64>::getPresentationChar() const {
  switch (presentation) {
    case Presentation::Binary:
      return Ok('b');
    case Presentation::Hex:
      return Ok('x');
    default:
      return Err(FormatErr::Kind::InvalidSpecification);
  }
}

// TODO cgustafsson: is this used?
Result<NoneType, FormatErr> Formatter<s64>::format(s64 value,
                                                   FormatContext& ctx) {
  // // TODO cgustafsson: is this the correct len?
  // constexpr u32 kBufSize = 20;  // strlen("18446744073709551616") == 20
  // char8 buf[kBufSize];

  // auto viewOrErr = toString(value, buf, kBufSize, presentation);
  // if (viewOrErr.isErr())
  //   // TODO cgustafsson: how to get position
  //   return Err(FormatErr{FormatErr::Kind::OutOfMemory, 0});

  // ctx.out.addRange(viewOrErr->begin(), viewOrErr->end());
  // return Ok(NoneType());
  negative = value < 0;
  const u64 uvalue = value >= 0 ? (u64)value : (u64)-value;
  return Formatter<u64>::format(uvalue, ctx);
}

Result<const char8*, FormatErr> Formatter<bool>::parse(ParseContext& ctx) {
  auto it = ctx.pattern.begin();
  auto end = ctx.pattern.end();
  for (; it != end; ++it) {
    if (*it == '}')
      break;
    else
      return Err(FormatErr{FormatErr::Kind::InvalidSpecification,
                           it - ctx.pattern.begin()});
  }

  return Ok(it);
}

Result<NoneType, FormatErr> Formatter<bool>::format(bool value,
                                                    FormatContext& ctx) {
  constexpr const char* t = "true";
  constexpr const char* f = "false";
  const char* s = value ? t : f;
  const u32 len = value ? 4 : 5;
  ctx.out.addRange(s, s + len);
  return Ok(None);
}

Result<NoneType, FormatErr> Formatter<char8>::format(char8 value,
                                                     FormatContext& ctx) {
  ctx.out.addRange(&value, &value + 1);
  return Ok(None);
}

Result<const char8*, FormatErr> doFormatArg(ParseContext& parseCtx,
                                            FormatContext& formatCtx,
                                            FormatArg& formatArg) {
  if (formatArg.type < FormatArg::Types::LastSignedIntegerType) {
    Formatter<s64> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      const s64 value = formatArg.type == FormatArg::Types::S32Type
                            ? formatArg.s32Value
                            : formatArg.s64Value;
      auto formatRes = f.format(value, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  }
  if (formatArg.type < FormatArg::Types::LastIntegerType) {
    Formatter<u64> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      const u64 value = formatArg.type == FormatArg::Types::U32Type
                            ? formatArg.u32Value
                            : formatArg.u64Value;
      auto formatRes = f.format(value, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type < FormatArg::Types::LastFloatType) {
    Formatter<f64> f;
    f64 value;
    if (formatArg.type == FormatArg::Types::F32Type) {
      value = formatArg.f32Value;
      f.decimals = FLT_DIG;
    } else /* if (formatArg.type == FormatArg::Types::F64Type) */
      value = formatArg.f64Value;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format(value, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::BoolType) {
    Formatter<bool> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format(formatArg.boolValue, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::CharType) {
    Formatter<char> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format(formatArg.charValue, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::CStringType) {
    Formatter<const char8*> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format(formatArg.cstringValue, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::StringViewType) {
    Formatter<StringView> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format(formatArg.stringViewValue, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::PointerType) {
    Formatter<u64> f;
    f.prefix = true;
    f.presentation = Presentation::Hex;
    auto res = f.parse(parseCtx);
    if (res.isOk()) {
      auto formatRes = f.format((u64)formatArg.pointerValue, formatCtx);
      if (formatRes.isErr()) return Err(dc::move(formatRes).unwrapErr());
    }
    return res;
  } else if (formatArg.type == FormatArg::Types::CustomType) {
    return formatArg.customValue.format(formatArg.customValue.value, parseCtx,
                                        formatCtx);
  } else {
    return Err(FormatErr{FormatErr::Kind::CannotFormatType, 0});
  }
  return Ok(parseCtx.pattern.begin());
}

// #ifdef _WIN32
// namespace detail {
// using dword = Conditional<sizeof(long) == 4, unsigned long, unsigned>;
// extern "C" __declspec(dllimport) int __stdcall WriteConsoleW(  //
//     void*, const void*, dword, dword*, void*);
// }  // namespace detail
// #endif

// Result<NoneType, FormatErr> rawPrintIfWChar(StringView str)
// {
// #ifdef _WIN32
// 	auto fd = _fileno(f);
// 	if (_isatty(fd)) {
// 		detail::utf8_to_utf16 u16(string_view(buffer.data(),
// buffer.size())); 		auto written = detail::dword(); if
// (detail::WriteConsoleW(reinterpret_cast<void*>(_get_osfhandle(fd)),
// 								  u16.c_str(),
// static_cast<uint32_t>(u16.size()),
// &written, nullptr)) { 			return;
// 		}
// 		// Fallback to fwrite on failure. It can happen if the output
// has been
// 		// redirected to NUL.
// 	}
// #endif
// 	detail::fwrite_fully(buffer.data(), 1, buffer.size(), f);
// }

// TODO cgustafsson: wchar for windows
namespace detail {
Result<NoneType, FormatErr> rawPrint(FILE* f, StringView str) {
  // if (std::fputws(str.c_str(), f) == -1)
  // 	return Err(FormatErr::CannotWriteToFile);
  if (fputs(str.begin(), f) == EOF)
    return Err(FormatErr{FormatErr::Kind::CannotWriteToFile, 0});
  return Ok(None);
}
}  // namespace detail

Result<StringView, s64> toString(s64 ivalue, char8* buf, s64 bufSize,
                                 Presentation presentation) {
  u64 uvalue;
  if (ivalue < 0) {
    uvalue = (u64)-ivalue;
    auto StrOrErr = toString(uvalue, buf, bufSize, presentation);
    if (StrOrErr.isErr()) return StrOrErr;
    if (StrOrErr->c_str() == buf) return Err(bufSize + 1);
    char8* begin = (char8*)((StrOrErr->c_str()) - 1);
    *(buf) = '-';
    return Ok(StringView{begin, static_cast<u64>(bufSize) + 1});
  } else {
    uvalue = (u64)ivalue;
    return toString(uvalue, buf, bufSize, presentation);
  }
}

Result<StringView, s64> toString(u64 value, char8* buf, s64 bufSize,
                                 Presentation presentation) {
  char8* s = buf + bufSize;

  if (presentation == Presentation::Decimal) {
    // NOTE: from stb's sprintf

    u32 n;
    for (;;) {
      // do in 32-bit chunks (avoid lots of 64-bit divides even with
      // constant denominators)
      char* o = s - 8;
      if (value >= 100000000) {
        n = (u32)(value % 100000000);
        value /= 100000000;
      } else {
        n = (u32)value;
        value = 0;
      }
      // if ((fl & STBSP__TRIPLET_COMMA) == 0) {
      //   do {
      //     s -= 2;
      //     *(stbsp__uint16 *)s =
      //         *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
      //     n /= 100;
      //   } while (n);
      // }
      while (n) {
        // if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
        //   l = 0;
        //   *--s = stbsp__comma;
        //   --o;
        // } else {
        *--s = (char8)(n % 10) + '0';
        n /= 10;
        //}
      }
      if (value == 0) {
        if ((s[0] == '0') && (s != (buf + bufSize))) ++s;
        break;
      }
      while (s != o)
        // if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
        //   l = 0;
        //   *--s = stbsp__comma;
        //   --o;
        // } else {
        *--s = '0';
      //}
    }

    if (((buf + bufSize) - s) == 0) {
      *--s = '0';
    }
  } else if (presentation == Presentation::Hex) {
    static char hex[] = "0123456789abcdefxp";
    u32 l = (4 << 4) | (4 << 8);
    u32 pr = 0;  // TODO cgustafsson: what is this?
    for (;;) {
      *--s = hex[value & ((1 << (l >> 8)) - 1)];
      value >>= (l >> 8);
      if (!((value) || (((buf + bufSize) - s) < pr))) break;
      // if (fl & STBSP__TRIPLET_COMMA) {
      // 	++l;
      // 	if ((l & 15) == ((l >> 4) & 15)) {
      // 		l &= ~15;
      // 		*--s = stbsp__comma;
      // 	}
      // }
    };
    // get the tens and the comma pos
    // u32 cs = (u32)((buf + bufSize) - s) +
    // 		 ((((l >> 4) & 15)) << 24);
    // // get the length that we copied
    // l = (u32)((buf + bufSize) - s);
  } else if (presentation == Presentation::Binary) {
    DC_FATAL_ASSERT(false, "todo");
  }

  return Ok(StringView{s, (u64)(buf + bufSize - s)});
}

namespace detail {
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

///////////////////////////////////////////////////////////////////////////////
// cleanup macros
//

#undef STBSP__SPECIAL
#undef STBSP__COPYFP
#undef stbsp__tento19th
#undef stbsp__ddmulthi
#undef stbsp__ddtoS64
#undef stbsp__ddrenorm
#undef stbsp__ddmultlo
#undef stbsp__ddmultlos
