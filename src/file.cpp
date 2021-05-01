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

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-include-dirs"
#endif

#include <cstdlib>
#include <dc/file.hpp>
#include <dc/platform.hpp>


namespace dc {

File::~File() { Close(); }

static const char* ModeToCString(const File::Mode mode) {
  switch (mode) {
    case File::Mode::kRead: {
#ifdef DC_PLATFORM_WINDOWS
      return "rb";
#endif
      return "r";
    }
    case File::Mode::kWrite: {
#ifdef DC_PLATFORM_WINDOWS
      return "wb";
#endif
      return "w";
    }
    case File::Mode::kAppend: {
#ifdef DC_PLATFORM_WINDOWS
      return "ab";
#endif
      return "a";
    }
  }

#ifdef DC_PLATFORM_WINDOWS
  return "rb";
#endif
  return "r";
}

File::Result File::Open(const std::string& path_out, const Mode mode) {
  path_ = path_out;

#if defined(__STDC_LIB_EXT1__) || defined(_WIN32)
  const errno_t err = fopen_s(&file_, path_out.c_str(), ModeToCString(mode));
  constexpr errno_t kSuccess = 0;
  return err == kSuccess ? Result::kSuccess : Result::kCannotOpenPath;
#else
  file_ = fopen(path_out.c_str(), ModeToCString(mode));
  return file_ != NULL ? Result::kSuccess : Result::kCannotOpenPath;
#endif
}

void File::Close() {
  if (file_ && file_ != NULL) {
    fclose(file_);
  }
}

template <typename TBuffer>
static File::Result ReadFromFile(std::FILE* file, TBuffer& buffer) {
  File::Result result = File::Result::kSuccess;

  if (file != nullptr) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        rewind(file);

        buffer.resize(static_cast<usize>(size + 1));  // extra for null termination
        const size_t bytes = fread(buffer.data(), 1, static_cast<usize>(size), file);
        if (bytes == static_cast<size_t>(size)) {
          buffer[static_cast<usize>(size)] = 0;  // ensure null termination
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
  return std::make_tuple<File::Result, std::string>(std::move(result),
                                                    std::move(string));
}

File::Result File::Read(std::string& string_out) {
  return ReadFromFile(file_, string_out);
}

std::tuple<File::Result, std::vector<u8>> File::Load() {
  std::vector<u8> buffer;
  File::Result result = Load(buffer);
  return std::make_tuple<File::Result, std::vector<u8>>(std::move(result),
                                                        std::move(buffer));
}

File::Result File::Load(std::vector<u8>& buffer) {
  return ReadFromFile(file_, buffer);
}

template <typename TBuffer>
static File::Result WriteToFile(std::FILE* file, const TBuffer& buffer) {
  File::Result result = File::Result::kSuccess;
  if (file != nullptr) {
    const size_t written =
        std::fwrite(buffer.data(), sizeof(typename TBuffer::value_type),
                    buffer.size(), file);
    if (written != buffer.size()) {
      result = File::Result::kWriteFailed;
    }
  } else {
    result = File::Result::kFileNotOpen;
  }

  return result;
}

File::Result File::Write(const std::string& string) {
  return WriteToFile(file_, string);
}

File::Result File::Write(const std::vector<u8>& buffer) {
  return WriteToFile(file_, buffer);
}

File::Result File::Remove(const std::string& path) {
  const int res = std::remove(path.c_str());
  constexpr int kSuccess = 0;
  return res == kSuccess ? Result::kSuccess : Result::kCannotOpenPath;
}

File::Result File::Rename(const std::string& old_path,
                          const std::string& new_path) {
  const int res = std::rename(old_path.c_str(), new_path.c_str());
  constexpr int kSuccess = 0;
  return res == kSuccess ? Result::kSuccess : Result::kFailedRename;
}

std::string File::ResultToString(const Result result) {
  std::string string;
  switch (result) {
    case Result::kSuccess: {
      string = "success";
      break;
    }
    case Result::kCannotOpenPath: {
      string = "cannot open path";
      break;
    }
    case Result::kFailedToSeek: {
      string = "failed to seek";
      break;
    }
    case Result::kFailedToRead: {
      string = "failed to read";
      break;
    }
    case Result::kFailedToGetPos: {
      string = "failed to get pos";
      break;
    }
    case Result::kUnknownError: {
      string = "unknown error";
      break;
    }
    case Result::kFileNotOpen: {
      string = "file not open";
      break;
    }
    case Result::kWriteFailed: {
      string = "write failed";
      break;
    }
    case Result::kFailedRename: {
      string = "failed rename";
      break;
    }
  }

  return string;
}

std::tuple<File::Result, long> File::GetSize() {
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

  return std::make_tuple<File::Result, long>(std::move(result),
                                             std::move(size));
}

bool File::FileExists(const std::string& path) {
  std::FILE* file;

#if defined(__STDC_LIB_EXT1__) || defined(_WIN32)
  const errno_t err = fopen_s(&file, path.c_str(), ModeToCString(Mode::kRead));
  constexpr errno_t kSuccess = 0;
  const Result res =
      err == kSuccess ? Result::kSuccess : Result::kCannotOpenPath;
#else
  file = fopen(path.c_str(), ModeToCString(Mode::kRead));
  const Result res = file != NULL ? Result::kSuccess : Result::kCannotOpenPath;
#endif

  if (file) {
    fclose(file);
  }
  return res == Result::kSuccess;
}

}  // namespace dc

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
