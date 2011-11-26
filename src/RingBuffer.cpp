#include "RingBuffer.hpp"


RingBuffer::RingBuffer(size_t chunk_size, size_t chunk_count)
  :chunk_size_(chunk_size)
  ,chunk_count_(chunk_count)
  ,next_writable_(0)
  ,next_readable_(0)
{
  data_ = new AudioBuffer*[chunk_count_];
  for (size_t i = 0; i < chunk_count_; i++) {
    data_[i] = new AudioBuffer(chunk_size_);
  }
  status_ = new Status[chunk_count_];
  for(size_t i = 0; i < chunk_count_; i++) {
    status_[i] = Writable;
  }
}

RingBuffer::~RingBuffer()
{
  for (size_t i = 0; i < chunk_count_; i++) {
    delete data_[i];
  }
  delete [] data_;
  delete [] status_;
}

size_t RingBuffer::available_read()
{
  return count(Readable);
}

size_t RingBuffer::available_write()
{
  return count(Writable);
}

size_t RingBuffer::count(Status kind)
{
  size_t count = 0;
  for (size_t i = 0; i < chunk_count_; i++) {
    if (status_[i] == kind) {
      count++;
    }
  }
  return count;
}

bool RingBuffer::write(AudioBuffer* buffer)
{
  if (next_writable_ == -1) {
#ifdef DEBUG_RINGBUFFER
    VAGG_LOG(VAGG_LOG_WARNING, "Could not add the buffer in the RingBuffer : it is full.");
#endif
    return true;
  }
  *data_[next_writable_] = *buffer;
  status_[next_writable_] = Readable;
  if (next_readable_ == -1) {
    next_readable_ = next_writable_;
  }
  next_writable_ = (next_writable_ + 1) % chunk_count_;
  if (available_write() == 0) {
    next_writable_ = -1;
  }
  return false;
}


bool RingBuffer::read(AudioBuffer* buffer)
{
  if (next_readable_ == -1) {
#ifdef DEBUG_RINGBUFFER
    VAGG_LOG(VAGG_LOG_WARNING, "Could not return buffer from RingBuffer, it is empty.");
#endif
    return true;
  }
  size_t next = next_readable_;
  if (next_writable_ == -1) {
    next_writable_ = next;
  }
  *buffer = *data_[next];
  status_[next] = Writable;
  next_readable_ = (next_readable_ + 1) % chunk_count_;
  if (available_read() == 0) {
    next_readable_ = -1;
  }
  return false;
}

bool RingBuffer::write_raw(SamplesType* data, size_t length)
{
  if (next_writable_ == -1) {
#ifdef DEBUG_RINGBUFFER
    VAGG_LOG(VAGG_LOG_WARNING, "Could not add the buffer in the RingBuffer : it is full.");
#endif
    return true;
  }
  VAGG_ASSERT(length == chunk_size_, "Bad length in write_raw");
  for (size_t i = 0; i < length; i++) {
    (*data_[next_writable_])[i] = data[i];
  }
  // memcpy(data_[next_writable_], data, length);
  status_[next_writable_] = Readable;
  if (next_readable_ == -1) {
    next_readable_ = next_writable_;
  }
  next_writable_ = (next_writable_ + 1) % chunk_count_;
  if (available_write() == 0) {
    next_writable_ = -1;
  }
  return false;
}

#ifdef DEBUG_RINGBUFFER
void RingBuffer::dump()
{
  printf("[\n");
  for (size_t i = 0; i < chunk_count_; i++) {
    printf("\t[");
    if (status_[i] == Writable) {
      printf("Free]\n");
    } else {
      for (size_t j = 0; j < chunk_size_ - 1; j++) {
        printf("%f ", (*data_[i])[j]);
      }
      printf("%f]\n", (*data_[i]).back());
    }
  }
  printf("]\n");
}
#endif
