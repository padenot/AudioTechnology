#ifndef AUDIOPLAYER_HPP
#define AUDIOPLAYER_HPP

#include "AudioFile.hpp"
#include "RingBuffer.hpp"

#include <atomic>
#include <portaudio.h>

#define HAS_DATA 0
#define NEED_DATA 1
#define SHOULD_STOP 2
#define STOPPED 3

class AudioPlayer
{
  public:
    AudioPlayer(const size_t chunk_size, const unsigned event_loop_frequency);
    ~AudioPlayer();
    int play();
    int pause();
    int load(const char* file);
    int unload();
    int seek(const double ms);
    bool state_machine();
  protected:
    /** Callbacks **/
    static int audio_callback(const void * inputBuffer,
                        void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData);

    int audio_callback_m(const void * inputBuffer,
                        void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData);

    static void finished_callback( void* user_data);
    void finished_callback_m( void* user_data);

    /** Members **/
    AudioFile* file_;
    const unsigned event_loop_frequency_;
    const size_t chunk_size_;
    RingBuffer<SamplesType,4> ring_buffer_;
    std::atomic<int> playback_state_;

    PaStreamParameters output_params_;
    PaStream *stream_;
};

#endif
