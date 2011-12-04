#include "AudioRecorder.hpp"

int main()
{
  AudioRecorder r(4096);

  r.open("recordings/out.wav");

  r.record();

  while(r.state_machine()) {
    Pa_Sleep(50);
    VAGG_LOG(VAGG_LOG_OK, "Duration : %lf", r.current_time());
  }

  return 0;
}
