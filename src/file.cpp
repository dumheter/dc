#include <cstdio>
#include <cstdlib>
#include <dc/file.hpp>
#include <dc/platform.hpp>
#include <dc/traits.hpp>

namespace dc {

File::~File() { close(); }

static const char8* modeToCString(const File::Mode mode) {
  switch (mode) {
    case File::Mode::kRead: {
      return "rb";
    }
    case File::Mode::kWrite: {
      return "wb";
    }
    case File::Mode::kAppend: {
      return "ab";
    }
  }

  return "rb";
}

dc::Result<dc::String, File::Result> File::open(const dc::String& path,
                                                const Mode mode) {
  m_path = path.clone();

#if defined(__STDC_LIB_EXT1__) || defined(_WIN32)
  FILE* file = nullptr;
  const errno_t err = fopen_s(&file, path.c_str(), modeToCString(mode));
  constexpr errno_t kSuccess = 0;
  if (err == kSuccess) {
    m_file = file;
    return Ok<dc::String>(m_path.clone());
  } else {
    return Err<File::Result>(File::Result::kCannotOpenPath);
  }
#else
  m_file = fopen(path.c_str(), modeToCString(mode));
  if (m_file != nullptr) {
    return Ok<dc::String>(m_path.clone());
  } else {
    return Err<File::Result>(File::Result::kCannotOpenPath);
  }
#endif
}

void File::close() {
  if (m_file != nullptr) {
    fclose(static_cast<FILE*>(m_file));
    m_file = nullptr;
  }
}

File::Result readFromFileString(FILE* file, dc::String& buffer) {
  File::Result result = File::Result::kSuccess;

  if (file != nullptr) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        rewind(file);

        buffer.resize(static_cast<usize>(size));
        const size_t bytes =
            fread(buffer.getData(), 1, static_cast<usize>(size), file);
        if (bytes != static_cast<size_t>(size)) {
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

File::Result readFromFileList(FILE* file, List<u8>& buffer) {
  File::Result result = File::Result::kSuccess;

  if (file != nullptr) {
    int res = fseek(file, 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      const long size = ftell(file);
      constexpr long FTELL_FAIL = -1L;
      if (size != FTELL_FAIL) {
        rewind(file);

        buffer.resize(static_cast<usize>(size));
        const size_t bytes =
            fread(buffer.begin(), 1, static_cast<usize>(size), file);
        if (bytes != static_cast<size_t>(size)) {
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

dc::Result<dc::String, File::Result> File::read() {
  dc::String string{};
  File::Result result = read(string);
  if (result == File::Result::kSuccess) {
    return Ok<dc::String>(dc::move(string));
  } else {
    return Err<File::Result>(result);
  }
}

File::Result File::read(dc::String& stringOut) {
  return readFromFileString(static_cast<FILE*>(m_file), stringOut);
}

dc::Result<List<u8>, File::Result> File::load() {
  List<u8> buffer;
  File::Result result = load(buffer);
  if (result == File::Result::kSuccess) {
    return Ok<List<u8>>(dc::move(buffer));
  } else {
    return Err<File::Result>(result);
  }
}

File::Result File::load(List<u8>& bufferOut) {
  return readFromFileList(static_cast<FILE*>(m_file), bufferOut);
}

File::Result writeToFileString(FILE* file, const dc::String& buffer) {
  File::Result result = File::Result::kSuccess;
  if (file != nullptr) {
    const size_t written =
        std::fwrite(buffer.getData(), sizeof(char8), buffer.getSize(), file);
    if (written != buffer.getSize()) {
      result = File::Result::kWriteFailed;
    }
  } else {
    result = File::Result::kFileNotOpen;
  }

  return result;
}

File::Result writeToFileList(FILE* file, const List<u8>& buffer) {
  File::Result result = File::Result::kSuccess;
  if (file != nullptr) {
    const size_t written =
        std::fwrite(buffer.begin(), sizeof(u8), buffer.getSize(), file);
    if (written != buffer.getSize()) {
      result = File::Result::kWriteFailed;
    }
  } else {
    result = File::Result::kFileNotOpen;
  }

  return result;
}

File::Result File::write(const dc::String& string) {
  return writeToFileString(static_cast<FILE*>(m_file), string);
}

File::Result File::write(const List<u8>& buffer) {
  return writeToFileList(static_cast<FILE*>(m_file), buffer);
}

File::Result File::remove(const dc::String& path) {
  const int res = std::remove(path.c_str());
  constexpr int kSuccess = 0;
  return res == kSuccess ? File::Result::kSuccess
                         : File::Result::kCannotOpenPath;
}

File::Result File::rename(const dc::String& oldPath,
                          const dc::String& newPath) {
  const int res = std::rename(oldPath.c_str(), newPath.c_str());
  constexpr int kSuccess = 0;
  return res == kSuccess ? File::Result::kSuccess : File::Result::kFailedRename;
}

dc::String File::resultToString(const Result result) {
  switch (result) {
    case Result::kSuccess: {
      return dc::String("success");
    }
    case Result::kCannotOpenPath: {
      return dc::String("cannot open path");
    }
    case Result::kFailedToSeek: {
      return dc::String("failed to seek");
    }
    case Result::kFailedToRead: {
      return dc::String("failed to read");
    }
    case Result::kFailedToGetPos: {
      return dc::String("failed to get pos");
    }
    case Result::kUnknownError: {
      return dc::String("unknown error");
    }
    case Result::kFileNotOpen: {
      return dc::String("file not open");
    }
    case Result::kWriteFailed: {
      return dc::String("write failed");
    }
    case Result::kFailedRename: {
      return dc::String("failed rename");
    }
  }

  return dc::String("unknown error");
}

dc::Result<s64, File::Result> File::getSize() {
  s64 size = 0;
  Result result = Result::kSuccess;

  if (m_file != nullptr) {
    int res = fseek(static_cast<FILE*>(m_file), 0, SEEK_END);
    constexpr int SEEK_SUCCESS = 0;
    if (res == SEEK_SUCCESS) {
      size = ftell((static_cast<FILE*>(m_file)));
      constexpr long FTELL_FAIL = -1L;
      if (size == FTELL_FAIL) {
        result = Result::kFailedToGetPos;
      } else {
        rewind(static_cast<FILE*>(m_file));
      }
    } else {
      result = Result::kFailedToSeek;
    }
  } else {
    result = Result::kFileNotOpen;
  }

  if (result == Result::kSuccess) {
    return Ok<s64>(size);
  } else {
    return Err<File::Result>(result);
  }
}

bool File::fileExists(const dc::String& path) {
  FILE* file;

#if defined(__STDC_LIB_EXT1__) || defined(_WIN32)
  const errno_t err = fopen_s(&file, path.c_str(), modeToCString(Mode::kRead));
  constexpr errno_t kSuccess = 0;
  const Result res =
      err == kSuccess ? Result::kSuccess : Result::kCannotOpenPath;
#else
  file = fopen(path.c_str(), modeToCString(Mode::kRead));
  const Result res =
      file != nullptr ? Result::kSuccess : Result::kCannotOpenPath;
#endif

  if (file != nullptr) {
    fclose(file);
  }
  return res == Result::kSuccess;
}

}  // namespace dc

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
