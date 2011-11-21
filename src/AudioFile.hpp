#ifndef AUDIOFILE_HPP
#define AUDIOFILE_HPP
#include "types.hpp"
#include "vagg/vagg_macros.h"
#include <sndfile.h>

/**
 * @brief Read an audiofile and provide data.
 */
class AudioFile
{
	public:
		AudioFile(const char* filename,
				size_t chunk_size = 4096,
				int samplerate = 44100,
				int channels = 2,
				int format = SF_FORMAT_WAV|SF_FORMAT_PCM_16);
		~AudioFile();
		void open();
		/**
		 * @brief Read some data from the file.
		 *
		 * @param buffer The buffer in which we should take the data.
		 *
		 * @return  The number of samples retrieved from the file.
		 */
		size_t read_some(AudioBuffer& buffer);
	protected:
		/**
		 * @brief The file handle, for libsndfile.
		 */
		SNDFILE* file_;
		/**
		 * @brief The infos of the file, such as samplerate, samples format and
		 * number of channels.
		 */
		SF_INFO infos_;
		/**
		 * @brief The filename.
		 */
		const char* filename_;
		/**
		 * @brief The size of the buffers used.
		 */
		const size_t chunk_size_;
};

#endif
