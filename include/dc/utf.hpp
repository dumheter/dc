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

#pragma once

#include <dc/result.hpp>
#include <dc/types.hpp>

namespace dc {

class String;

namespace utf8 {

using CodePoint = u32;
using CodeSize = usize;

void encode();

/// Decode a code point from a utf-8 string.
/// @pre Must be valid utf-8.
/// @param data Must be 1-4 bytes or bigger.
CodeSize decode(const char8* data, usize offset, CodePoint& codePointOut);
CodeSize decode(const dc::String& string, usize offset,
                CodePoint& codePointOut);

/// Try to read the size of the encoded code point.
/// @retval Some<CodeSize> Size of the valid utf8 encoded code point.
/// @retval None Not a valid utf8 encoding.
Option<CodeSize> validate(const char8* data);

}  // namespace utf8

}  // namespace dc
