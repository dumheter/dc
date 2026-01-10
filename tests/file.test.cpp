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

#include <cstring>
#include <dc/dtest.hpp>
#include <dc/file.hpp>
#include <dc/fmt.hpp>
#include <dc/string.hpp>
#include <dc/time.hpp>

using namespace dc;

static dc::String generateTestFileName() {
  u64 ns = getTimeNs();
  return dc::format("testfile_{}.txt", ns);
}

DTEST(fileWriteAndReadString) {
  const dc::String testContent("Hello, World!", TEST_ALLOCATOR);
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  File readFile;
  auto openReadResult = readFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openReadResult.isOk());

  auto readResult = readFile.read();
  ASSERT_TRUE(readResult.isOk());
  ASSERT_EQ(readResult.value(), testContent);

  readFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileWriteAndReadBuffer) {
  List<u8> testBuffer;
  testBuffer.add('H');
  testBuffer.add('e');
  testBuffer.add('l');
  testBuffer.add('l');
  testBuffer.add('o');
  testBuffer.add('\0');
  testBuffer.add('W');
  testBuffer.add('o');
  testBuffer.add('r');
  testBuffer.add('l');
  testBuffer.add('d');
  testBuffer.add('\0');

  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(testBuffer);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  File loadFile;
  auto openLoadResult = loadFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openLoadResult.isOk());

  auto loadResult = loadFile.load();
  ASSERT_TRUE(loadResult.isOk());
  ASSERT_EQ(loadResult.value().getSize(), testBuffer.getSize());
  ASSERT_EQ(memcmp(loadResult.value().begin(), testBuffer.begin(),
                   testBuffer.getSize()),
            0);

  loadFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileAppendMode) {
  const dc::String testFileName = generateTestFileName();
  const dc::String firstContent("First line\n", TEST_ALLOCATOR);
  const dc::String secondContent("Second line\n", TEST_ALLOCATOR);

  File writeFile;
  auto openWriteResult = writeFile.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openWriteResult.isOk());

  auto firstWriteResult = writeFile.write(firstContent);
  ASSERT_EQ(firstWriteResult, File::Result::kSuccess);

  writeFile.close();

  File appendFile;
  auto openAppendResult = appendFile.open(testFileName, File::Mode::kAppend);
  ASSERT_TRUE(openAppendResult.isOk());

  auto secondWriteResult = appendFile.write(secondContent);
  ASSERT_EQ(secondWriteResult, File::Result::kSuccess);

  appendFile.close();

  File readFile;
  auto openReadResult = readFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openReadResult.isOk());

  auto readResult = readFile.read();
  ASSERT_TRUE(readResult.isOk());

  dc::String combined(firstContent.clone());
  combined += secondContent;
  ASSERT_EQ(readResult.value(), combined);

  readFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileGetSize) {
  const dc::String testContent("This is a test string for checking file size.",
                               TEST_ALLOCATOR);
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  auto sizeResult = file.getSize();
  ASSERT_TRUE(sizeResult.isOk());
  ASSERT_EQ(sizeResult.value(), static_cast<s64>(testContent.getSize()));

  file.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileRename) {
  const dc::String oldFileName = generateTestFileName();
  const dc::String newFileName = generateTestFileName();
  const dc::String testContent("Content to rename", TEST_ALLOCATOR);

  File file;
  auto openResult = file.open(oldFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  auto renameResult = File::rename(oldFileName, newFileName);
  ASSERT_EQ(renameResult, File::Result::kSuccess);

  ASSERT_TRUE(File::fileExists(newFileName));
  ASSERT_FALSE(File::fileExists(oldFileName));

  File renamedFile;
  auto openRenamedResult = renamedFile.open(newFileName, File::Mode::kRead);
  ASSERT_TRUE(openRenamedResult.isOk());

  auto readResult = renamedFile.read();
  ASSERT_TRUE(readResult.isOk());
  ASSERT_EQ(readResult.value(), testContent);

  renamedFile.close();

  auto removeResult = File::remove(newFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(fileExists) {
  const dc::String testFileName = generateTestFileName();
  const dc::String testContent("Test content", TEST_ALLOCATOR);

  ASSERT_FALSE(File::fileExists(testFileName));

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  ASSERT_TRUE(File::fileExists(testFileName));

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);

  ASSERT_FALSE(File::fileExists(testFileName));
}

DTEST(fileOpenNonExistent) {
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openResult.isErr());
  ASSERT_EQ(openResult.errValue(), File::Result::kCannotOpenPath);
}

DTEST(fileRemoveNonExistent) {
  const dc::String testFileName = generateTestFileName();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kCannotOpenPath);
}

DTEST(renameNonExistentFile) {
  const dc::String oldFileName = generateTestFileName();
  const dc::String newFileName = generateTestFileName();

  auto renameResult = File::rename(oldFileName, newFileName);
  ASSERT_EQ(renameResult, File::Result::kFailedRename);
}

DTEST(writeAndReadEmptyFile) {
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(dc::String("", TEST_ALLOCATOR));
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  File readFile;
  auto openReadResult = readFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openReadResult.isOk());

  auto readResult = readFile.read();
  ASSERT_TRUE(readResult.isOk());
  ASSERT_TRUE(readResult.value().isEmpty());

  auto sizeResult = readFile.getSize();
  ASSERT_TRUE(sizeResult.isOk());
  ASSERT_EQ(sizeResult.value(), 0L);

  readFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(readWithReference) {
  const dc::String testContent("Test content", TEST_ALLOCATOR);
  const dc::String testFileName = generateTestFileName();

  File writeFile;
  auto openWriteResult = writeFile.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openWriteResult.isOk());

  auto writeResult = writeFile.write(testContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  writeFile.close();

  File readFile;
  auto openReadResult = readFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openReadResult.isOk());

  dc::String readContent(TEST_ALLOCATOR);
  auto readResult = readFile.read(readContent);
  ASSERT_EQ(readResult, File::Result::kSuccess);
  ASSERT_EQ(readContent, testContent);

  readFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(loadWithReference) {
  List<u8> testBuffer;
  testBuffer.add(0x00);
  testBuffer.add(0x01);
  testBuffer.add(0x02);
  testBuffer.add(0x03);
  testBuffer.add(0x04);

  const dc::String testFileName = generateTestFileName();

  File writeFile;
  auto openWriteResult = writeFile.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openWriteResult.isOk());

  auto writeResult = writeFile.write(testBuffer);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  writeFile.close();

  File loadFile;
  auto openLoadResult = loadFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openLoadResult.isOk());

  List<u8> loadBuffer;
  auto loadResult = loadFile.load(loadBuffer);
  ASSERT_EQ(loadResult, File::Result::kSuccess);
  ASSERT_EQ(loadBuffer.getSize(), testBuffer.getSize());
  ASSERT_EQ(
      memcmp(loadBuffer.begin(), testBuffer.begin(), testBuffer.getSize()), 0);

  loadFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(filePathTracking) {
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  ASSERT_EQ(file.getPath(), testFileName);
  ASSERT_TRUE(file.isOpen());

  file.close();

  ASSERT_EQ(file.getPath(), testFileName);
  ASSERT_FALSE(file.isOpen());

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}

DTEST(resultToString) {
  ASSERT_EQ(File::resultToString(File::Result::kSuccess),
            dc::String("success", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kCannotOpenPath),
            dc::String("cannot open path", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kFailedToSeek),
            dc::String("failed to seek", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kFailedToRead),
            dc::String("failed to read", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kFailedToGetPos),
            dc::String("failed to get pos", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kUnknownError),
            dc::String("unknown error", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kFileNotOpen),
            dc::String("file not open", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kWriteFailed),
            dc::String("write failed", TEST_ALLOCATOR));
  ASSERT_EQ(File::resultToString(File::Result::kFailedRename),
            dc::String("failed rename", TEST_ALLOCATOR));
}

DTEST(writeLargeContent) {
  dc::String largeContent(TEST_ALLOCATOR);
  for (int i = 0; i < 10000; ++i) {
    largeContent += dc::format("Line {}\n", i);
  }
  const dc::String testFileName = generateTestFileName();

  File file;
  auto openResult = file.open(testFileName, File::Mode::kWrite);
  ASSERT_TRUE(openResult.isOk());

  auto writeResult = file.write(largeContent);
  ASSERT_EQ(writeResult, File::Result::kSuccess);

  file.close();

  File readFile;
  auto openReadResult = readFile.open(testFileName, File::Mode::kRead);
  ASSERT_TRUE(openReadResult.isOk());

  auto readResult = readFile.read();
  ASSERT_TRUE(readResult.isOk());
  ASSERT_EQ(readResult.value(), largeContent);

  readFile.close();

  auto removeResult = File::remove(testFileName);
  ASSERT_EQ(removeResult, File::Result::kSuccess);
}
