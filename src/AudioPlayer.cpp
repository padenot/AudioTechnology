#include "AudioPlayer.hpp"
#include "vagg/vagg.h"


#define HANDLE_PA_ERROR(err)                                                            \
  Pa_Terminate();                                                                       \
  VAGG_LOG(VAGG_LOG_CRITICAL, "An error occured while using the portaudio stream, line %d", __LINE__ );  \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error number: %d", err );                              \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error message: %s", Pa_GetErrorText(err));             \
  return err;

AudioPlayer::AudioPlayer(const size_t chunk_size)
  :file_(0)
  ,chunk_size_(chunk_size)
  ,ring_buffer_(0)
  ,playback_state_(STOPPED)
  ,effect_(0)
  ,volume_(1.0)

{   
}

AudioPlayer::~AudioPlayer()
{
  unload();

   if(file_){
	delete file_;
	file_ = 0;
  }
  
  if(ring_buffer_){
	delete ring_buffer_;
	ring_buffer_ = 0;
  }
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

  return 0;
}

bool AudioPlayer::state_machine()
{
  switch(playback_state_) {
    case HAS_DATA:
      break;
    case NEED_DATA:
      {
        while(! ring_buffer_->full()) {
          size_t size = chunk_size_ * file_->channels();
          SamplesType b[size];
          if (file_->read_some(b, size) != size) {
            playback_state_ = SHOULD_STOP;
          }
          ring_buffer_->push(b, size);
        }
      }
      break;
    case SHOULD_STOP:
      break;
    case STOPPED:
      return false;
      break;
  }
  return true;
}

void AudioPlayer::finished_callback(void* user_data)
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
  PaError err = Pa_StopStream( stream_ );
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }
  return 0;
}

int AudioPlayer::seek(const double ms)
{
  if (! file_) {
    VAGG_LOG(VAGG_LOG_WARNING, "Cannot seek, no file loaded");
    return -1;
  }

  VAGG_LOG(VAGG_LOG_DEBUG, "Seeking to %lf", ms);

  file_->seek(ms);
  ring_buffer_->clear();
  if (playback_state_ != STOPPED) {
    prebuffer();
  }

  current_time_ = ms;

  return 0;
}

double AudioPlayer::current_time()
{
  return current_time_;
}

int AudioPlayer::insert(Effect* effect)
{
  effect_ = effect;
  return 0;
}

double AudioPlayer::duration()
{
  if (file_) {
    return file_->duration();
  }
  return 0;
}

int AudioPlayer::channels()
{
  if (file_) {
    return file_->channels();
  }
  return 0;
}

int AudioPlayer::samplerate()
{
  if (file_) {
    return file_->samplerate();
  }
  return 0;
}

int AudioPlayer::load(const char* file)
{
  VAGG_LOG(VAGG_LOG_DEBUG, "file_ %p",file_);
  VAGG_LOG(VAGG_LOG_DEBUG, "Loading %s.", file);
  PaError err;

  if(file_){
  	VAGG_LOG(VAGG_LOG_DEBUG, "Deleting old file_. %p",file_);
	delete file_;
	file_ = 0;
  }
  
  if(ring_buffer_){
   	VAGG_LOG(VAGG_LOG_DEBUG, "Deleting old ring-buffer.");
	delete ring_buffer_;
	ring_buffer_ = 0;
  }

  file_ = new AudioFile(file);
  if ((err = file_->open(AudioFile::Read))) {
    HANDLE_PA_ERROR(err);
    return err;
  }

  ring_buffer_ = new RingBuffer<SamplesType, 4>(chunk_size_ * file_->channels());

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
                      chunk_size_,
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

  prebuffer();

  return 0;
}

void AudioPlayer::prebuffer()
{
  while(! ring_buffer_->full()) {
    size_t size = chunk_size_ * file_->channels();
    SamplesType prebuffer[size];
    size_t count = file_->read_some(prebuffer, size);
    ring_buffer_->push(prebuffer, size);
    if (count != size) {
      break;
    }
  }
}

int AudioPlayer::unload()
{
  PaError err;

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
    stream_ = 0;
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
                                void *VAGG_UNUSED(userData))
{
  float* out = (float*)outputBuffer;

  // We have no data ! Output silence.
  if (ring_buffer_->empty()) {
    VAGG_LOG(VAGG_LOG_WARNING, "UNDERRUN");
    playback_state_ = NEED_DATA;
    for(size_t i = 0; i < framesPerBuffer; i++) {
      // Left
      *out++ = 0.0;
      // Right
      *out++ = 0.0;
    }
  } else {
    size_t channels = file_->channels();
    SamplesType buffer[framesPerBuffer * channels];
    ring_buffer_->pop(buffer, framesPerBuffer * channels);

    size_t i = 0;
    while( i < framesPerBuffer * channels) {
      for (size_t c = 0; c < channels; c++) {
        *out++ = buffer[i++]*volume_;
      }
    }

    if (effect_) {
      effect_->process(buffer, framesPerBuffer, file_->channels());
    }

    double pos = current_time_ + static_cast<double>(framesPerBuffer) / file_->samplerate();
    current_time_ = pos;

    if (playback_state_ == SHOULD_STOP) {
      if (ring_buffer_->available_read() == 0) {
        return paComplete;
      }
    } else {
      // Tell the other thread that it should start buffering.
      if (ring_buffer_->available_read() < 3) {
        playback_state_ = NEED_DATA;
      } else {
        playback_state_ = HAS_DATA;
      }
    }
  }
  return paContinue;
}


void AudioPlayer::set_volume(float vol){
  if (vol > 1 || vol < 0) {
    VAGG_LOG(VAGG_LOG_FATAL, "Volume out of range 0...1. Was %f", vol);
    vol = 0;
  }
  volume_ = vol;
}
