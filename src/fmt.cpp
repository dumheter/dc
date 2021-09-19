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
#include <dc/fmt.hpp>

namespace dc::xfmt {

///////////////////////////////////////////////////////////////////////////////

const char8* toString(FormatErr err) {
  switch (err) {
    case FormatErr::ParseInvalidChar:
      return "Parsed invalid format fields.";
    case FormatErr::CannotFormatType:
      return "Cannot format type. Specialize the struct Formatter<T> for your "
             "type.";
    default:
      return "Internal error.";
  }
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

// get float info
// static s32 stbsp__real_to_parts(s64 *bits, s32 *expo, double value)
// {
//    double d;
//    s64 b = 0;

//    // load value and round at the frac_digits
//    d = value;

//    STBSP__COPYFP(b, d);

//    *bits = b & ((((u64)1) << 52) - 1);
//    *expo = (s32)(((b >> 52) & 2047) - 1023);

//    return (s32)((u64) b >> 63);
// }

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
    *start = (bits & ((((u64)1) << 52) - 1)) ? "NaN" : "Inf";
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
  frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1)
                                           : (tens + frac_digits);
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
      e = dg - frac_digits;
      if ((u32)e >= 24) goto noround;
      r = stbsp__powten[e];
      bits = bits + (r / 2);
      if ((u64)bits >= stbsp__powten[dg]) ++tens;
      bits /= r;
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
  *len = e;
  return ng;
}

///////////////////////////////////////////////////////////////////////////////
// Parse Utils
//

// TODO cgustafsson: constexpr
Result<u32, FormatErr> parseInteger(const char8*& it, const char8* end) {
  const char8* numStart = it;

  while (it != end && *it >= '0' && *it <= '9') ++it;

  if (numStart == it) return Err(FormatErr::ParseInvalidChar);

  const char8* numEnd = it;
  u32 out = 0;
  while (numStart != numEnd) {
    DC_FATAL_ASSERT(numEnd - numStart - 1 < 20,
                    "todo handle x^y where y is more than 19");
    out += (*numStart - '0') * (u32)stbsp__powten[numEnd - numStart - 1];
    ++numStart;
  }

  // TODO cgustafsson: no move for primitive type
  return Ok(dc::move(out));
}

///////////////////////////////////////////////////////////////////////////////
// Formatters
//

template <>
struct Formatter<u64> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx) {
    auto it = ctx.pattern.beginChar8();
    for (; it != ctx.pattern.endChar8(); ++it) {
      if (*it == '}')
        break;
      else
        return Err(FormatErr::ParseInvalidChar);
    }

    // TODO cgustafsson: don't move primitive types
    return Ok(dc::move(it));
  }

  void format(u64 value, FormatContext& ctx) {
    constexpr u32 kBufSize = 20;  // strlen("18446744073709551616") == 20
    char8 buf[kBufSize];

	// NOTE: from stb's sprintf
    char8* s = buf + kBufSize;
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
        if ((s[0] == '0') && (s != (buf + kBufSize))) ++s;
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

    if (((buf + kBufSize) - s) == 0) {
      *--s = '0';
    }

    ctx.out.addRange(s, buf + kBufSize);
  }

  bool negative = false;
};

template <>
struct Formatter<f64> {
  Result<const char8*, FormatErr> parse(ParseContext& ctx) {
    auto it = ctx.pattern.beginChar8();
    for (; it != ctx.pattern.endChar8(); ++it) {
      if (*it == '}')
        break;
      else if (*it == '.') {
        // TODO cgustafsson: constexpr
        Result<u32, FormatErr> res = parseInteger(++it, ctx.pattern.endChar8());
        if (res.isOk())
          decimals = res.value();
        else  // TODO cgustafsson: trivial type should be copyable (lvalue)
          return Err(dc::move(res).unwrapErr());

        --it;  // revert the increase we did
      } else
        return Err(FormatErr::ParseInvalidChar);
    }

    // TODO cgustafsson: trivial type
    return Ok(dc::move(it));
  }

  void format(f64 value, FormatContext& ctx) {
    char const* start;
    u32 len;
    char buf[512];  // big enough for e308 (with commas) or e-307
    s32 decimalPos;
    s32 sign =
        stbsp__real_to_str(&start, &len, buf, &decimalPos, value, decimals);

    if (sign == 1) ctx.out.add('-');
    ctx.out.addRange(start, start + decimalPos);
    constexpr char kDecimal = '.';  // TODO cgustafsson: support locales?
    ctx.out.add(kDecimal);
    ctx.out.addRange(start + decimalPos, start + len);
  }

  u32 decimals = DBL_DIG;
};

template <>
struct Formatter<const char8*> {
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

  void format(const char8* str, FormatContext& ctx) {
    u64 len = strlen(str);
    len = dc::min(len, precision);
    ctx.out.addRange(str, str + len);
  }

  u64 precision = ~0llu;
};

///////////////////////////////////////////////////////////////////////////////
// doFormat
//

Result<const char8*, FormatErr> doFormatArg(ParseContext& parseCtx,
                                            FormatContext& formatCtx,
                                            FormatArg& formatArg) {
  if (formatArg.type < FormatArg::Types::LastIntegerType) {
    Formatter<u64> f;
    auto res = f.parse(parseCtx);
    u64 value;
    if (formatArg.type == FormatArg::Types::S32Type) {
      if (formatArg.s32Value < 0) {
        value = (u64)-formatArg.s32Value;
        f.negative = true;
      } else
        value = (u64)formatArg.s32Value;
    } else if (formatArg.type == FormatArg::Types::S64Type) {
      if (formatArg.s64Value < 0) {
        value = (u64)-formatArg.s64Value;
        f.negative = true;
      } else
        value = (u64)formatArg.s64Value;
    } else if (formatArg.type == FormatArg::Types::U32Type)
      value = formatArg.u32Value;
    else /* if (formatArg.type == FormatArg::Types::U64Type) */
      value = formatArg.u64Value;
    if (res.isOk()) f.format(value, formatCtx);
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
    if (res.isOk()) f.format(value, formatCtx);
    return res;
  } else if (formatArg.type == FormatArg::Types::CStringType) {
    Formatter<const char8*> f;
    auto res = f.parse(parseCtx);
    if (res.isOk()) f.format(formatArg.cstringValue, formatCtx);
    return res;
  } else if (formatArg.type == FormatArg::Types::CustomType) {
    return formatArg.customValue.format(formatArg.customValue.value, parseCtx,
                                        formatCtx);
  } else {
    return Err(FormatErr::CannotFormatType);
  }
  return Ok(parseCtx.pattern.beginChar8());
}

}  // namespace dc::xfmt
