#include <dc/dtest.hpp>
#include <dc/fmt.hpp>

DTEST_VIP;

using namespace dc;

DTEST(formatF32) {
  f32 a = 13.37f;
  auto res = dc::xfmt::format("hello {.2}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 13.37");
}

DTEST(formatF32WithPrecision) {
  f32 a = 13.37f;
  auto res = dc::xfmt::format("hello {.1}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello 13.4");
}

DTEST(formatInMiddleOfFmt) {
  f32 a = -202.78f;
  auto res = dc::xfmt::format("hello {.2} world", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "hello -202.78 world");
}

DTEST(formatDefaultF32Precision) {
  f32 a = 3.141592653f;
  auto res = dc::xfmt::format("{}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.141593");
}

DTEST(formatDefaultF64Precision) {
  f64 a = 3.14159265358979323;
  auto res = dc::xfmt::format("{}", a);
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "3.14159265358979");
}

DTEST(usingWrongPrecisionSignGivesErr) {
  f32 a = 3.141592653f;
  auto res = dc::xfmt::format("{,1}", a);
  ASSERT_TRUE(res.isErr());
  ASSERT_EQ(res.errValue(), dc::xfmt::FormatErr::ParseInvalidChar);
}

DTEST(canEscapeTheFormatCharacter) {
  auto res = dc::xfmt::format("{{}}");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "{}");
}

DTEST(canEscapeTheFormatCharacter2) {
  auto res = dc::xfmt::format("Hello {{World}}!");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Hello {World}!");
}

DTEST(canEscapeTheFormatCharacter3) {
  auto res = dc::xfmt::format("Hello {{W{}d}}!", "orl");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(res.value(), "Hello {World}!");
}

DTEST(formatCString) {
  auto res =
      dc::xfmt::format("What is the largest muscle in the human body? {}",
                       "The gluteus maximus.");
  ASSERT_TRUE(res.isOk());
  ASSERT_EQ(
      res.value(),
      "What is the largest muscle in the human body? The gluteus maximus.");
}

// TODO cgustafsson: NaN 1 / 0.0

// TODO cgustafsson: +/-inf

// TODO cgustafsson: test specifying a large number in precision field

/*
DTEST(formatS32) {
  s32 a = 7;
  String res = dc::xfmt::format("hello {}", a);

  ASSERT_EQ(res, "hello 7");
}

DTEST(formatU64) {
  u64 a = 2'500;
  String res = dc::xfmt::format("hello {}", a);

  ASSERT_EQ(res, "hello 2500");
}

DTEST(formatString) {
  String a("world");
  String res = dc::xfmt::format("hello {}", a);

  ASSERT_EQ(res, "hello world");
}
*/
