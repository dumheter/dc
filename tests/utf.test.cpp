#include <dc/dtest.hpp>
#include <dc/string.hpp>
#include <dc/utf.hpp>

DTEST(decode1) {
  dc::String str = "x";
  dc::CodePoint cp;
  dc::CodeSize size = dc::decode(str, 0, cp);
  ASSERT_EQ(cp, 'x');
  ASSERT_EQ(size, 1);
}

DTEST(decode2) {
  dc::String str;
  str += 0xC6;
  str += 0xB5;
  dc::CodePoint cp;
  dc::CodeSize size = dc::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x01B5);  // Latin Captial Letter Z with Stroke.
  ASSERT_EQ(size, 2);
}

DTEST(decode3) {
  dc::String str;
  str += 0xE1;
  str += 0xBD;
  str += 0xA8;
  dc::CodePoint cp;
  dc::CodeSize size = dc::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x1F68);  // Greek Capital Letter Omega with Psili
  ASSERT_EQ(size, 3);
}

DTEST(decode4) {
  dc::String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;
  dc::CodePoint cp;
  dc::CodeSize size = dc::decode(str, 0, cp);
  ASSERT_EQ(cp, 0x1'F525);  // Fire emoji
  ASSERT_EQ(size, 4);
}
