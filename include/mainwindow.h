#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "camprotocol.h"
#include <windows.h>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    uchar *predata = new uchar[18];

    QImage Matimgtoqt(const cv::Mat &src);

    void initwindow();

    void updatebbox();

//    void mousepress(QObject *obj,QEvent* event);

//    void mousemove(QObject *obj,QMouseEvent* event);

//    void mouserelease(QObject *obj,QMouseEvent* event);

    void keypressEvent(QKeyEvent *event);

    AVPacket *pAVPacket = av_packet_alloc();

    QString cur_ip;

    int cur_port;

    void paintEvent(QPaintEvent *event);


    void rtsp_open();

    ~MainWindow();


signals:
    void sendtcpdata(uchar *data);

private slots:
    void getmsg(uchar num,quint16 x,quint16 y,quint16 dis,quint8 dis1);

    void getshowbuff(uchar *buff,int len);

    void Showpic(QImage image,int w,int h);

    bool eventFilter(QObject *obj,QEvent *event);

    void sendcmd();

    void on_showvideo_comboBox_currentIndexChanged(int index);

    void on_camfocalen_lineEdit_editingFinished();

    void on_fusion_checkBox_clicked(bool checked);

    void on_laser_checkBox_clicked(bool checked);

    void on_detect_checkBox_clicked(bool checked);

    void on_track_checkBox_clicked(bool checked);

    void on_sysmode_combox_currentIndexChanged(int index);

    void opencvrtsp();

    void on_ip_lineEdit_editingFinished();

    void on_port_lineEdit_editingFinished();

    void on_connect_clicked();

private:
    Ui::MainWindow *ui;

    QTimer *timer1 = new QTimer(this);

    void initApplication(void);

    camprotocol *camhandle;
    CH264Decoder *decoder;

    QPainter m_painter;
    QPoint startpoint;
    QPoint finishpoint;
    QRectF rectangle;
    bool m_isMousePress;
    bool m_isdraw;
    int bbox_w;
    int bbox_h;
};

#endif // MAINWINDOW_H
