/**
 * MIT License
 *
 * Copyright (c) 2026 Christoffer Gustafsson
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

#include <cstring>
#include <dc/allocator.hpp>
#include <dc/assert.hpp>
#include <dc/hash.hpp>
#include <dc/list.hpp>
#include <dc/macros.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>

namespace dc {

// ========================================================================== //
// Map
// ========================================================================== //

/// Hash map with robin hood open addressing collision resolution.
///
/// @tparam Key The key type
/// @tparam Value The value type
/// @tparam HashFn Hash functor, defaults to Hash<Key>
/// @tparam EqualFn Equality functor, defaults to Equal<Key>
template <typename Key, typename Value, typename HashFn = Hash<Key>,
          typename EqualFn = Equal<Key>>
class Map {
 public:
  // ------------------------------------------------------------------------ //
  // Types
  // ------------------------------------------------------------------------ //

  struct Entry {
    Key key;
    Value value;
  };

 private:
  struct InternalEntry {
    u32 probeSequenceLength;  // 0 = empty/tombstone
    Entry entry;
  };

  static constexpr u32 kTombstone = 0;
  static constexpr f32 kDefaultMaxLoadFactor = 0.75f;
  static constexpr u64 kDefaultCapacity = 16;

 public:
  // ------------------------------------------------------------------------ //
  // Construction & Destruction
  // ------------------------------------------------------------------------ //

  explicit Map(IAllocator& allocator = getDefaultAllocator());
  Map(u64 capacity, f32 maxLoadFactor = kDefaultMaxLoadFactor,
      IAllocator& allocator = getDefaultAllocator());
  ~Map() = default;

  DC_DELETE_COPY(Map);
  Map(Map&& other) noexcept = default;
  Map& operator=(Map&& other) noexcept = default;

  [[nodiscard]] Map clone() const;

  // ------------------------------------------------------------------------ //
  // Core Operations
  // ------------------------------------------------------------------------ //

  /// Insert a key into the map.
  /// @param key The key to insert
  /// @return Pointer to the value slot to be filled, or nullptr if allocation
  ///         failed during resize.
  Value* insert(Key key);

  /// Try to get an entry by key.
  /// @param key The key to look up
  /// @return Pointer to the entry if found, nullptr otherwise
  [[nodiscard]] Entry* tryGet(const Key& key);
  [[nodiscard]] const Entry* tryGet(const Key& key) const;

  /// Access value by key, inserting default if not present.
  /// @param key The key to look up or insert
  /// @return Pointer to the value, or nullptr if allocation failed
  Value* operator[](const Key& key);

  /// Remove an entry by key.
  /// @param key The key to remove
  /// @return true if found and removed, false otherwise
  bool remove(const Key& key);

  /// Remove an entry by key and retrieve its value.
  /// @param key The key to remove
  /// @param valueOut Pointer to store the removed value (if found)
  /// @return true if found and removed, false otherwise
  bool remove(const Key& key, Value* valueOut);

  /// Remove an entry by reference.
  /// @param entry Reference to an entry obtained from tryGet() or iteration
  void remove(Entry& entry);

  /// Evaluate each entry in the map with @ref fn, remove those who match.
  /// @param Fn A function that takes a const Entry& and returns true if it
  /// should be removed
  template <typename Fn,
            bool enable = isInvocable<Fn, const Entry&> &&
                          isSame<InvokeResultT<Fn, const Entry&>, bool>>
  void removeIf(Fn fn);

  // ------------------------------------------------------------------------ //
  // Capacity
  // ------------------------------------------------------------------------ //

  [[nodiscard]] u64 getSize() const noexcept { return m_size; }
  [[nodiscard]] u64 getCapacity() const noexcept {
    return m_data.getCapacity();
  }
  [[nodiscard]] bool isEmpty() const noexcept { return m_size == 0; }

  void clear();
  void reserve(u64 newCapacity);

  // ------------------------------------------------------------------------ //
  // Iteration
  // ------------------------------------------------------------------------ //

  class Iterator {
   public:
    Iterator(InternalEntry* current, InternalEntry* end)
        : m_current(current), m_end(end) {
      skipEmpty();
    }

    Entry& operator*() const { return m_current->entry; }
    Entry* operator->() const { return &m_current->entry; }

    Iterator& operator++() {
      ++m_current;
      skipEmpty();
      return *this;
    }

    bool operator!=(const Iterator& other) const {
      return m_current != other.m_current;
    }

    bool operator==(const Iterator& other) const {
      return m_current == other.m_current;
    }

   private:
    void skipEmpty() {
      while (m_current != m_end &&
             m_current->probeSequenceLength == kTombstone) {
        ++m_current;
      }
    }

    InternalEntry* m_current;
    InternalEntry* m_end;
  };

  class ConstIterator {
   public:
    ConstIterator(const InternalEntry* current, const InternalEntry* end)
        : m_current(current), m_end(end) {
      skipEmpty();
    }

    const Entry& operator*() const { return m_current->entry; }
    const Entry* operator->() const { return &m_current->entry; }

    ConstIterator& operator++() {
      ++m_current;
      skipEmpty();
      return *this;
    }

    bool operator!=(const ConstIterator& other) const {
      return m_current != other.m_current;
    }

    bool operator==(const ConstIterator& other) const {
      return m_current == other.m_current;
    }

   private:
    void skipEmpty() {
      while (m_current != m_end &&
             m_current->probeSequenceLength == kTombstone) {
        ++m_current;
      }
    }

    const InternalEntry* m_current;
    const InternalEntry* m_end;
  };

  Iterator begin() { return Iterator(m_data.begin(), m_data.end()); }
  Iterator end() { return Iterator(m_data.end(), m_data.end()); }
  ConstIterator begin() const {
    return ConstIterator(m_data.begin(), m_data.end());
  }
  ConstIterator end() const {
    return ConstIterator(m_data.end(), m_data.end());
  }

 private:
  // ------------------------------------------------------------------------ //
  // Private Helpers
  // ------------------------------------------------------------------------ //

  bool resize(u64 newCapacity);
  void removeAtBucket(u64 bucket);

  List<InternalEntry> m_data;
  u64 m_size = 0;
  f32 m_maxLoadFactor = kDefaultMaxLoadFactor;
};

// ========================================================================== //
// Template Implementation
// ========================================================================== //

template <typename Key, typename Value, typename HashFn, typename EqualFn>
Map<Key, Value, HashFn, EqualFn>::Map(IAllocator& allocator)
    : Map(kDefaultCapacity, kDefaultMaxLoadFactor, allocator) {}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
Map<Key, Value, HashFn, EqualFn>::Map(u64 capacity, f32 maxLoadFactor,
                                      IAllocator& allocator)
    : m_data(capacity, allocator), m_size(0), m_maxLoadFactor(maxLoadFactor) {
  DC_ASSERT(maxLoadFactor <= 0.9f, "Max load factor must be <= 0.9");

  // Initialize all entries to empty (PSL = 0)
  m_data.resize(capacity);
  for (u64 i = 0; i < capacity; ++i) {
    m_data[i].probeSequenceLength = kTombstone;
  }
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
Map<Key, Value, HashFn, EqualFn> Map<Key, Value, HashFn, EqualFn>::clone()
    const {
  // Note: clone() requires Key to be copy-constructible
  Map result(getCapacity(), m_maxLoadFactor);
  for (const auto& entry : *this) {
    Key keyCopy = entry.key;
    Value* val = result.insert(dc::move(keyCopy));
    if (val) {
      *val = entry.value;
    }
  }
  return result;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
Value* Map<Key, Value, HashFn, EqualFn>::insert(Key key) {
  // Check if we need to resize
  if (static_cast<f32>(m_size) / static_cast<f32>(getCapacity()) >
      m_maxLoadFactor) {
    if (!resize(getCapacity() * 2)) {
      return nullptr;  // Resize failed
    }
  }

  const u64 hash = HashFn{}(key);
  u64 bucket = hash % getCapacity();

  u32 probeSequenceLength = 1;
  for (;;) {
    InternalEntry* entry = &m_data[bucket];

    if (probeSequenceLength > entry->probeSequenceLength) {
      if (entry->probeSequenceLength == 0) {
        // Bucket empty
        m_data[bucket].probeSequenceLength = probeSequenceLength;
        if constexpr (isTriviallyRelocatable<Key>) {
          memcpy(&m_data[bucket].entry.key, &key, sizeof(Key));
        } else {
          new (&m_data[bucket].entry.key) Key(dc::move(key));
        }
        m_size += 1;
        return &m_data[bucket].entry.value;
      }

      // Bucket occupied, lets move it. Robin hood!
      // We need to:
      // 1. Take the existing entry's key and value (displaced)
      // 2. Put our new key in this bucket
      // 3. Recursively insert the displaced key
      // 4. Move the displaced value to the new location

      // Swap PSLs
      dc::swap(m_data[bucket].probeSequenceLength, probeSequenceLength);

      // Swap keys (now 'key' holds the displaced key)
      dc::swap(m_data[bucket].entry.key, key);

      // Save displaced value before we potentially overwrite it
      Value displacedValue = dc::move(m_data[bucket].entry.value);

      // valueOut points to where caller will write the NEW value
      Value* valueOut = &m_data[bucket].entry.value;

      // Recursively insert the displaced key
      Value* insertValue = insert(dc::move(key));
      if (!insertValue) {
        // Fatal: we've already modified state and can't easily rollback
        DC_FATAL_ASSERT(false,
                        "Failed to insert displaced entry in robin hood");
        return nullptr;
      }

      // Move displaced value to its new home
      *insertValue = dc::move(displacedValue);

      return valueOut;
    }

    // Bucket occupied and with smaller PSL than ours, try next
    ++probeSequenceLength;
    ++bucket;
    if (bucket >= getCapacity()) {
      bucket = 0;
    }
  }
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
typename Map<Key, Value, HashFn, EqualFn>::Entry*
Map<Key, Value, HashFn, EqualFn>::tryGet(const Key& key) {
  if (getCapacity() == 0) {
    return nullptr;
  }

  const u64 hash = HashFn{}(key);
  u64 bucket = hash % getCapacity();
  u32 probeSequenceLength = 1;

  for (;;) {
    if (probeSequenceLength > m_data[bucket].probeSequenceLength) {
      // If empty OR our PSL exceeds what's stored here, key doesn't exist
      break;
    }

    if (EqualFn{}(key, m_data[bucket].entry.key)) {
      return &m_data[bucket].entry;
    }

    ++probeSequenceLength;
    ++bucket;
    if (bucket >= getCapacity()) {
      bucket = 0;
    }
  }

  return nullptr;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
const typename Map<Key, Value, HashFn, EqualFn>::Entry*
Map<Key, Value, HashFn, EqualFn>::tryGet(const Key& key) const {
  if (getCapacity() == 0) {
    return nullptr;
  }

  const u64 hash = HashFn{}(key);
  u64 bucket = hash % getCapacity();
  u32 probeSequenceLength = 1;

  for (;;) {
    if (probeSequenceLength > m_data[bucket].probeSequenceLength) {
      // If empty OR our PSL exceeds what's stored here, key doesn't exist
      break;
    }

    if (EqualFn{}(key, m_data[bucket].entry.key)) {
      return &m_data[bucket].entry;
    }

    ++probeSequenceLength;
    ++bucket;
    if (bucket >= getCapacity()) {
      bucket = 0;
    }
  }

  return nullptr;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
Value* Map<Key, Value, HashFn, EqualFn>::operator[](const Key& key) {
  Entry* entry = tryGet(key);
  if (entry) {
    return &entry->value;
  }

  Value* val = insert(key);
  if (val == nullptr) {
    return nullptr;  // Allocation failed
  }
  new (val) Value();
  return val;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
bool Map<Key, Value, HashFn, EqualFn>::remove(const Key& key) {
  return remove(key, nullptr);
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
bool Map<Key, Value, HashFn, EqualFn>::remove(const Key& key, Value* valueOut) {
  Entry* userEntry = tryGet(key);
  if (!userEntry) {
    return false;
  }

  // Copy out the value if requested
  if (valueOut) {
    if constexpr (isTriviallyRelocatable<Value>) {
      memcpy(valueOut, &userEntry->value, sizeof(Value));
    } else {
      new (valueOut) Value(dc::move(userEntry->value));
    }
  }

  InternalEntry* entry = reinterpret_cast<InternalEntry*>(
      reinterpret_cast<uintptr>(userEntry) - sizeof(u32));

  u64 bucket = (reinterpret_cast<uintptr>(entry) -
                reinterpret_cast<uintptr>(m_data.begin())) /
               sizeof(InternalEntry);

  removeAtBucket(bucket);
  return true;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
void Map<Key, Value, HashFn, EqualFn>::remove(Entry& entry) {
  InternalEntry* internalEntry = reinterpret_cast<InternalEntry*>(
      reinterpret_cast<uintptr>(&entry) - sizeof(u32));

  u64 bucket = (reinterpret_cast<uintptr>(internalEntry) -
                reinterpret_cast<uintptr>(m_data.begin())) /
               sizeof(InternalEntry);

  removeAtBucket(bucket);
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
void Map<Key, Value, HashFn, EqualFn>::removeAtBucket(u64 bucket) {
  m_size -= 1;

  // Backshift entries with PSL > 1 to fill the gap
  for (;;) {
    u64 nextBucket = bucket + 1;
    if (nextBucket >= getCapacity()) {
      nextBucket = 0;
    }

    if (m_data[nextBucket].probeSequenceLength <= 1) {
      // No more entries to backshift, mark current bucket as empty
      m_data[bucket].probeSequenceLength = kTombstone;

      // Destroy the key/value in this slot
      // (safe to call destructor on moved-from objects)
      if constexpr (!isTriviallyRelocatable<Key>) {
        m_data[bucket].entry.key.~Key();
      }
      if constexpr (!isTriviallyRelocatable<Value>) {
        m_data[bucket].entry.value.~Value();
      }
      break;
    }

    // Backshift the next entry into current bucket
    m_data[bucket].probeSequenceLength =
        m_data[nextBucket].probeSequenceLength - 1;

    // Move key
    if constexpr (isTriviallyRelocatable<Key>) {
      memcpy(&m_data[bucket].entry.key, &m_data[nextBucket].entry.key,
             sizeof(Key));
    } else {
      m_data[bucket].entry.key.~Key();
      new (&m_data[bucket].entry.key)
          Key(dc::move(m_data[nextBucket].entry.key));
    }

    // Move value
    if constexpr (isTriviallyRelocatable<Value>) {
      memcpy(&m_data[bucket].entry.value, &m_data[nextBucket].entry.value,
             sizeof(Value));
    } else {
      m_data[bucket].entry.value.~Value();
      new (&m_data[bucket].entry.value)
          Value(dc::move(m_data[nextBucket].entry.value));
    }

    bucket = nextBucket;
  }
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
template <typename Fn, bool enable>
void Map<Key, Value, HashFn, EqualFn>::removeIf(Fn fn) {
  static_assert(isInvocable<Fn, const Entry&>,
                "Cannot call 'Fn', is it a function with argument 'const "
                "Entry&'?");
  static_assert(isSame<InvokeResultT<Fn, const Entry&>, bool>,
                "The return type of 'Fn' is not bool.");

  if (isEmpty()) {
    return;
  }

  // Collect keys to remove (can't modify while iterating)
  List<Key, 16> keysToRemove;
  for (const auto& entry : *this) {
    if (fn(entry)) {
      keysToRemove.add(entry.key);
    }
  }

  // Remove all marked entries
  for (const auto& key : keysToRemove) {
    remove(key);
  }
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
void Map<Key, Value, HashFn, EqualFn>::clear() {
  // Destroy all entries
  if constexpr (!isTriviallyRelocatable<Key> ||
                !isTriviallyRelocatable<Value>) {
    for (u64 i = 0; i < getCapacity(); ++i) {
      if (m_data[i].probeSequenceLength != kTombstone) {
        if constexpr (!isTriviallyRelocatable<Key>) {
          m_data[i].entry.key.~Key();
        }
        if constexpr (!isTriviallyRelocatable<Value>) {
          m_data[i].entry.value.~Value();
        }
      }
    }
  }

  // Reset all PSL to 0
  for (u64 i = 0; i < getCapacity(); ++i) {
    m_data[i].probeSequenceLength = kTombstone;
  }
  m_size = 0;
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
void Map<Key, Value, HashFn, EqualFn>::reserve(u64 newCapacity) {
  if (newCapacity > getCapacity()) {
    resize(newCapacity);
  }
}

template <typename Key, typename Value, typename HashFn, typename EqualFn>
bool Map<Key, Value, HashFn, EqualFn>::resize(u64 newCapacity) {
  if (newCapacity <= getCapacity()) {
    return true;
  }

  // Save old data
  List<InternalEntry, 1> oldData = dc::move(m_data);
  const u64 oldSize = m_size;

  // Allocate new data
  m_data = List<InternalEntry, 1>(newCapacity);
  m_data.resize(newCapacity);
  for (u64 i = 0; i < newCapacity; ++i) {
    m_data[i].probeSequenceLength = kTombstone;
  }
  m_size = 0;

  // Rehash all entries
  for (u64 i = 0; i < oldData.getCapacity(); ++i) {
    if (oldData[i].probeSequenceLength == kTombstone) {
      continue;
    }

    Value* value = insert(dc::move(oldData[i].entry.key));
    if (!value) {
      // Revert the resizing - need to restore keys that were moved
      m_data = dc::move(oldData);
      m_size = oldSize;
      return false;
    }

    if constexpr (isTriviallyRelocatable<Value>) {
      memcpy(value, &oldData[i].entry.value, sizeof(Value));
    } else {
      new (value) Value(dc::move(oldData[i].entry.value));
      oldData[i].entry.value.~Value();
    }
  }

  // Keys were moved, values were moved, old data will be cleaned up by List
  // destructor

  return true;
}

}  // namespace dc
