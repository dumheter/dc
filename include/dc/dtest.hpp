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

#pragma once

#include <dc/assert.hpp>
#include <dc/log.hpp>
#include <dc/macros.hpp>
#include <dc/math.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>

///////////////////////////////////////////////////////////////////////////////
// Quick Start
//

/// 1. Put `DTEST_RUN();` in your main() function.
///
/// 2. Create a test file, such as 'integer.test.cpp'.
///
/// 3. Include dtest, `#include <dtest/dtest.hpp>`
///
/// 4. Write your test, example:
///    ```cpp
///   DTEST(integer_test)
///   {
///      ASSERT_TRUE(1 == 1);
///      ASSERT_NE(1, 2);
///   }
///   ```

///////////////////////////////////////////////////////////////////////////////
// Macros
//

/// Run all registered tests.
#define DTEST_RUN()                                                   \
  [](int dtestArgc, char** dtestArgv) {                               \
    dc::log::windowsFixConsole();                                     \
    dc::log::init();                                                  \
    const auto res = dtest::internal::runTests(dtestArgc, dtestArgv); \
    dc::log::deinit();                                                \
    return res;                                                       \
  }(argc, argv)

/// Register a new test.
#define DTEST(testName) DTEST_REGISTER(testName, DC_FILENAME, __FILE__)

/// Asserts
#define ASSERT_TRUE(expr) ASSERT_TRUE_IMPL(expr, __LINE__)
#define ASSERT_FALSE(expr) ASSERT_FALSE_IMPL(expr, __LINE__)
#define ASSERT_EQ(a, b) ASSERT_EQ_IMPL(a, b, __LINE__)
#define ASSERT_NE(a, b) ASSERT_NE_IMPL(a, b, __LINE__)

namespace dtest {

///////////////////////////////////////////////////////////////////////////////
// Helpers
//

struct LifetimeStats {
  int moves = 0;
  int copies = 0;
  int constructs = 0;
  int destructs = 0;

  static LifetimeStats& getInstance();
  static void resetInstance();
};

/// Track the lifetime of any object, stores the data in an external
/// LifetimeStats struct.
///
/// Example copies
///  dtest::LifetimeStats::resetInstance();
///  dtest::LifetimeStats& stats = dtest::LifetimeStats::getInstance();
///  dtest::LifetimeTracker<int> parent = 13;
///  dtest::LifetimeTracker<int> child1 = parent;
///  dtest::LifetimeTracker<int> child2 = child1;
///  ASSERT(stats.copies == 2);
///
/// Example moves
///  dtest::LifetimeStats::resetInstance();
///  dtest::LifetimeStats& stats = dtest::LifetimeStats::getInstance();
///  dtest::LifetimeTracker<int> parent = 13;
///  dtest::LifetimeTracker<int> child1 = dc::move(parent);
///  dtest::LifetimeTracker<int> child2 = dc::move(child1);
///  ASSERT(stats.moves == 2);
///
template <typename T>
struct LifetimeTracker {
  LifetimeTracker() : LifetimeTracker(T()) {}

  LifetimeTracker(T&& obj) : object(dc::forward<T>(obj)) {
    LifetimeStats& stats = LifetimeStats::getInstance();
    ++stats.constructs;
  }

  LifetimeTracker(const LifetimeTracker& other) : object(other.object) {
    LifetimeStats& stats = LifetimeStats::getInstance();
    ++stats.constructs;
    ++stats.copies;
  }

  LifetimeTracker& operator=(const LifetimeTracker& other) {
    LifetimeStats& stats = LifetimeStats::getInstance();
    object = other.object;
    ++stats.copies;
    return *this;
  }

  LifetimeTracker(LifetimeTracker&& other) noexcept
      : object(dc::move(other.object)) {
    LifetimeStats& stats = LifetimeStats::getInstance();
    ++stats.constructs;
    ++stats.moves;
  }

  LifetimeTracker& operator=(LifetimeTracker&& other) noexcept {
    if (&other != this) {
      LifetimeStats& stats = LifetimeStats::getInstance();
      object = dc::move(other.object);
      ++stats.moves;
    }
    return *this;
  }

  ~LifetimeTracker() {
    LifetimeStats& stats = LifetimeStats::getInstance();
    ++stats.destructs;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(
      const LifetimeTracker<U>& other) const noexcept {
    static_assert(dc::isEqualityComparable<T, U>);
    return object == other.object;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(
      const LifetimeTracker<U>& other) const noexcept {
    static_assert(dc::isEqualityComparable<T, U>);
    return object != other.object;
  }

  [[nodiscard]] constexpr bool operator==(const T& other) const {
    return object == other;
  }

  [[nodiscard]] constexpr bool operator!=(const T& other) const {
    return object != other;
  }

 public:
  T object;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class [[nodiscard]] NoCopy {
 public:
  NoCopy() = default;
  NoCopy(T object) : m_object(object) {}
  DC_DEFAULT_MOVE(NoCopy);
  DC_DELETE_COPY(NoCopy);

  [[nodiscard]] constexpr T& get() { return m_object; }
  [[nodiscard]] constexpr const T& get() const { return m_object; }

  [[nodiscard]] constexpr bool operator==(const NoCopy& other) const {
    return get() == other.get();
  }

  [[nodiscard]] constexpr bool operator!=(const NoCopy& other) const {
    return get() != other.get();
  }

  [[nodiscard]] constexpr bool operator==(const T& other) const {
    return get() == other;
  }

  [[nodiscard]] constexpr bool operator!=(const T& other) const {
    return get() != other;
  }

 private:
  T m_object;
};

}  // namespace dtest

///////////////////////////////////////////////////////////////////////////////
// Internal
//

namespace dtest::internal {

struct TestBodyState {
  const char* name = nullptr;
  int pass = 0;
  int fail = 0;
};

bool isSilentMode();

using TestFunction = std::function<void(TestBodyState&)>;

struct TestCase {
  TestBodyState state;
  TestFunction fn;
};

struct TestCategory {
  const char* name = nullptr;
  std::vector<TestCase> tests;
  int pass = 0;
  int fail = 0;
};

class Register {
 public:
  Register() = default;
  DC_DELETE_COPY(Register);

  void addTest(TestFunction fn, const char* testName, const char* fileName,
               u64 filePathHash);

  const std::unordered_map<u64, TestCategory>& getTestCategories() const {
    return m_testCategories;
  }

  std::unordered_map<u64, TestCategory>& getTestCategories() {
    return m_testCategories;
  }

 private:
  std::unordered_map<u64, TestCategory> m_testCategories;
};

/// Returnes a static instantitation of Register.
Register& getRegister();

int runTests(int argc, char** argv);

template <typename Fn>
void dtestAdd(Fn&& fn, const char* testName, const char* fileName,
              u64 filePathHash) {
  getRegister().addTest(dc::forward<Fn>(fn), testName, fileName, filePathHash);
}

constexpr const char* kFallbackFormatString = "<cannot format type>";
template <typename T>
dc::String formatOrFallback(const T&) {
  // TODO cgustafsson: have a better fallback formatter, format the
  // raw bytes?
  return dc::String(kFallbackFormatString);
}

template <typename T>
dc::String formatOrFallback(
    const typename dc::EnableIf<dc::isFundamental<T>, T>::Type& value) {
  return dc::format("{}", value).unwrapOr(dc::String(kFallbackFormatString));
}

template <>
dc::String formatOrFallback(const dc::String& value);

template <>
dc::String formatOrFallback<>(const dc::StringView& value);

dc::String formatOrFallback(const char* value);

}  // namespace dtest::internal

#define DTEST_STRINGIFY(expr) #expr

#define DTEST_MAKE_CLASS_NAME(testName, line) \
  DTEST_MAKE_CLASS_NAME_IMPL(testName, line)
#define DTEST_MAKE_CLASS_NAME_IMPL(testName, line) DTest_##testName##line##_

#define DTEST_MAKE_VAR_NAME(testName, line) \
  DTEST_MAKE_VAR_NAME_IMPL(testName, line)
#define DTEST_MAKE_VAR_NAME_IMPL(testName, line) dtest_##testName##line##_

#define DTEST_REGISTER(testName, fileName, filePath) \
  DTEST_REGISTER_AUX(testName, fileName, filePath)

#define DTEST_REGISTER_AUX(testName, fileName, filePath)        \
  static_assert(sizeof(DTEST_STRINGIFY(testName)) > 1,          \
                "Test names cannot be empty.");                 \
  struct DTEST_MAKE_CLASS_NAME(testName, __LINE__) {            \
    void testBody(dtest::internal::TestBodyState&               \
                      dtestBodyState__you_must_have_an_assert); \
    DTEST_MAKE_CLASS_NAME(testName, __LINE__)() {               \
      dtest::internal::dtestAdd(                                \
          [this](dtest::internal::TestBodyState&                \
                     dtestBodyState__you_must_have_an_assert) { \
            testBody(dtestBodyState__you_must_have_an_assert);  \
          },                                                    \
          #testName, fileName, dc::hash64fnv1a(filePath));      \
    }                                                           \
  };                                                            \
  DTEST_MAKE_CLASS_NAME(testName, __LINE__)                     \
  DTEST_MAKE_VAR_NAME(testName, __LINE__);                      \
  void DTEST_MAKE_CLASS_NAME(testName, __LINE__)::testBody(     \
      dtest::internal::TestBodyState& dtestBodyState__you_must_have_an_assert)

///////////////////////////////////////////////////////////////////////////////
// Assert Macro Impl
//

#define ASSERT_TRUE_IMPL(expr, line)                                       \
  do {                                                                     \
    if (!!(expr)) {                                                        \
      ++dtestBodyState__you_must_have_an_assert.pass;                      \
    } else {                                                               \
      ++dtestBodyState__you_must_have_an_assert.fail;                      \
      LOG_INFO("\t\t- Assert:{} true " #expr " {}", line,                  \
               dc::log::Paint<20>("failed", dc::log::Color::Red).c_str()); \
      const auto exprFmt = dtest::internal::formatOrFallback(!!(expr));    \
      dc::details::debugBreak();                                           \
      return;                                                              \
    }                                                                      \
  } while (0)

#define ASSERT_FALSE_IMPL(expr, line)                                      \
  do {                                                                     \
    if (!(expr)) {                                                         \
      ++dtestBodyState__you_must_have_an_assert.pass;                      \
    } else {                                                               \
      ++dtestBodyState__you_must_have_an_assert.fail;                      \
      LOG_INFO("\t\t- Assert:{} false " #expr " {}", line,                 \
               dc::log::Paint<20>("failed", dc::log::Color::Red).c_str()); \
      const auto exprFmt = dtest::internal::formatOrFallback(!!(expr));    \
      LOG_INFO("\t\t- Actual value: {}", exprFmt);                         \
      dc::details::debugBreak();                                           \
      return;                                                              \
    }                                                                      \
  } while (0)

#define ASSERT_EQ_IMPL(a, b, line)                                         \
  do {                                                                     \
    if ((a) == (b)) {                                                      \
      ++dtestBodyState__you_must_have_an_assert.pass;                      \
    } else {                                                               \
      ++dtestBodyState__you_must_have_an_assert.fail;                      \
      LOG_INFO("\t\t- Assert:{} " #a " == " #b " {}", line,                \
               dc::log::Paint<20>("failed", dc::log::Color::Red).c_str()); \
      const auto lhs = dtest::internal::formatOrFallback(a);               \
      const auto rhs = dtest::internal::formatOrFallback(b);               \
      LOG_INFO("\t\t- Actual values: {} == {}", lhs, rhs);                 \
      dc::details::debugBreak();                                           \
      return;                                                              \
    }                                                                      \
  } while (0)

#define ASSERT_NE_IMPL(a, b, line)                                         \
  do {                                                                     \
    if ((a) != (b)) {                                                      \
      ++dtestBodyState__you_must_have_an_assert.pass;                      \
    } else {                                                               \
      ++dtestBodyState__you_must_have_an_assert.fail;                      \
      LOG_INFO("\t\t- Assert:{} " #a " != " #b " {}", line,                \
               dc::log::Paint<20>("failed", dc::log::Color::Red).c_str()); \
      const auto lhs = dtest::internal::formatOrFallback(a);               \
      const auto rhs = dtest::internal::formatOrFallback(b);               \
      LOG_INFO("\t\t- Actual values: {} != {}", lhs, rhs);                 \
      dc::details::debugBreak();                                           \
      return;                                                              \
    }                                                                      \
  } while (0)
