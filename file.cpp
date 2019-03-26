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

#include "file.hpp"

#include <cstdlib>
#include <iostream>

#if defined(_MSC_VER)
// allow us to use fopen on windows
#pragma warning(disable : 4996)
#endif

namespace dutil
{

File::File(const std::string& path)
    : path_(path)
{
  FILE* file = fopen(path.c_str(), "rb");
  if (file) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {

      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        this->buf.resize(size);

        rewind(file);
        const size_t bytes = fread(this->buf.data(), 1, size, file);
        if (bytes == static_cast<size_t>(size)) {
          this->buf[size] = 0; // ensure null termination
          this->error = FileError::NO_ERROR;
        }
        else {
          this->error = FileError::FAILED_TO_READ;
        }
      }
      else {
        this->error = FileError::FAILED_TO_GET_POS;
      }
    }
    else {
      this->error = FileError::FAILED_TO_SEEK;
    }
  }
  else {
    this->error = FileError::CANNOT_OPEN_PATH;
  }

  if (file) {
    fclose(file);
  }
}

std::string File::error_to_string() const
{
  switch (this->error)
  {
    case FileError::NO_ERROR: {
      return "NO_ERROR";
    }
    case FileError::CANNOT_OPEN_PATH: {
      return "CANNOT_OPEN_PATH";
    }
    case FileError::FAILED_TO_SEEK: {
      return "FAILED_TO_SEEK";
    }
    case FileError::FAILED_TO_READ: {
      return "FAILED_TO_READ";
    }
    case FileError::FAILED_TO_GET_POS: {
      return "FAILED_TO_GET_POS";
    }
    case FileError::UNKNOWN_ERROR: {
      return "UNKNOWN_ERROR";
    }
  }

  return "UNKNOWN_ERROR"; // have to repeat myself
}

void File::die_if_error(const std::string& path) const
{
  if (this->has_error()) {
    std::cerr << "failed to read file [" << path << "] with error ["
              << this->error_to_string() << "].\n";
    std::exit(1);
  }
}

bool File::file_exists(const std::string& path)
{
  FILE* file = fopen(path.c_str(), "rb");
  bool exists = file;
  if (file) { fclose(file); }
  return exists;
}

}
