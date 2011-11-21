#ifndef TYPES_H
#define TYPES_H
#include <vector>
#include <list>

/**
 * @brief The type for a single sample.
 */
typedef float SamplesType;
/**
 * @brief A single audio buffer.
 */
typedef std::vector<SamplesType> AudioBuffer;
/**
 * @brief A group of sequential buffers.
 */
typedef std::list<AudioBuffer*> BufferList;

#endif
