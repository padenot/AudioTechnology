#ifndef DBMETER_H
#define DBMETER_H

#include <QWidget>

class dBMeter : public QWidget
{
  Q_OBJECT

  public:
    dBMeter(QWidget *parent = 0);

  public slots:
    void valueChanged(float* new_values, size_t size);

  protected:
    void paintEvent(QPaintEvent *event);
    float* values_;
    size_t size_;
    size_t width_;
    size_t height_;
};

#endif
