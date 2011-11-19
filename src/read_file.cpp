#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"
#include <portaudio.h>

using namespace std;

const char* FILENAME = "assets/amen.wav";
const size_t CHUNK_SIZE = 2*4096;
const size_t CHANNELS = 2;
const unsigned SAMPLERATE = 44100;
const size_t FRAMES_PER_BUFFER = 4096;

typedef float SamplesType ;
typedef vector<SamplesType> AudioBuffer;

struct PlaybackStatus {
  size_t index;
  AudioBuffer& samples;

  PlaybackStatus(AudioBuffer& samples)
  :index(0),samples(samples)
  { }
};

/**
 * @brief Read an entire WAV PCM 16 file in a vector
 *
 * @param filename the filename to open
 * @param samples the vector that will hold the samples
 */
void read_file(const char* filename, AudioBuffer& samples)
{
  SF_INFO infos_read;
  infos_read.samplerate = SAMPLERATE;
  infos_read.channels = CHANNELS;
  infos_read.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  VAGG_SYSCALL(sf_format_check(&infos_read));

  SNDFILE *file = sf_open(filename, SFM_READ, &infos_read);
  if (file == NULL) {
    VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file));
    abort();
  }

  size_t count = 0;
  do {
    SamplesType tmp[CHUNK_SIZE*CHANNELS];
    count = sf_read_float(file, tmp, CHUNK_SIZE*CHANNELS);
    samples.insert(samples.end(), tmp, tmp + count);
  } while (count == CHUNK_SIZE*CHANNELS);

  if (sf_close(file) != 0) {
    VAGG_LOG(VAGG_LOG_OK, "Error while closing the file.");
  }
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
static int callback(const void * VAGG_UNUSED(inputBuffer),
                    void *outputBuffer,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* VAGG_UNUSED(timeInfo),
                    PaStreamCallbackFlags VAGG_UNUSED(statusFlags),
                    void *userData)
{
  PlaybackStatus *status = (PlaybackStatus*)userData;
  float *out = (float*)outputBuffer;

  if (status->index >= status->samples.size()) {
    return paComplete;
  }

  for(size_t i=0; i < framesPerBuffer; i++)
  {
    // Left
    *out++ = (status->samples[status->index++]);
    // Right
    *out++ = (status->samples[status->index++]);
  }

  return paContinue;
}

static void stream_finished( void* VAGG_UNUSED(user_data))
{
  VAGG_LOG(VAGG_LOG_OK, "Finished.\n");
  exit(0);
}

int main(void)
{
  VAGG_LOG(VAGG_LOG_DEBUG, "Loading file %s", FILENAME);
  AudioBuffer samples;
  PlaybackStatus status(samples);

  read_file(FILENAME, samples);

  VAGG_LOG(VAGG_LOG_DEBUG, "%zu samples loaded", samples.size());

  PaStreamParameters output_params;
  PaStream *stream;
  PaError err;

  VAGG_LOG(VAGG_LOG_DEBUG, "Trying to play the file.");

  err = Pa_Initialize();
  if(err != paNoError) {
    goto error;
  }

  output_params.device = Pa_GetDefaultOutputDevice(); /* default output device */
  if (output_params.device == paNoDevice) {
    VAGG_LOG(VAGG_LOG_FATAL,"Error: No default output device.\n");
    goto error;
  }
  output_params.channelCount = 2;       /* stereo output */
  output_params.sampleFormat = paFloat32;
  output_params.suggestedLatency = Pa_GetDeviceInfo( output_params.device )->defaultLowOutputLatency;
  output_params.hostApiSpecificStreamInfo = NULL;


  err = Pa_OpenStream(
      &stream,
      NULL,
      &output_params,
      SAMPLERATE,
      FRAMES_PER_BUFFER,
      paClipOff,
      callback,
      &status);

  if(err != paNoError) {
    goto error;
  }

  err = Pa_SetStreamFinishedCallback( stream, &stream_finished);
  if(err != paNoError) {
    goto error;
  }

  err = Pa_StartStream( stream );
  if(err != paNoError) {
    goto error;
  }

  VAGG_LOG(VAGG_LOG_OK, "Press enter key to stop.");
  getchar();

  err = Pa_StopStream( stream );
  if(err != paNoError) {
    goto error;
  }

  err = Pa_CloseStream( stream );
  if(err != paNoError) {
    goto error;
  }

  Pa_Terminate();

  return 0;

error:
  Pa_Terminate();
  VAGG_LOG(VAGG_LOG_CRITICAL, "An error occured while using the portaudio stream\n" );
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error number: %d\n", err );
  VAGG_LOG(VAGG_LOG_CRITICAL, "Error message: %s\n", Pa_GetErrorText( err ) );
  return err;
}

