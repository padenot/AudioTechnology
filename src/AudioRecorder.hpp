#ifndef AUDIORECORDER_HPP
#define AUDIORECORDER_HPP

#include "RingBuffer.hpp"
#include "AudioFile.hpp"
#include "Effect.hpp"
#include "types.hpp"
#include <atomic>

#define STOP_REQUESTED  0
#define SHOULD_STOP  1
#define RECORDING  2
#define STOPPED  3

class AudioRecorder
{
  public:
    AudioRecorder(const size_t chunk_size);
    ~AudioRecorder();
    int open(const char* file);
    int record();
    bool state_machine();
    int insert(Effect* effect);
    int stop();
    double current_time();
    long long unsigned free_disk_space();
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
    const size_t chunk_size_;
    RingBuffer<SamplesType,4>* ring_buffer_;
    std::atomic<int> recording_status_;
    double current_time_;

    PaStreamParameters input_params_;
    PaStream *stream_;

    Effect* effect_;
    size_t buffer_recorded_;
};

#endif
