#include <QtGui>

#include "dbmeter.h"


dBMeter::dBMeter(QWidget *parent)
: QWidget(parent)
{
	val=0;
	setFixedSize(40, 220);
}

void dBMeter::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.fillRect(0, 0, 40, 200, Qt::black);
	painter.fillRect(0, 200, 40, -val, Qt::green);
	//	qDebug() << val;
}

void dBMeter::newval(int value){
	val = value;
	this->update();
}

