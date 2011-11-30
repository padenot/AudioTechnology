#include "AudioPlayer.hpp"

const char* filename = "assets/amen.wav";

int main()
{
  AudioPlayer p(4096, 50);
  p.load(filename);
  p.play();

  // Event loop
  while(p.state_machine()) {
    Pa_Sleep(50);
  }

  p.unload();

  return 0;
}
