#include <QtGui>

#include "dbmeter.h"
#include "vagg/vagg.h"

dBMeter::dBMeter(QWidget *parent)
: QWidget(parent),size_(1),width_(40),height_(260)
{
  setFixedSize(width_, height_);
}

void dBMeter::paintEvent(QPaintEvent *)
{
  if (values_) {
  QPainter painter(this);
  // paint background
  painter.fillRect(0, 0, width_, height_, Qt::black);
  float channel_width = width_ / size_;
  for (size_t i = 0; i < size_; i++) {
    float channel_height = height_ - values_[i];
    VAGG_LOG(VAGG_LOG_DEBUG, "channel_height : %f", channel_height);
    painter.fillRect(channel_width * i, -values_[i], channel_width, channel_height, Qt::green);
  }
  }
}

void dBMeter::valueChanged(float* values, size_t size) {
  if (size > MAX_CHANNELS) {
    VAGG_LOG(VAGG_LOG_FATAL, "Only %d channels at most", MAX_CHANNELS);
  }
  for (size_t i = 0; i < size; i++) {
    values_[i] = values[i];
  }
  size_ = size;
  this->update();
}

