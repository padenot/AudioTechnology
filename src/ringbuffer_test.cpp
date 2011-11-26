#include "RingBuffer.hpp"
#define VAGG_TEST
#include "vagg/vagg.h"


int main()
{
	vagg_start(vagg_display_success);
  RingBuffer buffer(4,2);


	AudioBuffer a = {0.1, 0.2, 0.3, 0.4};
	AudioBuffer b = {0.2, 1.2, 1.3, 2.4};
	AudioBuffer c = {3.1, 8.2, 3.3, 3.4};
	AudioBuffer d;
	for(int i = 0; i < 10; i++) {
		vagg_ok(buffer.available_read() == 0, "Zero buffer should be available to read.");
		vagg_ok(buffer.available_write() == 2, "Two buffers should be available to write.");
		vagg_ok(buffer.write(&a) == false, "Add a buffer on a not full RingBuffer.");
		vagg_ok(buffer.available_read() == 1, "One buffer should be available to read.");
		vagg_ok(buffer.available_write() == 1, "One buffer should be available to write.");
		vagg_ok(buffer.write(&b) == false, "Add a buffer on a not full RingBuffer.");
		vagg_ok(buffer.available_read() == 2, "Two buffers should be available to read.");
		vagg_ok(buffer.available_write() == 0, "Zero buffer should be available to write.");
		vagg_ok(buffer.write(&c) == true, "Add a buffer on a full RingBuffer.");
		vagg_ok(buffer.available_read() == 2, "Two buffers should be available to read.");
		vagg_ok(buffer.available_write() == 0, "Zero buffer should be available to write.");

		vagg_ok(buffer.available_read() == 2, "Two buffer should be available to read.");
		vagg_ok(buffer.available_write() == 0, "Zero buffer should be available to write.");
		vagg_ok(buffer.read(&d) == false, "Get a buffer back.");
		vagg_bufeq((void*)&a.front(), a.size(),(void*)&d.front(), d.size(), "First buffer out should be equal to first buffer in.");
		vagg_ok(buffer.available_read() == 1, "One buffer should be available to read.");
		vagg_ok(buffer.available_write() == 1, "One buffer should be available to write.");
		vagg_ok(buffer.read(&d) == false, "Get another buffer back.");
		vagg_ok(buffer.available_read() == 0, "Zero buffer should be available to read.");
		vagg_ok(buffer.available_write() == 2, "Two buffers should be available to write.");
		vagg_bufeq((void*)&b.front(), b.size(),(void*)&d.front(), d.size(), "Second buffer out should be equal to second buffer in.");
		vagg_ok(buffer.read(&d) == true, "Get a buffer back on an empty RingBuffer.");
		vagg_ok(buffer.available_read() == 0, "Zero buffer should be available to read.");
		vagg_ok(buffer.available_write() == 2, "Two buffers should be available to write.");
	}
	
	vagg_end();
  return 0;
}
