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

#include <cstdio>
#include <string>
#include <vector>
#include <tuple>
#include "types.hpp"

namespace dutil {

class File {
 public:

  ~File();

  enum class Mode {
    //explanation,             if file exists,       if file not exists
    //-----------------------------------------------------------------
    //open file for reading,   read from start,      failure to open
    Read,
    //create file for writing, destroy old file,     create new
    Write,
    //append to file,          write to end,         create new
    Append,
  };

  enum class Result {
    kUnknownError = 0,
    kSuccess,
    kCannotOpenPath,
    kFailedToSeek,
    kFailedToRead,
    kFailedToGetPos,
    kFileNotOpen,
    kWriteFailed,
    kFailedRename
  };

  Result Open(const std::string& path, const Mode mode);

  /**
   * Will be called by destructor.
   */
  void Close();

  /**
   * Read file to string.
   */
  std::tuple<Result, std::string> Read();
  Result Read(std::string& string_out);

  /**
   * Load file to buffer.
   */
  std::tuple<Result, std::vector<u8>> Load();
  Result Load(std::vector<u8>& buffer_out);

  Result Write(const std::string& string);
  Result Write(const std::vector<u8>& buffer);

  static Result Remove(const std::string& path);

  static Result Rename(const std::string& old_path,
                       const std::string& new_path);

  static std::string
  ResultToString(const Result result);

  /**
   * Size of file.
   */
  std::tuple<Result, long> GetSize();

  static bool FileExists(const std::string& path);

  /**
   * Get path of the latest opened file, as set by Open().
   */
  const std::string& path() const { return path_; }

 private:
  std::string path_;
  std::FILE* file_;
};

}  // namespace dutil

#endif  // DUTIL_FILE_HPP_
