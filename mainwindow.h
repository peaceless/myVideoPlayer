#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
using namespace std;
extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
    #include "libavformat/version.h"
    #include "libavutil/time.h"
    #include "libavutil/mathematics.h"
}
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent *event);
public slots:
    void open();//用于相应打开文件的按钮
    void sliderPressed();
    void sliderReleased();
private:
    Ui::MainWindow *ui;
    void timerEvent(QTimerEvent *event);
    bool isPressedSlider = false;
};

#endif // MAINWINDOW_H
