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
    enum Mode {
      Write = SFM_WRITE,
      Read = SFM_READ,
      ReadWrite = SFM_RDWR
    };
    AudioFile(const char* filename,
        int format = SF_FORMAT_WAV|SF_FORMAT_PCM_16);
    ~AudioFile();
    int open(Mode mode);
    /**
     * @brief Read some data from the file.
     *
     * @param buffer The buffer in which we should take the data.
     *
     * @return  The number of samples retrieved from the file.
     */
    size_t read_some(AudioBuffer buffer, size_t size);
    /**
     * @brief Write some data into a file.
     *
     * @param buffer The data to write in the file.
     */
    size_t write_some(AudioBuffer buffer, size_t size);

    int seek(double ms);

    /**
     * @brief Get the number of channels
     *
     * @return 0 if no file loaded, the number of channels otherwise
     */
    int channels();

    /**
     * @brief Get the samplerate
     *
     * @return 0 if no file loaded, the samplerate otherwise
     */
    int samplerate();

    double duration();
    const char* path();
  protected:
    void get_duration();
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
    char* filename_;

    /**
     * @brief The mode in which the file has been opened (read, write,
     * readwrite).
     */
    Mode mode_;

    double duration_;
};

#endif
