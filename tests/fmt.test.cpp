#include <dc/dtest.hpp>
#include <dc/fmt.hpp>
DTEST_VIP;
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

  // TODO cgustafsson:
  // res = dc::format("{}", 123.f);
  // ASSERT_TRUE(res.isOk());
  // ASSERT_EQ(res.value(), "123.0");
}

DTEST(formatDefaultF64Precision) {
  f64 a = 3.14159265358979323;
  auto res = dc::format("{}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.141592653589793");
}

// DTEST(formatLargeF64) {
// 	f64 a = 1e+301;
// 	auto res = dc::format("{}", a);
// 	// TODO cgustafsson: this is not working
// 	ASSERT_TRUE(res.isOk());
// 	//ASSERT_EQ(res.value(), "3.14159265358979");
// }

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

// TODO cgustafsson: NaN 1 / 0.0

// TODO cgustafsson: +/-inf

// TODO cgustafsson: test specifying a large number in precision field

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

DTEST(formatString) {
  String fact("Yellow is a color.");
  auto res = dc::format("Fact: {}", fact);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Fact: Yellow is a color.");
}

// DTEST(print) {
// 	String fact("Yellow is a color.");
// 	auto res = dc::print("Fact: {}", fact);
// 	ASSERT_TRUE(res.isOk());
// }

// TODO cgustafsson: test s32 0x80000000, s64 0x8000000000000000,
// n = -n doesnt work for those?
// ((s32)INT32_MIN and (s64)INT64_MIN respectively

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
