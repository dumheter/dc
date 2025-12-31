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

#include <dc/string.hpp>
#include <dc/utf.hpp>

namespace dc::utf8 {

/*
   Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   source: RFC-3629
 */

constexpr CodePoint kOctetCount1Mask = 0b1000'0000;
constexpr CodePoint kOctetCount2Mask = 0b1110'0000;
constexpr CodePoint kOctetCount3Mask = 0b1111'0000;
constexpr CodePoint kOctetCount4Mask = 0b1111'1000;
constexpr CodePoint kOctetSequenceMask = 0b1100'0000;

constexpr CodePoint kOctetCount1Value = 0b0000'0000;
constexpr CodePoint kOctetCount2Value = 0b1100'0000;
constexpr CodePoint kOctetCount3Value = 0b1110'0000;
constexpr CodePoint kOctetCount4Value = 0b1111'0000;
constexpr CodePoint kOctetSequenceValue = 0b1000'0000;

constexpr CodePoint kOctetUpperBound1 = 0x7F;
constexpr CodePoint kOctetUpperBound2 = 0x7FF;
constexpr CodePoint kOctetUpperBound3 = 0xFFFF;
constexpr CodePoint kOctetUpperBound4 = 0x10'FFFF;

void encode(CodePoint cp, String& string) {
  const bool hasOctetCount1 = cp <= kOctetUpperBound1;
  const bool hasOctetCount2 =
      (cp <= kOctetUpperBound2) && (cp > kOctetUpperBound1);
  const bool hasOctetCount3 =
      (cp <= kOctetUpperBound3) && (cp > kOctetUpperBound2);
  const bool hasOctetCount4 =
      (cp <= kOctetUpperBound4) && (cp > kOctetUpperBound3);

  const usize size = string.getSize();
  char* ptr = string.getData();

  if (hasOctetCount1) {
    string.resize(size + 1);
    ptr[size] = static_cast<char>((~kOctetCount1Mask) & cp);
  } else if (hasOctetCount2) {
    string.resize(size + 2);
    ptr[size] = static_cast<char>(kOctetCount2Value +
                                  ((~kOctetCount2Mask) & (cp >> 6)));
    ptr[size + 1] =
        static_cast<char>(kOctetSequenceValue + ((~kOctetSequenceMask) & cp));
  } else if (hasOctetCount3) {
    string.resize(size + 3);
    ptr[size] = static_cast<char>(kOctetCount3Value +
                                  ((~kOctetCount3Mask) & (cp >> 12)));
    ptr[size + 1] = static_cast<char>(kOctetSequenceValue +
                                      ((~kOctetSequenceMask) & (cp >> 6)));
    ptr[size + 2] =
        static_cast<char>(kOctetSequenceValue + ((~kOctetSequenceMask) & cp));
  } else if (hasOctetCount4) {
    string.resize(size + 4);
    ptr[size] = static_cast<char>(kOctetCount4Value +
                                  ((~kOctetCount4Mask) & (cp >> 18)));
    ptr[size + 1] = static_cast<char>(kOctetSequenceValue +
                                      ((~kOctetSequenceMask) & (cp >> 12)));
    ptr[size + 2] = static_cast<char>(kOctetSequenceValue +
                                      ((~kOctetSequenceMask) & (cp >> 6)));
    ptr[size + 3] =
        static_cast<char>(kOctetSequenceValue + ((~kOctetSequenceMask) & cp));
  }
}

CodeSize decode(const char8* data, usize offset, CodePoint& codePointOut) {
  CodeSize size;

  const bool octetCount1 =
      ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
        kOctetCount1Mask)) == kOctetCount1Value;
  const bool octetCount2 =
      ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
        kOctetCount2Mask)) == kOctetCount2Value;
  const bool octetCount3 =
      ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
        kOctetCount3Mask)) == kOctetCount3Value;
  // const bool octetCount4 = (data[offset] & kOctetCount4Mask) ==
  // kOctetCount4Value;

  if (octetCount1) {
    size = 1;
    codePointOut = static_cast<CodePoint>(static_cast<u8>(data[offset])) &
                   ((~kOctetCount1Mask) & 0xFF);
  } else if (octetCount2) {
    size = 2;
    codePointOut = ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
                     ((~kOctetCount2Mask) & 0xFF))
                    << 6) +
                   (static_cast<CodePoint>(static_cast<u8>(data[offset + 1])) &
                    ((~kOctetSequenceMask) & 0xFF));
  } else if (octetCount3) {
    size = 3;
    codePointOut = ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
                     ((~kOctetCount3Mask) & 0xFF))
                    << 12) +
                   ((static_cast<CodePoint>(static_cast<u8>(data[offset + 1])) &
                     ((~kOctetSequenceMask) & 0xFF))
                    << 6) +
                   (static_cast<CodePoint>(static_cast<u8>(data[offset + 2])) &
                    ((~kOctetSequenceMask) & 0xFF));
  } else /* if (octetCount4) */ {
    size = 4;
    codePointOut = ((static_cast<CodePoint>(static_cast<u8>(data[offset])) &
                     ((~kOctetCount4Mask) & 0xFF))
                    << 18) +
                   ((static_cast<CodePoint>(static_cast<u8>(data[offset + 1])) &
                     ((~kOctetSequenceMask) & 0xFF))
                    << 12) +
                   ((static_cast<CodePoint>(static_cast<u8>(data[offset + 2])) &
                     ((~kOctetSequenceMask) & 0xFF))
                    << 6) +
                   (static_cast<CodePoint>(static_cast<u8>(data[offset + 3])) &
                    ((~kOctetSequenceMask) & 0xFF));
  }

  return size;
}

CodeSize decode(const String& string, usize offset, CodePoint& codePointOut) {
  return decode(string.c_str(), offset, codePointOut);
}

Option<CodeSize> validate(const char8* data) {
  CodeSize size;

  const bool octetCount1 = (static_cast<CodePoint>(static_cast<u8>(data[0])) &
                            kOctetCount1Mask) == kOctetCount1Value;
  const bool octetCount2 = (static_cast<CodePoint>(static_cast<u8>(data[0])) &
                            kOctetCount2Mask) == kOctetCount2Value;
  const bool octetCount3 = (static_cast<CodePoint>(static_cast<u8>(data[0])) &
                            kOctetCount3Mask) == kOctetCount3Value;
  const bool octetCount4 = (static_cast<CodePoint>(static_cast<u8>(data[0])) &
                            kOctetCount4Mask) == kOctetCount4Value;

  if (octetCount1) {
    size = 1;
  } else if (octetCount2) {
    size = 2;
  } else if (octetCount3) {
    size = 3;
  } else if (octetCount4) {
    size = 4;
  } else {
    return None;
  }

  return Some(size);
}

}  // namespace dc::utf8
