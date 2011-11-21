#include <QtGui>

#include "dbmeter.h"

dBMeter::dBMeter(QWidget *parent)
: QWidget(parent)
{

val=0;
//setGeometry(100,220);
//setWindowTitle(tr("dbMeter! v0.1"));
//resize(100, 220);
setFixedSize(100, 220);
}

void dBMeter::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.fillRect(10, 10, 80, 200, Qt::black);
	painter.fillRect(10, 210, 80, -val, Qt::green);
}

void dBMeter::newval(int value){
	val = value;
	this->update();
}

