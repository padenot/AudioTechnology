#include "AudioRecorder.hpp"

#ifdef __linux__
  #include <sys/statvfs.h>
#endif

#include <signal.h>

#define HANDLE_PA_ERROR(err)                                              \
  Pa_Terminate();                                                         \
  VAGG_LOG(VAGG_LOG_CRITICAL, "An error occured while using the "         \
                              "portaudio stream, line %d", __LINE__ );    \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error number: %d", err );                  \
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error message: %s", Pa_GetErrorText(err)); \
  return err;

static long long unsigned get_free_disk_space(const char* path)
{
#ifdef __linux__
  struct statvfs b;
  VAGG_SYSCALL(statvfs(path, &b));
  long long unsigned size = (long long unsigned)b.f_bavail * b.f_bsize;
  return size;
#else
  #warning Free disk space is not supported on that system.
  return -1;
#endif
}

AudioRecorder::AudioRecorder(const size_t chunk_size)
:file_(0)
,chunk_size_(chunk_size)
,ring_buffer_(0)
,recording_status_(STOPPED)
,current_time_(0)
,stream_(0)
,effect_(0)
,buffer_recorded_(0)
{ }

AudioRecorder::~AudioRecorder()
{
  delete ring_buffer_;
  delete file_;
}

int AudioRecorder::open(const char* file)
{
  PaError err;
  file_ = new AudioFile(file);

  if ((err = file_->open(AudioFile::Write))) {
    HANDLE_PA_ERROR(err);
    return err;
  }

  ring_buffer_ = new RingBuffer<SamplesType, 4>(chunk_size_);

  err = Pa_Initialize();
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  input_params_.device = Pa_GetDefaultInputDevice();
  if (input_params_.device == paNoDevice) {
    HANDLE_PA_ERROR(err);
  }

  input_params_.channelCount = 1;
  input_params_.sampleFormat = paFloat32;
  input_params_.suggestedLatency = Pa_GetDeviceInfo( input_params_.device )->defaultLowInputLatency;
  input_params_.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
      &stream_,
      &input_params_,
      NULL,
      file_->samplerate(),
      chunk_size_,
      paClipOff,
      audio_callback,
      this);

  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }

  err = Pa_SetStreamFinishedCallback(stream_, &finished_callback);
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }
  return 0;
}

int AudioRecorder::record()
{
  PaError err;
  recording_status_ = RECORDING;
  err = Pa_StartStream(stream_);
  if(err != paNoError) {
    HANDLE_PA_ERROR(err);
  }
  return 0;
}

bool AudioRecorder::state_machine()
{
  switch(recording_status_) {
    case STOPPED:
      VAGG_LOG(VAGG_LOG_WARNING, "Stopped");
      return false;
      break;
    case STOP_REQUESTED:
      VAGG_LOG(VAGG_LOG_WARNING, "Stop requested");
      break;
    case SHOULD_STOP:
      VAGG_LOG(VAGG_LOG_WARNING, "Should stop");
      break;
    case RECORDING:
      VAGG_LOG(VAGG_LOG_WARNING, "Recording");
      if (ring_buffer_->available_read() != 0) {
        SamplesType b[chunk_size_];
        ring_buffer_->pop(b, chunk_size_);
        file_->write_some(b, chunk_size_);
      }
      // In gigaoctet
      VAGG_LOG(VAGG_LOG_OK, "Disk space availale : %lfGo",
          get_free_disk_space(file_->path())/1024./1024./1024.);
      break;
  }
  return true;
}

int AudioRecorder::insert(Effect* effect)
{
  effect_ = effect;
  return 0;
}

int AudioRecorder::stop()
{
  PaError err;

  if (recording_status_ != STOPPED && stream_) {
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

double AudioRecorder::current_time()
{
  return static_cast<double>(buffer_recorded_) * chunk_size_ / file_->samplerate();
}

int AudioRecorder::audio_callback(const void * inputBuffer,
                                  void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void *user_data)
{
  AudioRecorder* a = static_cast<AudioRecorder*>(user_data);
  return a->audio_callback_m(inputBuffer, outputBuffer, framesPerBuffer,
                                       timeInfo, statusFlags, user_data);
}

int AudioRecorder::audio_callback_m(const void * inputBuffer,
                                    void *VAGG_UNUSED(outputBuffer),
                                    unsigned long framesPerBuffer,
                                    const PaStreamCallbackTimeInfo* VAGG_UNUSED(timeInfo),
                                    PaStreamCallbackFlags VAGG_UNUSED(statusFlags),
                                    void *user_data)
{
  AudioRecorder* a = static_cast<AudioRecorder*>(user_data);
  RingBuffer<SamplesType, 4>* ring = a->ring_buffer_;
  SamplesType* in = (SamplesType*)inputBuffer;

  if (a->recording_status_ == SHOULD_STOP) {
    return paComplete;
  }

  a->recording_status_ = RECORDING;

  if (effect_) {
    effect_->process(in, framesPerBuffer, file_->channels());
  }

  buffer_recorded_++;
  ring->push(in, framesPerBuffer);

  return paContinue;
}

void AudioRecorder::finished_callback(void* user_data)
{
  AudioRecorder* a = static_cast<AudioRecorder*>(user_data);
  a->finished_callback_m(user_data);
}

void AudioRecorder::finished_callback_m(void* user_data)
{
  AudioRecorder* a = static_cast<AudioRecorder*>(user_data);
  VAGG_LOG(VAGG_LOG_OK, "Finished.");
  a->recording_status_ = STOPPED;
}

