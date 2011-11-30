#include "AudioFile.hpp"

AudioFile::AudioFile(const char* filename, int channels, int samplerate, int format)
:filename_(filename)
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

size_t AudioFile::read_some(AudioBuffer buffer, size_t size)
{
  if (!file_) {
    VAGG_LOG(VAGG_LOG_FATAL, "Could not call %s, file not opened.", __func__);
    return -1;
  }
  size_t count;
  count = sf_read_float(file_, buffer, size);
  if (count != size) {
    VAGG_LOG(VAGG_LOG_WARNING, "Bar read, asked=%zu, written=%zu", size, count);
  }
  return count;
}

size_t AudioFile::write_some(AudioBuffer buffer, size_t size)
{
  if (!file_) {
    VAGG_LOG(VAGG_LOG_FATAL, "Could not call %s, file not opened.", __func__);
    return -1;
  }

  size_t count;
  count = sf_writef_float(file_, buffer, size);
  if (count != size) {
    VAGG_LOG(VAGG_LOG_WARNING, "Bad read, asked=%zu, written=%zu", size, count);
  }
  return count;
}

int AudioFile::channels()
{
  return file_ ? infos_.channels : 0;
}

int AudioFile::samplerate()
{
  return file_ ? infos_.samplerate : 0;
}
