#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include <list>
#include "vagg/vagg_macros.h"
#include <portaudio.h>
#include <atomic>
#include "AudioBuffersQueue.hpp"
#include "AudioFile.hpp"
#include "types.hpp"

using namespace std;

const char* FILENAME = "assets/pod159.wav";
const size_t CHUNK_SIZE = 2*4096;
const size_t CHANNELS = 2;
const unsigned SAMPLERATE = 44100;
const size_t FRAMES_PER_BUFFER = 4096;
const unsigned EVENT_LOOP_FREQUENCY = 50;

#define HAS_DATA 0
#define NEED_DATA 1
#define SHOULD_STOP 2
#define STOPPED 3

atomic<int> playback_state;

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
  AudioBuffersQueue* queue = (AudioBuffersQueue*)userData;
  float* out = (float*)outputBuffer;

  size_t available = queue->available();
  VAGG_LOG(VAGG_LOG_WARNING, "Available : %zu", available);

  // We have no data ! Output silence.
  if (available == 0) {
    if ( playback_state == SHOULD_STOP) {
      return paComplete;
    } else {
    VAGG_LOG(VAGG_LOG_WARNING, "UNDERRUN");
    playback_state = NEED_DATA;
    for(size_t i = 0; i < framesPerBuffer; i++) {
      // Left
      *out++ = 0.0;
      // Right
      *out++ = 0.0;
    }
    return paContinue;
    }
  } else {
    AudioBuffer* buffer = queue->pop();
    VAGG_ASSERT(buffer->size() == framesPerBuffer, "Bad buffer size.");

    size_t i = 0;
    while( i < framesPerBuffer * 2 ) {
      *out++ = (*buffer)[i++];
      *out++ = (*buffer)[i++];
    }
    queue->dispose(buffer);

    // Tell the other thread that it should start buffering.
    if (playback_state == SHOULD_STOP) {
      return paContinue;
    } else if (available < framesPerBuffer * 4) {
      playback_state = NEED_DATA;
    } else {
      playback_state = HAS_DATA;
    }
  }
  return paContinue;
}

static void stream_finished( void* VAGG_UNUSED(user_data))
{
  VAGG_LOG(VAGG_LOG_OK, "Finished.\n");
  playback_state = STOPPED;
}

int main(void)
{
  AudioBuffer samples;
  BufferList list;

  PaStreamParameters output_params;
  PaStream *stream;
  PaError err;
  bool audio_playing = true;

  AudioBuffersQueue queue(FRAMES_PER_BUFFER * 2);
  AudioFile a(FILENAME);
  a.open();

  err = Pa_Initialize();
  if(err != paNoError) {
    goto error;
  }

  output_params.device = Pa_GetDefaultOutputDevice();
  if (output_params.device == paNoDevice) {
    VAGG_LOG(VAGG_LOG_FATAL,"Error: No default output device.\n");
    goto error;
  }
  output_params.channelCount = 2;
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
    &queue);

  if(err != paNoError) {
    goto error;
  }

  err = Pa_SetStreamFinishedCallback(stream, &stream_finished);
  if(err != paNoError) {
    goto error;
  }

  // Prebuffering
  {
    size_t c = 4;
    while(c--) {
      VAGG_LOG(VAGG_LOG_OK, "prebuffer %d", c);
      AudioBuffer* prebuffer = new AudioBuffer();
      prebuffer->reserve(CHUNK_SIZE);
      size_t count = a.read_some(*prebuffer);
      if (count != CHUNK_SIZE) {
        prebuffer->resize(count);
      }
      queue.push(prebuffer);
    }
  }

  err = Pa_StartStream(stream);
  if(err != paNoError) {
    goto error;
  }

  // Event loop
  while(audio_playing) {
    switch(playback_state) {
      case HAS_DATA:
        break;
      case NEED_DATA:
        VAGG_LOG(VAGG_LOG_OK, "Buffering");
        for(int i = 0; i < 2; i++) {
          AudioBuffer* buffer = new AudioBuffer();
          buffer->reserve(CHUNK_SIZE);
          if (a.read_some(*buffer) != CHUNK_SIZE) {
            playback_state = SHOULD_STOP;
            break;
          }
          queue.push(buffer);
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
    Pa_Sleep(EVENT_LOOP_FREQUENCY);
  }

  queue.close();
  VAGG_ASSERT(queue.available() == 0, "There should be nothing in the queue at this point.");

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

