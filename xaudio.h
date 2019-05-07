#ifndef XAUDIO_H
#include <QAudioOutput>
#include <QMutex>
#define XAUDIO_H


class xaudio
{
public:
    QAudioOutput *output = nullptr;
    QIODevice *io = nullptr;
    QMutex mutex;
    static xaudio *get(){
        static xaudio ap;
        return &ap;
    }//单例模
    bool start();
    void play(bool isPlay);
    bool write(const char * data,int datasize);
    void stop();
    int getFree();//获取剩余空间
    ~xaudio();
    int sampleRate = 48000;//样本lv
    int sampleSize = 16;//样本大小
    int channel = 2;//通道数
protected:
    xaudio();
};

#endif // XAUDIO_H
