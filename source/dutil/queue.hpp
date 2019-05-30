/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
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

#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <mutex>
#include <vector>
#include "types.hpp"

namespace dutil {

enum class QueueResult : u8 { kSuccess = 0, kFull, kEmpty };

constexpr bool kNoMutex = false;
constexpr bool kUseMutex = true;

// forward declaration for the iterator
template <typename T, bool TUseMutex>
class Queue;

/**
 * Iterator class to enable for range loops.
 */
template <typename T, bool TUseMutex>
class QueueIterator {
 public:
  QueueIterator(const Queue<T, TUseMutex>* queue, size_t pos)
      : pos_(pos), queue_(queue) {}

  bool operator!=(const QueueIterator& other) const;

  T operator*() const;

  const QueueIterator& operator++();

  const QueueIterator& operator--();

 private:
  size_t pos_;
  const Queue<T, TUseMutex>* queue_;
};

/**
 * Queue implemented with a ring buffer using std::vector as container.
 *
 * @tparam T The type the queue will hold
 * @tparam TUseMutex will decide if the class will be thread safe.
 *
 * TODO replace some mutex locks with atomic CAS operations for better perf.
 */
template <typename T, bool TUseMutex = kUseMutex>
class Queue {
 public:
  /**
   * @param size Note, with a size of 256 you can store 255 elements.
   */
  Queue(const size_t size = 256);

  Queue(const Queue& other);
  Queue& operator=(const Queue& other);

  Queue(Queue&& other) noexcept;
  Queue& operator=(Queue&& other) noexcept;

  /**
   * Insert @elem at the end of the queue.
   */
  QueueResult Push(const T& elem);
  QueueResult Push(T&& elem);

  /**
   * Remove the first element in the queue and put it in @dataOut.
   */
  QueueResult Pop(T& dataOut);

  /**
   * Is the queue empty?
   */
  bool Empty() {
    if constexpr (TUseMutex) {
      std::lock_guard<std::mutex> lock(mutex_);
      return queue_position_.Empty();
    } else {
      return queue_position_.Empty();
    }
  }

  /**
   * How many elements are in the queue.
   */
  size_t Size() {
    if constexpr (TUseMutex) {
      std::lock_guard<std::mutex> lock(mutex_);
      return queue_position_.getSize();
    } else {
      return queue_position_.getSize();
    }
  }

  /**
   * How many elements can fit in the queue.
   */
  size_t Capacity() const {
    return queue_position_.total_size();
  }

  /**
   * Raw access to the underlying container, used by iterator.
   */
  T UnsafeGet(const size_t pos) const { return vector_[pos]; }

  /**
   * Iterator pointing to beginning / front of the queue.
   */
  QueueIterator<T, TUseMutex> begin() const {
    const size_t front = queue_position_.front() + 1 < Capacity()
                             ? queue_position_.front() + 1
                             : 0;
    return QueueIterator<T, TUseMutex>(this, front);
  }

  /**
   * Iterator pointing to one step beyond the end / back of the queue.
   */
  QueueIterator<T, TUseMutex> end() const {
    const size_t back = queue_position_.back() + 1 < Capacity()
                      ? queue_position_.back() + 1
                      : 0;
    return QueueIterator<T, TUseMutex>(this, back);
  }

  /**
   * Get the item in the front of the queue.
   */
  T Front() const { return *begin(); }

  /**
   * Get the item in back of queue.
   * @pre Must not be empty queue.
   */
  T Back() const { return *--end(); }

 private:
  class QueuePosition {
   public:
    /**
     * As of now, when setting total_size, you can store total_size-1 elements.
     */
    QueuePosition(size_t total_size = 256)
        : front_(0), back_(0), total_size_(total_size) {}

    QueueResult AddBack() {
      const size_t tmp = back_;
      back_ = back_ + 1 < total_size_ ? back_ + 1 : 0;
      QueueResult res;
      if (back_ != front_) {
        res = QueueResult::kSuccess;
      } else {
        back_ = tmp;
        res = QueueResult::kFull;
      }
      return res;
    }

    QueueResult RemoveFront() {
      QueueResult res;
      if (front_ != back_) {
        front_ = front_ + 1 < total_size_ ? front_ + 1 : 0;
        res = QueueResult::kSuccess;
      } else {
        res = QueueResult::kEmpty;
      }
      return res;
    }

    size_t back() const { return back_; }

    size_t front() const { return front_; }

    bool Empty() const { return front_ == back_; }

    size_t GetSize() const {
      if (back_ >= front_)
        return back_ - front_;
      else
        return total_size_ - front_ + back_;
    }

    size_t total_size() const { return total_size_; }

   private:
    size_t front_;
    size_t back_;
    size_t total_size_;
  };

 private:
  std::vector<T> vector_;
  std::mutex mutex_{};
  QueuePosition queue_position_;
};

// ============================================================ //
// Template Definition
// ============================================================ //

template <typename T, bool TUseMutex>
Queue<T, TUseMutex>::Queue(size_t size)
    : vector_(size), queue_position_(size) {}

template <typename T, bool TUseMutex>
Queue<T, TUseMutex>::Queue(const Queue& other) : vector_(), queue_position_() {
  std::lock_guard<std::mutex> lock(mutex_);
  vector_ = other.vector_;
  queue_position_ = other.queue_position_;
}

template <typename T, bool TUseMutex>
Queue<T, TUseMutex>& Queue<T, TUseMutex>::operator=(const Queue& other) {
  if (this != &other) {
    std::lock_guard<std::mutex> lock(mutex_);
    vector_ = other.vector_;
    queue_position_ = other.queue_position_;
  }
  return *this;
}

template <typename T, bool TUseMutex>
Queue<T, TUseMutex>::Queue(Queue&& other) noexcept
    : vector_(), queue_position_(0) {
  std::lock_guard<std::mutex> lock(mutex_);
  vector_ = std::move(other.vector_);
  queue_position_ = std::move(other.queue_position_);
}

template <typename T, bool TUseMutex>
Queue<T, TUseMutex>& Queue<T, TUseMutex>::operator=(Queue&& other) noexcept {
  if (this != &other) {
    std::lock_guard<std::mutex> lock(mutex_);
    vector_ = std::move(other.vector_);
    queue_position_ = std::move(other.queue_position_);
  }
  return *this;
}

template <typename T, bool TUseMutex>
QueueResult Queue<T, TUseMutex>::Push(const T& elem) {
  if constexpr (TUseMutex == kUseMutex) {
    std::lock_guard<std::mutex> lock(mutex_);
    const QueueResult res = this->queue_position_.AddBack();
    if (res == QueueResult::kSuccess) {
      vector_[this->queue_position_.back()] = elem;
    }
    return res;
  } else {
    const QueueResult res = this->queue_position_.AddBack();
    if (res == QueueResult::kSuccess) {
      vector_[this->queue_position_.back()] = elem;
    }
    return res;
  }
}

template <typename T, bool TUseMutex>
QueueResult Queue<T, TUseMutex>::Push(T&& elem) {
  if constexpr (TUseMutex == kUseMutex) {
    std::lock_guard<std::mutex> lock(mutex_);
    const QueueResult res = this->queue_position_.AddBack();
    if (res == QueueResult::kSuccess) {
      vector_[this->queue_position_.back()] = std::move(elem);
    }
    return res;
  } else {
    const QueueResult res = this->queue_position_.AddBack();
    if (res == QueueResult::kSuccess) {
      vector_[this->queue_position_.back()] = std::move(elem);
    }
    return res;
  }
}

template <typename T, bool TUseMutex>
QueueResult Queue<T, TUseMutex>::Pop(T& dataOut) {
  QueueResult res;
  if constexpr (TUseMutex == kUseMutex) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!this->queue_position_.Empty()) {
      res = this->queue_position_.RemoveFront();
      dataOut = std::move(vector_[queue_position_.front()]);
    } else {
      res = QueueResult::kEmpty;
    }
  } else {
    if (!this->queue_position_.Empty()) {
      res = this->queue_position_.RemoveFront();
      dataOut = std::move(vector_[queue_position_.front()]);
    } else {
      res = QueueResult::kEmpty;
    }
  }
  return res;
}

// ============================================================ //
// Template definition for QueueIterator
// ============================================================ //

template <typename T, bool TUseMutex>
bool QueueIterator<T, TUseMutex>::operator!=(const QueueIterator& other) const {
  return pos_ != other.pos_;
}

template <typename T, bool TUseMutex>
T QueueIterator<T, TUseMutex>::operator*() const {
  return queue_->UnsafeGet(pos_);
}

template <typename T, bool TUseMutex>
const QueueIterator<T, TUseMutex>& QueueIterator<T, TUseMutex>::operator++() {
  pos_ = pos_ + 1 < queue_->Capacity() ? pos_ + 1 : 0;
  return *this;
}

template <typename T, bool TUseMutex>
const QueueIterator<T, TUseMutex>& QueueIterator<T, TUseMutex>::operator--() {
  pos_ = pos_ - 1 >= 0 ? pos_ - 1 : queue_->Capacity();
  return *this;
}

}  // namespace dutil

#endif  // QUEUE_HPP_
