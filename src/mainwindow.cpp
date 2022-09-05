#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QImage *img=new QImage;
    img->load("/home/zpwang/code/trackingboard/1.png");
    ui->videoDispLabel->setPixmap(QPixmap::fromImage(*img));
}

MainWindow::~MainWindow()
{
    delete ui;
}
