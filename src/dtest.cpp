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

#include <cstdio>
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

namespace dtest::internal {

using Paint = dc::log::Paint<100>;
using Color = dc::log::Color;

void Register::addTest(TestFunction fn, const char* testName,
                       const char* fileName, u64 filePathHash) {
  TestCategory& category = m_testCategories[filePathHash];
  category.name = fileName;
  TestBodyState bodyState;
  bodyState.name = testName;
  TestCase testCase;
  testCase.state = std::move(bodyState);
  testCase.fn = std::move(fn);
  category.tests.push_back(std::move(testCase));
}

void Register::addVip(u64 filePathHash) {
  m_vipCategories.emplace(filePathHash);
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

int runTests() {
  FixConsole();
  Register& r = getRegister();

  const bool vipActive = r.hasVipCategories();
  LOG_INFO("___|_ D T E S T _|___");
  LOG_INFO("{}Running {} test categories.", vipActive ? "! VIP Active ! " : "",
           vipActive ? static_cast<int>(r.vipCount())
                     : static_cast<int>(r.getTestCategories().size()));

  dc::Stopwatch stopwatch;

  usize testCount = 0;
  usize assertCount = 0;
  int warnings = 0;
  for (auto& [hash, category] : r.getTestCategories()) {
    if (vipActive && !r.containsVipCategory(hash)) continue;

    testCount += category.tests.size();
    LOG_INFO(
        "---------------------------------------------------------------------"
        "-");
    LOG_INFO("{}, running {} tests.",
             Paint(category.name, Color::Magenta).c_str(),
             static_cast<int>(category.tests.size()));

    const u64 catBefore = dc::getTimeUs();
    int i = 0;
    for (TestCase& test : category.tests) {
      LOG_INFO("\t{} {} ...... ", i,
               Paint(test.state.name, i % 2 == 0 ? Color::Blue : Color::Teal)
                   .c_str());
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
      LOG_INFO("\t{} {} {} in {:.6f}s, {} asserts.", i,
               Paint(test.state.name, i % 2 == 0 ? Color::Blue : Color::Teal)
                   .c_str(),
               !test.state.fail ? Paint("PASSED", Color::Green).c_str()
                                : Paint("FAILED", Color::Red).c_str(),
               (testAfter - testBefore) / 1'000'000.f, test.state.pass);
      ++i;
    }
    const u64 catAfter = dc::getTimeUs();

    LOG_INFO("{} {} in {:.6f}s", Paint(category.name, Color::Magenta).c_str(),
             !category.fail ? Paint("PASSED", Color::Green).c_str()
                            : Paint("FAILED", Color::Red).c_str(),
             (catAfter - catBefore) / 1'000'000.f);
  }

  stopwatch.stop();

  LOG_INFO(
      "----------------------------------------------------------------------");
  LOG_INFO("SUMMARY:\t(ran {} tests containing {} asserts in {:.9f}s )",
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
  if (failedCategories == 0)
    LOG_INFO("ALL {}!", Paint("PASSED", Color::Green).c_str());
  if (warnings)
    LOG_INFO("With {} {}", warnings,
             Paint("warning(s)", Color::BrightYellow).c_str());

  return failedCategories;
}

}  // namespace dtest::internal
