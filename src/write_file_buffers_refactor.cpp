#include "AudioPlayer.hpp"
#include "RMS.hpp"

const char* filename = "assets/amen.wav";

float rms2db(float value)
{
  return 10 * log10(value);
}

void rmscallback(float* values, size_t size)
{
  printf("[");
  for (size_t i = 0; i < size - 1; i++) {
    printf("%f ", rms2db(values[i]));
  }
  printf("%f]\n", rms2db(values[size - 1]));
}

int main()
{
  // 4096 : chunk size
  AudioPlayer p(4096);
  p.load(filename);

  // Create an RMS effect. It takes a callback which is called when results are
  // available.
  RMS rms(&rmscallback);
  // Insert the effect in the player a player can have only a single effect for
  // now.
  p.insert(&rms);

  // start the playback
  p.play();

  // Call the state machine while there is still things to play.
  while(p.state_machine()) {
    Pa_Sleep(50);
  }

  // Release the data (AudioPlayer is an RAII class, so this is not mandatory).
  p.unload();

  return 0;
}
