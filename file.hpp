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

#ifndef DUTIL_FILE_HPP_
#define DUTIL_FILE_HPP_

#include <string>

namespace dutil {

enum class FileError {
  kUnknownError = 0,
  kNoError,
  kCannotOpenPath,
  kFailedToSeek,
  kFailedToRead,
  kFailedToGetPos
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
  bool HasError() const { return error_ != FileError::kNoError; }
  std::string ErrorToString() const;

  // access
  std::string& Get() { return buf_; }
  const std::string& Get() const { return buf_; }

  // lookup
  size_t GetSize() const { return buf_.size(); }
  static bool FileExists(const std::string& path);
  const std::string& path() { return path_; }

 private:
  FileError error_ = FileError::kUnknownError;
  std::string path_;
  std::string buf_{};
};

}  // namespace dutil

#endif  // DUTIL_FILE_HPP_
