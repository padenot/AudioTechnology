#ifndef DBMETER_H
#define DBMETER_H

#include <QWidget>

class dBMeter : public QWidget
{
Q_OBJECT

public:
dBMeter(QWidget *parent = 0);

public slots:
void newval(int val);

protected:
void paintEvent(QPaintEvent *event);
int val; 
};

#endif
