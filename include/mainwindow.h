#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "camprotocol.h"
#include "h264_decoder.h"
#include <windows.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    uchar *predata = new uchar[10];

    QImage Matimgtoqt(const cv::Mat &src);

    AVPacket *pAVPacket = av_packet_alloc();

    QString cur_ip;

    int cur_port;


    void rtsp_open();

    ~MainWindow();


signals:
    void sendtcpdata(uchar *data);

private slots:
    void getmsg(uchar num,quint16 x,quint16 y);

    void getshowbuff(uchar *buff,int len);

    void Showpic(QImage image);

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

private:
    Ui::MainWindow *ui;

    void initApplication(void);

    camprotocol *camhandle;
    CH264Decoder *decoder;
};

#endif // MAINWINDOW_H
