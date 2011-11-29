#include "AudioPlayer.hpp"

int main()
{
  const char* filename = "assets/amen.wav";

  AudioPlayer p(4096, 10);
  p.load(filename);
  p.play();
  p.unload();

  return 0;
}
