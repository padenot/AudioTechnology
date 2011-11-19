#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "../vagg/vagg_macros.h"
#include <portaudio.h>

using namespace std;

typedef float SamplesType ;
typedef vector<SamplesType> AudioBuffer;

const char* FILENAME = "recordings/out.wav";
const size_t CHUNK_SIZE = 2*4096;
const size_t CHANNELS = 2;
const unsigned SAMPLERATE = 44100;
const size_t FRAMES_PER_BUFFER = 4096;
const SamplesType SILENCE = 0;

bool stop_requested = false;

struct RecordingStatus {
  AudioBuffer& samples;

  RecordingStatus(AudioBuffer& samples)
  :samples(samples)
  { }
};

void write_to_file(const char* filename, AudioBuffer& samples)
{
  VAGG_LOG(VAGG_LOG_OK, "Writing : %zu", samples.size());

	int rv = 0;
	if ((rv = remove(filename))) {
		if (errno != ENOENT) {
			perror("could not remove the file");
			abort();
		}
	}

  SF_INFO infos_write;
  infos_write.samplerate = 44100;
  infos_write.channels = 1;
  infos_write.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  if (sf_format_check(&infos_write) == 0) {
    VAGG_LOG(VAGG_LOG_FATAL, "Error while checking output file format.");
    abort();
  }

  SNDFILE *file_to_write = sf_open(filename, SFM_WRITE, &infos_write);
  if (file_to_write == NULL) {
    VAGG_LOG(VAGG_LOG_FATAL, "%s\n", sf_strerror(file_to_write));
    abort();
  }

  sf_count_t countWrite = sf_writef_float(file_to_write, &samples.front(), samples.size());
  if (countWrite != samples.size()) {
    VAGG_LOG(VAGG_LOG_FATAL, "Error while writing samples: %llu written instead of %d.\n", countWrite, samples.size());
    abort();
  } else {
    VAGG_LOG(VAGG_LOG_OK, "Wrote %llu samples to %s file.", countWrite, filename);
  }

  if (sf_close(file_to_write) != 0) {
    VAGG_LOG(VAGG_LOG_WARNING, "Error while closing the file.");
  }
}

/**
 * @brief The main callback that will push audio samples in input to the buffer.
 *
 * @param inputBuffer The input buffer.
 * @param outputBuffer (output buffer, unused)
 * @param framesPerBuffer The number of frames per buffer.
 * @param timeInfo A time information for synchronization (unused).
 * @param statusFlags Some status flag (unused).
 * @param userData A user-defined pointer to carry data around.
 *
 * @return paContinue or paComplete, accordingly.
 */
static int callback(const void * inputBuffer,
                    void * VAGG_UNUSED(outputBuffer),
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* VAGG_UNUSED(timeInfo),
                    PaStreamCallbackFlags VAGG_UNUSED(statusFlags),
                    void *userData)
{
  RecordingStatus *status = (RecordingStatus*)userData;
  float *in = (float*)inputBuffer;

  if (inputBuffer == NULL) {
    for(size_t i=0; i < framesPerBuffer; i++)
    {
      status->samples.push_back(SILENCE);
    }
  } else {
    for(size_t i=0; i < framesPerBuffer; i++)
    {
      status->samples.push_back(*in++);
    }
  }

  if (stop_requested == true) {
    return paComplete;
  }

  return paContinue;
}

static void stream_finished( void* VAGG_UNUSED(user_data))
{
  VAGG_LOG(VAGG_LOG_OK, "Finished.\n");
}

int main(void)
{
  AudioBuffer samples;
  RecordingStatus status(samples);

  PaStreamParameters input_params;
  PaStream *stream;
  PaError err;

  err = Pa_Initialize();
  if(err != paNoError) {
    goto error;
  }

  input_params.device = Pa_GetDefaultInputDevice();
  if (input_params.device == paNoDevice) {
    VAGG_LOG(VAGG_LOG_FATAL,"Error: No default output device.\n");
    goto error;
  }
  input_params.channelCount = 1;       /* mono input */
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
      &status);

  if(err != paNoError) {
    VAGG_LOG(VAGG_LOG_OK, "GOTO ERROR");
    goto error;
  }

  err = Pa_SetStreamFinishedCallback( stream, &stream_finished);
  if(err != paNoError) {
    goto error;
  }

  err = Pa_StartStream( stream );
  if(err != paNoError) {
    VAGG_LOG(VAGG_LOG_OK, "GOTO ERROR");
    goto error;
  }

  VAGG_LOG(VAGG_LOG_OK, "Press enter key to stop.");
  getchar();

  stop_requested = true;

	err = Pa_CloseStream( stream );
	if(err != paNoError) {
		goto error;
	}

	write_to_file(FILENAME, samples);
	VAGG_LOG(VAGG_LOG_OK, "Wrote a %f seconds file.", (float)samples.size() / SAMPLERATE);

  Pa_Terminate();

  return 0;

error:
  Pa_Terminate();
  VAGG_LOG(VAGG_LOG_OK, "An error occured while using the portaudio stream\n" );
  VAGG_LOG(VAGG_LOG_OK, "Error number: %d\n", err );
  VAGG_LOG(VAGG_LOG_OK, "Error message: %s\n", Pa_GetErrorText( err ) );
  return err;
}

