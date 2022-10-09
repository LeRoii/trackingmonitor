#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
//    m_isMousePress(false),
//    m_isdraw(true)
{
    ui->setupUi(this);
    this->initApplication();

    ui->ip_lineEdit->setText("192.168.1.1");

   // ui->videoDispLabel->installEventFilter(this);
//    timer1 -> setTimerType(Qt::PreciseTimer);
//    connect(timer1,SIGNAL(timeout()),this,SLOT(sendcmd()));
    //timer1->start(1000*60*3);
    connect(ui->videoDispLabel,SIGNAL(emitbboox(QPoint,int,int)),this,SLOT(getbbox(QPoint,int,int)));
    camhandle = new camprotocol();
    decoder = new CH264Decoder();
    initwindow();
    //this->opencvrtsp();
    //decoder->rtsp_open();
    //this->rtsp_open();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete camhandle;
    delete decoder;
    //timer1->stop();
    //delete timer1;
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

void MainWindow::initwindow()
{
    this->setMouseTracking(true);
    //this->setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowActive);
}

void MainWindow::updatebbox(QPoint startpoint,int bbox_w,int bbox_h)
{
    predata[10] = uchar(startpoint.x()>>8);
    predata[11] = startpoint.x()&0xff;

    predata[12] = uchar(startpoint.y()>>8);
    predata[13] = startpoint.y()&0xff;

    predata[14] = uchar(bbox_w>>8);
    predata[15] = bbox_w&0xff;

    predata[16] = uchar(bbox_h>>8);
    predata[17] = bbox_h&0xff;

    emit sendtcpdata(predata);
}

//void MainWindow::mousepress(QObject *obj,QEvent *event)
//{

//    if(obj==ui->videoDispLabel)
//    {

//        if(event->type()==QEvent::MouseButtonPress)
//        {
//            qDebug("mousepress");
//            m_isMousePress = true;
//            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//            startpoint = ev->pos();
//            qDebug()<<ev->pos();
//        }
//    }
//    return this->mousepress(obj,event);
//}

//void MainWindow::mousemove(QObject *obj, QMouseEvent *event)
//{
//    if(obj==ui->videoDispLabel)
//    {
//        if(m_isMousePress)
//        {
//            qDebug()<<"painting";
//            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//            finishpoint = ev->pos();
//            update();
//        }
//    }
//    return this->mousemove(obj,event);
//}

//void MainWindow::mouserelease(QObject *obj, QMouseEvent *event)
//{
//    if(obj==ui->videoDispLabel)
//    {
//        if(event->type()==QEvent::MouseButtonRelease)
//        {
//            qDebug("mouserelease");
//            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//            finishpoint = ev->pos();
//            m_isMousePress = false;
//            qDebug()<<ev->pos();
//        }
//    }
//    return this->mouserelease(obj,event);
//}

//void MainWindow::keypressEvent(QKeyEvent *event)
//{

//    if (event->key() == Qt::Key_Escape)
//        {
//            close();
//        }
//    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
//    {


//        close();
//    }
//}



//bool MainWindow::eventFilter(QObject *obj, QEvent *event)
//{
//    if(obj==ui->videoDispLabel)
//    {
//        if(event->type()==QEvent::MouseButtonPress)
//        {
//            qDebug("mousepress");
//            m_isMousePress = true;
//            m_isdraw = true;
//            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//            startpoint = ev->pos();
//            qDebug()<<ev->pos();
//            return true;
//        }

//        if(event->type()==QEvent::MouseMove)
//        {
//            if(m_isMousePress)
//                    {
//                        qDebug()<<"painting";
//                        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//                        finishpoint = ev->pos();
//                        update();
//                    }
//        }

//        if(event->type()==QEvent::MouseButtonRelease)
//        {
//            qDebug("mouserelease");

//            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
//            finishpoint = ev->pos();
//            m_isMousePress = false;
//            qDebug()<<ev->pos();

//            return true;
//        }


//        if(event->type()==QEvent::KeyPress)
//        {
//            if(!m_isMousePress)
//            {
//             QKeyEvent *key_event=static_cast<QKeyEvent *>(event);
//             if (key_event->key() == Qt::Key_Escape)
//                 {

//                 }
//             if (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter)
//             {
//                updatebbox();
//             }
//            }
//        }


//        return false;

//    }
//    return eventFilter(obj,event);
//}

void MainWindow::sendcmd()
{
    emit sendtcpdata(predata);
}


//void MainWindow::paintEvent(QPaintEvent *event)
//{

//    m_painter.begin(this);
//    m_painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine, Qt::FlatCap));
//   // m_painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

//    if (m_isMousePress)
//    {
//        bbox_w = finishpoint.x()-startpoint.x();
//        bbox_h = finishpoint.y()-startpoint.y();
//        QRectF rectangle(startpoint.x(), startpoint.y(), bbox_w,  bbox_h);
//        qDebug()<< finishpoint.x()<<finishpoint.y();
//        m_painter.drawRect(rectangle);
//    }
//    ui->videoDispLabel->show();

//    m_painter.end();
//}

void MainWindow::rtsp_open()
{

   // QCoreApplication::processEvents();

    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    const AVCodec *pCodec = NULL;
    AVFrame *pFrame,*pFrameRGB;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx;

    unsigned char *out_buffer;
    int i,videoIndex;
    int ret;
    char errors[1024] = "";

    //char filepath[] = "demo.avi";
    //rtsp地址:
    char url[] = "rtsp://192.168.1.114:8554/live";
    //char url[] = "rtsp://admin:admin@192.168.1.108";

    //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
    pFormatCtx = avformat_alloc_context();  //init FormatContext
    //初始化FFmpeg网络模块
    avformat_network_init();    //init FFmpeg network


    //open Media File
    ret = avformat_open_input(&pFormatCtx,url,NULL,NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        //cout <<"Failed to open video: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    //Get audio information
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        //cout <<"Failed to get audio info: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    videoIndex = -1;

    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 videoIndex
    ///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
       // QCoreApplication::processEvents();

        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
        }
    }

    ///videoIndex-1 说明没有找到视频流
    if (videoIndex == -1) {
        printf("Didn't find a video stream.\n");
        return;
    }

    //配置编码上下文，AVCodecContext内容
    //1.查找解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    //2.初始化上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    //3.配置上下文相关参数
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[videoIndex]->codecpar);
    //4.打开解码器
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        //cout <<"Failed to open Codec Context: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    //初始化视频帧
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    //为out_buffer申请一段存储图像的内存空间
    out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height,1));
    //实现AVFrame中像素数据和Bitmap像素数据的关联
    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize, out_buffer,
                   AV_PIX_FMT_RGB32,pCodecCtx->width, pCodecCtx->height,1);
    //为AVPacket申请内存
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //打印媒体信息
    av_dump_format(pFormatCtx,0,url,0);
    //初始化一个SwsContext
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

    //设置视频label的宽高为视频宽高
    ui->videoDispLabel->setGeometry(0, 0, pCodecCtx->width, pCodecCtx->height);

    //读取帧数据，并通过av_read_frame的返回值确认是不是还有视频帧
    while(av_read_frame(pFormatCtx,packet) >=0){
        //判断视频帧

        //QCoreApplication::processEvents();
        if(packet->stream_index == videoIndex){
            //解码视频帧
            ret = avcodec_send_packet(pCodecCtx, packet);
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if(ret != 0){
                av_strerror(ret,errors,sizeof(errors));
               // cout <<"Failed to decode video frame: ["<< ret << "]"<< errors << endl;
            }
            if (ret == 0) {
                //处理图像数据
                sws_scale(img_convert_ctx,
                                        (const unsigned char* const*) pFrame->data,
                                        pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                                        pFrameRGB->linesize);
                QImage img((uchar*)pFrameRGB->data[0],pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);

                ui->videoDispLabel->setPixmap(QPixmap::fromImage(img));
                //释放前需要一个延时
                //Delay(1);
            }
        }
        //释放packet空间
        av_packet_unref(packet);
    }

    //close and release resource
    av_free(out_buffer);
    av_free(pFrameRGB);

    sws_freeContext(img_convert_ctx);
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);
    exit(0);
}

void MainWindow::Showpic(QImage image,int w,int h)
{

   // ui->videoDispLabel->setGeometry(0, 0, w, h);
    ui->videoDispLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::getmsg(uchar num, qint16 x, qint16 y,quint16 dis,quint8 dis1)
{
    QString text;
    text = "检测数目： "+QString::number(num)+"跟踪靶坐标："+QString::number(x)+","+QString::number(y)
            +"距离："+QString::number(dis)+"."+QString::number(dis1);
    //ui->videoDispLabel->setText(text);
    ui->statusBar->showMessage(text);
}

void MainWindow::getbbox(QPoint start, int w, int h)
{
    updatebbox(start,w,h);
}

void MainWindow::getshowbuff(uchar *buff,int len)
{
    cv::Mat res;
    QImage image;
    qDebug()<<len;
    decoder->decode(buff,len,res);
    image = this->Matimgtoqt(res);
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    //this->Showpic(image);
}


void MainWindow::initApplication()
{
    predata[0] = uchar(0x55);
    predata[1] = uchar(0xAA);
    predata[2] = uchar(0x02);
    predata[3] = uchar(0x00);
    predata[4] = uchar(0x01);
    predata[5] = uchar(0x00);
    predata[6] = uchar(0x00);
    predata[7] = uchar(0x00);
    predata[8] = uchar(0x00);
    predata[9] = uchar(0x00);
    predata[10] = uchar(0x00);
    predata[11] = uchar(0x00);
    predata[12] = uchar(0x00);
    predata[13] = uchar(0x00);
    predata[14] = uchar(0x00);
    predata[15] = uchar(0x00);
    predata[16] = uchar(0x00);
    predata[17] = uchar(0x00);
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
    QString text = QString::number(focal_len,10);
    predata[4] = uchar(focal_len);
    qDebug("%x",predata[4]);

    emit sendtcpdata(predata);
    ui->cam_focal_lenth_status_label->setText("x"+text);
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

void MainWindow::opencvrtsp()
{
     cv::VideoCapture cap;
     cv::Mat frame;
     cap.open("rtsp://192.168.1.102:8554/live");
     if(cap.isOpened())
     {
         while(1)
         {
             cap.read(frame);
             cv::imshow("video",frame);

             int c = cv::waitKey(0);
             if(c == 0x27)
             {
                 break;
             }

         }
     }
     else {
         qDebug()<<"拉流失败";
     }

}

void MainWindow::on_ip_lineEdit_editingFinished()
{
    cur_ip = ui->ip_lineEdit->text();
}

void MainWindow::on_port_lineEdit_editingFinished()
{
    cur_port = ui->port_lineEdit->text().toInt();
}

void MainWindow::on_connect_clicked()
{
   camhandle->setMai(this,cur_ip,cur_port);
   //this->rtsp_open();
}
