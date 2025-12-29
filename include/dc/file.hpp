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

#pragma once

#include <dc/list.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>
#include <dc/types.hpp>

namespace dc {

class File {
 public:
  ~File();

  enum class Mode {
    // explanation,             if file exists,       if file not exists
    //-----------------------------------------------------------------
    // open file for reading,   read from start,      failure to open
    kRead,
    // create file for writing, destroy old file,     create new
    kWrite,
    // append to file,          write to end,         create new
    kAppend,
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

  [[nodiscard]] dc::Result<dc::String, File::Result> open(
      const dc::String& path, const Mode mode);

  /// Will be called by destructor.
  void close();

  /// Read file to string.
  [[nodiscard]] dc::Result<dc::String, File::Result> read();
  [[nodiscard]] File::Result read(dc::String& stringOut);

  /// Load file to buffer.
  [[nodiscard]] dc::Result<List<u8>, File::Result> load();
  [[nodiscard]] File::Result load(List<u8>& bufferOut);

  [[nodiscard]] File::Result write(const dc::String& string);
  [[nodiscard]] File::Result write(const List<u8>& buffer);

  [[nodiscard]] static File::Result remove(const dc::String& path);

  [[nodiscard]] static File::Result rename(const dc::String& oldPath,
                                           const dc::String& newPath);

  [[nodiscard]] static dc::String resultToString(const Result result);

  /// Size of file.
  [[nodiscard]] dc::Result<s64, File::Result> getSize();

  [[nodiscard]] static bool fileExists(const dc::String& path);

  /// Get path of the latest opened file, as set by open().
  [[nodiscard]] const dc::String& getPath() const { return m_path; }

  /// Check if file is open. Note: this can only be reliably tested by
  /// performing an operation such as read.
  [[nodiscard]] bool isOpen() const { return m_file != nullptr; }

 private:
  dc::String m_path;
  void* m_file = nullptr;  // FILE*
};

}  // namespace dc
