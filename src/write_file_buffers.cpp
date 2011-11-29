#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include <signal.h>
#include "../vagg/vagg_macros.h"
#include <portaudio.h>
#include "RingBuffer.hpp"
#include "AudioFile.hpp"
#include <atomic>

using namespace std;

const char* FILENAME = "recordings/out_buffers.wav";
const size_t CHUNK_SIZE = 4096;
const size_t CHANNELS = 1;
const unsigned SAMPLERATE = 44100;
const size_t FRAMES_PER_BUFFER = 4096;
const SamplesType SILENCE = 0;
const unsigned EVENT_LOOP_FREQUENCY = 50;

const int STOP_REQUESTED = 0;
const int SHOULD_STOP = 1;
const int RECORDING = 2;
const int STOPPED = 3;

std::atomic<int> recording_status;

void handle_exit(int signal_number, siginfo_t* infos, void* context)
{
  VAGG_LOG(VAGG_LOG_OK, "End of recording requested.");
  recording_status = SHOULD_STOP;
}

/**
 * @brief The main callback that will receive audio data and put them in a
 * queue.
 *
 * @param inputBuffer The buffer that gives us the data.
 * @param outputBuffer A possible input buffer (unused).
 * @param framesPerBuffer The number of frames per buffer.
 * @param timeInfo A time information for synchronization (unused).
 * @param statusFlags Some status flag (unused).
 * @param userData A user-defined pointer to carry data around.
 *
 * @return paContinue or paComplete, accordingly.
 */
static int callback(const void * inputBuffer,
                    void* VAGG_UNUSED(outputBuffer),
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* VAGG_UNUSED(timeInfo),
                    PaStreamCallbackFlags VAGG_UNUSED(statusFlags),
                    void *userData)
{
  RingBuffer* ring = (RingBuffer*)userData;
  SamplesType* in = (SamplesType*)inputBuffer;

  if (recording_status == SHOULD_STOP) {
    return paComplete;
  }

  recording_status = RECORDING;

  ring->write_raw(in, framesPerBuffer);

  return paContinue;
}

static void stream_finished( void* VAGG_UNUSED(user_data))
{
  recording_status = STOPPED;
  VAGG_LOG(VAGG_LOG_OK, "Finished.\n");
}

int main(void)
{
  bool record = true;
  AudioFile file(FILENAME, CHUNK_SIZE, SAMPLERATE, CHANNELS);
  file.open(AudioFile::Write);

  PaStreamParameters input_params;
  PaStream *stream;
  PaError err;

  RingBuffer buffer(4096, 2);

  struct sigaction action;
  struct sigaction save;

  // Prepare the SIGTINT signal handler. CTRL+C will gently close the program
  sigemptyset(&action.sa_mask);
  action.sa_sigaction = handle_exit;
  action.sa_flags = SA_SIGINFO;
  VAGG_SYSCALL(sigaction(SIGINT,&action,&save));

  err = Pa_Initialize();
  if(err != paNoError) {
    goto error;
  }

  input_params.device = Pa_GetDefaultInputDevice();
  if (input_params.device == paNoDevice) {
    VAGG_LOG(VAGG_LOG_FATAL,"Error: No default output device.\n");
    goto error;
  }
  input_params.channelCount = 1;
  input_params.sampleFormat = paFloat32;
  input_params.suggestedLatency = Pa_GetDeviceInfo( input_params.device )->defaultLowInputLatency;
  input_params.hostApiSpecificStreamInfo = NULL;


  err = Pa_OpenStream(
      &stream,
      &input_params,
      NULL,
      SAMPLERATE,
      FRAMES_PER_BUFFER,
      paClipOff,
      callback,
      &buffer);

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

  VAGG_LOG(VAGG_LOG_OK, "Recording. CTRL+C to stop.");

  while(record) {
    switch(recording_status) {
      case STOPPED:
        record = false;
        break;
      case STOP_REQUESTED:
        break;
      case SHOULD_STOP:
        break;
      case RECORDING:
        if (buffer.available_read() != 0) {
          AudioBuffer b(CHUNK_SIZE);
          buffer.read(&b);
          file.write_some(b);
        }
        break;
    }
    Pa_Sleep(EVENT_LOOP_FREQUENCY);
  }

	err = Pa_CloseStream( stream );
	if(err != paNoError) {
		goto error;
	}

  Pa_Terminate();

  return 0;

error:
  Pa_Terminate();
  VAGG_LOG(VAGG_LOG_OK, "An error occured while using the portaudio stream\n" );
  VAGG_LOG(VAGG_LOG_OK, "Error number: %d\n", err );
  VAGG_LOG(VAGG_LOG_OK, "Error message: %s\n", Pa_GetErrorText( err ) );
  return err;
}

