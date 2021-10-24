#include <dc/dtest.hpp>
#include <dc/fmt.hpp>

using namespace dc;

DTEST(formatF32) {
  f32 a = 13.37f;
  auto res = dc::format("hello {.2}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 13.37");
}

DTEST(formatF32WithPrecision) {
  f32 a = 13.37f;
  auto res = dc::format("hello {.1}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 13.4");
}

DTEST(formatInMiddleOfFmt) {
  f32 a = -202.78f;
  auto res = dc::format("hello {.2} world", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello -202.78 world");
}

DTEST(formatDefaultF32Precision) {
  f32 a = 3.141592653f;
  auto res = dc::format("{}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.141593");
}

DTEST(f32TrailingZero) {
  auto res = dc::format("{.1}", 3.f);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.0");

  res = dc::format("{.1}", 123.f);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "123.0");

  res = dc::format("{}", 123.f);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "123.000000");
}

DTEST(formatDefaultF64Precision) {
  f64 a = 3.14159265358979323;
  auto res = dc::format("{}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.141592653589793");
}

DTEST(usingWrongPrecisionSignGivesErr) {
  f32 a = 3.141592653f;
  auto res = dc::format("{,1}", a);
  ASSERT_TRUE(res.isErr());
  ASSERT_EQ(res.errValue().kind, dc::FormatErr::Kind::InvalidSpecification);
}

DTEST(canEscapeTheFormatCharacter) {
  auto res = dc::format("{{}}");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "{}");
}

DTEST(canEscapeTheFormatCharacter2) {
  auto res = dc::format("Hello {{World}}!");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Hello {World}!");
}

DTEST(canEscapeTheFormatCharacter3) {
  auto res = dc::format("Hello {{W{}d}}!", "orl");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Hello {World}!");
}

DTEST(formatCString) {
  auto res = dc::format("What is the largest muscle in the human body? {}",
                        "The gluteus maximus.");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(
      res.value(),
      "What is the largest muscle in the human body? The gluteus maximus.");
}

DTEST(formatInfForF32) {
  auto res = dc::format("{}", std::numeric_limits<f32>::infinity());
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Inf");
}

DTEST(formatInfForF64) {
  auto res = dc::format("{}", std::numeric_limits<f64>::infinity());
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Inf");
}

DTEST(formatNaNForF32) {
  auto res = dc::format("{}", std::numeric_limits<f32>::quiet_NaN());
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "NaN");
}

DTEST(formatNaNForF64) {
  auto res = dc::format("{}", std::numeric_limits<f64>::quiet_NaN());
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "NaN");
}

DTEST(formatU64) {
  u64 value = 1234567890;
  auto res = dc::format("hello {}!", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 1234567890!");
}

DTEST(formatLargeU64) {
  u64 value = ULLONG_MAX;
  auto res = dc::format("hello {}!", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 18446744073709551615!");
}

DTEST(formatU8Limits) {
  u8 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = 255;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [255]");
}

DTEST(formatS8Limits) {
  s8 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = -128;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [-128]");

  value = 127;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [127]");
}

DTEST(formatU16Limits) {
  u16 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = 65'535;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [65535]");
}

DTEST(formatS16Limits) {
  s16 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = -32'768;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [-32768]");

  value = 32'767;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [32767]");
}

DTEST(formatU32Limits) {
  u32 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = 4'294'967'295;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [4294967295]");
}

DTEST(formatS32Limits) {
  s32 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = -(s32)2'147'483'648;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [-2147483648]");

  value = 2'147'483'647;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [2147483647]");
}

DTEST(formatU64Limits) {
  u64 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = 18'446'744'073'709'551'615ull;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [18446744073709551615]");
}

DTEST(formatS64Limits) {
  s64 value = 0;
  auto res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [0]");

  value = -9'223'372'036'854'775'808ll;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [-9223372036854775808]");

  value = 9'223'372'036'854'775'807;
  res = dc::format("x = [{}]", value);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x = [9223372036854775807]");
}

DTEST(formatString) {
  String fact("Yellow is a color.");
  auto res = dc::format("Fact: {}", fact);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Fact: Yellow is a color.");
}

DTEST(print) {
  String fact("Yellow is a color.");
  auto res = dc::format("Fact: {}", fact);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "Fact: Yellow is a color.");
}

DTEST(formatIntAsHexWithPrefix) {
  auto res = format("{#x}", 15);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "0xf");
}

DTEST(formatIntAsHex) {
  auto res = format("{x}", 15);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "f");
}

DTEST(formatIntAsHexWithPrefix) {
  auto res = format("{#x}", 0xF5C6F515);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "0xf5c6f515");
}

DTEST(formatWithLeftFillInt) {
  auto res = dc::format("{<03}", 7);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "700");
}

DTEST(formatWithRightFillInt) {
  auto res = dc::format("{>03}", 7);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "007");
}

DTEST(formatWithCenterFillInt) {
  auto res = dc::format("{^03}", 7);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "070");
}

DTEST(formatWithLeftFillFloat) {
  auto res = dc::format("{<05.1}", 7.1f);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "7.100");
}

DTEST(formatWithRightFillFloat) {
  auto res = dc::format("{>05.1}", 7.1f);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "007.1");
}

DTEST(formatWithCenterFillFloat) {
  auto res = dc::format("{^05.1}", 7.1f);
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "07.10");
}

DTEST(formatWithLeftFillString) {
  auto res = dc::format("[{<~7}]", "TEST");
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "[TEST~~~]");
}

DTEST(formatWithRightFillString) {
  auto res = dc::format("[{>~7}]", "TEST");
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "[~~~TEST]");
}

DTEST(formatWithCenterFillString) {
  auto res = dc::format("[{^~7}]", "TEST");
  ASSERT_TRUE(res);
  ASSERT_EQ(*res, "[~~TEST~]");
}

DTEST(correctDecimalWithLeadingZeros) {
  auto res = dc::format("{}", 0.007f);
  ASSERT_TRUE(res);
  // by default, the trailing three 0's should be shown for sprintf like
  // behavior do we want that?
  ASSERT_EQ(*res, "0.007000");
}

DTEST(formatChar) {
  auto res = dc::format("{}", 'x');
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "x");
}

DTEST(formatBoolTrue) {
  auto res = dc::format("{}", true);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "true");
}

DTEST(formatBoolFalse) {
  auto res = dc::format("{}", false);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(*res, "false");
}
