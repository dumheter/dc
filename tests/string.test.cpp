#include <dc/dtest.hpp>
#include <dc/file.hpp>
#include <dc/string.hpp>

using namespace dc;

///////////////////////////////////////////////////////////////////////////////
// Test Util
//

static const StringView kLoremIpsum(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.");

///////////////////////////////////////////////////////////////////////////////
// Iterator
//

DTEST(utf8IteratorEndComparison) {
  const String abc = "abc";

  Utf8Iterator iterBeforeBegin(abc.c_str(), abc.getSize(), -1);
  Utf8Iterator iterBegin(abc.c_str(), abc.getSize(), 0);
  Utf8Iterator iterEnd(abc.c_str(), abc.getSize(), 3);

  ASSERT_TRUE(iterBegin != iterBeforeBegin);
  ASSERT_TRUE(iterBegin != iterEnd);
  ASSERT_TRUE(iterBeforeBegin != iterEnd);

  Utf8Iterator iterBeforeBegin2(abc.c_str(), abc.getSize(), -1);
  ASSERT_TRUE(iterBeforeBegin == iterBeforeBegin2);
}

DTEST(utf8IteratorCanIncrementToEnd) {
  const String abc = "abc";

  Utf8Iterator iter(abc.c_str(), abc.getSize(), 0);
  Utf8Iterator iterEnd(abc.c_str(), abc.getSize(), 3);

  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 'a');

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 'b');

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 'c');

  ++iter;
  ASSERT_TRUE(iter == iterEnd);
}

DTEST(utf8IteratorCanDecrementToBeforeBegin) {
  const String abc = "abc";

  Utf8Iterator iter(abc.c_str(), abc.getSize(), 2);
  Utf8Iterator iterBeforeBegin(abc.c_str(), abc.getSize(), -1);

  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 'c');

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 'b');

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 'a');

  --iter;
  ASSERT_TRUE(iter == iterBeforeBegin);
}

DTEST(utf8IteratorCanIncrementToEndWithLargeUtf8Characters) {
  String str;
  utf8::encode(0x1'F525, str);  // Fire emoji
  utf8::encode(' ', str);
  utf8::encode(0x1F68, str);  // Greek Capital Letter Omega with Psili
  utf8::encode(' ', str);
  utf8::encode(0x01B5, str);  // Latin Capital Letter Z with Stroke

  Utf8Iterator iter(str.c_str(), str.getSize(), 0);
  Utf8Iterator iterEnd(str.c_str(), str.getSize(),
                       static_cast<s64>(str.getSize()));

  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 0x1'F525);

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, ' ');

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 0x1F68);

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, ' ');

  ++iter;
  ASSERT_TRUE(iter != iterEnd);
  ASSERT_EQ(*iter, 0x01B5);

  ++iter;
  ASSERT_TRUE(iter == iterEnd);
}

DTEST(utf8IteratorCanDecrementToBeforeBeginWithLargeUtf8Characters) {
  String str;
  utf8::encode(0x1'F525, str);  // Fire emoji
  utf8::encode(' ', str);
  utf8::encode(0x1F68, str);  // Greek Capital Letter Omega with Psili
  utf8::encode(' ', str);
  utf8::encode(0x01B5, str);  // Latin Capital Letter Z with Stroke

  Utf8Iterator iter(str.c_str(), str.getSize(),
                    static_cast<s64>(str.getSize()));
  Utf8Iterator iterBeforeBegin(str.c_str(), str.getSize(), -1);

  --iter;  // we are at end, place ourself at the last utf8 character's start
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 0x01B5);

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, ' ');

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 0x1F68);

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, ' ');

  --iter;
  ASSERT_TRUE(iter != iterBeforeBegin);
  ASSERT_EQ(*iter, 0x1'F525);

  --iter;
  ASSERT_TRUE(iter == iterBeforeBegin);
}

///////////////////////////////////////////////////////////////////////////////
// StringView
//

// DTEST(stringViewCompTime) {
// 	 constexpr StringView view("comptime length");

//   ASSERT_EQ(view.getSize(), strlen("comptime length"));
//   ASSERT_TRUE(strcmp(view.c_str(), "comptime length") == 0);
// }

DTEST(stringViewRunTime) {
  String runtimeString("runtime length");
  StringView view = runtimeString.toView();

  ASSERT_EQ(runtimeString.getSize(), view.getSize());
  ASSERT_EQ(view.getSize(), strlen("runtime length"));
  ASSERT_TRUE(strcmp(view.c_str(), "runtime length") == 0);
}

DTEST(stringViewUtf8Iterator) {
  String str;

  utf8::CodePoint cps[3];
  utf8::CodePoint cp;

  str += 0xC6;
  str += 0xB5;
  utf8::decode(str, 0, cp);
  cps[0] = 0x01B5;  // Latin Captial Letter Z with Stroke.

  str += 0xE1;
  str += 0xBD;
  str += 0xA8;
  utf8::decode(str, 0, cp);
  cps[1] = 0x1F68;  // Greek Capital Letter Omega with Psili

  str += 'x';
  utf8::decode(str, 0, cp);
  cps[2] = 'x';

  int i = 0;
  for (utf8::CodePoint c : str.utf8Iterator()) {
    if (i == 0)
      ASSERT_EQ(cps[0], c);
    else if (i == 1)
      ASSERT_EQ(cps[1], c);
    else
      ASSERT_EQ(cps[2], c);

    ++i;
  }
  ASSERT_EQ(i, 3);
}

DTEST(stringViewSubString) {
  StringView view("Hello World");

  StringView sub1 = view.subString(0, 5);
  ASSERT_EQ(sub1.getSize(), 5);
  ASSERT_EQ(memcmp(sub1.c_str(), "Hello", 5), 0);

  StringView sub2 = view.subString(6, 5);
  ASSERT_EQ(sub2.getSize(), 5);
  ASSERT_EQ(memcmp(sub2.c_str(), "World", 5), 0);

  StringView sub3 = view.subString(0, 100);
  ASSERT_EQ(sub3.getSize(), view.getSize());
  ASSERT_EQ(memcmp(sub3.c_str(), "Hello World", view.getSize()), 0);
}

/////////////////////////////////////////////////////////////////////////////
// String
//

DTEST(empty) {
  String nothing("");
  ASSERT_TRUE(nothing.isEmpty());
}

DTEST(notEmpty) {
  String something("abc");
  ASSERT_FALSE(something.isEmpty());
}

DTEST(emptyAfterBeingBigString) {
  String str("123456789.123456789.123456789.123456789.123456789");
  ASSERT_FALSE(str.isEmpty());
  str = "";
  ASSERT_TRUE(str.isEmpty());
}

DTEST(clone) {
  const String original("friday");
  const String copy = original.clone();
  ASSERT_EQ(original, copy);
}

DTEST(size) {
  String msg("123");
  ASSERT_EQ(msg.getSize(), 3);

  String empty;
  ASSERT_EQ(empty.getSize(), 0);
}

DTEST(sizeWhenEmpty) {
  String msg("");
  ASSERT_EQ(msg.getSize(), 0);
}

DTEST(sizeWhenBigString) {
  const usize len = strlen("abc, abc, abc, abc, abc, abc, ");
  String str = "abc, abc, abc, abc, abc, abc, ";

  ASSERT_EQ(str.getSize(), len);
}

DTEST(subString) {
  String str("Hello World");

  String sub1 = str.subString(0, 5);
  ASSERT_EQ(sub1.getSize(), 5);
  ASSERT_EQ(sub1, "Hello");

  String sub2 = str.subString(6, 5);
  ASSERT_EQ(sub2.getSize(), 5);
  ASSERT_EQ(sub2, "World");

  String sub3 = str.subString(0, 100);
  ASSERT_EQ(sub3.getSize(), str.getSize());
  ASSERT_EQ(sub3, "Hello World");
}

DTEST(isSameAsCString) {
  const char* cstr = "abc";
  String str = "abc";

  ASSERT_TRUE(strcmp(cstr, str.c_str()) == 0);
}

DTEST(isSameAsCStringLooong) {
  const char* cstr = "abc, abc, abc, abc, abc, abc, ";
  String str = "abc, abc, abc, abc, abc, abc, ";

  ASSERT_TRUE(strcmp(cstr, str.c_str()) == 0);
}

DTEST(canIterate) {
  String str = "The quick brown fox jumps over the fence.";
  usize i = 0;
  auto iter = str.utf8Iterator();
  for (auto it = iter.begin(); it != iter.end(); ++it) {
    (void)*it;
    ++i;
  }
  ASSERT_EQ(i, str.getLength());
}

DTEST(lengthOf1CodePointString) {
  String str = "abc";
  ASSERT_EQ(3, str.getLength());
  ASSERT_EQ(3, str.getSize());
}

DTEST(lengthOfMultiCodePointString) {
  String str;
  str += 0xF0;
  str += 0x9F;
  str += 0x94;
  str += 0xA5;
  ASSERT_EQ(1, str.getLength());
  ASSERT_EQ(4, str.getSize());
}

DTEST(appendSmallToBig) {
  String str = "small";
  ASSERT_TRUE(str.getCapacity() <= detail::kCachelineMinusListBytes);
  str += " The quick brown fox jumps over the fence.";
  ASSERT_TRUE(str.getCapacity() > detail::kCachelineMinusListBytes);
}

DTEST(insertInMiddleOfString) {
  String str = "Hellx World";
  str.insert("o", 4);
  ASSERT_EQ(str, "Hello World");
}

DTEST(insertMakesStringGrow) {
  String str = "The ...";
  const usize before = str.getCapacity();
  str.insert("quick brown fox jumped over the fence.", 4);
  ASSERT_EQ(str, "The quick brown fox jumped over the fence.");
  ASSERT_TRUE(str.getCapacity() > before);
}

// DTEST(insertWithNoOverwrite) {
// 	String str = "Hellld";
// 	str.insert("o wor", 4);
// 	ASSERT_EQ("Hello world", str);
// }

DTEST(resize) {
  String str;

  for (usize i = 0; i < 100; ++i) {
    str.resize(i);
    ASSERT_EQ(str.getSize(), i);
  }

  for (usize i = 99; i > 0; --i) {
    str.resize(i);
    ASSERT_EQ(str.getSize(), i);
  }
}

/*
DTEST(findEasy) {
        const String lorem("Lorem ipsum dolor sit amet, consectetur adipisicing
elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim
ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea
commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit
esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

        Option<usize> found = lorem.find("ipsum");
        ASSERT_TRUE(found.isSome());
        ASSERT_TRUE(found.contains((usize)6));
}
*/

DTEST(appendAfterMove) {
  String str;
  str += "str";
  ASSERT_EQ(str, "str");

  String b;
  {
    String a = move(str);
    a += " a";
    ASSERT_EQ(a, "str a");

    b = move(a);
  }
  b += " b";
  ASSERT_EQ(b, "str a b");
}

DTEST(constructStringByTakingList) {
  List<char8> list;
  list.resize(kLoremIpsum.getSize() + 1);
  memcpy(list.begin(), kLoremIpsum.c_str(), kLoremIpsum.getSize() + 1);

  String str(dc::move(list));

  ASSERT_EQ(str, kLoremIpsum);
}

///////////////////////////////////////////////////////////////////////////////
// Find
//

DTEST(findBasicPattern) {
  const String text("Hello World");
  Option<u64> found = text.find("World");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findPatternAtBeginning) {
  const String text("Hello World");
  Option<u64> found = text.find("Hello");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findPatternAtEnd) {
  const String text("Hello World");
  Option<u64> found = text.find("World");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findPatternNotFound) {
  const String text("Hello World");
  Option<u64> found = text.find("Python");
  ASSERT_TRUE(found.isNone());
}

DTEST(findEmptyPattern) {
  const String text("Hello World");
  Option<u64> found = text.find("");
  ASSERT_TRUE(found.isNone());
}

DTEST(findEmptyText) {
  const String text("");
  Option<u64> found = text.find("Hello");
  ASSERT_TRUE(found.isNone());
}

DTEST(findPatternLongerThanText) {
  const String text("Hi");
  Option<u64> found = text.find("Hello World");
  ASSERT_TRUE(found.isNone());
}

DTEST(findMultipleOccurrencesReturnsFirst) {
  const String text("abcabcabc");
  Option<u64> found = text.find("abc");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findSingleCharacter) {
  const String text("Hello World");
  Option<u64> found = text.find("W");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findWithUtf8Characters) {
  String str;
  utf8::encode(0x1'F525, str);
  str += "abc";
  utf8::encode(0x1'F525, str);

  String pattern;
  utf8::encode(0x1'F525, pattern);
  pattern += "abc";

  Option<u64> found = str.find(pattern.toView());
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findUtf8PatternInMiddle) {
  String str;
  str += "Hello ";
  utf8::encode(0x1'F525, str);
  str += " World";

  String pattern;
  utf8::encode(0x1'F525, pattern);

  Option<u64> found = str.find(pattern.toView());
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findPatternNotInUtf8String) {
  String str;
  utf8::encode(0x1'F525, str);
  str += "abc";

  String pattern;
  utf8::encode(0x1'F68, pattern);

  Option<u64> found = str.find(pattern.toView());
  ASSERT_TRUE(found.isNone());
}

DTEST(findEntireString) {
  const String text("Hello World");
  Option<u64> found = text.find("Hello World");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findInLongerString) {
  const String text("The quick brown fox jumps over the lazy dog");
  Option<u64> found = text.find("fox");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(16));
}

DTEST(findOverlappingPatterns) {
  const String text("aaaaa");
  Option<u64> found = text.find("aa");
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findLongSubstringInBook) {
  dc::File book;
  auto r1 = book.Open("tests/book.txt", dc::File::Mode::kRead);
  ASSERT_EQ(r1, dc::File::Result::kSuccess);

  auto r2 = book.Read();
  ASSERT_EQ(std::get<0>(r2), dc::File::Result::kSuccess);

  const String text(std::get<1>(r2).c_str());

  const Option<u64> found = text.find(
      "gobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr ahrositenarsei "
      "iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern stoiawfyou nawfoyt "
      "unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr "
      "ahrositenarsei iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern "
      "stoiawfyou nawfoyt unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr "
      "ahrositenarsei iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern "
      "stoiawfyou nawfoyt unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr "
      "ahrositenarsei iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern "
      "stoiawfyou nawfoyt unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr "
      "ahrositenarsei iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern "
      "stoiawfyou nawfoyt unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbagabba gobbagabbagobbagabba gobbagabba ioearstoiaenr "
      "ahrositenarsei iarsneaioers ieaerisotienarsietn nwyuftnwyfuntaern "
      "stoiawfyou nawfoyt unawfoyu tnawfyuotnaowyfntyaowfn toyuant "
      "oyauwntunfwuyatnwofyntaowuyftnaowyuftaowuyfn touyaw oyutwfo "
      "uytngobbaga");

  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(177733));
}

DTEST(findShortSubstringInBook) {
  dc::File book;
  auto r1 = book.Open("tests/book.txt", dc::File::Mode::kRead);
  ASSERT_EQ(r1, dc::File::Result::kSuccess);

  auto r2 = book.Read();
  ASSERT_EQ(std::get<0>(r2), dc::File::Result::kSuccess);

  const String text(std::get<1>(r2).c_str());

  const Option<u64> found = text.find("toyuant");

  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(177908));
}

DTEST(findWithOffset) {
  const String text("Hello World Hello World");
  Option<u64> found = text.find("World", 10);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(18));
}

DTEST(findWithOffsetNotFound) {
  const String text("Hello World");
  Option<u64> found = text.find("Hello", 5);
  ASSERT_TRUE(found.isNone());
}

DTEST(findWithOffsetBeyondEnd) {
  const String text("Hello World");
  Option<u64> found = text.find("World", 20);
  ASSERT_TRUE(found.isNone());
}

DTEST(findWithOffsetZero) {
  const String text("Hello World");
  Option<u64> found = text.find("World", 0);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findWithOffsetMultipleOccurrences) {
  const String text("abcabcabc");
  Option<u64> found = text.find("abc", 3);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(3));
}

DTEST(findWithOffsetAtPatternStart) {
  const String text("Hello World");
  Option<u64> found = text.find("World", 6);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findViewWithOffset) {
  const StringView view("Hello World Hello World");
  Option<u64> found = view.find("World", 10);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(18));
}

DTEST(findViewWithOffsetNotFound) {
  const StringView view("Hello World");
  Option<u64> found = view.find("Hello", 5);
  ASSERT_TRUE(found.isNone());
}

DTEST(findViewWithOffsetBeyondEnd) {
  const StringView view("Hello World");
  Option<u64> found = view.find("World", 20);
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InString) {
  const String text("Hello World");
  Option<u64> found = text.find(static_cast<char8>('W'));
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findChar8InStringNotFound) {
  const String text("Hello World");
  Option<u64> found = text.find(static_cast<char8>('z'));
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InStringWithOffset) {
  const String text("Hello World");
  Option<u64> found = text.find(static_cast<char8>('o'), 5);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(7));
}

DTEST(findChar8InStringWithOffsetNotFound) {
  const String text("Hello World");
  Option<u64> found = text.find(static_cast<char8>('H'), 1);
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InStringWithOffsetBeyondEnd) {
  const String text("Hello World");
  Option<u64> found = text.find(static_cast<char8>('H'), 20);
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InStringView) {
  const StringView view("Hello World");
  Option<u64> found = view.find(static_cast<char8>('W'));
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(6));
}

DTEST(findChar8InStringViewNotFound) {
  const StringView view("Hello World");
  Option<u64> found = view.find(static_cast<char8>('z'));
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InStringViewWithOffset) {
  const StringView view("Hello World");
  Option<u64> found = view.find(static_cast<char8>('o'), 5);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(7));
}

DTEST(findChar8InStringViewWithOffsetNotFound) {
  const StringView view("Hello World");
  Option<u64> found = view.find(static_cast<char8>('H'), 1);
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InStringViewWithOffsetBeyondEnd) {
  const StringView view("Hello World");
  Option<u64> found = view.find(static_cast<char8>('H'), 20);
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8MultipleOccurrencesReturnsFirst) {
  const String text("aaa");
  Option<u64> found = text.find(static_cast<char8>('a'));
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(0));
}

DTEST(findChar8WithOffsetMultipleOccurrences) {
  const String text("aaa");
  Option<u64> found = text.find(static_cast<char8>('a'), 1);
  ASSERT_TRUE(found.isSome());
  ASSERT_EQ(found.value(), static_cast<u64>(1));
}

DTEST(findChar8InEmptyString) {
  const String text("");
  Option<u64> found = text.find(static_cast<char8>('a'));
  ASSERT_TRUE(found.isNone());
}

DTEST(findChar8InEmptyStringView) {
  const StringView view("");
  Option<u64> found = view.find(static_cast<char8>('a'));
  ASSERT_TRUE(found.isNone());
}
