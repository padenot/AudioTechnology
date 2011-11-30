#include "RingBuffer.hpp"

#define VAGG_TEST

#include "vagg/vagg.h"


int main()
{
  vagg_start(vagg_display_success);
  RingBuffer<AudioBuffer, 2> buffer;


  AudioBuffer a = {0.1, 0.2, 0.3, 0.4};
  AudioBuffer b = {0.2, 1.2, 1.3, 2.4};
  AudioBuffer c = {3.1, 8.2, 3.3, 3.4};
  AudioBuffer d;

  for(int i = 0; i < 10; i++) {
    vagg_ok(! buffer.full(), "Not full.");
    vagg_ok(buffer.empty(), "Empty.");
    float raw[4] = {0.1, 0.3, 1.8, 4.2};
    vagg_ok(buffer.push_raw(raw, 4), "Add a buffer on a not full RingBuffer.");
    vagg_ok(! buffer.empty(), "Not full.");
    vagg_ok(! buffer.full(), "Not empty.");
    vagg_ok(buffer.push(b), "Add a buffer on a not full RingBuffer.");
    vagg_ok(buffer.full(), "Should be full");
    vagg_ok(! buffer.empty(), "Should not be empty");
    vagg_ok(! buffer.push(c), "Add a buffer on a full RingBuffer.");
    vagg_ok(buffer.full(), "Should be full");
    vagg_ok(! buffer.empty(), "Should not be empty");

    vagg_ok(buffer.pop(d), "Get a buffer back.");
    vagg_bufeq((void*)&a.front(), a.size(),(void*)&d.front(), d.size(), "First buffer out should be equal to first buffer in.");
    vagg_ok(! buffer.empty(), "Should not be empty");
    vagg_ok(! buffer.full(), "Should not be full");
    vagg_ok(buffer.pop(d), "Get another buffer back.");
    vagg_ok(buffer.empty(), "Should be empty.");
    vagg_ok(! buffer.full(), "Should not be full.");
    vagg_bufeq((void*)&b.front(), b.size(),(void*)&d.front(), d.size(), "Second buffer out should be equal to second buffer in.");
    vagg_ok(! buffer.pop(d), "Get a buffer back on an empty RingBuffer should not be possible.");
    vagg_ok(buffer.empty(), "Should be empty.");
    vagg_ok(! buffer.full(), "Should not be full.");
  }

  vagg_end();
  return 0;
}
