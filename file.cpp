/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
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

#include "file.hpp"
#include <cstdlib>

#if defined(_MSC_VER)
// allow us to use fopen on windows without warning
#pragma warning(disable : 4996)
#endif

namespace dutil {

File::File(const std::string& path) : path_(path) {
  FILE* file = fopen(path.c_str(), "rb");
  if (file) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        buf_.resize(size);

        rewind(file);
        const size_t bytes = fread(buf_.data(), 1, size, file);
        if (bytes == static_cast<size_t>(size)) {
          buf_[size] = 0;  // ensure null termination
          error_ = FileError::kNoError;
        } else {
          error_ = FileError::kFailedToRead;
        }
      } else {
        error_ = FileError::kFailedToGetPos;
      }
    } else {
      error_ = FileError::kFailedToSeek;
    }
  } else {
    error_ = FileError::kCannotOpenPath;
  }

  if (file) {
    fclose(file);
  }
}

std::string File::ErrorToString() const {
  switch (error_) {
    case FileError::kNoError: {
      return "no error";
    }
    case FileError::kCannotOpenPath: {
      return "cannot open path";
    }
    case FileError::kFailedToSeek: {
      return "failed to seek";
    }
    case FileError::kFailedToRead: {
      return "failed to read";
    }
    case FileError::kFailedToGetPos: {
      return "failed to get pos";
    }
    case FileError::kUnknownError: {
      return "unknown error";
    }
  }

  return "unknown error";  // have to repeat myself
}

bool File::FileExists(const std::string& path) {
  FILE* file = fopen(path.c_str(), "rb");
  bool exists = file;
  if (file) {
    fclose(file);
  }
  return exists;
}

}  // namespace dutil
