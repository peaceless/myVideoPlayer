#ifndef XSLIDER_H
#include <QSlider>
#define XSLIDER_H

#include <QObject>

class xslider : public QSlider
{
    Q_OBJECT
public:
    xslider(QWidget *parent);
    ~xslider();
    void mousePressEvent(QMouseEvent *event);//鼠标点击事件
};

#endif // XSLIDER_H
