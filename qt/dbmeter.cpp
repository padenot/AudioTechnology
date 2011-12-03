#include <QtGui>

#include "dbmeter.h"
#include "vagg/vagg.h"

dBMeter::dBMeter(QWidget *parent)
: QWidget(parent),size_(1),width_(100),height_(260)
{
  memset(values_, 0, MAX_CHANNELS * sizeof(float));
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
      float channel_height = height_ - values_[i] * 4;
      painter.fillRect(channel_width * i, -values_[i] * 4, channel_width, channel_height, Qt::green);
      QString text = QString::number(values_[i], 'f', 1);
      painter.drawText(channel_width * i + 4, height_ - 4, text);
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

