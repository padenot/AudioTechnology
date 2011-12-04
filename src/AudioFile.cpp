#include "AudioFile.hpp"

AudioFile::AudioFile(const char* filename, int format)
:filename_(filename)
{
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
  delete [] filename_;
}

int AudioFile::open(const AudioFile::Mode mode)
{
  if (mode == Write) {
    infos_.samplerate = 44100;
    infos_.channels = 1;
  }
  file_ = sf_open(filename_, mode, &infos_);
  if (file_ == NULL) {
    VAGG_LOG(VAGG_LOG_FATAL, "Open file error : %s", sf_strerror(file_));
    return -1;
  } else {
    VAGG_LOG(VAGG_LOG_OK, "File %s opened", filename_);
    get_duration();
    return 0;
  }
}

int AudioFile::seek(double ms)
{
  if (infos_.seekable) {
    sf_count_t offset = ms * infos_.samplerate * infos_.channels;
    sf_count_t count = sf_seek(file_, offset, SEEK_SET);
    if (count == -1) {
      VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file_));
    }
  }
  return 0;
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
    VAGG_LOG(VAGG_LOG_WARNING, "End of file, asked=%zu, written=%zu", size, count);
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

double AudioFile::duration()
{
  return duration_;
}

void AudioFile::get_duration() {
  sf_count_t count;
  if (infos_.seekable) {
    count = sf_seek(file_, 0, SEEK_SET);
    if (count == -1) {
      VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file_));
    }

    count = sf_seek(file_, 0, SEEK_END);
    if (count == -1) {
      VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file_));
    }
    duration_ = (double)count / infos_.samplerate;
    VAGG_LOG(VAGG_LOG_DEBUG, "Duration for %s is %lf", filename_, duration_);
  } else {
    duration_ = -1.0f;
  }

  count = sf_seek(file_, 0, SEEK_SET);
  if (count == -1) {
    VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file_));
  }
}

const char* AudioFile::path()
{
  return filename_;
}
