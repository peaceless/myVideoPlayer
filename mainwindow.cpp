#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xffmpeg.h"
#include "xaudio.h"
#include <QFileDialog>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startTimer(40);
 }
void MainWindow::timerEvent(QTimerEvent *event)
{
    long long min = (xffmpeg::Get()->pts) / 60;
    long long sec = (xffmpeg::Get()->pts) % 60;
    char buf[1024] = {0};
    sprintf(buf,"%03lld:%02lld",min,sec);
    ui->playTime->setText(buf);

   // qDebug() << xffmpeg::Get()->pts << " and " <<xffmpeg::Get()->total_sec <<
    //            static_cast<float>(xffmpeg::Get()->pts) / static_cast<float>(xffmpeg::Get()->total_sec);
    if (xffmpeg::Get()->total_sec > 0) {//判断视频总时间
        float rate = static_cast<float>(xffmpeg::Get()->pts) / static_cast<float>(xffmpeg::Get()->total_sec) * 100;
        if (!isPressedSlider) {//当松开时继续刷新进度条位置
            ui->playSlider->setValue(static_cast<int>(rate));
        }
    }
}
void MainWindow::open()
{
    QString name = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择视频文件"));
    if (name.isEmpty()) {return ;}
    this->setWindowTitle(name);//设置窗口标题
    long long totalMs = xffmpeg::Get()->open(name.toLocal8Bit());
    if (totalMs <= 0) {
        QMessageBox::information(this,"err","file open failed");
        return ;
    }
    xaudio::get()->sampleRate = xffmpeg::Get()->sampleRate;
    xaudio::get()->channel = xffmpeg::Get()->channel;
    xaudio::get()->sampleSize = 16;
    xaudio::get()->start();
    char buf[1024] = {0};//用于存放总时间
    long long min = (totalMs)/60;
    long long sec = (totalMs)%60;
    sprintf(buf,"/%03lld:%02lld",min,sec);//存入buf中
    ui->totalTime->setText(buf);
}

void MainWindow::sliderPressed()
{
    isPressedSlider = true;
}
void MainWindow::sliderReleased()
{
    isPressedSlider = false;
    float pos = 0;
    pos = static_cast<float>(ui->playSlider->value() / static_cast<float>(ui->playSlider->maximum() + 1));
    xffmpeg::Get()->seek(pos);
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->vw->resize(size());
    ui->stopButton->move(this->width() / 2 + 50,this->height()-80);
    ui->filePlay->move(this->width() / 2-50,this->height() - 80);
    ui->playSlider->move(25,this->height()-120);
    ui->playSlider->resize(this->width()-50,ui->playSlider->height());
    ui->playTime->move(25,ui->stopButton->y());
    ui->totalTime->move(130,ui->stopButton->y());
}
MainWindow::~MainWindow()
{
    delete ui;
}
