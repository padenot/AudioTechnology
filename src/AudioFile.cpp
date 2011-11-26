#include "AudioFile.hpp"

AudioFile::AudioFile(const char* filename, size_t chunk_size, int samplerate, int channels, int format) 
:filename_(filename),
	chunk_size_(chunk_size)
{
	infos_.samplerate = samplerate;
	infos_.channels = channels;
	infos_.format = format;
	VAGG_SYSCALL(sf_format_check(&infos_));
}

AudioFile::~AudioFile()
{
	if (sf_close(file_) != 0) {
		VAGG_LOG(VAGG_LOG_OK, "Error while closing the file.");
	} else {
		VAGG_LOG(VAGG_LOG_OK, "File %s closed.", filename_);
	}
}

void AudioFile::open(const AudioFile::Mode mode) 
{
	file_ = sf_open(filename_, mode, &infos_);
	if (file_ == NULL) {
		VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file_));
		abort();
	} else {
		VAGG_LOG(VAGG_LOG_OK, "File %s opened", filename_);
	}
}

size_t AudioFile::read_some(AudioBuffer& buffer)
{
  if (!file_) {
    VAGG_LOG(VAGG_LOG_FATAL, "Could not call %s, file not opened.", __func__);
    return -1;
  }
	size_t count;
	count = sf_read_float(file_, &buffer.front(), chunk_size_ * infos_.channels);
  if (count != chunk_size_ * infos_.channels) {
    VAGG_LOG(VAGG_LOG_WARNING, "Bar read, asked=%zu, written=%zu", chunk_size_ * infos_.channels, count);
  }
	return count;
}

size_t AudioFile::write_some(AudioBuffer& buffer)
{
  if (!file_) {
    VAGG_LOG(VAGG_LOG_FATAL, "Could not call %s, file not opened.", __func__);
    return -1;
  }

  VAGG_LOG(VAGG_LOG_DEBUG, "Size : %zu, channels : %d, chunk_size: %zu", buffer.size(), infos_.channels, chunk_size_);
  VAGG_ASSERT(buffer.size() == chunk_size_ * infos_.channels, "Bad size for write_some");
  size_t count;
  count = sf_writef_float(file_, &buffer.front(), chunk_size_ * infos_.channels);
  if (count != chunk_size_ * infos_.channels) {
    VAGG_LOG(VAGG_LOG_WARNING, "Bad read, asked=%zu, written=%zu", chunk_size_ * infos_.channels, count);
  }
  return count;
}
