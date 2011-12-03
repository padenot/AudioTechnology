#ifndef RMS_HPP
#define RMS_HPP

#include "types.hpp"
#include <math.h>

class RMS : public Effect
{
  public:
    RMS(void (*callback)(float*, size_t))
      :callback_(callback)
    { }

    // |length * channels| is the size of |samples|.
    virtual void process(SamplesType* samples, size_t length, size_t channels)
    {
      float acc[channels];
      for(size_t c = 0; c < channels; c++) {
        for (size_t i = c; i < length * channels; i+=channels) {
          acc[c] += samples[i] * samples[i];
        }
        acc[c] = sqrt(acc[c] / length);
      }
      callback_(acc, channels);
    }

  protected:
   void (*callback_)(float*, size_t);
};

#endif
