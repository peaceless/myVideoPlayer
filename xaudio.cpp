#include "xaudio.h"
#include <QAudioOutput>
#include <QMutex>
#include <QDebug>
void xaudio::stop()
{
        mutex.lock();
        if (output) {//为打开audioOutput
            output->stop();
            delete output;
            output = nullptr;
            io = nullptr;
        }
        mutex.unlock();
}

//首先设置播放的格式以及参数
bool xaudio::start()
{
    stop();
    mutex.lock();
//    QAudioOutput *out;//播放音频
    QAudioFormat fmt;//设置音频输出格式
    fmt.setSampleRate(48000);//美妙采样lv
    fmt.setSampleSize(16);//声音样本的大小
    fmt.setChannelCount(2);//省道
    fmt.setCodec("audio/pcm");//解码格式
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);//设置音频类型
    output = new QAudioOutput(fmt);
    io = output->start();//播放开始
    mutex.unlock();
    return true;
}

void xaudio::play(bool isplay)
{
    mutex.lock();
    if (!output) {
        mutex.unlock();
        return ;
    }
    if (isplay) {
        output->resume();
    } else {
        output->suspend();
    }
    mutex.unlock();
}

int xaudio::getFree()
{
    mutex.lock();
    if (!output) {
        mutex.unlock();
        return 0;
    }
    int free = output->bytesFree();//剩余空间
    mutex.unlock();
    return free;
}

bool xaudio::write(const char *data,int datasize)
{
    mutex.lock();
    if (io) {
        io->write(data,datasize);
    }
    mutex.unlock();
    return true;
}
xaudio::xaudio()
{}
xaudio::~xaudio()
{}
