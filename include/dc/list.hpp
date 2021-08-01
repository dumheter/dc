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

#include <dc/allocator.hpp>
#include <dc/assert.hpp>
#include <dc/result.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>

namespace dc {

/// This is an array list, a block of contigeous memory.
template <typename T>
class [[nodiscard]] List {
 public:
  /// Support for range loops. Not used for anything else.
  // class [[nodiscard]] Iterator {
  //  public:
  //   Iterator(T* data, usize pos) : m_data(data), m_pos(pos) {}

  //   [[nodiscard]] T& operator*() { return m_data[m_pos]; }
  //   [[nodiscard]] const T& operator*() const { return m_data[m_pos]; }

  //   [[nodiscard]] T* operator->() { return &(m_data[m_pos]); }
  //   [[nodiscard]] const T* operator->() const { return &(m_data[m_pos]); }

  //   [[nodiscard]] constexpr bool operator==(const Iterator& other) const {
  //     return m_data == other.m_data && m_pos == other.m_pos;
  //   }
  //   [[nodiscard]] constexpr bool operator!=(const Iterator& other) const {
  //     return m_data != other.m_data || m_pos != other.m_pos;
  //   }
  //   constexpr void operator++() { ++m_pos; }

  //  private:
  //   T* m_data;
  //   usize m_pos;
  // };

 public:
  List(IAllocator& allocator = getDefaultAllocator());
  List(usize capacity, IAllocator& allocator = getDefaultAllocator());

  // explicitly use clone()
  List(const List& other) = delete;
  List& operator=(const List& other) = delete;

  List(List&& other) noexcept;
  List& operator=(List&& other) noexcept;

  ~List();

  [[nodiscard]] List clone() const;

  /// Grow the memory usage. Leave at 0 to let grow figure out the new capacity.
  void grow(usize newCapacity = 0);

  /// Reserve a capacity. If newCapacity is less than current capacity, noop.
  void reserve(usize newCapacity);

  /// Add an element to the back of the list. May grow the list if at
  /// capacity.
  void add(T elem);
  [[nodiscard]] T& add();

  /// Remove the element at position.
  void remove(usize pos);

  // TODO cgustafsson:
  template <typename Fn>
  void removeIf(Fn fn);

  /// find the first element that equals elem
  [[nodiscard]] Option<usize> find(const T& elem) const;

  [[nodiscard]] constexpr const T& operator[](usize pos) const;
  [[nodiscard]] constexpr T& operator[](usize pos);

  [[nodiscard]] constexpr usize size() const { return m_size; }
  [[nodiscard]] constexpr usize capacity() const { return m_capacity; }

  [[nodiscard]] constexpr T* begin() { return &m_data[0]; }
  [[nodiscard]] constexpr T* end() { return &m_data[size()]; }

  [[nodiscard]] constexpr const T* begin() const { return &m_data[0]; }
  [[nodiscard]] constexpr const T* end() const { return &m_data[size()]; }

 private:
  void free();

 private:
  IAllocator& m_allocator;
  T* m_data;
  usize m_size;
  usize m_capacity;
};

///////////////////////////////////////////////////////////////////////////////
// impl
//

template <typename T>
List<T>::List(IAllocator& allocator) : List(8, allocator) {}

template <typename T>
List<T>::List(usize capacity, IAllocator& allocator)
    : m_allocator(allocator), m_size(0), m_capacity(capacity) {
  m_data = (T*)m_allocator.alloc(sizeof(T) * capacity);
}

template <typename T>
List<T>::List(List<T>&& other) noexcept
    : m_allocator(other.m_allocator),
      m_data(other.m_data),
      m_size(other.m_size),
      m_capacity(other.m_capacity) {
  other.m_data = nullptr;
  other.m_size = 0;
  other.m_capacity = 0;
}

template <typename T>
List<T>& List<T>::operator=(List<T>&& other) noexcept {
  if (this != &other) {
    free();

    m_allocator = other.m_allocator;
    m_data = other.m_data;
    m_size = other.m_size;
    m_capacity = other.m_capacity;

    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }
  return *this;
}

template <typename T>
List<T>::~List<T>() {
  free();
}

template <typename T>
List<T> List<T>::clone() const {
  List<T> out(capacity());
  // TODO cgustafsson: will not work for non pod types
  memcpy((void*)out.m_data, (void*)m_data, sizeof(T) * m_size);
  out.m_size = m_size;
  return out;
}

template <typename T>
void List<T>::grow(usize newCapacity) {
  if (newCapacity == 0) newCapacity = capacity() * 2;

  if (newCapacity < capacity()) return;

  void* ptr = m_allocator.realloc((void*)m_data, sizeof(T) * newCapacity);
  DC_FATAL_ASSERT(ptr, "we failed to reallocate memory");
  m_data = (T*)ptr;
  m_capacity = newCapacity;
}

template <typename T>
void List<T>::reserve(usize newCapacity) {
  if (newCapacity < capacity()) return;

  void* ptr = m_allocator.realloc((void*)m_data, sizeof(T) * newCapacity);
  DC_FATAL_ASSERT(ptr, "we failed to reallocate memory");
  m_data = (T*)ptr;
  m_capacity = newCapacity;
}

template <typename T>
void List<T>::add(T elem) {
  if (size() == capacity()) grow();

  m_data[m_size] = elem;
  ++m_size;
}

template <typename T>
T& List<T>::add() {
  if (size() == capacity()) grow();

  T& out = m_data[m_size];
  ++m_size;
  return out;
}

template <typename T>
void List<T>::remove(usize pos) {
  DC_ASSERT(pos < size(), "trying to erase an element outside of bounds");

  T& elem = m_data[pos];
  elem.~T();

  if (pos + 1 != size()) {
    for (usize i = pos + 1; i != m_size; ++i)
      m_data[i - 1] = dc::move(m_data[i]);
  }
  --m_size;
}

template <typename T>
Option<usize> List<T>::find(const T& elem) const {
  for (usize i = 0; i < m_size; ++i) {
    if (m_data[i] == elem) return makeSome(i);
  }
  return None;
}

template <typename T>
constexpr const T& List<T>::operator[](usize pos) const {
  DC_ASSERT(pos < size(), "trying to access an element outside of bounds");
  return m_data[pos];
}

template <typename T>
constexpr T& List<T>::operator[](usize pos) {
  DC_ASSERT(pos < size(), "trying to access an element outside of bounds");
  return m_data[pos];
}

template <typename T>
void List<T>::free() {
  for (T& elem : *this) {
    elem.~T();
  }
  m_allocator.free(m_data);
}

}  // namespace dc
