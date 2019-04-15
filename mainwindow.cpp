#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
  //  qDebug() << avcodec_configuration();
  //  unsigned version = avcodec_version();
    //QString ch = QString::number(version,10);
    //qDebug() << "version:" << version;
}

MainWindow::~MainWindow()
{
    delete ui;
}
