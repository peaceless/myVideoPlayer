#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QtWidgets/qwidget.h>
#include <QOpenGLWidget>
#include <QPainter>
class video_widget:public QOpenGLWidget
{
public :
    video_widget(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);//窗口的重新绘制
    void timerEvent(QTimerEvent *event);//定时器
   // virtual  ~video_widget();
};

#endif // VIDEO_WIDGET_H
