#include <dc/dtest.hpp>
#include <dc/string.hpp>
#include <dc/utf.hpp>

using namespace dc;

DTEST(encode1) {
  String str;
  utf8::encode('x', str);

  String correct;
  correct += 'x';
  ASSERT_EQ(str, correct);
  ASSERT_EQ(str.getSize(), 1);
}

DTEST(encode2) {
  String str;
  utf8::encode(0x01B5, str);  // Latin Capital Letter Z with Stroke.

  String correct;
  correct += 0xC6;
  correct += 0xB5;
  ASSERT_EQ(str, correct);
  ASSERT_EQ(str.getSize(), 2);
}

DTEST(encode3) {
  String str;
  utf8::encode(0x2605, str);  // Black Star

  String correct;
  correct += 0xE2;
  correct += 0x98;
  correct += 0x85;
  ASSERT_EQ(str, correct);
  ASSERT_EQ(str.getSize(), 3);
}

DTEST(encode4) {
  String str;
  utf8::encode(0x1'F525, str);  // Fire emoji

  String correct;
  correct += 0xF0;
  correct += 0x9F;
  correct += 0x94;
  correct += 0xA5;
  ASSERT_EQ(str, correct);
  ASSERT_EQ(str.getSize(), 4);
}

DTEST(decode1) {
  String str = "x";
  utf8::CodePoint cp;
  utf8::CodeSize size = utf8::decode(str, 0, cp);
  ASSERT_EQ(cp, 'x');
  ASSERT_EQ(size, 1);
}

DTEST(decode2) {
  String str;
  str += 0xC6;
  str += 0xB5;
  utf8::CodePoint cp;
  utf8::CodeSize size = utf8::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x01B5);  // Latin Captial Letter Z with Stroke.
  ASSERT_EQ(size, 2);
}

DTEST(decode3) {
  String str;
  str += 0xE1;
  str += 0xBD;
  str += 0xA8;
  utf8::CodePoint cp;
  utf8::CodeSize size = utf8::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x1F68);  // Greek Capital Letter Omega with Psili
  ASSERT_EQ(size, 3);
}

DTEST(decode4) {
  String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;
  utf8::CodePoint cp;
  utf8::CodeSize size = utf8::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x1'F525);  // Fire emoji
  ASSERT_EQ(size, 4);
}

DTEST(validateOnValidUtf8) {
  String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;
  Option<utf8::CodeSize> size = utf8::validate(str.c_str());
  ASSERT_TRUE(size.isSome());
  ASSERT_TRUE(size.contains((utf8::CodeSize)4));
}

DTEST(validateOnInvalidUtf8) {
  String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;

  Option<utf8::CodeSize> size1 = utf8::validate(str.c_str() + 1);
  ASSERT_TRUE(size1.isNone());

  Option<utf8::CodeSize> size2 = utf8::validate(str.c_str() + 2);
  ASSERT_TRUE(size2.isNone());

  Option<utf8::CodeSize> size3 = utf8::validate(str.c_str() + 3);
  ASSERT_TRUE(size3.isNone());
}
