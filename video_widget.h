#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QtWidgets/qwidget.h>
#include <QOpenGLWidget>
#include <QPainter>
class video_widget:public QOpenGLWidget
{
    Q_OBJECT
public :
    video_widget(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);//窗口的重新绘制
    void timerEvent(QTimerEvent *event);//定时器
   // virtual  ~video_widget();
public slots:
    void pause();
private:
    bool isPlay = true;
};

#endif // VIDEO_WIDGET_H
