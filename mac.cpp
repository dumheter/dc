/**
 * MIT License
 *
 * Copyright (c) 2018 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mac.hpp"

namespace dutil
{
  struct byte_to_nibble
  {
    u8 first : 4, second : 4;
  };

  char u8tohexc(u8 val, bool lsb)
  {
    constexpr char table[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    auto nibble = reinterpret_cast<byte_to_nibble*>(&val);
    char hexc;
    if (lsb) hexc = table[nibble->first];
    else     hexc = table[nibble->second];
    return hexc;
  }

  std::string mac_to_string(const u8* mac)
  {
    std::string str{};
    for (int i=0; i<MAC_SIZE; i++) {
      str += u8tohexc(mac[i], false);
      str += u8tohexc(mac[i], true);
      if (i != MAC_SIZE - 1) str += ":";
    }
    return str;
  }

  std::string mac_to_string(uint_mac mac)
  {
    u8* macptr = reinterpret_cast<u8*>(&mac);
    return mac_to_string(macptr);
  }
}
