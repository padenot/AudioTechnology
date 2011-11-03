#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"

using namespace std;

const char* FILENAME = "amen.wav";
const size_t CHUNK_SIZE = 2*4096;
const size_t CHANNELS = 2;
const unsigned SAMPLERATE = 44100;

/**
 * @brief Read an entire WAV PCM 16 file in a vector
 *
 * @param filename the filename to open
 * @param samples the vector that will hold the samples
 */
void read_file(const char* filename, vector<short>& samples)
{
	SF_INFO infos_read;
	infos_read.samplerate = SAMPLERATE;
	infos_read.channels = CHANNELS;
	infos_read.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	VAGG_SYSCALL(sf_format_check(&infos_read));

	SNDFILE *file = sf_open(filename, SFM_READ, &infos_read);
	if (file == NULL) {
		VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file));
		abort();
	}

	size_t count = 0;
	do {
		short tmp[CHUNK_SIZE*CHANNELS];
		count = sf_read_short(file, tmp, CHUNK_SIZE*CHANNELS);
		samples.insert(samples.end(), tmp, tmp + count);
	} while (count == CHUNK_SIZE*CHANNELS);

	if (sf_close(file) != 0) {
		fprintf(stderr, "Error while closing the file.");
	}
}

int main(void)
{
	VAGG_LOG(VAGG_LOG_DEBUG, "Loading file %s", FILENAME);
	vector<short> samples;
	read_file(FILENAME, samples);

	VAGG_LOG(VAGG_LOG_DEBUG, "%zu samples loaded", samples.size());

	return 0;
}
