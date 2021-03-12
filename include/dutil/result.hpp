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

// NOTE: For a better and more complete result library, check out:
// https://github.com/lamarrr/STX

#pragma once

#include <dutil/traits.hpp>
#include <dutil/types.hpp>
#include <utility>

namespace dutil {

template <typename V>
struct Ok;

template <typename E>
struct Err;

// ========================================================================== //

template <typename V>
struct [[nodiscard]] Ok {
  static_assert(isMovable<V>, "Value type 'V' in 'Ok<V>' must be movable.");
  static_assert(
      !isReference<V>,
      "Value type 'V' in 'Ok<V>' cannot be a reference."
      " dutil::Ref, or the stricter dutil::ConstRef and dutil::MutRef can"
      " be used for this case.");

  using value_type = V;

  /// Can only be constructed with an r-value.
  explicit constexpr Ok(V&& value) : m_value(std::forward<V&&>(value)) {}

  constexpr Ok(const Ok&) = default;
  constexpr Ok& operator=(const Ok&) = default;
  constexpr Ok(Ok&&) = default;
  constexpr Ok& operator=(Ok&&) = default;

  [[nodiscard]] constexpr V const& value() const& noexcept { return m_value; }
  [[nodiscard]] constexpr V& value() & noexcept { return m_value; }
  [[nodiscard]] constexpr V const value() const&& { return std::move(m_value); }
  [[nodiscard]] constexpr V value() && { return std::move(m_value); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Ok<U> const& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Ok<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U'");
    return value() == other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Ok<U> const& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Ok<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U'");
    return value() != other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Err<U> const& other) const noexcept {
    return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Err<U> const& other) const noexcept {
    return true;
  }

 private:
  V m_value;
};

// ========================================================================== //

template <typename E>
struct Err {
  static_assert(isMovable<E>, "Error type 'E' in 'Err<E>' must be movable.");
  static_assert(
      !isReference<E>,
      "Error type 'E' in 'Err<E>' cannot be a reference."
      " dutil::Ref, or the stricter dutil::ConstRef, dutil::MutRef can be"
      " used for this case.");

  using value_type = E;

  Err(E&& value) : m_value(std::move(value)) {}

  constexpr Err(const Err&) = default;
  constexpr Err& operator=(const Err&) = default;
  constexpr Err(Err&&) = default;
  constexpr Err& operator=(Err&&) = default;

  [[nodiscard]] constexpr E const& value() const& noexcept { return m_value; }
  [[nodiscard]] constexpr E& value() & noexcept { return m_value; }
  [[nodiscard]] constexpr E const value() const&& { return std::move(m_value); }
  [[nodiscard]] constexpr E value() && { return std::move(m_value); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Err<U> const& other) const {
    static_assert(isEqualityComparable<E, U>,
                  "Error type 'E' in 'Err<E>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U'");
    return value() == other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Err<U> const& other) const {
    static_assert(isEqualityComparable<E, U>,
                  "Error type 'E' in 'Err<E>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U'");
    return value() != other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Ok<U> const& other) const noexcept {
    return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Ok<U> const& other) const noexcept {
    return true;
  }

 private:
  E m_value;
};

// ========================================================================== //

/// @tparam V Value type.
/// @tparam E Error type.
template <typename V, typename E>
class [[nodiscard]] Result {
 public:
  static_assert(isMovable<V>,
                "Value type 'V' in 'Result<V, E>' must be movable.");
  static_assert(
      !isReference<V>,
      "Value type 'V' in 'Result<V, E>' cannot be a reference."
      " dutil::Ref, or the stricter dutil::ConstRef and dutil::MutRef can"
      " be used for this case.");
  static_assert(isMovable<E>,
                "Error type 'E' in 'Result<V, E>' must be movable.");
  static_assert(
      !isReference<E>,
      "Error type 'E' in 'Result<V, E>' cannot be a reference."
      " dutil::Ref, or the stricter dutil::ConstRef, dutil::MutRef can be"
      " used for this case.");

  using value_type = V;
  using error_type = E;

  Result(Ok<V>&& ok) : m_value(std::forward<V>(ok.value())), m_isOk(true) {}
  Result(Err<E>&& err) : m_err(std::forward<E>(err.value())), m_isOk(false) {}

  Result(Result&& other) : m_isOk(other.m_isOk) {
    if (other.isOk())
      new (&m_value) V(std::move(other.m_value));
    else
      new (&m_err) E(std::move(other.m_err));
  }

  Result& operator=(Result&& other) noexcept {
    if (isOk() && other.isOk()) {
      std::swap(valueRef(), other.valueRef());
    } else if (isOk() && other.isErr()) {
      m_value.~V();
      new (&m_err) E(std::move(other.m_err));
      m_isOk = false;
    } else if (isErr() && other.isOk()) {
      m_err.~V();
      new (&m_value) V(std::move(other.m_value));
      m_isOk = true;
    } else {
      std::swap(errRef(), other.errRef());
    }
    return *this;
  }

  Result() = delete;
  DUTIL_DELETE_COPY(Result);

  ~Result() noexcept {
    if (isOk())
      m_value.~V();
    else
      m_err.~E();
  }

  // TODO cgustafsson: getters with fatal assert

  // TODO cgustafsson: equality

  [[nodiscard]] constexpr bool isOk() const noexcept { return m_isOk; }
  [[nodiscard]] constexpr bool isErr() const noexcept { return !isOk(); }
  [[nodiscard]] constexpr operator bool() const noexcept { return isOk(); }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(
      OkFn&& okFn, ErrFn&& errFn) && -> InvokeResult<OkFn&&, V&&> {
    static_assert(isInvocable<OkFn&&, V&&>,
                  "Cannot call 'OkFn', is it a function?");
    static_assert(isInvocable<ErrFn&&, E&&>,
                  "Cannot call 'ErrFn', is it a function?");
    static_assert(isSame<InvokeResult<OkFn&&, V&&>, InvokeResult<ErrFn&&, E&&>>,
				  "The result type of 'OkFn' and 'ErrFn' does not match, they must.");

    if (isOk())
      return std::forward<OkFn&&>(okFn)(std::move(valueRef()));
    else
      return std::forward<ErrFn&&>(errFn)(std::move(errRef()));
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(
      OkFn&& okFn, ErrFn&& errFn) & -> InvokeResult<OkFn&&, V&> {
    static_assert(isInvocable<OkFn&&, V&>,
                  "Cannot call 'OkFn', is it a function?");
    static_assert(isInvocable<ErrFn&&, E&>,
                  "Cannot call 'ErrFn', is it a function?");
	static_assert(isSame<InvokeResult<OkFn&&, V&>, InvokeResult<ErrFn&&, E&>>,
				  "The result type of 'OkFn' and 'ErrFn' does not match, they must.");

    if (isOk())
      return std::forward<OkFn&&>(okFn)(valueRef());
    else
      return std::forward<ErrFn&&>(errFn)(errRef());
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(
      OkFn&& okFn, ErrFn&& errFn) const& -> InvokeResult<OkFn&&, const V&> {
    static_assert(isInvocable<OkFn&&, const V&>,
                  "Cannot call 'OkFn', is it a function?");
    static_assert(isInvocable<ErrFn&&, const E&>,
                  "Cannot call 'ErrFn', is it a function?");
	static_assert(isSame<InvokeResult<OkFn&&, const V&>, InvokeResult<ErrFn&&, const E&>>,
				  "The result type of 'OkFn' and 'ErrFn' does not match, they must.");

    if (isOk())
      return std::forward<OkFn&&>(okFn)(valueCRef());
    else
      return std::forward<ErrFn&&>(errFn)(errCRef());
  }

  // TODO cgustafsson: clone

 private:
  [[nodiscard]] constexpr V& valueRef() noexcept { return m_value; }
  [[nodiscard]] constexpr const V& valueCRef() const noexcept {
    return m_value;
  }

  [[nodiscard]] constexpr E& errRef() noexcept { return m_err; }
  [[nodiscard]] constexpr const E& errCRef() const noexcept { return m_err; }

 private:
  union {
    V m_value;
    E m_err;
  };
  bool m_isOk;
};

// ========================================================================== //

template <typename U, typename V, typename E>
[[nodiscard]] constexpr bool operator==(const Ok<U>& ok, const Result<V, E>& result)
{
	return result == ok;
}

template <typename U, typename V, typename E>
[[nodiscard]] constexpr bool operator!=(const Ok<U>& ok, const Result<V, E>& result)
{
	return result != ok;
}

template <typename F, typename V, typename E>
[[nodiscard]] constexpr bool operator==(const Err<F>& err, const Result<V, E>& result)
{
	return result == err;
}

template <typename F, typename V, typename E>
[[nodiscard]] constexpr bool operator!=(const Err<F>& err, const Result<V, E>& result)
{
	return result == err;
}

template <typename V, typename E>
[[nodiscard]] constexpr auto make_ok(V value) -> Result<V, E>
{
	return Ok<V>(std::forward<V>(value));
}

template <typename V, typename E>
[[nodiscard]] constexpr auto make_err(E err) -> Result<V, E>
{
	return Err<E>(std::forward<E>(err));
}

}  // namespace dutil
