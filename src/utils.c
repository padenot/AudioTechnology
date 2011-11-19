#include "macros.h"
#include "utils.h"

inline double s_to_ms(double s)
{
  ASSERT(POSITIVE(s), "Time must be positive.");
  return s/1000.;
}

inline double ms_to_s(double ms)
{
  ASSERT(POSITIVE(ms), "Time must be positive.");
  return ms*1000.;
}

inline double sample_to_ms(size_t samples)
{
  return samples*SAMPLE_DURATION_MS;
}

inline size_t ms_to_samples(double ms)
{
  ASSERT(POSITIVE(ms), "Time must be positive.");
  return ms * RATE / 1000.;
}

void dump_float_array(float* array, size_t len)
{
  unsigned i;
  for (i = 0; i < len - 1; i++) {
    printf("%f \n", array[i]);
  }
  printf("%f\n", array[len-1]);
}

