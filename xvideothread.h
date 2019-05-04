#ifndef XVIDEOTHREAD_H
#define XVIDEOTHREAD_H

#include <QThread>
extern "C"
{
#include <libavformat/avformat.h>
}
class xvideoThread:public QThread
{
public:
    static xvideoThread *get()//创建单例模式
    {
        static xvideoThread vt;
        return &vt;
    }
    void run();//线程的运行
    xvideoThread();
    ~xvideoThread();
    AVFrame *yuv = nullptr;
};

#endif // XVIDEOTHREAD_H
