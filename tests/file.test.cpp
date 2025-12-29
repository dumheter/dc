/**
 * MIT License
 *
 * Copyright (c) 2025 Christoffer Gustafsson
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

#include <dc/dtest.hpp>
#include <dc/file.hpp>
#include <dc/time.hpp>

using namespace dc;

static std::string generateTestFileName() {
  u64 ns = getTimeNs();
  return "testfile_" + std::to_string(ns) + ".txt";
}

DTEST(fileWriteAndReadString) {
  const std::string testContent = "Hello, World!";
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  File readFile;
  auto openReadResult = readFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openReadResult, File::Result::kSuccess);

  auto [readResult, readContent] = readFile.Read();
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, testContent);

  readFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileWriteAndReadBuffer) {
  std::vector<u8> testBuffer = {'H', 'e', 'l', 'l', 'o', '\0',
                                'W', 'o', 'r', 'l', 'd', '\0'};
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(testBuffer);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  File loadFile;
  auto openLoadResult = loadFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openLoadResult, File::Result::kSuccess);

  auto [loadResult, loadBuffer] = loadFile.Load();
  ASSERT_EQ(loadResult, File::Result::kSuccess);
  ASSERT_EQ(loadBuffer.size(), testBuffer.size());
  ASSERT_EQ(memcmp(loadBuffer.data(), testBuffer.data(), testBuffer.size()), 0);

  loadFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileAppendMode) {
  const std::string testFileName = generateTestFileName();
  const std::string firstContent = "First line\n";
  const std::string secondContent = "Second line\n";

  File writeFile;
  auto openWriteResult = writeFile.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openWriteResult, File::Result::kSuccess);

  auto firstWriteResult = writeFile.Write(firstContent);
  ASSERT_EQ(firstWriteResult, File::Result::kSuccess);

  writeFile.Close();

  File appendFile;
  auto openAppendResult = appendFile.Open(testFileName, File::Mode::kAppend);
  ASSERT_EQ(openAppendResult, File::Result::kSuccess);

  auto secondWriteResult = appendFile.Write(secondContent);
  ASSERT_EQ(secondWriteResult, File::Result::kSuccess);

  appendFile.Close();

  File readFile;
  auto openReadResult = readFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openReadResult, File::Result::kSuccess);

  auto [readResult, readContent] = readFile.Read();
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, firstContent + secondContent);

  readFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileGetSize) {
  const std::string testContent =
      "This is a test string for checking file size.";
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  auto [sizeResult, size] = file.GetSize();
  ASSERT_EQ(sizeResult, File::Result::kSuccess);
  ASSERT_EQ(static_cast<usize>(size), testContent.size());

  file.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileRename) {
  const std::string oldFileName = generateTestFileName();
  const std::string newFileName = generateTestFileName();
  const std::string testContent = "Content to rename";

  File file;
  auto openResult = file.Open(oldFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  auto renameResult = File::Rename(oldFileName, newFileName);
  ASSERT_EQ(renameResult, File::Result::kSuccess);

  ASSERT_TRUE(File::FileExists(newFileName));
  ASSERT_FALSE(File::FileExists(oldFileName));

  File renamedFile;
  auto openRenamedResult = renamedFile.Open(newFileName, File::Mode::kRead);
  ASSERT_EQ(openRenamedResult, File::Result::kSuccess);

  auto [readResult, readContent] = renamedFile.Read();
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, testContent);

  renamedFile.Close();

  auto removeResult = File::Remove(newFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileExists) {
  const std::string testFileName = generateTestFileName();
  const std::string testContent = "Test content";

  ASSERT_FALSE(File::FileExists(testFileName));

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  ASSERT_TRUE(File::FileExists(testFileName));

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);

  ASSERT_FALSE(File::FileExists(testFileName));
}

DTEST(fileOpenNonExistent) {
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openResult, File::Result::kCannotOpenPath);
}

DTEST(fileRemoveNonExistent) {
  const std::string testFileName = generateTestFileName();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kCannotOpenPath);
}

DTEST(renameNonExistentFile) {
  const std::string oldFileName = generateTestFileName();
  const std::string newFileName = generateTestFileName();

  auto renameResult = File::Rename(oldFileName, newFileName);
  ASSERT_EQ(renameResult, File::Result::kFailedRename);
}

DTEST(writeAndReadEmptyFile) {
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write("");
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  File readFile;
  auto openReadResult = readFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openReadResult, File::Result::kSuccess);

  auto [readResult, readContent] = readFile.Read();
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_TRUE(readContent.empty());

  auto [sizeResult, size] = readFile.GetSize();
  ASSERT_EQ(sizeResult, File::Result::kSuccess);
  ASSERT_EQ(size, 0L);

  readFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(readWithReference) {
  const std::string testContent = "Test content";
  const std::string testFileName = generateTestFileName();

  File writeFile;
  auto openWriteResult = writeFile.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openWriteResult, File::Result::kSuccess);

  auto writeResult = writeFile.Write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  writeFile.Close();

  File readFile;
  auto openReadResult = readFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openReadResult, File::Result::kSuccess);

  std::string readContent;
  auto readResult = readFile.Read(readContent);
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, testContent);

  readFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(loadWithReference) {
  std::vector<u8> testBuffer = {0x00, 0x01, 0x02, 0x03, 0x04};
  const std::string testFileName = generateTestFileName();

  File writeFile;
  auto openWriteResult = writeFile.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openWriteResult, File::Result::kSuccess);

  auto writeResult = writeFile.Write(testBuffer);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  writeFile.Close();

  File loadFile;
  auto openLoadResult = loadFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openLoadResult, File::Result::kSuccess);

  std::vector<u8> loadBuffer;
  auto loadResult = loadFile.Load(loadBuffer);
  ASSERT_EQ(loadResult, File::Result::kSuccess);
  ASSERT_EQ(loadBuffer.size(), testBuffer.size());
  ASSERT_EQ(memcmp(loadBuffer.data(), testBuffer.data(), testBuffer.size()), 0);

  loadFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(filePathTracking) {
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  ASSERT_EQ(file.path(), testFileName);
  ASSERT_TRUE(file.IsOpen());

  file.Close();

  ASSERT_EQ(file.path(), testFileName);
  ASSERT_FALSE(file.IsOpen());

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(resultToString) {
  ASSERT_EQ(File::ResultToString(File::Result::kSuccess), "success");
  ASSERT_EQ(File::ResultToString(File::Result::kCannotOpenPath),
            "cannot open path");
  ASSERT_EQ(File::ResultToString(File::Result::kFailedToSeek),
            "failed to seek");
  ASSERT_EQ(File::ResultToString(File::Result::kFailedToRead),
            "failed to read");
  ASSERT_EQ(File::ResultToString(File::Result::kFailedToGetPos),
            "failed to get pos");
  ASSERT_EQ(File::ResultToString(File::Result::kUnknownError), "unknown error");
  ASSERT_EQ(File::ResultToString(File::Result::kFileNotOpen), "file not open");
  ASSERT_EQ(File::ResultToString(File::Result::kWriteFailed), "write failed");
  ASSERT_EQ(File::ResultToString(File::Result::kFailedRename), "failed rename");
}

DTEST(writeLargeContent) {
  std::string largeContent;
  for (int i = 0; i < 10000; ++i) {
    largeContent += "Line " + std::to_string(i) + "\n";
  }
  const std::string testFileName = generateTestFileName();

  File file;
  auto openResult = file.Open(testFileName, File::Mode::kWrite);
  ASSERT_EQ(openResult, File::Result::kSuccess);

  auto writeResult = file.Write(largeContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.Close();

  File readFile;
  auto openReadResult = readFile.Open(testFileName, File::Mode::kRead);
  ASSERT_EQ(openReadResult, File::Result::kSuccess);

  auto [readResult, readContent] = readFile.Read();
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, largeContent);

  readFile.Close();

  auto removeResult = File::Remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}
