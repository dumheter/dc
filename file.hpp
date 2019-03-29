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

#ifndef FILE_HPP__
#define FILE_HPP__

#include <string>

namespace dutil {

enum class FileError {
  UNKNOWN_ERROR = 0,
  NO_ERROR,
  CANNOT_OPEN_PATH,
  FAILED_TO_SEEK,
  FAILED_TO_READ,
  FAILED_TO_GET_POS
};

/**
 * Read file from disk and store in buffer.
 *
 * Usage:
 * 1. Create the object with a valid path.
 * 2. Check that it does not has_error() or die_if_error().
 * 3. Read from the buffer with get().
 */
class File {
 public:
  explicit File(const std::string& path);

  // error related
  bool HasError() const { return this->error != FileError::NO_ERROR; }
  std::string ErrorToString() const;

  // access
  std::string& Get() { return this->buf; }
  const std::string& Get() const { return this->buf; }

  // lookup
  size_t GetSize() const { return this->buf.size(); }
  static bool FileExists(const std::string& path);
  const std::string& path() { return path_; }

 private:
  FileError error = FileError::UNKNOWN_ERROR;
  std::string path_;
  std::string buf{};
};

}  // namespace dutil

#endif  // FILE_HPP__
