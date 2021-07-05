#include <dc/dtest.hpp>
#include <dc/string.hpp>

DTEST(empty) {
  dc::String nothing("");
  ASSERT_TRUE(nothing.isEmpty());
}

DTEST(notEmpty) {
  dc::String something("abc");
  ASSERT_FALSE(something.isEmpty());
}

DTEST(emptyAfterBeingBigString) {
  dc::String str("123456789.123456789.123456789.123456789.123456789");
  ASSERT_FALSE(str.isEmpty());
  str = "";
  ASSERT_TRUE(str.isEmpty());
}

DTEST(size) {
  dc::String msg("123");
  ASSERT_EQ(msg.getSize(), 3);
}

DTEST(sizeWhenEmpty) {
  dc::String msg("");
  ASSERT_EQ(msg.getSize(), 0);
}

DTEST(sizeWhenBigString) {
  const usize len = strlen("abc, abc, abc, abc, abc, abc, ");
  dc::String str = "abc, abc, abc, abc, abc, abc, ";

  ASSERT_EQ(str.getSize(), len);
}

DTEST(isSameAsCString) {
  const char* cstr = "abc";
  dc::String str = "abc";

  ASSERT_TRUE(strcmp(cstr, str.c_str()) == 0);
}

DTEST(isSameAsCStringLooong) {
  const char* cstr = "abc, abc, abc, abc, abc, abc, ";
  dc::String str = "abc, abc, abc, abc, abc, abc, ";

  ASSERT_TRUE(strcmp(cstr, str.c_str()) == 0);
}
