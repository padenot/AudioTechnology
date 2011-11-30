#ifndef RINGBUFFER_HPP 
#define RINGBUFFER_HPP

#include "vagg/vagg_macros.h"
#include "types.hpp"

#define DEBUG_RINGBUFFER

/**
 * @brief A ring buffer, with a fixed size.
 */
template<typename T, size_t Slots>
class RingBuffer {
  public:
    RingBuffer(size_t slots_size)
      :head_(0),tail_(0),slots_size_(slots_size)
    {
      for (size_t i = 0; i < Slots; i++) {
        data_[i] = new T[slots_size];
      }
    }

    ~RingBuffer() {
      for (size_t i = 0; i < Slots; i++) {
        delete data_[i];
      }
    }
    bool empty() const
    {
      return head_ == tail_;
    }

    bool full() const
    {
      return ((tail_ + 1) % Capacity == head_);
    }

    bool push(SamplesType* data, size_t length)
    {
      if (length != slots_size_) {
        VAGG_LOG(VAGG_LOG_FATAL, "Bad push size");
        return false;
      }
      size_t next = increment(tail_);
      if(next != head_) {
        for (size_t i = 0; i < length; i++) {
          data_[tail_][i] = data[i];
        }
        tail_ = next;
        return true;
      }
      return false;
    }

    bool pop(SamplesType* data, size_t length)
    {
      if (length != slots_size_) {
        VAGG_LOG(VAGG_LOG_FATAL, "Bad pop size");
      }
      if(head_ == tail_) {
         return false;
      }
      for (size_t i = 0; i < length; i++) {
        data[i] = data_[head_][i];
      }
      head_ = increment(head_);
      return true;
    }

    enum {
      Capacity = Slots
    };

    size_t available_read()
    {
      if (head_ <= tail_) {
        return tail_ - head_;
      }
      return Capacity - (head_ - tail_);
    }

  protected:
    size_t increment(size_t index)
    {
      return (index + 1) % Capacity;
    }

    volatile size_t head_;
    volatile size_t tail_;
    const size_t slots_size_;
    T* data_[Capacity];
};

#endif
