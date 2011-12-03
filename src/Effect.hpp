#ifndef EFFECT_HPP
#define EFFECT_HPP

#include "types.hpp"

class Effect
{
  public:
    // |length * channels| is the size of |samples|.
    virtual void process(SamplesType* samples, size_t length, size_t channels) = 0;
};

#endif
