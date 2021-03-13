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

#include <dutil/misc.hpp>
#include <dutil/types.hpp>
#include <functional>

#include "dtest.hpp"
#include "dutil/traits.hpp"

// ========================================================================== //
// DTEST
// ========================================================================== //

/// Add a test.
///
/// Example
///   DTEST(ExampleTest)
///   {
///      ...
///   }
#define DTEST(testName) DTEST_REGISTER(testName, DUTIL_FILENAME, __FILE__)

/// Run all registered tests.
#define DTEST_RUN() dtest::details::runTests()

#define DASSERT_TRUE(expr)                                                 \
  do {                                                                     \
    if (!!(expr)) {                                                        \
      ++dtest_details_testBodyState.pass;                                  \
      printf("\t\t+ Assert true " #expr " %s\n",                           \
             dtest::details::Paint("passed", dtest::details::Color::Green) \
                 .c_str());                                                \
    } else {                                                               \
      ++dtest_details_testBodyState.fail;                                  \
      printf("\t\t- Assert true " #expr " %s\n",                           \
             dtest::details::Paint("failed", dtest::details::Color::Red)   \
                 .c_str());                                                \
    }                                                                      \
  } while (0)

#define DASSERT_FALSE(expr)                                                \
  do {                                                                     \
    if (!(expr)) {                                                         \
      ++dtest_details_testBodyState.pass;                                  \
      printf("\t\t+ Assert false " #expr " %s\n",                          \
             dtest::details::Paint("passed", dtest::details::Color::Green) \
                 .c_str());                                                \
    } else {                                                               \
      ++dtest_details_testBodyState.fail;                                  \
      printf("\t\t- Assert false " #expr " %s\n",                          \
             dtest::details::Paint("failed", dtest::details::Color::Red)   \
                 .c_str());                                                \
    }                                                                      \
  } while (0)

#define DASSERT_EQ(a, b)                                                   \
  do {                                                                     \
    if ((a) == (b)) {                                                      \
      ++dtest_details_testBodyState.pass;                                  \
      printf("\t\t+ Assert " #a " == " #b " %s\n",                         \
             dtest::details::Paint("passed", dtest::details::Color::Green) \
                 .c_str());                                                \
    } else {                                                               \
      ++dtest_details_testBodyState.fail;                                  \
      printf("\t\t- Assert " #a " == " #b " %s\n",                         \
             dtest::details::Paint("failed", dtest::details::Color::Red)   \
                 .c_str());                                                \
    }                                                                      \
  } while (0)

#define DASSERT_NE(a, b)                                                   \
  do {                                                                     \
    if ((a) != (b)) {                                                      \
      ++dtest_details_testBodyState.pass;                                  \
      printf("\t\t+ Assert " #a " != " #b " %s\n",                         \
             dtest::details::Paint("passed", dtest::details::Color::Green) \
                 .c_str());                                                \
    } else {                                                               \
      ++dtest_details_testBodyState.fail;                                  \
      printf("\t\t- Assert " #a " != " #b " %s\n",                         \
             dtest::details::Paint("failed", dtest::details::Color::Red)   \
                 .c_str());                                                \
    }                                                                      \
  } while (0)

// ========================================================================== //
// HELPERS
// ========================================================================== //

namespace dtest {

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
class TrackLifetime {
 public:
  TrackLifetime(T object) : m_object(std::move(object)) {}

  /// Amount of times this object has been copied.
  [[nodiscard]] constexpr int getCopies() const noexcept { return m_copies; }

  /// Amount of times this object has been moved.
  [[nodiscard]] constexpr int getMoves() const noexcept { return m_moves; }

  [[nodiscard]] constexpr const T& getObject() const { return m_object; }
  [[nodiscard]] constexpr T& getObject() { return m_object; }

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

  template <typename U>
  [[nodiscard]] constexpr bool operator==(
      const TrackLifetime<U>& other) const noexcept {
    static_assert(dutil::isEqualityComparable<T, U>);
    return getObject() == other.getObject();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(
      const TrackLifetime<U>& other) const noexcept {
    static_assert(dutil::isEqualityComparable<T, U>);
    return getObject() != other.getObject();
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
};

}  // namespace dtest

// ========================================================================== //
// INTERNAL DETAILS
// ========================================================================== //

namespace dtest::details {

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
  DUTIL_DELETE_COPY(Register);

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

void runTests();

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
  DUTIL_DELETE_COPY(Paint);

 private:
  static constexpr uint kStrLen = 100;
  char m_str[kStrLen];
};

}  // namespace dtest::details

#define DTEST_REGISTER(testName, fileName, filePath)                           \
  struct DTest_Reg_##testName##_ {                                             \
    void testBody(dtest::details::TestBodyState& dtest_details_testBodyState); \
    DTest_Reg_##testName##_() {                                                \
      dtest::details::dtestAdd(                                                \
          [this](dtest::details::TestBodyState& dtest_details_testBodyState) { \
            testBody(dtest_details_testBodyState);                             \
          },                                                                   \
          #testName, fileName, dutil::hash64fnv1a(filePath));                  \
    }                                                                          \
  };                                                                           \
  DTest_Reg_##testName##_ dtest_Reg_##testName{};                              \
  void DTest_Reg_##testName##_::testBody(                                      \
      dtest::details::TestBodyState& dtest_details_testBodyState)
