typedef struct {
  // in ms
  double attack_time;
  double attack_gain;
  double decay_time;
  double decay_gain;
  double sustain_time;
  double release_time;
} adsr_param;

long gain(float* in, size_t buflen, size_t offset, size_t len, double gain)
{
  ASSERT(buflen >= offset + len, "offset + len greater that buffer length");
  ASSERT(POSITIVE(gain), "Gain shall be positive 0.0 and 1.0");

  LOG(LOG_DEBUG, "Gain : [%u,%u], gain : %lf", offset, offset+len, gain);

  size_t i = offset;
  for(; i < offset + len; i++) {
    in[i]*=gain;
  }
  return len;
}

float max(float* in, size_t buflen)
{
  float max = in[0];
  size_t i = 0;
  for(;i < buflen; i++) {
    if(max < in[i])
      max = in[i];
  }
  return max;
}

float min(float* in, size_t buflen)
{
  float min = in[0];
  size_t i = 0;
  for(;i < buflen; i++) {
    if(min > in[i])
      min = in[i];
  }
  return min;
}

long ramp(float* in, size_t buflen, size_t offset, size_t len, double start_gain, double end_gain)
{
  ASSERT(buflen >= offset + len, "offset + len greater that buffer length");
  ASSERT(POSITIVE(start_gain) && POSITIVE(end_gain), "Gain shall be positive");

  size_t i = offset;
  double increment = (end_gain - start_gain) / len;
  double gain_acc = start_gain;
  LOG(LOG_DEBUG, "Ramp : [%u, %u], start_gain : %lf, end_gain : %lf, increment : %lf", offset, offset+len, start_gain, end_gain, increment);

  for(; i < offset+len; i++) {
    in[i]*=gain_acc;
    gain_acc+=increment;
  }
  return len;
}

long adsr(float* in, size_t len, adsr_param* param)
{
  ASSERT(ms_to_samples(param->decay_time + param->attack_time + param->sustain_time + param->release_time) > len, "adsr filter is to long.");
  // attack
  if (param->attack_time != 0) {
    ramp(in, len, 0, ms_to_samples(param->attack_time), 0.0, param->attack_gain);
  }
  // decay
  ramp(in, len, ms_to_samples(param->attack_time), ms_to_samples(param->decay_time), param->attack_gain, param->decay_gain);
  // sustain
  gain(in, len, ms_to_samples(param->decay_time + param->attack_time), ms_to_samples(param->sustain_time), param->decay_gain);
  // release
  ramp(in, len, ms_to_samples(param->decay_time + param->attack_time + param->sustain_time), ms_to_samples(param->release_time), param->decay_gain, 0);
  // silence
  gain(in, len, ms_to_samples(param->decay_time + param->attack_time + param->sustain_time + param->release_time), len - ms_to_samples(param->decay_time + param->attack_time + param->sustain_time + param->release_time), 0.0);
  return len;
}

long square(float* out, size_t buflen, size_t offset, size_t len, double frequency)
{
  double change = RATE / frequency;
  size_t i = offset;
  int index = offset;
  for(;index < offset + len; i++, index++) {
    if(fmod(i,change) > change/2)
      out[index] = 1;
    else
      out[index] = -1;
  }
}

long triangle(float* out, size_t buflen, size_t offset, size_t len, double frequency)
{
  double change = RATE / frequency;
  size_t i = offset;
  for(; i < offset + len; i++) {
    if(fmod(i,change) < change / 2)
      out[i] = fmod(i,change)/(change/2) * 2 - 1;
    else
      out[i] = out[i - (int)change/2];
  }
}

long sawtooth(float* out, size_t buflen, size_t offset, size_t len, double frequency)
{
  double change = RATE/frequency;
  size_t i = offset;
  for(; i < offset + len; i++) {
    out[i] = fmod(i,change)/change * 2 - 1;
  }
}

long noise(float* out, size_t buflen, size_t offset, size_t len)
{
  size_t i = offset;
  for(; i < offset + len; i++) {
    out[i] = (float)(random())/RAND_MAX * 2 - 1;
  }
}

void delay(float* in, size_t len, double delay, double feedback)
{
  size_t DELAY = ms_to_samples(delay);
  int cursor = 0;
  double buffer[(size_t)(2*RATE)] = { 0.0 };
  while(--len > 0)
  {
    double x = *in;
    double y = buffer[cursor];

    buffer[cursor++] = x + y * feedback;
    *(in++) = buffer[cursor-1];

    cursor = cursor%DELAY;
  }
}

void reverb(float* in, size_t len)
{
  size_t i = 0;
  float t1 = 200.0;
  float g1 = 0.2;
  float rev = -3 * t1 / log10(g1);
  for(;i < 4; i++) {
    float dt = t1 / pow(2, ((float)i / 4));
    float g = pow(10, -((3*dt) / rev));
    delay(in, len, dt, g);
    printf("d%d t=%.3f g=%.3f\n", i, dt, g);
  }
}

void bitcrush(float* in, size_t len, size_t bits)
{
  // number of value possible for a nbBits integer
  int coeff = (unsigned)pow(2, bits);
  int tmp = 0;

  while(--len != 0)
  {
    tmp = (int)(*in * coeff);
    *in++ = (float)tmp/coeff;
  }
}

long sinus(float* out, size_t buflen, size_t offset, size_t len, double frequency, unsigned long* angle)
{
  ASSERT(buflen >= (offset + len), "offset + len greater that buffer length");

  size_t i = offset;
  for(; i < offset+len; i++) {
    out[i] = sin(*(angle)*W*frequency);
    *angle = *(angle) + 1;
  }
  return len;
}

long sine_swipe(float* out, size_t buflen, size_t offset, size_t len, double frequency_start, double frequency_end, unsigned long* angle) {
  ASSERT(buflen >= (offset + len), "offset + len greater that buffer length");

  double increment = (frequency_end - frequency_start) / len;
  double frequency = frequency_start;
  size_t i = offset;
  for(; i < offset + len; i++) {
    out[i] = sin((*angle)*W*frequency);
    frequency+=increment;
    if (out[i-1] < 0 && out[i] > 0)
      *angle = 0;
    (*angle)++;
  }
  return len;
}

void write_to_file(float* samples, size_t len, char* filename)
{
  SF_INFO infos_write;
  infos_write.samplerate = 44100;
  infos_write.channels = 1;
  infos_write.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  if (sf_format_check(&infos_write) == 0) {
    fprintf(stderr, "Error while checking output file format.");
    abort();
  }

  SNDFILE *file_to_write = sf_open(filename, SFM_WRITE, &infos_write);
  if (file_to_write == NULL) {
    fprintf(stderr, "%s\n", sf_strerror(file_to_write));
    abort();
  }

  sf_count_t countWrite = sf_writef_float(file_to_write, samples, len);
  if (countWrite != len) {
    fprintf(stderr, "Error while writing samples: %llu written instead of %d.\n", countWrite, len);
    abort();
  } else {
    LOG(LOG_DEBUG, "Wrote %llu samples to %s file.", countWrite, filename);
  }

  if (sf_close(file_to_write) != 0) {
    fprintf(stderr, "Error while closing the file.");
  }
}

void hardclip(float* buffer, size_t len, double amount)
{
  ASSERT(TEST_BOUND(amount), "Amount must be between 0.0 and 1.0.");
  size_t i = 0;
  float maxi = max(buffer, len) * amount;
  float mini = min(buffer, len) * amount;
  for (; i < len; i++) {
    if (buffer[i] > maxi) {
      buffer[i] = maxi;
    }
    if (buffer[i] < mini) {
      buffer[i] = mini;
    }
  }
}

void softclip(float* buffer, size_t len, double amount)
{
  ASSERT(TEST_BOUND(amount), "Amount must be between 0.0 and 1.0.");
  size_t i = 0;
  for (; i < len; i++) {
    if (buffer[i] > amount) {
      buffer[i] = amount + (1.0 - amount) * tanh((buffer[i]-amount)/(1-amount));
    }
    if (buffer[i] < -amount) {
      buffer[i] = -(amount + (1.0 - amount) * tanh((buffer[i]-amount)/(1-amount)));
    }
  }
}

void foldback_dist(float* buffer, size_t len, double threshold)
{
  size_t i = 0;
  for(; i < len; i++) {
    if (buffer[i] > threshold || buffer[i] < -threshold) {
      buffer[i] = fabs(fabs(fmod(buffer[i] - threshold, threshold*4)) - threshold*2) - threshold;
    }
  }
}

void waveshape(float* buffer, size_t len, double threshold)
{
  size_t i = 0;
  float maxi = max(buffer, len);
  for(; i < len; i++) {
    buffer[i] = (maxi / 2.0) * atan(16 * buffer[i] / maxi);
  }
}

void waveshape2(float* buffer, size_t len, double threshold)
{
  size_t i = 0;
  float maxi = max(buffer, len);
  for(; i < len; i++) {
    buffer[i] = maxi * tanh(buffer[i]/maxi);
  }
}

void kick(framebuffer* buffer, size_t offset)
{
  int angle = 0;
  int i=0;
  framebuffer* swipe = fb_new(ms_to_samples(4000));
  framebuffer* thump = fb_new(ms_to_samples(4000));
  framebuffer* sine2 = fb_new(ms_to_samples(4000));
  framebuffer* sine3 = fb_new(ms_to_samples(4000));
  framebuffer* channels[4] = {swipe, thump, sine2, sine3};

  sinus(swipe->buffer, swipe->len, offset, ms_to_samples(150), 75, &angle);
  angle = 0;
  triangle(thump->buffer, thump->len, offset, ms_to_samples(100), 37.5);
  angle = 0;
  sinus(sine2->buffer, sine2->len, offset, ms_to_samples(100), 150, &angle);
  angle = 0;
  sinus(sine3->buffer, sine3->len, offset, ms_to_samples(100), 300, &angle);

  LOG(LOG_DEBUG, ("Kick 2"));

  float gains[4] = {0.8, 0.8, 0., 0.};
  mix(channels, gains, 2, buffer);
  for(i=offset; i < offset + ms_to_samples(150); i++) {
    buffer->buffer[i] = swipe->buffer[i] * thump->buffer[i] * 2;
  }
  adsr_param param_thump = {
    0, 1, 140, 0.0, 10, 10
  };
  adsr(buffer->buffer+offset, ms_to_samples(100), &param_thump);
  gain(buffer->buffer, buffer->len, 0, buffer->len, 0.9);
}

void hh(framebuffer* buffer, size_t offset)
{
  noise(buffer->buffer, buffer->len, offset, ms_to_samples(75));
  adsr_param param = {
    0, 0.5, 75, 0, 0, 0
  };
  adsr(buffer->buffer+offset, ms_to_samples(100), &param);
}

void snare(framebuffer* buffer, size_t offset)
{
  static unsigned long angle = 0;
  int i = 0;
  framebuffer* channels[5];
  for(; i < 5; i++) {
    channels[i] = fb_new(ms_to_samples(4000));
  }
  sine_swipe(channels[0]->buffer, channels[0]->len, offset, ms_to_samples(200), 240, 180, &angle);
  sine_swipe(channels[1]->buffer, channels[1]->len, offset, ms_to_samples(200), 440, 330, &angle);
  triangle(channels[2]->buffer, channels[2]->len, offset, ms_to_samples(200), 175/4);
  triangle(channels[3]->buffer, channels[3]->len, offset,ms_to_samples(200), 224/4);
  noise(channels[4]->buffer, channels[4]->len, offset,ms_to_samples(200));

  float gains[5] = {1., 1., 0.5, 0.5, 0.5};

  mix(channels, gains, 5, buffer);

  //gain(buffer->buffer, buffer->len, offset, ms_to_samples(200), 1.2);
  adsr_param param = {
    0, 1, 100, 0.0, 250, 10
  };
  adsr(buffer->buffer+offset, ms_to_samples(200), &param);
}

void mix(framebuffer** buffers, float* gain,size_t size, framebuffer* out)
{
  int i = 0;
  //float ratio = 1.0 / size;
  for(; i < size; i++) {
    int j = 0;
    for(; j < buffers[i]->len; j++) {
      out->buffer[j] += buffers[i]->buffer[j] * gain[i];
    }
  }
}

