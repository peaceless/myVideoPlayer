#include "video_widget.h"
#include "xffmpeg.h"
#include "xvideoThread.h"
#include <QDebug>
void video_widget::paintEvent(QPaintEvent *event)
{
    static QImage *image = nullptr;
    static int w = 0;
    static int h = 0;
    if (w != width() || h != height()) {
        if (image) {
            delete image->bits();
            delete image;
            image = nullptr;
        }
    }

    if (image == nullptr) {
        uchar *buf = new uchar[width() * height() *4];//存放解码后的视频空间
        image = new QImage(buf,width(),height(),QImage::Format_ARGB32);
    }
    if (xffmpeg::Get()->yuv == nullptr) return ;
    //将解码后的视频帧转化为rgb
    xffmpeg::Get()->torgb(xffmpeg::Get()->yuv,image->bits(),width(),height());
    qDebug() << "yuv not null" << xffmpeg::Get()->yuv->pts;
    QPainter painter;
    painter.begin(this);
    painter.drawImage(QPoint(0,0),*image);//绘制ffmpeg生成的图像
    painter.end();
}

video_widget::video_widget(QWidget *parent) :QOpenGLWidget (parent)
{
//    xffmpeg::Get()->open("E:\\test\\fortest.mp4");
    xffmpeg::Get()->open("rtmp://live.hkstv.hk.lxdns.com/live/hks");
    startTimer(10);//设置定时器
    xvideoThread::get()->start();
}
void video_widget::timerEvent(QTimerEvent *event)
{
    this->update();//定时器更新
}

void video_widget::pause()
{
    isPlay = !isPlay;
    xffmpeg::Get()->isPlay = isPlay;
    qDebug() << "isplayed button pressed!!";
}
