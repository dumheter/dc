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

#include <type_traits>
#include <utility>

namespace dc {

// ========================================================================== //
// Traits
// ========================================================================== //

template <typename T>
constexpr bool isMovable =
    std::is_object<T>::value&& std::is_move_constructible<T>::value&&
        std::is_assignable<T&, T>::value&& std::is_swappable<T>::value;

template <typename T>
constexpr bool isReference = std::is_reference<T>::value;

template <typename T>
constexpr bool isCopyConstructible = std::is_copy_constructible<T>::value;

template <typename T, typename Other = T, typename = void>
struct equalityComparable : std::false_type {};

template <typename T, typename Other>
struct equalityComparable<
    T, Other,
    typename std::enable_if_t<
        true,
        decltype((std::declval<std::remove_reference_t<T> const&>() ==
                  std::declval<std::remove_reference_t<Other> const&>()) &&
                     (std::declval<std::remove_reference_t<T> const&>() !=
                      std::declval<std::remove_reference_t<Other> const&>()),
                 (void)0)>> : std::true_type {};

/// Does T and Other have compatible operator== and operator!=?
template <typename T, typename Other>
constexpr bool isEqualityComparable = equalityComparable<T, Other>::value;

/// Can Fn be called with the given Args.
template <typename Fn, typename... Args>
constexpr bool isInvocable = std::is_invocable<Fn, Args...>::value;

template <typename T, typename U>
constexpr bool isSame = std::is_same<T, U>::value;

// ========================================================================== //
// Type Aliases
// ========================================================================== //

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using ConstRef =
    std::reference_wrapper<std::add_const_t<std::remove_reference_t<T>>>;

template <typename T>
using MutRef =
    std::reference_wrapper<std::remove_const_t<std::remove_reference_t<T>>>;

/// Result type of a function call, with the given args.
template <typename Fn, typename... Args>
using InvokeResult = typename std::invoke_result<Fn, Args...>::type;

}  // namespace dc
