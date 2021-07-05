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

  dc::String empty;
  ASSERT_EQ(empty.getSize(), 0);
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

DTEST(canIterate) {
  dc::String str = "The quick brown fox jumps over the fence.";
  usize i = 0;
  for (u8 c : str) {
    (void)c;
    ++i;
  }
  ASSERT_EQ(i, str.getSize());
}

DTEST(lengthOf1CodePointString) {
  dc::String str = "abc";
  ASSERT_EQ(3, str.getLength());
}

DTEST(lengthOfMultiCodePointString) {
  dc::String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;
  // TODO cgustafsson:
  // ASSERT_EQ(1, str.getLength());
  ASSERT_EQ(4, str.getSize());
}

DTEST(appendSmallToBig) {
  dc::String str = "small";
  ASSERT_TRUE(str.getCapacity() < sizeof(dc::String));
  str += " The quick brown fox jumps over the fence.";
  ASSERT_TRUE(str.getCapacity() > sizeof(dc::String));
}

DTEST(insertInMiddleOfString) {
  dc::String str = "Hellx World";
  str.insert("o", 4);
  ASSERT_EQ(str, "Hello World");
}

DTEST(insertMakesStringGrow) {
  dc::String str = "The ...";
  const usize before = str.getCapacity();
  str.insert("quick brown fox jumped over the fence.", 4);
  ASSERT_EQ(str, "The quick brown fox jumped over the fence.");
  ASSERT_TRUE(str.getCapacity() > before);
}

// DTEST(insertWithNoOverwrite) {
// 	dc::String str = "Hellld";
// 	str.insert("o wor", 4);
// 	ASSERT_EQ("Hello world", str);
// }

DTEST(StringViewCompTime)
{
	constexpr dc::StringView view("comptime length");

  ASSERT_EQ(view.getSize(), strlen("comptime length"));
  ASSERT_TRUE(strcmp(view.getString(), "comptime length") == 0);
}
