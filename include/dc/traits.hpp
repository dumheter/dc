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

namespace dc {

///////////////////////////////////////////////////////////////////////////////
// Traits
//

template <typename T, T v>
struct IntegralConstant {
  using ValueType = T;
  static constexpr T value = v;
};

///////////////////////////////////////////////////////////////////////////////

using TrueType = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, false>;

///////////////////////////////////////////////////////////////////////////////

struct Unused {};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct AddRValueReference {
  using Type = T&&;
};

template <typename T>
struct AddRValueReference<T&> {
  using Type = T&;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RemoveReference {
  using Type = T;
};

template <typename T>
struct RemoveReference<T&> {
  using Type = T;
};

template <typename T>
struct RemoveReference<T&&> {
  using Type = T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsReference : public FalseType {};

template <typename T>
struct IsReference<T&> : public TrueType {};

template <typename T>
constexpr bool isReference = IsReference<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsVoid : public FalseType {};

template <>
struct IsVoid<void> : public TrueType {};

template <typename T>
constexpr bool isVoid = IsVoid<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsLValueReference : public FalseType {};
template <typename T>
struct IsLValueReference<T&> : public TrueType {};

template <typename T>
constexpr bool isLValueReference = IsLValueReference<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsFunction : public FalseType {};

template <typename ReturnValue, typename... Args>
struct IsFunction<ReturnValue(Args...)> : public TrueType {};

// NOTE is this one needed?
template <typename ReturnValue, typename... Args>
struct IsFunction<ReturnValue(Args..., ...)> : public TrueType {};

template <typename T>
constexpr bool isFunction = IsFunction<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr typename RemoveReference<T>::Type&& move(T&& x) noexcept {
  return static_cast<typename RemoveReference<T>::Type&&>(x);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void swap(T& a, T& b) noexcept  // assumes following operations does not throw -
                                // TODO, check that we dont throw on copy
{
  T temp(move(a));
  a = move(b);
  b = move(temp);
}

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsRValueReference : public FalseType {};
template <typename T>
struct IsRValueReference<T&&> : public TrueType {};

template <typename T>
constexpr bool isRValueReference = IsRValueReference<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsObject : public IntegralConstant<bool, !isReference<T> && !isVoid<T> &&
                                                    !isFunction<T>> {};

template <typename T>
constexpr bool isObject = IsObject<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
struct IsConstructible
    : public IntegralConstant<bool, __is_constructible(T, Args...)> {};

template <typename T, typename... Args>
constexpr bool isConstructible = IsConstructible<T, Args...>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsMoveConstructible
    : public IsConstructible<T, typename AddRValueReference<T>::Type> {};

template <typename T>
constexpr bool isMoveConstructible = IsMoveConstructible<T>::value;

///////////////////////////////////////////////////////////////////////////////

using YesType = char;  // sizeof(YesType) == 1
struct NoType {
  char padding[8];
};  // sizeof(NoType)  != 1

///////////////////////////////////////////////////////////////////////////////

template <typename T>
typename AddRValueReference<T>::Type declval() noexcept;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct AddLValueReference {
  using Type = T&;
};

template <typename T>
struct AddLValueReference<T&> {
  using Type = typename RemoveReference<T>::Type&;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct IsAssignableHelper {
  template <typename, typename>
  static constexpr NoType is(...);

  template <typename T1, typename U1>
  static constexpr decltype(declval<T1>() = declval<U1>(), YesType()) is(int);

  static constexpr bool value = (sizeof(is<T, U>(0)) == sizeof(YesType));
};

template <typename T, typename U>
struct IsAssignable
    : public IntegralConstant<bool, IsAssignableHelper<T, U>::value> {};

template <typename T, typename U>
constexpr bool isAssignable = IsAssignable<T, U>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct IsSame : public FalseType {};

template <typename T>
struct IsSame<T, T> : public TrueType {};

template <typename T, typename U>
constexpr bool isSame = IsSame<T, U>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsSwappable
    : public IntegralConstant<
          bool, !IsSame<decltype(dc::swap(declval<T&>(), declval<T&>())),
                        Unused>::value> {};

template <typename T>
constexpr bool isSwappable = IsSwappable<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr bool isMovable =
    isObject<T>&& isMoveConstructible<T>&& isAssignable<T&, T>&& isSwappable<T>;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsConst : public FalseType {};

template <typename T>
struct IsConst<const T> : public TrueType {};

template <typename T>
constexpr bool isConst = IsConst<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T, bool = isConst<T> || isFunction<T>>
struct AddConst {
  using Type = T;
};

template <typename T>
struct AddConst<T, false> {
  using Type = const T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RemoveConst {
  using Type = T;
};

template <typename T>
struct RemoveConst<const T> {
  using Type = T;
};

template <typename T>
struct RemoveConst<const T[]> {
  using Type = T[];
};

template <typename T, size_t kSize>
struct RemoveConst<const T[kSize]> {
  using Type = T[kSize];
};

///////////////////////////////////////////////////////////////////////////////

template <typename...>
using VoidArgs = void;

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename = void>
struct IsIterable : public FalseType {};

template <typename T>
struct IsIterable<
    T, VoidArgs<decltype(declval<T>().begin()), decltype(declval<T>().end())>>
    : public TrueType {};

template <typename T>
constexpr bool isIterable = IsIterable<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsArray : public FalseType {};

template <typename T>
struct IsArray<T[]> : public TrueType {};

template <typename T, size_t kSize>
struct IsArray<T[kSize]> : public TrueType {};

template <typename T>
constexpr bool isArray = IsArray<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RemoveExtent {
  using Type = T;
};

template <typename T>
struct RemoveExtent<T[]> {
  using Type = T;
};

template <typename T, size_t kSize>
struct RemoveExtent<T[kSize]> {
  using Type = T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct AddPointer {
  using Type = typename RemoveReference<T>::Type*;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RemoveVolatile {
  using Type = T;
};

template <typename T>
struct RemoveVolatile<volatile T> {
  using Type = T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct RemoveCV {
  using Type = typename RemoveVolatile<typename RemoveConst<T>::Type>::Type;
};

///////////////////////////////////////////////////////////////////////////////

template <bool B, typename T, typename F>
struct Conditional {
  using Type = T;
};

template <typename T, typename F>
struct Conditional<false, T, F> {
  using Type = F;
};

///////////////////////////////////////////////////////////////////////////////

/// Decay the type, just as the compiler does for arguments passed as value
/// to functions.
template <typename T>
struct Decay {
  using U = RemoveReference<T>;
  using Type = typename Conditional<
      isArray<U>, typename RemoveExtent<U>::Type*,
      typename Conditional<isFunction<U>, typename AddPointer<U>::Type,
                           typename RemoveCV<U>::Type>::Type>::Type;
};

///////////////////////////////////////////////////////////////////////////////

template <bool, typename = void>
struct EnableIf {};

template <typename T>
struct EnableIf<true, T> {
  using Type = T;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Base, typename Derived>
struct IsBaseOf : public IntegralConstant<bool, __is_base_of(Base, Derived) ||
                                                    isSame<Base, Derived>> {};

template <typename Base, typename Derived>
constexpr bool isBaseOf = IsBaseOf<Base, Derived>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename Other = T, typename = void>
struct IsEqualityComparable : FalseType {};

template <typename T, typename Other>
struct IsEqualityComparable<
    T, Other,
    typename EnableIf<true,
                      decltype((declval<RemoveReference<T> const&>() ==
                                declval<RemoveReference<Other> const&>()) &&
                                   (declval<RemoveReference<T> const&>() !=
                                    declval<RemoveReference<Other> const&>()),
                               (void)0)>::Type> : TrueType {};

/// Does T and Other have compatible operator== and operator!=?
template <typename T, typename Other>
constexpr bool isEqualityComparable = IsEqualityComparable<T, Other>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsCopyConstructible
    : public IsConstructible<
          T, typename AddLValueReference<typename AddConst<T>::Type>::Type> {};

template <typename T>
constexpr bool isCopyConstructible = IsCopyConstructible<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr T&& forward(typename RemoveReference<T>::Type& x) noexcept {
  return static_cast<T&&>(x);
}

template <typename T>
constexpr T&& forward(typename RemoveReference<T>::Type&& x) noexcept {
  static_assert(!IsLValueReference<T>::value,
                "You tried to forward an non l-value.");
  return static_cast<T&&>(x);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T* addressOf(T& x) noexcept {
  return reinterpret_cast<T*>(
      const_cast<char*>(&reinterpret_cast<const volatile char&>(x)));
}

///////////////////////////////////////////////////////////////////////////////

// Similar to the std::reference_wrapper

template <typename T>
class Ref {
  Ref(T& data) noexcept : m_data(addressOf(data)) {}
  Ref(T&& data) =
      delete;  // we don't own data, we keep a reference/pointer to it

  Ref(const Ref& data) noexcept : m_data(addressOf(data)) {}
  Ref& operator=(const Ref& other) noexcept { m_data(other.m_data); }

  operator T&() noexcept { return *m_data; }
  T& get() noexcept { return *m_data; }

 private:
  T* m_data;
};

template <typename T>
using MutRef = Ref<RemoveConst<T>>;

template <typename T>
using ConstRef = Ref<AddConst<T>>;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsRef : public FalseType {};

template <typename T>
struct IsRef<Ref<T>> : public TrueType {};

template <typename T>
constexpr bool isRef = IsRef<T>::value;

///////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
struct InvokeImpl {
  template <typename F, typename... Args>
  static auto call(F&& f, Args&&... args)
      -> decltype(forward<F>(f)(forward<Args>(args)...));
};

template <typename B, typename MT>
struct InvokeImpl<MT B::*> {
  template <typename T, typename Td = typename Decay<T>::Type,
            typename = typename EnableIf<isBaseOf<B, Td>>::Type>
  static auto get(T&&) -> T&&;

  template <typename T, typename Td = typename Decay<T>::Type,
            typename = typename EnableIf<isRef<Td>>::Type>
  static auto get(T&& t) -> decltype(t.get());

  template <typename T, typename Td = typename Decay<T>::Type,
            typename = typename EnableIf<!isBaseOf<B, Td>>::Type,
            typename = typename EnableIf<!isRef<Td>>::Type>
  static auto get(T&& t) -> decltype(*forward<T>(t));

  template <typename T, typename... Args, typename MT1,
            typename = typename EnableIf<isFunction<MT1>>::Type>
  static auto call(MT B::*pmf, T&& t, Args&&... args)
      -> decltype((InvokeImpl::get(forward<T>(t)).*
                   pmf)(forward<Args>(args)...));

  template <typename T>
  static auto call(MT B::*pmd, T&& t)
      -> decltype(InvokeImpl::get(forward<T>(t)).*pmd);
};

template <typename T, typename... Args, typename F = typename Decay<T>::Type>
auto invoke(T&& t, Args&&... args)
    -> decltype(InvokeImpl<F>::call(forward<T>(t), forward<Args>(args)...));

template <typename AlwaysVoid, typename, typename...>
struct InvokeResultImpl {};

template <typename F, typename... Args>
struct InvokeResultImpl<
    decltype(void(invoke(declval<F>(), declval<Args>()...))), F, Args...> {
  using Type = decltype(invoke(declval<F>(), declval<Args>()...));
};

template <typename F, typename... Args>
struct InvokeResult : public detail::InvokeResultImpl<void, F, Args...> {};
}  // namespace detail

template <typename F, typename... Args>
using InvokeResult = typename detail::InvokeResult<F, Args...>::Type;

///////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename F, typename = void, typename... Args>
struct IsInvocable : public FalseType {};

template <typename F, typename... Args>
struct IsInvocable<F, VoidArgs<InvokeResult<F, Args...>>, Args...>
    : public TrueType {};
}  // namespace detail

template <typename F, typename... Args>
struct IsInvocable : public detail::IsInvocable<F, void, Args...> {};

template <typename F, typename... Args>
constexpr bool isInvocable = detail::IsInvocable<F, Args...>::value;

}  // namespace dc
