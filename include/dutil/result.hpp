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

template <typename V, typename E>
class Result;

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

  /// Can only be constructed with an r-value. Use dutil::Ref, dutil::CRef and
  /// dutil::MutRef to capture references to non owned objects.
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

// TODO cgustafsson: remove this
// namespace internal::result
// {

// template <typename Va, typename Er>
// Va&& unsafeValueMove(Result<Va, Er>& result)
// {
// 	return std::move(result.valueRef());
// }

// template <typename Va, typename Er>
// Er&& unsafErrMove(Result<Va, Er>& result)
// {
// 	return std::move(result.errRef());
// }

// }

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

  // TODO cgustafsson: ok() -> Option<V>

  // TODO cgustafsson: err() -> Option<E>

	[[nodiscard]] V& value() & noexcept
	{
		// TODO cgustafsson: crash on not ok
		return valueRef();
	}

	[[nodiscard]] const V& value() const& noexcept
	{
		// TODO cgustafsson: crash on not ok
		return valueCRef();
	}

	[[nodiscard]] E& err() & noexcept
	{
		// TODO cgustafsson: crash on not err
		return errRef();
	}

	[[nodiscard]] const E& err() const& noexcept
	{
		// TODO cgustafsson: crash on not err
		return errCRef();
	}
	
  // TODO cgustafsson: getter that gives ownership / unwrap

	/// Create a non owning result, referencing the immutable data of the original.
	[[nodiscard]] constexpr Result<ConstRef<V>, ConstRef<E>> asConstRef() const& noexcept
	{
		if (isOk())
			return Ok<ConstRef<V>>(ConstRef<V>(valueCRef()));
		else
			return Err<ConstRef<E>>(ConstRef<E>(errCRef()));
	}

	/// Create a non owning result, referencing the mutable data of the original.
	[[nodiscard]] constexpr Result<MutRef<V>, MutRef<E>> asMutRef() & noexcept
	{
		if (isOk())
			return Ok<MutRef<V>>(MutRef<V>(valueRef()));
		else
			return Err<MutRef<E>>(MutRef<E>(errRef()));
	}

  template <typename T>
  [[nodiscard]] constexpr bool contains(const T& other) const {
    static_assert(isEqualityComparable<V, T>,
                  "Cannot compare 'T' with 'V' in 'Result<V, E>::contain(T)'.");
    if (isOk())
      return valueCRef() == other;
    else
      return false;
  }

	template <typename F>
	[[nodiscard]] constexpr bool containsErr(const F& other) const
	{
		static_assert(isEqualityComparable<E, F>, "Cannot compare 'F' with 'E' in 'Result<V, E>::containErr(F)'.");
		if (isErr())
			return errCRef() == other;
		else
			return false;
	}

  template <typename U>
  [[nodiscard]] constexpr bool operator==(const Ok<U>& other) const {
    static_assert(
        isEqualityComparable<V, U>,
        "'V' and 'U' cannot be compared, in 'Result<V, E>' and 'Ok<U>'.");
    if (isOk())
      return valueCRef() == other.value();
    else
      return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(const Ok<U>& other) const {
    static_assert(
        isEqualityComparable<V, U>,
        "'V' and 'U' cannot be compared, in 'Result<V, E>' and 'Ok<U>'.");
    if (isOk())
      return valueCRef() != other.value();
    else
      return true;
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator==(const Err<F>& other) const {
    static_assert(
        isEqualityComparable<E, F>,
        "'E' and 'F' cannot be compared, in 'Result<V, E>' and 'Err<F>'.");
    if (isErr())
      return errCRef() == other.value();
    else
      return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator!=(const Err<F>& other) const {
    static_assert(
        isEqualityComparable<E, F>,
        "'E' and 'F' cannot be compared, in 'Result<V, E>' and 'Err<F>'.");
    if (isErr())
      return errCRef() != other.value();
    else
      return true;
  }

  template <typename U, typename F>
  [[nodiscard]] constexpr bool operator==(const Result<U, F>& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "'V' and 'U' cannot be compared, in 'Result<V, E>' and "
                  "'Result<U, F>'.");
    static_assert(isEqualityComparable<E, F>,
                  "'E' and 'F' cannot be compared, in 'Result<V, E>' and "
                  "'Result<U, F>'.");

    if (isOk() && other.isOk())
      return valueCRef() == other.valueCRef();
    else if (isErr() && other.isErr())
      return errCRef() == other.errCRef();
    else
      return false;
  }

  template <typename U, typename F>
  [[nodiscard]] constexpr bool operator!=(const Result<U, F>& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "'V' and 'U' cannot be compared, in 'Result<V, E>' and "
                  "'Result<U, F>'.");
    static_assert(isEqualityComparable<E, F>,
                  "'E' and 'F' cannot be compared, in 'Result<V, E>' and "
                  "'Result<U, F>'.");

    if (isOk() && other.isOk())
      return valueCRef() != other.valueCRef();
    else if (isErr() && other.isErr())
      return errCRef() != other.errCRef();
    else
      return true;
  }

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
    static_assert(
        isSame<InvokeResult<OkFn&&, V&&>, InvokeResult<ErrFn&&, E&&>>,
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
    static_assert(
        isSame<InvokeResult<OkFn&&, V&>, InvokeResult<ErrFn&&, E&>>,
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
    static_assert(
        isSame<InvokeResult<OkFn&&, const V&>, InvokeResult<ErrFn&&, const E&>>,
        "The result type of 'OkFn' and 'ErrFn' does not match, they must.");

    if (isOk())
      return std::forward<OkFn&&>(okFn)(valueCRef());
    else
      return std::forward<ErrFn&&>(errFn)(errCRef());
  }

  [[nodiscard]] constexpr Result<V, E> clone() const {
    static_assert(isCopyConstructible<V>, "Cannot copy 'V' in 'Result<V, E>'.");
    static_assert(isCopyConstructible<E>, "Cannot copy 'E' in 'Result<V, E>'.");

    if (isOk())
      return Ok<V>(std::move(V(valueCRef())));
    else
      return Err<E>(std::move(E(errCRef())));
  }

 private:
  [[nodiscard]] constexpr V& valueRef() noexcept { return m_value; }
  [[nodiscard]] constexpr const V& valueCRef() const noexcept {
    return m_value;
  }

  [[nodiscard]] constexpr E& errRef() noexcept { return m_err; }
  [[nodiscard]] constexpr const E& errCRef() const noexcept { return m_err; }

  // TODO cgustafsson: remove this
  // template <typename Va, typename Er>
  // friend Va&& internal::result::unsafeValueMove(Result<Va, Er>&);
  // template <typename Va, typename Er>
  // friend Er&& internal::result::unsafErrMove(Result<Va, Er>&);

 private:
  union {
    V m_value;
    E m_err;
  };
  bool m_isOk;
};

// ========================================================================== //

template <typename U, typename V, typename E>
[[nodiscard]] constexpr bool operator==(const Ok<U>& ok,
                                        const Result<V, E>& result) {
  return result == ok;
}

template <typename U, typename V, typename E>
[[nodiscard]] constexpr bool operator!=(const Ok<U>& ok,
                                        const Result<V, E>& result) {
  return result != ok;
}

template <typename F, typename V, typename E>
[[nodiscard]] constexpr bool operator==(const Err<F>& err,
                                        const Result<V, E>& result) {
  return result == err;
}

template <typename F, typename V, typename E>
[[nodiscard]] constexpr bool operator!=(const Err<F>& err,
                                        const Result<V, E>& result) {
  return result == err;
}

template <typename V, typename E>
[[nodiscard]] constexpr auto makeOk(V value) -> Result<V, E> {
  return Ok<V>(std::forward<V>(value));
}

template <typename V, typename E>
[[nodiscard]] constexpr auto makeErr(E err) -> Result<V, E> {
  return Err<E>(std::forward<E>(err));
}

}  // namespace dutil
