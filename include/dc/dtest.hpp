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

#include <dc/core.hpp>
#include <dc/log.hpp>
#include <dc/math.hpp>
#include <dc/traits.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>

// ========================================================================== //
// Quick Start
// ========================================================================== //

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
///      DASSERT_TRUE(1 == 1);
///      DASSERT_NE(1, 2);
///   }
///   ```

// ========================================================================== //
// Macros
// ========================================================================== //

/// Run all registered tests.
#define DTEST_RUN()                               \
  [] {                                            \
    dc::log::windowsFixConsole();                 \
    dc::log::init();                              \
    const auto res = dtest::internal::runTests(); \
    dc::log::deinit();                            \
    return res;                                   \
  }()

/// Register a new test.
#define DTEST(testName) DTEST_REGISTER(testName, DC_FILENAME, __FILE__)

/// Asserts
#define DASSERT_TRUE(expr) DASSERT_TRUE_IMPL(expr, __LINE__)
#define DASSERT_FALSE(expr) DASSERT_FALSE_IMPL(expr, __LINE__)
#define DASSERT_EQ(a, b) DASSERT_EQ_IMPL(a, b, __LINE__)
#define DASSERT_NE(a, b) DASSERT_NE_IMPL(a, b, __LINE__)

/// Mark a test file as VIP, if a VIP is marked, only it and other VIP's will
/// run.
#define DTEST_VIP DTEST_MARK_CATEGORY_VIP(__FILE__)

namespace dtest {

// ========================================================================== //
// HELPERS
// ========================================================================== //

/// Track the lifetime of any object, with lifetime meaning:
///    - track COPIES made of the original.
///    - track MOVES  made of the original.
///
/// For lifetime tracking to work properly, the parent object may not leave
/// scope before the last child is done.
///
/// Examples
///  dtest::TrackLifetime<int> parent = 13;
///  dtest::TrackLifetime<int> child1 = parent;
///  dtest::TrackLifetime<int> child2 = child1;
///  ASSERT(parent.getCopies() == 2);
///
///  dtest::TrackLifetime<int> parent = 13;
///  dtest::TrackLifetime<int> child1 = std::move(parent);
///  dtest::TrackLifetime<int> child2 = std::move(child1);
///  ASSERT(parent.getMoves() == 2);
///
template <typename T>
class [[nodiscard]] TrackLifetime {
 public:
  TrackLifetime(T object) : m_object(std::move(object)) {}

  /// Amount of times this object has been copied.
  [[nodiscard]] constexpr int getCopies() const noexcept { return m_copies; }

  /// Amount of times this object has been moved.
  [[nodiscard]] constexpr int getMoves() const noexcept { return m_moves; }

  [[nodiscard]] constexpr const T& getObject() const { return m_object; }
  [[nodiscard]] constexpr T& getObject() { return m_object; }

  [[nodiscard]] constexpr int getDestructs() const { return m_destructs; }

  TrackLifetime(const TrackLifetime& other)
      : m_object(other.m_object), m_parent(other.m_parent) {
    getParent()->m_copies++;
  }

  TrackLifetime& operator=(const TrackLifetime& other) {
    m_object = other.m_object;
    getParent()->m_copies++;
    return *this;
  }

  TrackLifetime(TrackLifetime&& other)
      : m_object(other.m_object), m_parent(other.getParent()) {
    getParent()->m_moves++;
  }

  TrackLifetime& operator=(TrackLifetime&& other) noexcept {
    m_object = other.m_object;
    getParent()->m_moves++;
    return *this;
  }

  ~TrackLifetime() { getParent()->m_destructs++; }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(
      const TrackLifetime<U>& other) const noexcept {
    static_assert(dc::isEqualityComparable<T, U>);
    return getObject() == other.getObject();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(
      const TrackLifetime<U>& other) const noexcept {
    static_assert(dc::isEqualityComparable<T, U>);
    return getObject() != other.getObject();
  }

  [[nodiscard]] constexpr bool operator==(const T& other) const {
    return m_object == other;
  }

  [[nodiscard]] constexpr bool operator!=(const T& other) const {
    return m_object != other;
  }

 private:
  TrackLifetime<T>* getParent() {
    if (m_parent) return m_parent;
    return &(*this);
  }

 private:
  T m_object;
  TrackLifetime<T>* m_parent = this;
  mutable int m_copies = 0;
  int m_moves = 0;
  int m_destructs = 0;
};

// ========================================================================== //

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

// ========================================================================== //
// INTERNAL
// ========================================================================== //

namespace dtest::internal {

struct TestBodyState {
  const char* name = nullptr;
  int pass = 0;
  int fail = 0;
};

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

  void addVip(u64 filePathHash);

  bool hasVipCategories() const { return !m_vipCategories.empty(); }

  bool containsVipCategory(u64 vip) const {
    return m_vipCategories.count(vip) > 0;
  }

  std::unordered_set<u64>::size_type vipCount() const {
    return m_vipCategories.size();
  }

  const std::unordered_map<u64, TestCategory>& getTestCategories() const {
    return m_testCategories;
  }

  std::unordered_map<u64, TestCategory>& getTestCategories() {
    return m_testCategories;
  }

 private:
  std::unordered_map<u64, TestCategory> m_testCategories;
  std::unordered_set<u64> m_vipCategories;
};

/// Returnes a static instantitation of Register.
Register& getRegister();

int runTests();

template <typename Fn>
void dtestAdd(Fn&& fn, const char* testName, const char* fileName,
              u64 filePathHash) {
  getRegister().addTest(std::forward<Fn>(fn), testName, fileName, filePathHash);
}

/**
 * Note, it's common to "theme" the terminal, changing the presented color of
 * these following colors. Thus, the color "kYellow" might appear as some
 * other color, for some users.
 */
using ColorType = int;
enum class Color : ColorType {
  Gray = 90,
  BrightRed = 91,
  BrightGreen = 92,
  BrightYellow = 93,
  BrightBlue = 94,
  Magenta = 95,
  Teal = 96,
  White = 97,

  Black = 30,
  Red = 31,
  Green = 32,
  Yellow = 33,
  DarkBlue = 34,
  Purple = 35,
  Blue = 36,
  BrightGray = 37,
};

class Paint {
 public:
  Paint(const char* str, Color color);
  const char* c_str() const;
  DC_DELETE_COPY(Paint);

 private:
  static constexpr uint kStrLen = 100;
  char m_str[kStrLen];
};

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

#define DTEST_MARK_CATEGORY_VIP(filePath)                               \
  struct DTEST_MAKE_CLASS_NAME(vip_maker, __LINE__) {                   \
    DTEST_MAKE_CLASS_NAME(vip_maker, __LINE__)() {                      \
      dtest::internal::getRegister().addVip(dc::hash64fnv1a(filePath)); \
    }                                                                   \
  };                                                                    \
  DTEST_MAKE_CLASS_NAME(vip_maker, __LINE__)                            \
  DTEST_MAKE_VAR_NAME(vip_maker, __LINE__)

// ========================================================================== //
// ASSERT MACRCO IMPL
// ========================================================================== //

#define DASSERT_TRUE_IMPL(expr, line)                                          \
  do {                                                                         \
    if (!!(expr)) {                                                            \
      ++dtestBodyState__you_must_have_an_assert.pass;                          \
      LOG_INFO("\t\t+ Assert:{} true " #expr " {}", line,                      \
               dtest::internal::Paint("passed", dtest::internal::Color::Green) \
                   .c_str());                                                  \
    } else {                                                                   \
      ++dtestBodyState__you_must_have_an_assert.fail;                          \
      LOG_INFO("\t\t- Assert:{} true " #expr " {}", line,                      \
               dtest::internal::Paint("failed", dtest::internal::Color::Red)   \
                   .c_str());                                                  \
    }                                                                          \
  } while (0)

#define DASSERT_FALSE_IMPL(expr, line)                                         \
  do {                                                                         \
    if (!(expr)) {                                                             \
      ++dtestBodyState__you_must_have_an_assert.pass;                          \
      LOG_INFO("\t\t+ Assert:{} false " #expr " {}", line,                     \
               dtest::internal::Paint("passed", dtest::internal::Color::Green) \
                   .c_str());                                                  \
    } else {                                                                   \
      ++dtestBodyState__you_must_have_an_assert.fail;                          \
      LOG_INFO("\t\t- Assert:{} false " #expr " {}", line,                     \
               dtest::internal::Paint("failed", dtest::internal::Color::Red)   \
                   .c_str());                                                  \
    }                                                                          \
  } while (0)

#define DASSERT_EQ_IMPL(a, b, line)                                            \
  do {                                                                         \
    if ((a) == (b)) {                                                          \
      ++dtestBodyState__you_must_have_an_assert.pass;                          \
      LOG_INFO("\t\t+ Assert:{} " #a " == " #b " {}", line,                    \
               dtest::internal::Paint("passed", dtest::internal::Color::Green) \
                   .c_str());                                                  \
    } else {                                                                   \
      ++dtestBodyState__you_must_have_an_assert.fail;                          \
      LOG_INFO("\t\t- Assert:{} " #a " == " #b " {}", line,                    \
               dtest::internal::Paint("failed", dtest::internal::Color::Red)   \
                   .c_str());                                                  \
    }                                                                          \
  } while (0)

#define DASSERT_NE_IMPL(a, b, line)                                            \
  do {                                                                         \
    if ((a) != (b)) {                                                          \
      ++dtestBodyState__you_must_have_an_assert.pass;                          \
      LOG_INFO("\t\t+ Assert:{} " #a " != " #b " {}", line,                    \
               dtest::internal::Paint("passed", dtest::internal::Color::Green) \
                   .c_str());                                                  \
    } else {                                                                   \
      ++dtestBodyState__you_must_have_an_assert.fail;                          \
      LOG_INFO("\t\t- Assert:{} " #a " != " #b " {}", line,                    \
               dtest::internal::Paint("failed", dtest::internal::Color::Red)   \
                   .c_str());                                                  \
    }                                                                          \
  } while (0)
