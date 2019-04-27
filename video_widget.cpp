#include "video_widget.h"
#include "xffmpeg.h"
#include <QDebug>
void video_widget::paintEvent(QPaintEvent *event)
{
    static QImage *image = nullptr;

    if (image == nullptr) {
        uchar *buf = new uchar[width() * height() *4];//存放解码后的视频空间
        image = new QImage(buf,width(),height(),QImage::Format_ARGB32);
    }
    AVPacket pkt = xffmpeg::Get()->read();//读取视频帧
    if (pkt.stream_index != static_cast<int>(xffmpeg::Get()->video_stream)) {//不为视频帧，释放pkt的空间
        av_packet_unref(&pkt);
        qDebug() << "its not a video frame";
        return;
    }
    if (pkt.size == 0) return ;
    AVFrame *yuv = xffmpeg::Get()->decode(&pkt);//解码读取到的视频帧
    av_packet_unref(&pkt);//解码成功后释放空间
    if (yuv == nullptr) return ;
    //将解码后的视频帧转化为rgb
    xffmpeg::Get()->torgb(yuv,image->bits(),width(),height());
    qDebug() << "yuv not null";
    QPainter painter;
    painter.begin(this);
    painter.drawImage(QPoint(0,0),*image);//绘制ffmpeg生成的图像
    painter.end();
}

video_widget::video_widget(QWidget *parent) :QOpenGLWidget (parent)
{
    xffmpeg::Get()->open("E:\\test\\fortest.mp4");
    startTimer(10);//设置定时器
}
void video_widget::timerEvent(QTimerEvent *event)
{
    this->update();//定时器更新
}
