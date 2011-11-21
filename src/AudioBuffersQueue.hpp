#ifndef AUDIOBUFFERSQUEUE_H
#define AUDIOBUFFERSQUEUE_H

#include "vagg/vagg_macros.h"
#include "types.hpp"

class AudioBuffersQueue {
  public:
    AudioBuffersQueue(size_t chunk_size);
    size_t available();
    void push(AudioBuffer* buffer);
    AudioBuffer* pop();
    void dispose(AudioBuffer* buffer);
    void close();
  protected:
    void clean();
    BufferList buffers_;
    BufferList dirty_buffers_;
    const size_t chunks_size_;
};

#endif
