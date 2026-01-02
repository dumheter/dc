/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
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

#include <dc/assert.hpp>
#include <dc/dtest.hpp>
#include <dc/time.hpp>

#if defined(_MSC_VER)
#if !defined(MEAN_AND_LEAN)
#define MEAN_AND_LEAN
#endif
#if !defined(NO_MIN_MAX)
#define NO_MIN_MAX
#endif
#include <Windows.h>
#endif

namespace dtest {

LifetimeStats& LifetimeStats::getInstance() {
  static LifetimeStats instance{};
  return instance;
}

void LifetimeStats::resetInstance() {
  LifetimeStats& instance = getInstance();
  instance.moves = 0;
  instance.copies = 0;
  instance.constructs = 0;
  instance.destructs = 0;
}

namespace internal {

using Paint = dc::log::Paint<100>;
using Color = dc::log::Color;

static bool g_silentMode = false;
static dc::String g_filterPattern;

bool isSilentMode() { return g_silentMode; }

static bool matchesPattern(const dc::StringView testFullName,
                           const dc::StringView pattern) {
  const usize testLen = testFullName.getSize();
  const usize patternLen = pattern.getSize();

  usize testIdx = 0;
  usize patternIdx = 0;

  while (patternIdx < patternLen) {
    if (pattern[patternIdx] == '*') {
      ++patternIdx;

      while (patternIdx < patternLen && pattern[patternIdx] == '*') {
        ++patternIdx;
      }

      if (patternIdx == patternLen) {
        return true;
      }

      usize nextStar = patternIdx;
      while (nextStar < patternLen && pattern[nextStar] != '*') {
        ++nextStar;
      }

      const usize subPatternLen = nextStar - patternIdx;

      bool found = false;
      while (testIdx <= testLen) {
        if (testIdx + subPatternLen <= testLen) {
          if (memcmp(testFullName.c_str() + testIdx,
                     pattern.c_str() + patternIdx, subPatternLen) == 0) {
            found = true;
            testIdx += subPatternLen;
            break;
          }
        }
        ++testIdx;
      }

      if (!found) {
        return false;
      }

      patternIdx = nextStar;
    } else {
      if (testIdx >= testLen) {
        return false;
      }

      if (pattern[patternIdx] != testFullName[testIdx]) {
        return false;
      }

      ++patternIdx;
      ++testIdx;
    }
  }

  return testIdx == testLen;
}

void Register::addTest(TestFunction fn, const char* testName,
                       const char* fileName, u64 filePathHash) {
  TestCategory& category = m_testCategories[filePathHash];
  category.name = fileName;
  TestBodyState bodyState;
  bodyState.name = testName;
  TestCase testCase;
  testCase.state = dc::move(bodyState);
  testCase.fn = dc::move(fn);
  category.tests.push_back(dc::move(testCase));
}

Register& getRegister() {
  static Register r{};
  return r;
}

static inline void FixConsole() {
#if defined(_MSC_VER)
  // Set console encoding
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  // Enable virtual terminal processing
  const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(out, &mode);
  SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

static void listTests() {
  Register& r = getRegister();
  for (const auto& [hash, category] : r.getTestCategories()) {
    LOG_RAW("{}.\n", category.name);
    for (const auto& test : category.tests) {
      LOG_RAW("  {}\n", test.state.name);
    }
  }
}

int runTests(int argc, char** argv) {
  FixConsole();

  for (int i = 1; i < argc; ++i) {
    dc::StringView arg(argv[i]);
    if (arg == "-s" || arg == "--silent") {
      g_silentMode = true;
    } else if (arg == "-l" || arg == "--list-tests" ||
               arg == "--gtest_list_tests") {
      listTests();
      return 0;
    } else if (memcmp(arg.c_str(), "-f=", 3) == 0 ||
               memcmp(arg.c_str(), "--filter=", 9) == 0 ||
               memcmp(arg.c_str(), "--gtest_filter=", 15) == 0) {
      const char* equals = strchr(argv[i], '=');
      if (equals) {
        g_filterPattern = dc::String(equals + 1);
      }
    }
  }

  Register& r = getRegister();

  const bool filterActive = !g_filterPattern.isEmpty();

  LOG_INFO("~~~ D T E S T ~~~");
  LOG_INFO("Found {} test files.",
           static_cast<int>(r.getTestCategories().size()));

  dc::Stopwatch stopwatch;

  usize testCount = 0;
  usize assertCount = 0;
  int warnings = 0;
  for (auto& [hash, category] : r.getTestCategories()) {
    usize categoryTestsRan = 0;
    const u64 catBefore = dc::getTimeUs();
    int i = 0;
    for (TestCase& test : category.tests) {
      dc::String fullName = dc::format("{}.{}", category.name, test.state.name);

      if (filterActive &&
          !matchesPattern(fullName.toView(), g_filterPattern.toView())) {
        ++i;
        continue;
      }

      if (categoryTestsRan == 0 && !g_silentMode) {
        LOG_INFO(
            "------------------------------------------------------------------"
            "---"
            "-");
        LOG_INFO("{}, running matched tests.",
                 Paint(category.name, Color::Magenta).c_str());
      }

      if (!g_silentMode) {
        LOG_INFO("\t{} {} ...... ", i,
                 Paint(test.state.name, i % 2 == 0 ? Color::Blue : Color::Teal)
                     .c_str());
      }
      const u64 testBefore = dc::getTimeUs();
      test.fn(test.state);
      const u64 testAfter = dc::getTimeUs();
      category.fail += (test.state.fail > 0);
      if (test.state.fail == 0) category.pass++;
      assertCount += test.state.pass + test.state.fail;
      if (test.state.pass + test.state.fail == 0) {
        LOG_INFO("\t\t{}",
                 Paint("Warning, no assert ran.", Color::BrightYellow).c_str());
        ++warnings;
      }
      ++testCount;
      ++categoryTestsRan;
      if (!g_silentMode || test.state.fail > 0) {
        LOG_INFO("\t{} {} {} in {:.6f}s, {} asserts.", i,
                 Paint(test.state.name, i % 2 == 0 ? Color::Blue : Color::Teal)
                     .c_str(),
                 !test.state.fail ? Paint("PASSED", Color::Green).c_str()
                                  : Paint("FAILED", Color::Red).c_str(),
                 (testAfter - testBefore) / 1'000'000.f, test.state.pass);
      }
      ++i;
    }
    const u64 catAfter = dc::getTimeUs();

    if (categoryTestsRan > 0) {
      if (!g_silentMode || category.fail > 0) {
        LOG_INFO("{} {} in {:.6f}s ({} tests ran)",
                 Paint(category.name, Color::Magenta).c_str(),
                 !category.fail ? Paint("PASSED", Color::Green).c_str()
                                : Paint("FAILED", Color::Red).c_str(),
                 (catAfter - catBefore) / 1'000'000.f,
                 static_cast<int>(categoryTestsRan));
      }
    }
  }

  stopwatch.stop();

  LOG_INFO(
      "----------------------------------------------------------------------");
  LOG_INFO("SUMMARY:\t(ran {} tests containing {} asserts in {:.9f}s)",
           testCount, assertCount, stopwatch.fs());

  int failedCategories = 0;
  for (const auto& [_, category] : r.getTestCategories()) {
    if (category.fail) {
      failedCategories += category.fail;
      LOG_INFO("{}: {} with {}/{} failed tests.",
               Paint("FAILED", Color::Red).c_str(), category.name,
               category.fail, category.fail + category.pass);
    }
  }
  if (testCount == 0)
    LOG_INFO("{}!", Paint("NO TESTS RAN", Color::BrightYellow).c_str());
  else if (failedCategories == 0)
    LOG_INFO("ALL {}!", Paint("PASSED", Color::Green).c_str());
  if (warnings)
    LOG_INFO("With {} {}", warnings,
             Paint("warning(s)", Color::BrightYellow).c_str());

  return failedCategories;
}

}  // namespace internal

}  // namespace dtest
