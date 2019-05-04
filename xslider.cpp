#include "xslider.h"
#include <QMouseEvent>
#include <QDebug>
xslider::xslider(QWidget *widget) : QSlider (widget)
{

}
xslider::~xslider()
{

}

void xslider::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->pos().x() << " and " << width();
    double pos = static_cast<double>(event->pos().x()) / static_cast<double>(width());
    qDebug() << static_cast<int>(pos * (this->maximum()+1));
    setValue(static_cast<int>(pos * (this->maximum()+1)));
    QSlider::mousePressEvent(event);
}
