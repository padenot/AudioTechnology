#include "AudioBuffersQueue.hpp"

// Full memory barrier using GCC intrinsics (or equivalent on Mac), to avoid
// using heavyweight synchronization like mutexes.
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#   define SYNC() __sync_synchronize();
#elif defined(__APPLE__)
#   include <libkern/OSAtomic.h>
#   define SYNC()  OSMemoryBarrier()
#else
#   error "This works only with a good enough GCC (>=4.1) or a Mac (maybe)."
#endif

AudioBuffersQueue::AudioBuffersQueue(size_t chunk_size)
:chunks_size_(chunk_size)
{ }

size_t AudioBuffersQueue::available()
{
  SYNC();
  if (buffers_.empty()) {
    return 0;
  }
  return (buffers_.size() - 1) * (chunks_size_) + buffers_.back()->size();
}

void AudioBuffersQueue::push(AudioBuffer* buffer)
{
  SYNC();
  buffers_.push_back(buffer);
  clean();
  SYNC();
}

AudioBuffer* AudioBuffersQueue::pop()
{
  SYNC();
  AudioBuffer* b = buffers_.front();
  buffers_.pop_front();
  SYNC();
  return b;
}

void AudioBuffersQueue::dispose(AudioBuffer* buffer)
{
  SYNC();
  dirty_buffers_.push_back(buffer);
  SYNC();
}

void AudioBuffersQueue::close()
{
  SYNC();
  clean();
  SYNC();
}

void AudioBuffersQueue::clean()
{
  SYNC();
  while(! dirty_buffers_.empty()) {
    AudioBuffer* b = dirty_buffers_.front();
    dirty_buffers_.pop_front();
    delete b;
  }
  SYNC();
}
