#ifndef FUNCTORS_H
#define FUNCTORS_H

#include <fstream>
#include <sndfile.h>
#include "macros.h"

template <typename T>
class Functor
{
	public:
	virtual void Apply(T& fb) = 0;
};

template <typename T>
class Printer : public Functor<T>
{
	public:
		void Apply(T& fb)
		{
			for(size_t i = 0; i < fb.size(); i++) {
				std::cout << fb[i] << ", ";
			}
			std::cout << std::endl;
		}
};

// TODO center frames vertically
template <typename T>
class WebPrinter : public Functor<T>
{
	public:
		void Apply(T& fb)
		{
			std::ofstream file("dump.html");
			file << "<html><style>div {width:1px; position:absolute;bottom:0px; background-color:black;}</style><body>";
			for(size_t i = 0; i < fb.size(); i++) {
				file << "<div style='left:" << i << "px;height:" << (float)((float)(random())/RAND_MAX * 2)*300 << "px;'></div>";
			}
			file << "</body></html>";
			system("firefox dump.html");
		}
};

template <typename T>
class Gain : public Functor<T>
{
	public:
		Gain(T value)
			:gain_(value)
		{ }
		void Apply(T& fb)
		{
			for(size_t i = 0; i < fb.size(); i++) {
				fb[i] *= gain_;
			}
		}
	protected:
		T gain_;
};

template <typename T>
class WaveWriter : public Functor<T>
{
	public:
		WaveWriter(std::string name)
			:filename(name)
		{
			SF_INFO infos_write;
			infos_write.samplerate = 44100;
			infos_write.channels = 1;
			infos_write.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

			if (sf_format_check(&infos_write) == 0) {
				LOG(LOG_CRITICAL, "Error while checking output file format.");
				abort();
			}

			file_= sf_open(name.c_str(), SFM_WRITE, &infos_write);

			if (file_ == NULL) {
				LOG(LOG_CRITICAL, "%s\n", sf_strerror(file_));
				abort();
			}
		}

		~WaveWriter()
		{
			if (sf_close(file_) != 0) {
				LOG(LOG_CRITICAL, "Error while closing the file.");
			}
		}

		void Apply(T& fb)
		{
			sf_count_t countWrite = sf_writef_float(file_, fb.get(), fb.size());
			if (countWrite != fb.size()) {
				LOG(LOG_DEBUG, "Error while writing samples: %llu written instead of %d.", countWrite, fb.size());
				abort();
			} else {
				LOG(LOG_DEBUG, "Wrote %llu samples to %s file.", countWrite, filename.c_str());
			}
		}
	protected:
		SNDFILE *file_;
		std::string filename;
};

#endif
