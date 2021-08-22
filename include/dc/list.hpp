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
#include <dc/traits.hpp>
#include <dc/types.hpp>

namespace dc {

namespace detail {

/// Implements the allocator interface, but by default, it keeps track of an
/// external buffer. And calling free without once allocating, will be a noop.
class BufferAwareAllocator final : public IAllocator {
 public:
  /// @param externalAllocator Must point to valid data throughout this objects
  /// lifetime, will not delete it on this destruction.
  BufferAwareAllocator(IAllocator& externalAllocator)
      : m_externalAllocator(externalAllocator) {}

  BufferAwareAllocator(const BufferAwareAllocator& externalAllocator)
      : m_externalAllocator(externalAllocator.m_externalAllocator),
        m_haveAllocated(externalAllocator.m_haveAllocated) {}

  virtual void* alloc(usize count, usize align = kMinimumAlignment) override;

  virtual void* realloc(void* data, usize count,
                        usize align = kMinimumAlignment) override;

  virtual void free(void* data) override;

 private:
  IAllocator& m_externalAllocator;
  bool m_haveAllocated = false;
};

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////
// List
//

/// Small size optimized list (dynamic array list). Starts out with the small
/// size buffer, unless specifically specified otherwise.
///
/// @tparam T Element type that list stores.
/// @tparam N Internal buffer size, in terms of element count.
template <typename T,
          u64 N =
              (64 - (36 + sizeof(detail::BufferAwareAllocator))) / sizeof(T)>
class List {
 public:
  /// Construct a new list. Starts off with the internal buffer memory.
  List(IAllocator& allocator = getDefaultAllocator())
      : List(allocator, buffer, buffer, N) {}

  /// Construct a new list. Depending on capacity, will either use internal
  /// buffer, or allocate from allocator.
  List(u64 capacity, IAllocator& allocator = getDefaultAllocator());

 private:
  /// Take ownership of an already allocated set of memory, based on allocator.
  List(IAllocator& allocator, T* begin, T* end, u64 capacity)
      : m_allocator(allocator),
        m_begin(begin),
        m_end(end),
        m_capacity(capacity) {}

  List(u64 capacity, const detail::BufferAwareAllocator& allocator);

 public:
  /// Use clone instead
  List(const List& other) = delete;
  List& operator=(const List& other) = delete;

  List(List&& other);
  List& operator=(List&& other) noexcept;

  ~List();

  void add(T item);

  void remove(T* item);

  void remove(u64 pos);

  [[nodiscard]] u64 getSize() const noexcept { return m_end - m_begin; }

  [[nodiscard]] u64 getCapacity() const noexcept { return m_capacity; }

  [[nodiscard]] List clone() const;

  /// Reserve block of memory. May (re)allocate. All active elements are moved
  /// on reallocation. Iterators and references are invaliated.
  /// @return New capacity
  void reserve(u64 capacity);

  /// Set the current size of the list. May allocate if larger than current
  /// capacity.
  void resize(u64 newSize);

  void clear();

  [[nodiscard]] T& operator[](u64 pos);

  constexpr T* begin() const { return m_begin; }

  constexpr T* end() const { return m_end; }

  /// @return Pointer to the element on found, or pointer to m_end on not found.
  T* find(const T& elem);
  const T* find(const T& elem) const;

 private:
  detail::BufferAwareAllocator m_allocator;
  T *m_begin = nullptr, *m_end = nullptr;
  u64 m_capacity = 0;
  T buffer[N];
};

///////////////////////////////////////////////////////////////////////////////
// Template Implementation
//

template <typename T, u64 N>
List<T, N>::List(u64 capacity, IAllocator& allocator) : m_allocator(allocator) {
  if (capacity <= N) {
    m_begin = buffer;
    m_end = buffer;
    m_capacity = N;
  } else
    reserve(capacity);
}

template <typename T, u64 N>
List<T, N>::List(u64 capacity, const detail::BufferAwareAllocator& allocator)
    : m_allocator(allocator) {
  if (capacity <= N) {
    m_begin = buffer;
    m_end = buffer;
    m_capacity = N;
  } else
    reserve(capacity);
}

template <typename T, u64 N>
List<T, N>::List(List&& other)
    : m_allocator(other.m_allocator),
      m_begin(other.m_begin),
      m_end(other.m_end),
      m_capacity(other.m_capacity) {
  if (&other != this) {
    other.m_begin = nullptr;
    other.m_end = nullptr;
    other.m_capacity = 0;
  }
}

template <typename T, u64 N>
List<T, N>& List<T, N>::operator=(List&& other) noexcept {
  if (&other != this) {
    m_begin = other.m_begin;
    m_end = other.m_end;
    m_capacity = other.m_capacity;
    other.m_begin = nullptr;
    other.m_end = nullptr;
    other.m_capacity = 0;
  }

  return *this;
}

template <typename T, u64 N>
List<T, N>::~List() {
  for (T& item : *this) item.~T();

  m_allocator.free(m_begin);
}

template <typename T, u64 N>
void List<T, N>::add(T item) {
  if (getSize() >= m_capacity) reserve(getSize() * 2 + 32);

  *m_end = item;
  m_end += 1;
}

template <typename T, u64 N>
void List<T, N>::remove(T* item) {
  DC_ASSERT(item < m_end, "Trying to erase an element outside of bounds.");

  // item->~T();

  // TODO cgustafsson: if trivial, then we could just memcpy

  for (; (item + 1) != m_end; ++item) *item = dc::move(*(item + 1));

  --m_end;
}

template <typename T, u64 N>
void List<T, N>::remove(u64 pos) {
  remove(m_begin + pos);
}

template <typename T, u64 N>
List<T, N> List<T, N>::clone() const {
  List<T> out(getCapacity(),
              static_cast<const detail::BufferAwareAllocator&>(m_allocator));

  for (const T& iter : *this) out.add(iter);

  return out;
}

template <typename T, u64 N>
void List<T, N>::reserve(u64 capacity) {
  if (capacity > m_capacity) {
    const u64 size = getSize();
    T* newBegin = static_cast<T*>(m_allocator.alloc(sizeof(T) * capacity));

    for (T* item = m_begin; item != m_end; ++item)
      *(newBegin + (item - m_begin)) = dc::move(*item);

    m_begin = newBegin;
    m_end = m_begin + size;
    m_capacity = capacity;
  }
}

template <typename T, u64 N>
void List<T, N>::resize(u64 newSize) {
  if (newSize > m_capacity) reserve(newSize);

  m_end = m_begin + newSize;
}

template <typename T, u64 N>
void List<T, N>::clear() {
  if (m_end - m_begin == 0) return;

  for (T* item = m_end - 1; item != m_begin; --item) item->~T();

  m_begin->~T();
  m_end = m_begin;
}

template <typename T, u64 N>
[[nodiscard]] T& List<T, N>::operator[](u64 pos) {
  return m_begin[pos];
}

template <typename T, u64 N>
const T* List<T, N>::find(const T& elem) const {
  for (const T* iter = m_begin; iter != m_end; ++iter) {
    if (*iter == elem) return iter;
  }
  return m_end;
}

template <typename T, u64 N>
T* List<T, N>::find(const T& elem) {
  for (T* iter = m_begin; iter != m_end; ++iter) {
    if (*iter == elem) return iter;
  }
  return m_end;
}

}  // namespace dc
