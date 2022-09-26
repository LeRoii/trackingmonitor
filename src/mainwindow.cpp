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

    //camhandle->setMai(this);
    //this->opencvrtsp();
    //decoder->rtsp_open();
    this->rtsp_open();
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

void MainWindow::rtsp_open()
{

           AVFormatContext* format_ctx = NULL;
           //AVFormatContext* ptr = NULL;
           AVCodecContext *pAVCodecContext_video = nullptr;
           const AVCodec *pAVCodec_video = nullptr;
            AVFrame *pAVFrame_video,*pAVFrameRGB32_video;
             char url[] = "rtsp://admin:admin@192.168.1.108";

           format_ctx = avformat_alloc_context();
           AVCodecParameters *pAVCodePar_video = avcodec_parameters_alloc();
          // AVPacket *pAVPacket = av_packet_alloc();                                  // ffmpeg单帧数据包
           pAVFrame_video = av_frame_alloc();                                 // ffmpeg单帧缓存
           pAVFrameRGB32_video = av_frame_alloc();                            // ffmpeg单帧缓存转换颜色空间后的缓存
           AVCodecParserContext *pAVCodeParseContext_video = nullptr;
           struct SwsContext *pSwsContext_video = nullptr;                             // ffmpeg编码数据格式转换
           AVDictionary * opts = nullptr;

           int ret = -1;
           long long numBytes = 0;                                                           // 解码后的数据长度
           uchar *outBuffer = nullptr;                                                // 解码后的数据存放缓存区
           //av_register_all();

        // open rtsp: Open an input stream and read the header. The codecs are not opened
           //const char* url = "rtsp://admin:genepoint2020@192.168.100.14:554/cam/realmonitor?channel=1&subtype=0";
           av_dict_set(&opts, "rtsp_transport", "tcp", 0);
           av_dict_set(&opts, "stimeout", "2000000", 0);
           // audio/video stream index
           int video_stream_index = -1;
          ret = avformat_open_input(&format_ctx, "rtsp://admin:admin@192.168.1.108", nullptr, nullptr);
               if (ret != 0)
               {
                   printf("Can not open this file");
                   //return;
               }
               // Read packets of a media file to get stream information
               ret = avformat_find_stream_info(format_ctx, nullptr);
               if (ret < 0) {
                   printf("Unable to get stream info");
                  // return ;
               }

               fprintf(stdout, "Number of elements in AVFormatContext.streams: %d\n", format_ctx->nb_streams);
               for (int i = 0; i < format_ctx->nb_streams; ++i) {
                   const AVStream *stream = format_ctx->streams[i];
                   fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
                   if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                       video_stream_index = i;
                       // 对找到的视频流寻解码器
                       pAVCodePar_video = stream->codecpar;
                       pAVCodec_video = avcodec_find_decoder(pAVCodePar_video->codec_id);
                       if (!pAVCodec_video) {
                           video_stream_index = -1;
                           break;
                       }
                       pAVCodeParseContext_video = av_parser_init(pAVCodec_video->id);
                       if (!pAVCodeParseContext_video) {
                           video_stream_index = -1;
                           break;
                       }
                       pAVCodecContext_video = avcodec_alloc_context3(pAVCodec_video);
                        avcodec_parameters_to_context(pAVCodecContext_video,format_ctx->streams[video_stream_index]->codecpar);
                       if (avcodec_open2(pAVCodecContext_video, pAVCodec_video, NULL) < 0) {
                           video_stream_index = -1;
                           break;
                       }
                       fprintf(stdout, "dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
                               stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);

                   }


                                                     //解码后的h264数据转换成RGB32
                   if(pAVCodecContext_video->pix_fmt==AV_PIX_FMT_NONE)
                   {
                       continue;
                   }


                     pSwsContext_video = sws_getContext(pAVCodecContext_video->width, pAVCodecContext_video->height,
                                   pAVCodecContext_video->pix_fmt,pAVCodecContext_video->width, pAVCodecContext_video->height,
                                           AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
               }
               numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pAVCodecContext_video->width,pAVCodecContext_video->height,1);
               //outBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
                outBuffer = (uchar *) av_malloc(numBytes);
               av_image_fill_arrays(pAVFrameRGB32_video->data, pAVFrameRGB32_video->linesize, outBuffer, AV_PIX_FMT_RGB32, pAVCodecContext_video->width, pAVCodecContext_video->height, 1);

               int av_size =  pAVCodecContext_video->width * pAVCodecContext_video->height;

               pAVPacket = (AVPacket *) malloc(sizeof(AVPacket));

               av_dump_format(format_ctx,0,url,0);
               av_new_packet(pAVPacket,av_size);

               int as = 0;
               while(av_read_frame(format_ctx,pAVPacket)>=0)
               {
                   if(pAVPacket->stream_index==video_stream_index)
                   {
                       qDebug("a == %d\n",++as);

                       int ret = avcodec_send_packet(pAVCodecContext_video, pAVPacket);
                       int got_picture = avcodec_receive_frame(pAVCodecContext_video, pAVFrame_video);
                       if (ret<0)
                       {
                           qDebug()<<"decode failed！！！";
                       }

                       if(!got_picture)
                       {
                           sws_scale(pSwsContext_video,(const unsigned char* const*)pAVFrame_video->data,pAVFrame_video->linesize
                                     ,0,pAVCodecContext_video->height,pAVFrameRGB32_video->data,pAVFrameRGB32_video->linesize);

                           QImage img((uchar*)pAVFrameRGB32_video->data[0],pAVCodecContext_video->width,
                                   pAVCodecContext_video->height,QImage::Format_RGB32);

                            ui->videoDispLabel->setPixmap(QPixmap::fromImage(img));

                            Sleep(1000);
                       }

                   }

                   av_packet_unref(pAVPacket);
               }





           av_parser_close(pAVCodeParseContext_video);
           av_frame_free(&pAVFrame_video);
           av_frame_free(&pAVFrameRGB32_video);
           av_free(outBuffer);
           av_free(pSwsContext_video);
           avcodec_free_context(&pAVCodecContext_video);
           avformat_close_input(&format_ctx);
           exit(0);

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
