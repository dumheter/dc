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

/// NOTE: Inspired by https://github.com/lamarrr/STX, check it out!

#pragma once

#include <dc/assert.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>
#include <utility>

namespace dc {

struct NoneType;

template <typename V>
struct Some;

template <typename V>
class Option;

namespace experimental {
template <typename V, V noneValue>
class [[nodiscard]] IntrusiveOption;
}

template <typename V>
struct Ok;

template <typename E>
struct Err;

template <typename V, typename E>
class Result;

// ========================================================================== //

struct [[nodiscard]] NoneType {
  constexpr NoneType() noexcept = default;
  constexpr NoneType(const NoneType&) noexcept = default;
  constexpr NoneType& operator=(const NoneType&) noexcept = default;
  constexpr NoneType(NoneType&&) noexcept = default;
  constexpr NoneType& operator=(NoneType&&) noexcept = default;

  [[nodiscard]] constexpr bool operator==(const NoneType&) const noexcept {
    return false;
  }

  [[nodiscard]] constexpr bool operator!=(const NoneType&) const noexcept {
    return true;
  }

  template <typename V>
  [[nodiscard]] constexpr bool operator==(const Some<V>&) const noexcept {
    return false;
  }

  template <typename V>
  [[nodiscard]] constexpr bool operator!=(const Some<V>&) const noexcept {
    return true;
  }
};

constexpr const NoneType None{};

// ========================================================================== //

template <typename V>
struct [[nodiscard]] Some {
  static_assert(isMovable<V>, "Value type 'V' in 'Some<V>' must be movable.");
  static_assert(!isReference<V>,
                "Value type 'V' in 'Some<V>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

  using value_type = V;

  explicit constexpr Some(V&& value) : m_value(std::forward<V&&>(value)) {}

  constexpr Some(const Some<V>& other) = default;
  constexpr Some& operator=(const Some<V>& other) = default;
  constexpr Some(Some<V>&& other) = default;
  constexpr Some& operator=(Some<V>&& other) = default;

  [[nodiscard]] constexpr const V& value() const& noexcept { return m_value; }
  [[nodiscard]] constexpr V& value() & noexcept { return m_value; }
  [[nodiscard]] constexpr const V value() const&& noexcept {
    return std::move(m_value);
  }
  [[nodiscard]] constexpr V value() && noexcept { return std::move(m_value); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(const Some<U> other) const noexcept {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Some<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Some<U>'.");
    return value() == other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(const Some<U> other) const noexcept {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Some<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Some<U>'.");
    return value() != other.value();
  }

  [[nodiscard]] constexpr bool operator==(const NoneType) const noexcept {
    return false;
  }

  [[nodiscard]] constexpr bool operator!=(const NoneType) const noexcept {
    return true;
  }

  Some() = delete;

 private:
  V m_value;

  template <typename Vu>
  friend class Option;

  template <typename Vu, Vu noneValue>
  friend class experimental::IntrusiveOption;
};

// ========================================================================== //

template <typename V>
class [[nodiscard]] Option {
 public:
  static_assert(isMovable<V>, "Value type 'V' in 'Option<V>' must be movable.");
  static_assert(!isReference<V>,
                "Value type 'V' in 'Option<V>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

  using value_type = V;

  constexpr Option() noexcept : m_isSome(false) {}

  constexpr Option(Some<V>&& some)
      : m_some(std::forward<V>(some.m_value)), m_isSome(true) {}

  constexpr Option(const NoneType&) noexcept : m_isSome(false) {}

  Option(Option&& other) : m_isSome(other.m_isSome) {
    if (other.isSome()) new (&m_some) V(std::move(other.m_some));
  }

  Option& operator=(Option&& other) {
    if (isSome() && other.isSome())
      std::swap(m_some, other.m_some);  // TODO cgustafsson: Revisit this swap,
                                        // maybe should be removed.
    else if (isNone() && other.isSome()) {
      m_isSome = true;
      new (&m_some) V(std::move(other.m_some));
      other.m_isSome = false;
    } else if (isSome() && other.isNone()) {
      m_isSome = false;
      m_some.~V();
    }
    /* else // both are none, do nothing */

    return *this;
  }

  ~Option() noexcept {
    if (isSome()) m_some.~V();
  }

  DC_DELETE_COPY(Option);

  [[nodiscard]] constexpr Option<V> clone() const {
    static_assert(isCopyConstructible<V>,
                  "'V' in 'Some<V>' is not copy constructible.");

    if (isSome())
      return Some<V>(std::move(V(valueCRef())));
    else
      return None;
  }

  [[nodiscard]] constexpr auto asMutRef() & -> Option<MutRef<V>> {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return Some<MutRef<V>>(MutRef<V>(valueRef()));
  }

  [[nodiscard]] constexpr auto asConstRef() const& -> Option<ConstRef<V>> {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return Some<ConstRef<V>>(ConstRef<V>(valueCRef()));
  }

  // TODO cgustafsson: okOr(E error) && -> Result<U, E>

  // TODO cgustafsson: okOrElse(...) && -> ...

  // TODO cgustafsson: take -> Option

  // TODO cgustafsson: replace??

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(
      SomeFn someFn, NoneFn noneFn) && -> InvokeResult<SomeFn&&, V&&> {
    static_assert(
        isInvocable<SomeFn&&, V&&>,
        "Cannot call 'SomeFn', is it a function with argument 'V&&'?");
    static_assert(isInvocable<NoneFn&&>,
                  "Cannot call 'NoneFn', is it a function with no arguments?");
    static_assert(
        isSame<InvokeResult<SomeFn&&, V&&>, InvokeResult<NoneFn&&>>,
        "The result type of 'SomeFn' and 'NoneFn' does not match, they must.");

    if (isSome())
      return std::forward<SomeFn&&>(someFn)(std::move(valueRef()));
    else
      return std::forward<NoneFn&&>(noneFn)();
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(
      SomeFn someFn, NoneFn noneFn) & -> InvokeResult<SomeFn&&, V&> {
    static_assert(isInvocable<SomeFn&&, V&>,
                  "Cannot call 'SomeFn', is it a function with argument 'V&'");
    static_assert(isInvocable<NoneFn&&>,
                  "Cannot call 'NoneFn', is it a function with no arguments?");
    static_assert(
        isSame<InvokeResult<SomeFn&&, V&>, InvokeResult<NoneFn&&>>,
        "The result type of 'SomeFn' and 'NoneFn' does not match, they must.");

    if (isSome())
      return std::forward<SomeFn&&>(someFn)(valueRef());
    else
      return std::forward<NoneFn&&>(noneFn)();
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(
      SomeFn someFn, NoneFn noneFn) const& -> InvokeResult<SomeFn&&, const V&> {
    static_assert(
        isInvocable<SomeFn&&, const V&>,
        "Cannot call 'SomeFn', is it a function with argument 'const V&'?");
    static_assert(isInvocable<NoneFn&&>,
                  "Cannot call 'NoneFn', is it a function with no arguments?");
    static_assert(
        isSame<InvokeResult<SomeFn&&, const V&>, InvokeResult<NoneFn&&>>,
        "The result type of 'SomeFn' and 'NoneFn' does not match, they must.");

    if (isSome())
      return std::forward<SomeFn&&>(someFn)(valueCRef());
    else
      return std::forward<NoneFn&&>(noneFn)();
  }

  [[nodiscard]] constexpr V& value() & {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return valueRef();
  }

  [[nodiscard]] constexpr const V& value() const& {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return valueCRef();
  }

  [[nodiscard]] constexpr V unwrap() && {
    DC_ASSERT(isSome(), "Tried to unwrap value 'Some' when Option is 'None'.");
    return std::move(valueRef());
  }

  // TODO cgustafsson: unwrapOr

  // TODO cgustafsson: unwrapOrElse

  // TODO cgustafsson: map

  [[nodiscard]] constexpr bool isSome() const { return m_isSome; }
  [[nodiscard]] constexpr bool isNone() const { return !m_isSome; }
  [[nodiscard]] constexpr operator bool() const noexcept { return isSome(); }

  template <typename U>
  [[nodiscard]] constexpr bool contains(const U& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "Cannot compare 'U' with 'V' in 'Option<V>::contains(U)'.");
    if (isSome())
      return value() == other;
    else
      return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(const Option<U>& other) const {
    static_assert(
        isEqualityComparable<V, U>,
        "Value type 'V' in 'Option<V>' is not equality comparable "
        "(operator== and operator!= defined) with 'U' in 'Option<U>'.");
    if (isSome() && other.isSome())
      return value() == other.value();
    else
      return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(const Option<U>& other) const {
    static_assert(
        isEqualityComparable<V, U>,
        "Value type 'V' in 'Option<V>' is not equality comparable "
        "(operator== and operator!= defined) with 'U' in 'Option<U>'.");
    if (isSome() && other.isSome())
      return value() != other.value();
    else
      return true;
  }

 private:
  [[nodiscard]] constexpr V& valueRef() { return m_some; }
  [[nodiscard]] constexpr const V& valueCRef() const { return m_some; }

 private:
  union {
    V m_some;
  };
  bool m_isSome;
};

// ========================================================================== //

namespace experimental {
template <typename V, V noneValue>
class [[nodiscard]] IntrusiveOption {
 public:
  static_assert(isMovable<V>, "Value type 'V' in 'Option<V>' must be movable.");
  static_assert(!isReference<V>,
                "Value type 'V' in 'Option<V>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

  using value_type = V;

  /////////////////////////
  // Lifetime

  constexpr IntrusiveOption() noexcept : m_some(noneValue) {}
  constexpr IntrusiveOption(Some<V>&& value)
      : m_some(std::forward<V>(value.m_value)) {}
  constexpr IntrusiveOption(NoneType) noexcept : m_some(noneValue) {}

  IntrusiveOption(IntrusiveOption&& other) {
    if (other.isSome()) new (&m_some) V(std::move(other.m_some));
  }

  IntrusiveOption& operator=(IntrusiveOption&& other) {
    if (isSome() && other.isSome())
      std::swap(m_some, other.m_some);  // TODO cgustafsson: Revisit this swap,
    // maybe should be removed.
    else if (isNone() && other.isSome()) {
      new (&m_some) V(std::move(other.m_some));
    } else if (isSome() && other.isNone()) {
      m_some.~V();
    }
    /* else // both are none, do nothing */

    return *this;
  }

  ~IntrusiveOption() noexcept {
    if (isSome()) m_some.~V();
  }

  DC_DELETE_COPY(IntrusiveOption);

  /////////////////////////
  // Value Status

  [[nodiscard]] constexpr bool isSome() const { return m_some != noneValue; }
  [[nodiscard]] constexpr bool isNone() const { return m_some == noneValue; }
  [[nodiscard]] constexpr operator bool() const { return m_some != noneValue; }

  ////////////////////////
  // Access

  [[nodiscard]] constexpr V& value() & {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return m_some;
  }
  [[nodiscard]] constexpr const V& value() const& {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return m_some;
  }
  [[nodiscard]] constexpr V&& value() && {
    DC_ASSERT(isSome(), "Tried to access value 'Some' when Option is 'None'.");
    return std::move(m_some);
  }

 private:
  union {
    V m_some;
  };
};
}  // namespace experimental

// ========================================================================== //

template <typename V>
struct [[nodiscard]] Ok {
  static_assert(isMovable<V>, "Value type 'V' in 'Ok<V>' must be movable.");
  static_assert(!isReference<V>,
                "Value type 'V' in 'Ok<V>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

  using value_type = V;

  /// Can only be constructed with an r-value. Use dc::Ref, dc::CRef and
  /// dc::MutRef to capture references to non owned objects.
  explicit constexpr Ok(V&& value) : m_value(std::forward<V&&>(value)) {}

  constexpr Ok(const Ok&) = default;
  constexpr Ok& operator=(const Ok&) = default;
  constexpr Ok(Ok&&) = default;
  constexpr Ok& operator=(Ok&&) = default;

  [[nodiscard]] constexpr const V& value() const& noexcept { return m_value; }
  [[nodiscard]] constexpr V& value() & noexcept { return m_value; }
  [[nodiscard]] constexpr const V value() const&& { return std::move(m_value); }
  [[nodiscard]] constexpr V value() && { return std::move(m_value); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Ok<U> const& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Ok<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Ok<U>'.");
    return value() == other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Ok<U> const& other) const {
    static_assert(isEqualityComparable<V, U>,
                  "Value type 'V' in 'Ok<V>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Ok<U>'.");
    return value() != other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Err<U> const&) const noexcept {
    return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Err<U> const&) const noexcept {
    return true;
  }

 private:
  V m_value;
};

// ========================================================================== //

template <typename E>
struct [[nodiscard]] Err {
  static_assert(isMovable<E>, "Error type 'E' in 'Err<E>' must be movable.");
  static_assert(!isReference<E>,
                "Error type 'E' in 'Err<E>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

  using value_type = E;

  Err(E&& value) : m_value(std::move(value)) {}

  constexpr Err(const Err&) = default;
  constexpr Err& operator=(const Err&) = default;
  constexpr Err(Err&&) = default;
  constexpr Err& operator=(Err&&) = default;

  [[nodiscard]] constexpr const E& value() const& noexcept { return m_value; }
  [[nodiscard]] constexpr E& value() & noexcept { return m_value; }
  [[nodiscard]] constexpr const E value() const&& { return std::move(m_value); }
  [[nodiscard]] constexpr E value() && { return std::move(m_value); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Err<U> const& other) const {
    static_assert(isEqualityComparable<E, U>,
                  "Error type 'E' in 'Err<E>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Err<U>'.");
    return value() == other.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Err<U> const& other) const {
    static_assert(isEqualityComparable<E, U>,
                  "Error type 'E' in 'Err<E>' is not equality comparable "
                  "(operator== and operator!= defined) with 'U' in 'Err<U>'.");
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
  static_assert(!isReference<V>,
                "Value type 'V' in 'Result<V, E>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");
  static_assert(isMovable<E>,
                "Error type 'E' in 'Result<V, E>' must be movable.");
  static_assert(!isReference<E>,
                "Error type 'E' in 'Result<V, E>' cannot be a reference."
                " You might want to use dc::Ref, or the stricter "
                "dc::ConstRef and dc::MutRef.");

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
  DC_DELETE_COPY(Result);

  ~Result() noexcept {
    if (isOk())
      m_value.~V();
    else
      m_err.~E();
  }

  [[nodiscard]] constexpr auto ok() && -> Option<V> {
    if (isOk())
      return Some<V>(std::move(valueRef()));
    else
      return None;
  }

  [[nodiscard]] constexpr auto err() && -> Option<E> {
    if (isErr())
      return Some<E>(std::move(errRef()));
    else
      return None;
  }

  [[nodiscard]] V unwrapOr(V other) && {
    if (isOk())
      return std::move(valueRef());
    else
      return std::move(other);
  }

  // TODO cgustafsson: unwrapOrElse(Fn(E)) -> V

  [[nodiscard]] V unwrap() && {
    DC_ASSERT(isOk(), "Tried to unwrap a result that was not 'Ok'.");
    return std::move(valueRef());
  }

  [[nodiscard]] E unwrapErrOr(E other) && {
    if (isErr())
      return std::move(errRef());
    else
      return std::move(other);
  }

  [[nodiscard]] E unwrapErr() && {
    DC_ASSERT(isErr(), "Tried to unwrapErr a result that was not 'Err'.");
    return std::move(errRef());
  }

  // TODO cgustafsson: map

  [[nodiscard]] V& value() & noexcept {
    DC_ASSERT(isOk(), "Tried to access 'value' when 'isOk()' is false.");
    return valueRef();
  }

  [[nodiscard]] const V& value() const& noexcept {
    DC_ASSERT(isOk(), "Tried to access 'value' when 'isOk()' is false.");
    return valueCRef();
  }

  [[nodiscard]] E& errValue() & noexcept {
    DC_ASSERT(isErr(), "Tried to access 'err' when 'isErr()' is false.");
    return errRef();
  }

  [[nodiscard]] const E& errValue() const& noexcept {
    DC_ASSERT(isErr(), "Tried to access 'err' when 'isErr()' is false.");
    return errCRef();
  }

  /// Create a non owning result, referencing the immutable data of the
  /// original.
  [[nodiscard]] constexpr Result<ConstRef<V>, ConstRef<E>> asConstRef()
      const& noexcept {
    if (isOk())
      return Ok<ConstRef<V>>(ConstRef<V>(valueCRef()));
    else
      return Err<ConstRef<E>>(ConstRef<E>(errCRef()));
  }

  /// Create a non owning result, referencing the mutable data of the original.
  [[nodiscard]] constexpr Result<MutRef<V>, MutRef<E>> asMutRef() & noexcept {
    if (isOk())
      return Ok<MutRef<V>>(MutRef<V>(valueRef()));
    else
      return Err<MutRef<E>>(MutRef<E>(errRef()));
  }

  template <typename U>
  [[nodiscard]] constexpr bool contains(const U& other) const {
    static_assert(
        isEqualityComparable<V, U>,
        "Cannot compare 'U' with 'V' in 'Result<V, E>::contains(U)'.");
    if (isOk())
      return valueCRef() == other;
    else
      return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool containsErr(const F& other) const {
    static_assert(
        isEqualityComparable<E, F>,
        "Cannot compare 'F' with 'E' in 'Result<V, E>::containsErr(F)'.");
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
                  "Cannot call 'OkFn', is it a function with argument 'V&&'?");
    static_assert(isInvocable<ErrFn&&, E&&>,
                  "Cannot call 'ErrFn', is it a function with argument 'E&&'?");
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
                  "Cannot call 'OkFn', is it a function with argument 'V&'?");
    static_assert(isInvocable<ErrFn&&, E&>,
                  "Cannot call 'ErrFn', is it a function with argument 'E&'?");
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
    static_assert(
        isInvocable<OkFn&&, const V&>,
        "Cannot call 'OkFn', is it a function with argument 'const V&'?");
    static_assert(
        isInvocable<ErrFn&&, const E&>,
        "Cannot call 'ErrFn', is it a function with argument 'const E&'?");
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

template <typename V>
[[nodiscard]] constexpr auto makeNone() noexcept -> Option<V> {
  return None;
}

template <typename V>
[[nodiscard]] constexpr auto makeSome(V value) -> Option<V> {
  return Some<V>(std::forward<V>(value));
}

template <typename V, typename E>
[[nodiscard]] constexpr auto makeOk(V value) -> Result<V, E> {
  return Ok<V>(std::forward<V>(value));
}

template <typename V, typename E>
[[nodiscard]] constexpr auto makeErr(E err) -> Result<V, E> {
  return Err<E>(std::forward<E>(err));
}

template <typename V, typename E>
[[nodiscard]] auto makeOkRef(V& value) noexcept -> Result<Ref<V>, Ref<E>> {
  return Ok<V>(Ref<V>(std::forward<V&>(value)));
}

template <typename V, typename E>
[[nodiscard]] auto makeErrRef(E& err) noexcept -> Result<Ref<V>, Ref<E>> {
  return Err<E>(Ref<E>(std::forward<E&>(err)));
}

}  // namespace dc
