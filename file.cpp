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

File::~File() {
  Close();
}

File::Result File::Open(const std::string& path_out) {
  path_ = path_out;
  file_ = fopen(path_out.c_str(), "rb");
  return file_ != NULL ? File::Result::kSuccess : File::Result::kCannotOpenPath;
}

void File::Close() {
  if (file_ && file_ != NULL) {
    fclose(file_);
  }
}

template <typename TBuffer>
static File::Result ReadFile(FILE* file, TBuffer& buffer) {
  File::Result result = File::Result::kSuccess;

  if (file != NULL) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        rewind(file);

        buffer.resize(size + 1);  // extra for null termination
        const size_t bytes = fread(buffer.data(), 1, size, file);
        if (bytes == static_cast<size_t>(size)) {
          buffer[size] = 0;  // ensure null termination
        } else {
          result = File::Result::kFailedToRead;
        }
      } else {
        result = File::Result::kFailedToGetPos;
      }
    } else {
      result = File::Result::kFailedToSeek;
    }
  } else {
    result = File::Result::kFileNotOpen;
  }

  return result;
}

std::tuple<File::Result, std::string> File::Read() {
  std::string string{};
  File::Result result = Read(string);
  return std::make_tuple<File::Result, std::string>(std::move(result), std::move(string));
}

File::Result File::Read(std::string& string_out) {
  return ReadFile(file_, string_out);
}

std::tuple<File::Result, std::vector<u8>> File::Load() {
  std::vector<u8> buffer;
  File::Result result = Load(buffer);
  return std::make_tuple<File::Result, std::vector<u8>>(std::move(result), std::move(buffer));
}

File::Result File::Load(std::vector<u8>& buffer) {
  return ReadFile(file_, buffer);
}

std::string File::ResultToString(const Result result) {
  std::string string;
  switch (result) {
    case Result::kSuccess: {
      string = "success"; break;
    }
    case Result::kCannotOpenPath: {
      string = "cannot open path"; break;
    }
    case Result::kFailedToSeek: {
      string = "failed to seek"; break;
    }
    case Result::kFailedToRead: {
      string = "failed to read"; break;
    }
    case Result::kFailedToGetPos: {
      string = "failed to get pos"; break;
    }
    case Result::kUnknownError: {
      string = "unknown error"; break;
    }
    case Result::kFileNotOpen: {
      string = "file not open"; break;
    }
  }

  return string;
}

std::tuple<File::Result, long> File::GetSize() const {
  long size = 0;
  Result result = Result::kSuccess;

  if (file_ && file_ != NULL) {
    int res = fseek(file_, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      size = ftell(file_);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        rewind(file_);
      } else {
        result = Result::kFailedToGetPos;
      }
    } else {
      result = Result::kFailedToSeek;
    }
  } else {
    result = Result::kFileNotOpen;
  }

  return std::make_tuple<File::Result, long>(std::move(result), std::move(size));
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
