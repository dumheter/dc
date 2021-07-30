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

constexpr CodePoint kOctetCount1Mask = 0b1000'0000;
constexpr CodePoint kOctetCount2Mask = 0b1110'0000;
constexpr CodePoint kOctetCount3Mask = 0b1111'0000;
constexpr CodePoint kOctetCount4Mask = 0b1111'1000;
constexpr CodePoint kOctetSequenceMask = 0b1100'0000;

constexpr CodePoint kOctetCount1Value = 0b0000'0000;
constexpr CodePoint kOctetCount2Value = 0b1100'0000;
constexpr CodePoint kOctetCount3Value = 0b1110'0000;
// constexpr CodePoint kOctetCount4Value = 0b1111'0000;
// constexpr CodePoint kOctetSequenceValue = 0b1000'0000;

CodeSize decode(const u8* data, usize offset, CodePoint& codePointOut) {
  CodeSize size;

  const bool octetCount1 =
      (data[offset] & kOctetCount1Mask) == kOctetCount1Value;
  const bool octetCount2 =
      (data[offset] & kOctetCount2Mask) == kOctetCount2Value;
  const bool octetCount3 =
      (data[offset] & kOctetCount3Mask) == kOctetCount3Value;
  // const bool octetCount4 = (data[offset] & kOctetCount4Mask) ==
  // kOctetCount4Value;

  if (octetCount1) {
    size = 1;
    codePointOut = data[offset] & (~kOctetCount1Mask);
  } else if (octetCount2) {
    size = 2;
    codePointOut = ((data[offset] & (~kOctetCount2Mask)) << 6) +
                   (data[offset + 1] & (~kOctetSequenceMask));
  } else if (octetCount3) {
    size = 3;
    codePointOut = ((data[offset] & (~kOctetCount3Mask)) << 12) +
                   ((data[offset + 1] & (~kOctetSequenceMask)) << 6) +
                   (data[offset + 2] & (~kOctetSequenceMask));
  } else /* if (octetCount4) */ {
    size = 4;
    codePointOut = ((data[offset] & (~kOctetCount4Mask)) << 18) +
                   ((data[offset + 1] & (~kOctetSequenceMask)) << 12) +
                   ((data[offset + 2] & (~kOctetSequenceMask)) << 6) +
                   (data[offset + 3] & (~kOctetSequenceMask));
  }

  return size;
}

CodeSize decode(const String& string, usize offset, CodePoint& codePointOut) {
  return decode(string.getData(), offset, codePointOut);
}

}  // namespace dc::utf8
