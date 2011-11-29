#include "AudioPlayer.hpp"
#include "vagg/vagg.h"

#define HANDLE_PA_ERROR(err)                                                            \
  Pa_Terminate();                                                                       \
  VAGG_LOG(VAGG_LOG_CRITICAL, "An error occured while using the portaudio stream, line %d", __LINE__ );  \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error number: %d", err );                              \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error message: %s", Pa_GetErrorText(err));             \
  return err;

AudioPlayer::AudioPlayer(const size_t chunk_size, const unsigned event_loop_frequency)
  :event_loop_frequency_(event_loop_frequency)
  ,chunk_size_(chunk_size)
  ,playback_state_(STOPPED)
{
  queue_ = new AudioBuffersQueue(chunk_size_);
}

AudioPlayer::~AudioPlayer()
{
  queue_->close();
  delete queue_;
}

int AudioPlayer::play()
{
  PaError err;
  if (! file_) {
    VAGG_LOG(VAGG_LOG_WARNING, "Cannot play, no file loaded");
    return -1;
  }

  playback_state_ = HAS_DATA;

  err = Pa_StartStream(stream_);
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  bool audio_playing = true;
  // Event loop
  while(audio_playing) {
    switch(playback_state_) {
      case HAS_DATA:
        break;
      case NEED_DATA:
        //VAGG_LOG(VAGG_LOG_OK, "Buffering");
        for(int i = 0; i < 2; i++) {
          AudioBuffer* buffer = new AudioBuffer();
          buffer->reserve(chunk_size_);
          if (file_->read_some(*buffer) != chunk_size_ * file_->channels()) {
            playback_state_ = SHOULD_STOP;
            break;
          }
          queue_->push(buffer);
        }
        break;
      case SHOULD_STOP:
        VAGG_LOG(VAGG_LOG_OK, "Draining audio.");
        break;
      case STOPPED:
        VAGG_LOG(VAGG_LOG_OK, "Stopping audio.");
        audio_playing = false;
        break;
    }
    Pa_Sleep(event_loop_frequency_);
  }
  VAGG_LOG(VAGG_LOG_OK, "Finished with playing");
  return 0;
}

void AudioPlayer::finished_callback( void* user_data)
{
  AudioPlayer* a = static_cast<AudioPlayer*>(user_data);
  a->finished_callback_m(user_data);
}

void AudioPlayer::finished_callback_m(void* VAGG_UNUSED(user_data))
{
  VAGG_LOG(VAGG_LOG_OK, "Finished.\n");
  playback_state_ = STOPPED;
}

int AudioPlayer::pause()
{
  if (! file_) {
    VAGG_LOG(VAGG_LOG_WARNING, "Cannot pause, no file loaded");
    return -1;
  }
  VAGG_LOG(VAGG_LOG_WARNING, "Not implemented.");
  return -1;
}

int AudioPlayer::seek(const double VAGG_UNUSED(ms))
{
  if (! file_) {
    VAGG_LOG(VAGG_LOG_WARNING, "Cannot pause, no file loaded");
    return -1;
  }
  VAGG_LOG(VAGG_LOG_WARNING, "Not implemented.");
  return -1;
}

int AudioPlayer::load(const char* file)
{
  PaError err;

  file_ = new AudioFile(file);
  file_->open(AudioFile::Read);

  err = Pa_Initialize();
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  output_params_.device = Pa_GetDefaultOutputDevice();
  if (output_params_.device == paNoDevice) {
    VAGG_LOG(VAGG_LOG_FATAL,"Error: No default output device.\n");
    HANDLE_PA_ERROR(err)
  }
  output_params_.channelCount = file_->channels();
  output_params_.sampleFormat = paFloat32;
  output_params_.suggestedLatency =
          Pa_GetDeviceInfo( output_params_.device )->defaultLowOutputLatency;
  output_params_.hostApiSpecificStreamInfo = 0;

  err = Pa_OpenStream(&stream_,
                      NULL,
                      &output_params_,
                      file_->samplerate(),
                      chunk_size_ * file_->channels(),
                      paClipOff,
                      &AudioPlayer::audio_callback,
                      this);

  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  err = Pa_SetStreamFinishedCallback(stream_, &AudioPlayer::finished_callback);
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  // Prebuffering
  {
    size_t c = 4;
    while(c--) {
      VAGG_LOG(VAGG_LOG_OK, "prebuffer %d", c);
      AudioBuffer* prebuffer = new AudioBuffer();
      prebuffer->reserve(chunk_size_ * file_->channels());
      size_t count = file_->read_some(*prebuffer);
      if (count != chunk_size_ * file_->channels()) {
        prebuffer->resize(count);
      }
      queue_->push(prebuffer);
    }
  }
  return 0;
}

int AudioPlayer::unload()
{
  PaError err;

  queue_->close();

  if (playback_state_ != STOPPED && stream_) {
    err = Pa_StopStream( stream_ );
    if(err != paNoError) {
      HANDLE_PA_ERROR(err);
    }
  }
  if (stream_) {
    err = Pa_CloseStream( stream_ );
    if(err != paNoError) {
      HANDLE_PA_ERROR(err);
    }
  }

  Pa_Terminate();
  return 0;
}

int AudioPlayer::audio_callback(const void * inputBuffer,
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData)
{
  AudioPlayer* a = static_cast<AudioPlayer*>(userData);
  return a->audio_callback_m(inputBuffer, outputBuffer, framesPerBuffer,
                                       timeInfo, statusFlags, userData);
}


/**
 * @brief The main callback that will push audio samples to the system's audio backend.
 *
 * @param inputBuffer A possible input buffer (unused).
 * @param outputBuffer The place to put the samples we want to play.
 * @param framesPerBuffer The number of frames per buffer.
 * @param timeInfo A time information for synchronization (unused).
 * @param statusFlags Some status flag (unused).
 * @param userData A user-defined pointer to carry data around.
 *
 * @return paContinue or paComplete, accordingly.
 */
int AudioPlayer::audio_callback_m(const void * VAGG_UNUSED(inputBuffer),
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* VAGG_UNUSED(timeInfo),
                                PaStreamCallbackFlags VAGG_UNUSED(statusFlags),
                                void *userData)
{
  float* out = (float*)outputBuffer;

  size_t available = queue_->available();
  //VAGG_LOG(VAGG_LOG_WARNING, "Available : %zu", available);

  // We have no data ! Output silence.
  if (available == 0) {
    VAGG_LOG(VAGG_LOG_DEBUG, "No data available");
    if ( playback_state_ == SHOULD_STOP) {
      return paComplete;
    } else {
    VAGG_LOG(VAGG_LOG_WARNING, "UNDERRUN");
    playback_state_ = NEED_DATA;
    for(size_t i = 0; i < framesPerBuffer; i++) {
      // Left
      *out++ = 0.0;
      // Right
      *out++ = 0.0;
    }
    return paContinue;
    }
  } else {
    AudioBuffer* buffer = queue_->pop();
    // XXX RMS
    //VAGG_LOG(VAGG_LOG_DEBUG, "%f", 10. * log10(rms(buffer, framesPerBuffer*2)));
    VAGG_ASSERT(buffer->size() == framesPerBuffer, "Bad buffer size.");

    size_t i = 0;
    while( i < framesPerBuffer * 2 ) {
      *out++ = (*buffer)[i++];
      *out++ = (*buffer)[i++];
    }
    queue_->dispose(buffer);

    // Tell the other thread that it should start buffering.
    if (playback_state_ == SHOULD_STOP) {
      return paContinue;
    } else if (available < framesPerBuffer * 4) {
      playback_state_ = NEED_DATA;
    } else {
      playback_state_ = HAS_DATA;
    }
  }
  return paContinue;
}
