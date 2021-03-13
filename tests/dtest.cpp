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

#include "dtest.hpp"

#include <cstdio>
#include <dutil/assert.hpp>
#include <dutil/stopwatch.hpp>

#if defined(_MSC_VER)
#if !defined(MEAN_AND_LEAN)
#define MEAN_AND_LEAN
#endif
#if !defined(NO_MIN_MAX)
#define NO_MIN_MAX
#endif
#include <Windows.h>
#endif

namespace dtest::details {

void Register::addTest(TestFunction fn, const char* testName,
                       const char* fileName, u64 filePathHash) {
  TestCategory& category = m_testCategories[filePathHash];
  category.name = fileName;
  category.tests.push_back({{testName, 0, 0}, std::move(fn)});
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

void runTests() {
  FixConsole();
  Register& r = getRegister();

  printf("___|_ D T E S T _|___\nRunning %d test categories.\n",
         static_cast<int>(r.getTestCategories().size()));

  dutil::Stopwatch stopwatch;

  size_t testCount = 0;
  int warnings = 0;
  for (auto& [_, category] : r.getTestCategories()) {
    testCount += category.tests.size();
    printf(
        "----------------------------------------------------------------------"
        "\n=== %s, running %d tests.\n",
        category.name, static_cast<int>(category.tests.size()));

    int i = 0;
    for (TestCase& test : category.tests) {
      printf("\t=%d= %s, \n", i, test.state.name);
      test.fn(test.state);
      category.fail += (test.state.fail > 0);
      if (test.state.pass + test.state.fail == 0) {
        printf(
            "\t\t%s\n",
            Paint("Warning, no asserts in test.", Color::BrightYellow).c_str());
        ++warnings;
      }
      printf("\t=%d= %s, %s\n", i++, test.state.name,
             !test.state.fail ? Paint("PASSED", Color::Green).c_str()
                              : Paint("FAILED", Color::Red).c_str());
    }

    printf("=== %s, %s\n", category.name,
           !category.fail ? Paint("PASSED", Color::Green).c_str()
                          : Paint("FAILED", Color::Red).c_str());
  }

  stopwatch.Stop();

  printf(
      "\n--------------------------------------------------------------------"
      "--"
      "\nSUMMARY:\t(ran %zu tests in %.3fs )\n",
      testCount, stopwatch.fs());

  int failedCategories = 0;
  for (const auto& [_, category] : r.getTestCategories()) {
    if (category.fail) {
      failedCategories += category.fail;
      printf("%s: %s with %d/%d failed tests.\n",
             Paint("FAILED", Color::Red).c_str(), category.name, category.fail,
             category.fail + category.pass);
    }
  }
  if (failedCategories == 0)
    printf("ALL %s!\n", Paint("PASSED", Color::Green).c_str());
  if (warnings)
    printf("With %i %s\n", warnings,
           Paint("warning(s)", Color::BrightYellow).c_str());
}

Paint::Paint(const char* str, Color color) {
  DUTIL_ASSERT(strlen(str) < Paint::kStrLen,
               "Trying to paint a too large string.");
  const auto res = snprintf(m_str, kStrLen, "\033[%dm%s\033[0m",
                            static_cast<ColorType>(color), str);
  DUTIL_ASSERT(res >= 0, "Failed to copy string");
}

const char* Paint::c_str() const { return m_str; }

}  // namespace dtest::details
