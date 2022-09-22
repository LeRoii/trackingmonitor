#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initApplication();

    camhandle = new camprotocol();
    decoder = new CH264Decoder();

    camhandle->setMai(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete camhandle;
    delete decoder;
}

QImage MainWindow::Matimgtoqt(const cv::Mat &src)
{
    //Mat转成QImage
    {
        //CV_8UC1 8位无符号的单通道---灰度图片
        if(src.type() == CV_8UC1)
        {
            //使用给定的大小和格式构造图像
            //QImage(int width, int height, Format format)
            QImage qImage(src.cols,src.rows,QImage::Format_Indexed8);
            //扩展颜色表的颜色数目
            qImage.setColorCount(256);

            //在给定的索引设置颜色
            for(int i = 0; i < 256; i ++)
            {
                //得到一个黑白图
                qImage.setColor(i,qRgb(i,i,i));
            }
            //复制输入图像,data数据段的首地址
            uchar *pSrc = src.data;
            //
            for(int row = 0; row < src.rows; row ++)
            {
                //遍历像素指针
                uchar *pDest = qImage.scanLine(row);
                //从源src所指的内存地址的起始位置开始拷贝n个
                //字节到目标dest所指的内存地址的起始位置中
                memcmp(pDest,pSrc,src.cols);
                //图像层像素地址
                pSrc += src.step;
            }
            return qImage;
        }
        //为3通道的彩色图片
        else if(src.type() == CV_8UC3)
        {
            //得到图像的的首地址
            const uchar *pSrc = (const uchar*)src.data;
            //以src构造图片
            QImage qImage(pSrc,src.cols,src.rows,src.step,QImage::Format_RGB888);
           // QImage qImage(pSrc,src.cols,src.rows,src.cols*src.channels(),QImage::Format_RGB888);
            //在不改变实际图像数据的条件下，交换红蓝通道
            return qImage.rgbSwapped();
        }
        //四通道图片，带Alpha通道的RGB彩色图像
        else if(src.type() == CV_8UC4)
        {
            const uchar *pSrc = (const uchar*)src.data;
            QImage qImage(pSrc, src.cols, src.rows, src.step, QImage::Format_ARGB32);
            //返回图像的子区域作为一个新图像
            return qImage.copy();
        }
        else
        {
            return QImage();
        }
    }

}

void MainWindow::Showpic(QImage image)
{
    ui->videoDispLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::getmsg(uchar num, quint16 x, quint16 y)
{
    QString text;
    text = "检测数目： "+QString::number(num)+"跟踪靶坐标："+QString::number(x)+","+QString::number(y);
    //ui->videoDispLabel->setText(text);
    ui->statusBar->showMessage(text);
}

void MainWindow::getshowbuff(uchar *buff,int len)
{
    cv::Mat res;
    QImage image;
    qDebug()<<len;
    decoder->decode(buff,len,res);
    image = this->Matimgtoqt(res);
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    this->Showpic(image);
}


void MainWindow::initApplication()
{
    predata[0] = uchar(0x55);
    predata[1] = uchar(0xAA);
    predata[2] = uchar(0x02);
    predata[3] = uchar(0x00);
    predata[4] = uchar(0x05);
    predata[5] = uchar(0x00);
    predata[6] = uchar(0x00);
    predata[7] = uchar(0x00);
    predata[8] = uchar(0x00);
    predata[9] = uchar(0x00);
    emit sendtcpdata(predata);
}

void MainWindow::on_showvideo_comboBox_currentIndexChanged(int index)
{
    qDebug()<<index;
    switch (index) {
    case 0:
        predata[3] = 0x00;
        ui->showvideostatus_label->setText("可见光");
        ui->ircam_status_label->setText("关闭");
        break;
    case 1:
        predata[3] = 0x01;
         ui->showvideostatus_label->setText("红外");
         ui->ircam_status_label->setText("打开");

        break;
    case 2:
        predata[3] = 0x03;
         ui->showvideostatus_label->setText("融合");
         ui->ircam_status_label->setText("打开");

        break;
    }

    emit sendtcpdata(predata);
}

void MainWindow::on_camfocalen_lineEdit_editingFinished()
{
    short focal_len = ui->camfocalen_lineEdit->text().toShort();
    predata[4] = uchar(focal_len);
    qDebug("%x",predata[4]);

    emit sendtcpdata(predata);
}

void MainWindow::on_fusion_checkBox_clicked(bool checked)
{
    //qDebug()<<checked;
    if(checked){
        predata[8] = 0x01;
        emit sendtcpdata(predata);
    }
    else {
        predata[8] = 0x00;
        emit sendtcpdata(predata);
    }
}

void MainWindow::on_laser_checkBox_clicked(bool checked)
{
    if(checked)
    {
        predata[7] = 0x01;
        emit sendtcpdata(predata);
        ui->laser_status_label->setText("打开");
    }
    else {
        predata[7] = 0x00;
        emit sendtcpdata(predata);
        ui->laser_status_label->setText("关闭");
    }
}

void MainWindow::on_detect_checkBox_clicked(bool checked)
{
    if(checked)
    {
        predata[5] = 0x01;
        emit sendtcpdata(predata);
        ui->detect_status_label->setText("打开");
    }
    else {
        predata[5] = 0x00;
        emit sendtcpdata(predata);
        ui->detect_status_label->setText("关闭");

    }
}

void MainWindow::on_track_checkBox_clicked(bool checked)
{
    if(checked)
    {
        predata[6] = 0x01;
        emit sendtcpdata(predata);
    }
    else {
        predata[6] = 0x00;
        emit sendtcpdata(predata);
    }
}

void MainWindow::on_sysmode_combox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        predata[9] = 0x00;
        ui->sysmode_status_label->setText("定位");
        break;
    case 1:
        predata[9] = 0x01;
         ui->sysmode_status_label->setText("跟踪");
        break;
    }

    emit sendtcpdata(predata);
}
