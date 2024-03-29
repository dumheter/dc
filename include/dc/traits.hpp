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

#include <dc/types.hpp>

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
  using U = typename RemoveReference<T>::Type;
  using Type = typename Conditional<
      isArray<U>, typename RemoveExtent<U>::Type*,
      typename Conditional<isFunction<U>, typename AddPointer<U>::Type,
                           typename RemoveCV<U>::Type>::Type>::Type;
};

template <typename T>
using DecayT = typename Decay<T>::Type;

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

template <typename A, typename B = A, typename = void>
struct IsEqualityComparable : FalseType {};

template <typename A, typename B>
struct IsEqualityComparable<A, B,
                            VoidArgs<decltype(declval<A>() == declval<B>()),
                                     decltype(declval<A>() != declval<B>())>>
    : public TrueType {};

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
 public:
  Ref(T& data) noexcept : m_data(addressOf(data)) {}
  Ref(T&& data) =
      delete;  // we don't own data, we keep a reference/pointer to it

  Ref(const Ref& other) noexcept : m_data(addressOf(other.m_data)) {}
  Ref& operator=(const Ref& other) noexcept {
    m_data(other.m_data);
    return *this;
  }

  Ref(Ref&& other) noexcept : m_data(other.m_data) {}
  Ref& operator=(Ref&& other) noexcept {
    m_data = other.m_data;
    return *this;
  }

  constexpr operator T&() const noexcept { return *m_data; }

  constexpr T& get() const noexcept { return *m_data; }

 private:
  T* m_data;
};

template <typename T>
Ref(T&) -> Ref<T>;

template <typename T>
using MutRef = Ref<typename RemoveConst<T>::Type>;

template <typename T>
using ConstRef = Ref<typename AddConst<T>::Type>;

///////////////////////////////////////////////////////////////////////////////

template <typename>
struct IsRef : public FalseType {};

template <typename T>
struct IsRef<Ref<T>> : public TrueType {};

template <typename T>
constexpr bool isRef = IsRef<T>::value;

///////////////////////////////////////////////////////////////////////////////

// template <typename R, typename C, typename T, typename... Args>
// auto invokeImpl(R C::*fn, T&& obj, Args&&... args) ->
//     typename EnableIf<isBaseOf<C, DecayT<decltype(obj)>>,
//                       decltype((forward<T>(obj).*
//                                 fn)(forward<Args>(args)...))>::Type {
//   return (forward<T>(obj).*fn)(forward<Args>(args)...);
// }

// template <typename F, typename... Args>
// auto invokeImpl(F&& fn, Args&&... args)
//     -> decltype(forward<F>(fn)(forward<Args>(args)...)) {
//   return forward<F>(fn)(forward<Args>(args)...);
// }

// template <typename R, typename C, typename T, typename... Args>
// auto invokeImpl(R C::*fn, T&& obj, Args&&... args)
//     -> decltype(((*forward<T>(obj)).*fn)(forward<Args>(args)...)) {
//   return ((*forward<T>(obj)).*fn)(forward<Args>(args)...);
// }

// template <typename M, typename C, typename T>
// auto invokeImpl(M C::*member, T&& obj) ->
//     typename EnableIf<isBaseOf<C, DecayT<decltype(obj)>>,
//                       decltype(obj.*member)>::Type {
//   return obj.*member;
// }

// template <typename M, typename C, typename T>
// auto invokeImpl(M C::*member, T&& obj) ->
// decltype((*forward<T>(obj)).*member) {
//   return (*forward<T>(obj)).*member;
// }

// template <typename F, typename... Args>
// inline decltype(auto) invoke(F&& fn, Args&&... args) {
//   return invokeImpl(forward<F>(fn), forward<Args>(args)...);
// }

// template <typename F, typename = void, typename... Args>
// struct InvokeResultImpl {};

// template <typename F, typename... Args>
// struct InvokeResultImpl<
//     F, VoidArgs<decltype(invokeImpl(declval<DecayT<F>>(),
//     declval<Args>()...))>, Args...> {
//   using Type = decltype(invokeImpl(declval<DecayT<F>>(),
//   declval<Args>()...));
// };

// template <typename F, typename... Args>
// struct InvokeResult : public InvokeResultImpl<F, void, Args...> {};

// template <typename F, typename... Args>
// using InvokeResultT = typename InvokeResult<F, Args...>::Type;

// template <typename F, typename... Args>
// using InvokeResultT = std::invoke_result_t<F, Args...>;

///////////////////////////////////////////////////////////////////////////////

// template <typename F, typename = void, typename... Args>
// struct IsInvocableImpl : public FalseType {};

// template <typename F, typename... Args>
// struct IsInvocableImpl<F, VoidArgs<typename InvokeResult<F, Args...>::Type>,
//                        Args...> : public TrueType {};

// template <typename F, typename... Args>
// struct IsInvocable : public IsInvocableImpl<F, void, Args...> {};

// template <typename F, typename... Args>
// constexpr bool isInvocable = IsInvocable<F, Args...>::value;

// template <typename F, typename... Args>
// constexpr bool isInvocable = std::is_invocable_v<F, Args...>;

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
  static auto call(MT1 B::*pmf, T&& t, Args&&... args)
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

}  // namespace detail

template <typename F, typename... Args>
struct InvokeResult : public detail::InvokeResultImpl<void, F, Args...> {};

/// NOTE: if you have a compile error that takes you here, you are probably
/// trying to query the result type of a function that doesnt exist.
template <typename F, typename... Args>
using InvokeResultT = typename InvokeResult<F, Args...>::Type;

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
constexpr bool isInvocable = detail::IsInvocable<F, void, Args...>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct IsPod : public IntegralConstant<bool, __is_pod(T)> {};

template <typename T, size_t N>
struct IsPod<T[N]> : public IsPod<T> {};

template <typename T>
constexpr bool isPod = IsPod<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct FloatingPoint : public FalseType {};

template <>
struct FloatingPoint<f32> : public TrueType {};

template <>
struct FloatingPoint<f64> : public TrueType {};

template <typename T>
constexpr bool isFloatingPoint = FloatingPoint<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Integral : public FalseType {};

template <>
struct Integral<bool> : public TrueType {};

template <>
struct Integral<u8> : public TrueType {};

template <>
struct Integral<s8> : public TrueType {};

template <>
struct Integral<u16> : public TrueType {};

template <>
struct Integral<s16> : public TrueType {};

template <>
struct Integral<u32> : public TrueType {};

template <>
struct Integral<s32> : public TrueType {};

template <>
struct Integral<u64> : public TrueType {};

template <>
struct Integral<s64> : public TrueType {};

template <>
struct Integral<char8> : public TrueType {};

template <typename T>
constexpr bool isIntegral = Integral<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Arithmetic
    : public IntegralConstant<bool, isIntegral<T> || isFloatingPoint<T>> {};

template <typename T>
constexpr bool isArithmetic = Arithmetic<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Enum : public IntegralConstant<bool, __is_enum(T)> {};

template <typename T>
constexpr bool isEnum = Enum<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Pointer : public FalseType {};

template <typename T>
struct Pointer<T*> : public TrueType {};

template <typename T>
struct Pointer<T* const> : public TrueType {};

template <typename T>
struct Pointer<T* volatile> : public TrueType {};

template <typename T>
struct Pointer<T* const volatile> : public TrueType {};

/// Is type 'T' a pointer? Unlike the standard, this includes pointers to
/// members.
template <typename T>
constexpr bool isPointer = Pointer<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Fundamental
    : public IntegralConstant<bool, isArithmetic<T> || isVoid<T> || isEnum<T> ||
                                        isPointer<T>> {};

template <typename T>
constexpr bool isFundamental = Fundamental<T>::value;

///////////////////////////////////////////////////////////////////////////////

/// Is type 'T' trivial to copy, as in, a simple memcpy of an instance fully
/// initializes the object, no call to constructor needed.
///
/// A case of a class that is not trivially relocatable: A class that holds a
/// pointer to memory inside itself. If we simply copy the object, then the copy
/// will hold a pointer to the original memory.
///
/// Mark your custom type by defining the type `IsTriviallyRelocatable`.
///
/// Example:
///   struct Rel { using IsTriviallyRelocatable = bool; };
///   static_assert(isTriviallyRelocatable<Rel>);
///
template <typename T, typename = void>
struct TriviallyRelocatable : public IntegralConstant<bool, isFundamental<T>> {
};

template <typename T>
struct TriviallyRelocatable<T, VoidArgs<typename T::IsTriviallyRelocatable>>
    : public TrueType {};

template <typename T>
constexpr bool isTriviallyRelocatable = TriviallyRelocatable<T>::value;

}  // namespace dc
