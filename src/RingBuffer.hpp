#ifndef RINGBUFFER_HPP 
#define RINGBUFFER_HPP

#include "vagg/vagg_macros.h"
#include "types.hpp"

#define DEBUG_RINGBUFFER

/**
 * @brief A ring buffer, with a fixed size.
 */
class RingBuffer {
  public:
		/**
		 * @brief Create a ringbuffer.
		 *
		 * @param chunk_size The size of a buffer.
		 * @param chunk_count The number of buffers available.
		 */
    RingBuffer(size_t chunk_size, size_t chunk_count);
		/**
		 * @brief Deallocate the buffers.
		 */
    ~RingBuffer();
		/**
		 * @brief Get the number of available buffers that can be written to the
		 * ringbuffer.
		 *
		 * @return The number of buffer that can be written to the ringbuffer.
		 */
    size_t available_read();
		/**
		 * @brief Get the number of available buffers that can be read from the
		 * ringbuffer.
		 *
		 * @return The number of buffer that can be read from the ringbuffer.
		 */
    size_t available_write();
		/**
		 * @brief Write a buffer to the ring buffer.
		 *
		 * @param buffer The buffer to write in the ring buffer.
		 *
		 * @return True if failure (if the buffer is full), false if success.
		 */
    bool write(AudioBuffer* buffer);
		/**
		 * @brief Write an array to the ring buffer.
		 *
		 * This method should be used in place where we receive a array of samples,
		 * and we can't create a AudioBuffer (because it mallocs), for example in a
		 * callback.
		 *
		 * @param buffer The array to write in the ring buffer.
		 *
		 * @return True if failure (if the buffer is full), false if success.
		 */
		bool write_raw(SamplesType* data, size_t length);
		/**
		 * @brief Get a buffer from the ringbuffer.
		 *
		 * @param buffer A pointer, user allocated, in which the data will be
		 * copied.
		 *
		 * @return  True if failure (if the buffer is empty), false if success.
		 */
    bool read(AudioBuffer* buffer);
#ifdef DEBUG_RINGBUFFER
		/**
		 * @brief For debuggging purposes, dumps the content of the buffer in the
		 * error output.
		 */
    void dump();
#endif
  protected:
		/**
		 * @brief This enum describe the status of a chunk : eigher readable (full)
		 * or writable (empty).
		 */
    enum Status {
      Writable,
      Readable
    };
		/**
		 * @brief Count the number of buffers either writable or readable.
		 *
		 * @param kind A type of buffer.
		 *
		 * @return The number of buffers either readable or writable.
		 */
    size_t count(Status kind);
		/**
		 * @brief The actual data.
		 */
    AudioBuffer** data_;
		/**
		 * @brief The status of the buffers.
		 */
    Status* status_;
		/**
		 * @brief The size of a slot in the ringbuffer.
		 */
    const size_t chunk_size_;
		/**
		 * @brief The number of slots available in this ringbuffer
		 */
    const size_t chunk_count_;
		/**
		 * @brief The index of the next writable buffer.
		 */
    int next_writable_;
		/**
		 * @brief The index of the next readable buffer.
		 */
    int next_readable_;
};

#endif
