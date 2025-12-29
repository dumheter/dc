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
#include <dc/pointer_int_pair.hpp>
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
      : m_pair(&externalAllocator) {
    m_pair.setInt(kHaveNotAllocated);
  }

  BufferAwareAllocator(const BufferAwareAllocator& other)
      : m_pair(other.m_pair) {}

  BufferAwareAllocator(BufferAwareAllocator&& other) noexcept
      : m_pair(dc::move(other.m_pair)) {}

  BufferAwareAllocator& operator=(BufferAwareAllocator&& other) noexcept {
    if (&other != this) {
      m_pair = dc::move(other.m_pair);
    }
    return *this;
  }

  virtual void* alloc(usize count, usize align = kMinimumAlignment) override;

  virtual void* realloc(void* data, usize count,
                        usize align = kMinimumAlignment) override;

  virtual void free(void* data) override;

  bool hasAllocated() const { return m_pair.getInt() == kHaveAllocated; }

  PointerIntPair<IAllocator*, u32> m_pair;

  static constexpr u32 kHaveAllocated = 1;
  static constexpr u32 kHaveNotAllocated = 0;
};

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////
// List
//

namespace detail {
constexpr s64 kCachelineBytes = 64;
constexpr s64 kCachelineMinusListBytes =
    (kCachelineBytes - (2 * sizeof(intptr_t) + sizeof(u64) +
                        sizeof(detail::BufferAwareAllocator))) < 0
        ? 0
        : kCachelineBytes - (2 * sizeof(intptr_t) + sizeof(u64) +
                             sizeof(detail::BufferAwareAllocator));
}  // namespace detail

/// Small size optimized list (dynamic array list). Starts out with the small
/// size buffer, unless specifically specified otherwise.
///
/// @tparam T Element type that list stores.
/// @tparam N Internal buffer size, in terms of element count.
template <typename T,
          u64 N = (detail::kCachelineMinusListBytes / sizeof(T)) < 1
                      ? 1
                      : (detail::kCachelineMinusListBytes / sizeof(T))>
class List {
 public:
  /// Construct a new list. Starts off with the internal buffer memory.
  List(IAllocator& allocator = getDefaultAllocator())
      : List(allocator, m_buffer, m_buffer, N) {}

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

  /// Add to the end of the list
  /// Noop on if allocation is needed but failed.
  void add(T elem);

  /// Default construct a T at the end of the list, then return a reference to
  /// it.
  /// Noop on if allocation is needed but failed.
  T& add();

  /// Add to the end of the list, a range of elements.
  /// Noop on if allocation is needed but failed.
  /// @param begin Pointer to the first element.
  /// @param end Pointer to one element _past_ the last element.
  void addRange(const T* begin, const T* end);

  /// Remove a specific entry in the list.
  void remove(T* elem);

  /// Remove the first instance of the element. Noop if not found.
  void remove(const T& elem);

  /// Remove a specific element at a given posistion
  void removeAt(u64 pos);

  /// Evaluate each element in list with @ref fn, remove those who match.
  /// @param Fn A function that takes a const T& and returns true if it should
  /// be removed
  template <typename Fn,
            // TODO cgustafsson:  is it nicer to have trait req here, instead of
            // impl?
            bool enable = isInvocable<Fn, const T&> &&
                          isSame<InvokeResultT<Fn, const T&>, bool> >
  void removeIf(Fn fn);

  [[nodiscard]] u64 getSize() const noexcept {
    return static_cast<u64>(m_end - m_begin);
  }

  [[nodiscard]] u64 getCapacity() const noexcept { return m_capacity; }

  [[nodiscard]] bool isEmpty() const noexcept { return getSize() == 0; }

  [[nodiscard]] List clone() const;

  /// Reserve block of memory. May (re)allocate. All active elements are moved
  /// on reallocation. Iterators and references are invaliated.
  /// Noop on if allocation is needed but failed.
  /// @return New capacity
  void reserve(u64 capacity);

  /// Set the current size of the list. May allocate if larger than current
  /// capacity.
  /// Will invalidate iterators on reallocation.
  /// Noop on if allocation is needed but failed.
  /// Note: Will not call destructor of elements that may be lost when shrinking
  /// the list.
  void resize(u64 newSize);

  void clear();

  [[nodiscard]] T& operator[](u64 pos);
  [[nodiscard]] const T& operator[](u64 pos) const;

  /// Get a reference to the first element in the list.
  T& getFirst() {
    DC_ASSERT(!isEmpty(), "Tried to access first element when empty.");
    return *m_begin;
  }

  /// Get a reference to the last element in the list.
  T& getLast() {
    DC_ASSERT(!isEmpty(), "Tried to access last element when empty.");
    return *(m_end - 1);
  }

  /// Iterator to first element.
  constexpr T* begin() const { return m_begin; }

  /// Iterator to one element _past_ the last elements.
  constexpr T* end() const { return m_end; }

  /// @return Pointer to the element on found, or pointer to m_end on not found.
  T* find(const T& elem);
  const T* find(const T& elem) const;

  static constexpr u64 kInternalBuffer = N;

 private:
  detail::BufferAwareAllocator m_allocator;
  T *m_begin = nullptr, *m_end = nullptr;
  u64 m_capacity = 0;
  T m_buffer[N];
};

///////////////////////////////////////////////////////////////////////////////
// Template Implementation
//

template <typename T, u64 N>
List<T, N>::List(u64 capacity, IAllocator& allocator) : m_allocator(allocator) {
  if (capacity <= N) {
    m_begin = m_buffer;
    m_end = m_buffer;
    m_capacity = N;
  } else
    reserve(capacity);
}

template <typename T, u64 N>
List<T, N>::List(u64 capacity, const detail::BufferAwareAllocator& allocator)
    : m_allocator(allocator) {
  if (capacity <= N) {
    m_begin = m_buffer;
    m_end = m_buffer;
    m_capacity = N;
  } else
    reserve(capacity);
}

template <typename T, u64 N>
List<T, N>::List(List&& other)
    : m_allocator(other.m_allocator), m_capacity(other.m_capacity) {
  if (other.m_allocator.hasAllocated()) {
    m_begin = other.m_begin;
    m_end = other.m_end;
  } else {
    m_begin = m_buffer;
    m_end = m_buffer + other.getSize();
    for (u64 i = 0; i < other.getSize(); ++i) {
      new (m_begin + i) T(dc::move(*(other.m_begin + i)));
    }
  }

  other.m_begin = nullptr;
  other.m_end = nullptr;
  other.m_capacity = 0;
}

template <typename T, u64 N>
List<T, N>& List<T, N>::operator=(List&& other) noexcept {
  if (&other != this) {
    this->~List();

    m_allocator.m_pair = dc::move(other.m_allocator.m_pair);
    m_capacity = other.m_capacity;

    if (other.m_allocator.hasAllocated()) {
      m_begin = other.m_begin;
      m_end = other.m_end;
    } else {
      m_begin = m_buffer;
      m_end = m_buffer + other.getSize();
      for (u64 i = 0; i < other.getSize(); ++i) {
        new (m_begin + i) T(dc::move(*(other.m_begin + i)));
      }
    }

    other.m_begin = nullptr;
    other.m_end = nullptr;
    other.m_capacity = 0;
  }

  return *this;
}

template <typename T, u64 N>
List<T, N>::~List() {
  for (T& elem : *this) elem.~T();

  m_allocator.free(m_begin);
  m_begin = nullptr;
}

constexpr u64 kDefaultExtraBytes = 32;

template <typename T, u64 N>
void List<T, N>::add(T elem) {
  if (getSize() >= m_capacity) reserve(getSize() * 2 + kDefaultExtraBytes);

  if (getSize() < m_capacity) {  // only add if reserve increased our capacity
    new (m_end) T(dc::move(elem));
    m_end += 1;
  }
}

template <typename T, u64 N>
T& List<T, N>::add() {
  if (getSize() >= m_capacity) reserve(getSize() * 2 + kDefaultExtraBytes);

  if (getSize() < m_capacity) {
    T& elem = new (m_end) T();
    m_end += 1;
    return elem;
  } else {
    return getLast();
  }
}

template <typename T, u64 N>
void List<T, N>::addRange(const T* begin, const T* end) {
  DC_FATAL_ASSERT(end >= begin, "end is less than begin.");
  if constexpr (isTriviallyRelocatable<T>) {
    const u64 oldSize = getSize();
    const u64 rangeSize = static_cast<u64>(end - begin);
    resize(oldSize + rangeSize);
    if (oldSize + rangeSize <= m_capacity)  // did resize work?
      memcpy(m_begin + oldSize, begin, rangeSize);
  } else {
    const u64 oldSize = getSize();
    const u64 rangeSize = static_cast<u64>(end - begin);
    resize(oldSize + rangeSize);
    if (oldSize + rangeSize <= m_capacity) {
      T* newIt = m_begin + oldSize;
      while (begin != end) {
        new (newIt++) T(*begin++);
      }
    }
  }
}

template <typename T, u64 N>
void List<T, N>::remove(T* elem) {
  DC_ASSERT(elem < m_end && elem >= m_begin,
            "Trying to erase an element outside of bounds.");

  // TODO cgustafsson: if trivial, then we could just memcpy
  elem->~T();

  for (; (elem + 1) != m_end; ++elem) {
    new (elem) T(dc::move(*(elem + 1)));
  }

  --m_end;
}

template <typename T, u64 N>
void List<T, N>::remove(const T& elem) {
  T* it = find(elem);

  if (it == m_end) return;

  for (; (it + 1) != m_end; ++it) *it = dc::move(*(it + 1));

  --m_end;
}

template <typename T, u64 N>
void List<T, N>::removeAt(u64 pos) {
  remove(m_begin + pos);
}

template <typename T, u64 N>
template <typename Fn, bool enable>
void List<T, N>::removeIf(Fn fn) {
  static_assert(isInvocable<Fn, const T&>,
                "Cannot call 'Fn', is it a function with argument 'const T&'?");
  static_assert(isSame<InvokeResultT<Fn, const T&>, bool>,
                "The return type of 'Fn' is not bool.");

  if (isEmpty()) return;

  // swap
  T* lastElem = m_end - 1;
  T* swapIter = lastElem;
  for (T* elem = lastElem; elem >= m_begin; --elem) {
    if (fn(*elem)) {
      dc::swap(*elem, *swapIter);
      --swapIter;

      // Now also restore the order of the swapped elem.
      // Would be more efficent to do in a restore passa after,
      // but lets keep it simple for now.
      for (T* restoreElem = elem; restoreElem < swapIter; ++restoreElem) {
        dc::swap(*restoreElem, *(restoreElem + 1));
      }
    }
  }

  // destruct
  for (T* elem = lastElem; elem >= swapIter; --elem) {
    elem->~T();
  }
  m_end = swapIter + 1;
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
    T* newBegin =
        static_cast<T*>(m_allocator.realloc(m_begin, sizeof(T) * capacity));
    if (!newBegin) return;  // failed to alloc, noop

    for (T* elem = m_begin; elem != m_end; ++elem)
      new (newBegin + (elem - m_begin)) T(dc::move(*elem));

    m_begin = newBegin;
    m_end = m_begin + size;
    m_capacity = capacity;
  }
}

template <typename T, u64 N>
void List<T, N>::resize(u64 newSize) {
  if (newSize > m_capacity) reserve(newSize);

  if (newSize <= m_capacity) {
    m_end = m_begin + newSize;
  }
}

template <typename T, u64 N>
void List<T, N>::clear() {
  if (m_end - m_begin == 0) return;

  for (T* elem = m_end - 1; elem != m_begin; --elem) elem->~T();

  m_begin->~T();
  m_end = m_begin;
}

template <typename T, u64 N>
[[nodiscard]] T& List<T, N>::operator[](u64 pos) {
  return m_begin[pos];
}

template <typename T, u64 N>
[[nodiscard]] const T& List<T, N>::operator[](u64 pos) const {
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
