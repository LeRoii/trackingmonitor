#include "camprotocol.h"

class rtsp : public mythread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        AVFormatContext *pFormatCtx = NULL;
        AVCodecContext *pCodecCtx = NULL;
        const AVCodec *pCodec = NULL;
        AVFrame *pFrame,*pFrameRGB;
        AVPacket *packet;
        struct SwsContext *img_convert_ctx;

        Run_stopped = false;

        while (!Run_stopped)
        {
        unsigned char *out_buffer;
        int i,videoIndex;
        int ret;
        char errors[1024] = "";

        //char filepath[] = "demo.avi";
        //rtsp地址:
        char url[] = "rtsp://192.168.168.200:8554/live";
       // char url[] = "rtsp://192.168.1.108:8554/live";
        //char url[] = "rtsp://admin:admin@192.168.1.108";

        //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
        pFormatCtx = avformat_alloc_context();  //init FormatContext
        //初始化FFmpeg网络模块
        avformat_network_init();    //init FFmpeg network


        //open Media File
        qDebug()<<"open rtsp";
        ret = avformat_open_input(&pFormatCtx,url,NULL,NULL);
        if(ret != 0){
            av_strerror(ret,errors,sizeof(errors));
            //cout <<"Failed to open video: ["<< ret << "]"<< errors << endl;
            qDebug()<<"open rtsp failed";
            exit(ret);

        }


        //Get audio information
        ret = avformat_find_stream_info(pFormatCtx,NULL);
        if(ret != 0){
            av_strerror(ret,errors,sizeof(errors));
            std::cout <<"Failed to get audio info: ["<< ret << "]"<< errors << std::endl;
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
      //  ui->videoDispLabel->setGeometry(0, 0, pCodecCtx->width, pCodecCtx->height);

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

                    qDebug()<<"show";
                    emit sendimg(img,pCodecCtx->width, pCodecCtx->height);
                   // ui->videoDispLabel->setPixmap(QPixmap::fromImage(img));
                    //释放前需要一个延时
                    msleep(1);
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
        }
        exit(0);
    }
};




class cam : public mythread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        QTcpSocket *tcpsocket = new QTcpSocket();
        char *Rec_Temp = new char[75000];
        int Rec_len = 0;

        Run_stopped = false;
       // Tcp_send = false;

        tcpsocket->abort();
       // tcpsocket->connectToHost("127.0.0.1",777);
        tcpsocket->connectToHost(m_ip,m_port);
        qDebug()<<m_ip<<m_port;

        while (!Run_stopped) {
//            tcpsocket->abort();
//            tcpsocket->connectToHost(m_ip,m_port);

            if(tcpsocket->waitForConnected(3000))
            {
                 if(Tcp_send)
                 {
                     /*发送指令*/
                     qApp->processEvents();
                     for (int i = 0;i<18;i++)
                     {
                        qDebug("%02x",data[i]);
                     }

                     int num = tcpsocket->write((char*)data,18);
                     tcpsocket->flush();


                     Tcp_send = false;
                 }

                 /*接收数据*/
                // qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<"start r";
                 if(tcpsocket->waitForReadyRead(1))
                 {
                    Rec_len = tcpsocket->read(Rec_Temp, 75000);

                    qDebug()<<"rec_len:"<<Rec_len;

                    for (int i=0; i<Rec_len;)
                    {
                        if (m_freeBytes->tryAcquire(1))
                        {
                            m_Rec_Buffer[Writer_Ptr++] = (uchar)Rec_Temp[i];

                            if (Writer_Ptr == BUFFER_SIZE)
                                Writer_Ptr = 0;

                            m_usedBytes->release(1);

                            i++;
                        }
                        else
                        {
                            usleep(1);

                            if (Run_stopped)
                                break;
                        }
                    }
                }
                // qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<"finish r";

            }
            else
            {
                qDebug()<<"连接失败";
            }
        }

        tcpsocket->close();
        delete tcpsocket;
        Run_stopped = true;
    }


};

class decode : public mythread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        uchar *Pre_Decoder = new uchar[11];
        uchar detect_num = 0;
        quint16 tracker_x = 0;
        quint16 tracker_y = 0;
        quint16 distance = 0;
        quint8 distance1 = 0;

        Run_stopped = false;

        while(!Run_stopped)
        {
            if(m_usedBytes->tryAcquire(11,1))
            {

                Pre_reader_Ptr = Reader_Ptr;
                 Pre_Decoder[0] = m_Rec_Buffer[Pre_reader_Ptr++];

                 if (Pre_Decoder[0] == (uchar)0x55)
                 {
                    Pre_Decoder[1] = m_Rec_Buffer[Pre_reader_Ptr++];
                    Pre_Decoder[2] = m_Rec_Buffer[Pre_reader_Ptr++];

                    if ((Pre_Decoder[1] == (uchar)0xAA)&&(Pre_Decoder[2]==0x01))
                    {
                        //for (int i=3;i<12;i++)
                        for (int i=3;i<11;i++)
                        {
                            Pre_Decoder[i] = m_Rec_Buffer[Pre_reader_Ptr++];
                        }
                        detect_num = Pre_Decoder[3];
                        tracker_x = Pre_Decoder[5]|Pre_Decoder[4]<<8;
                        tracker_y = Pre_Decoder[7]|Pre_Decoder[6]<<8;

                        distance = Pre_Decoder[9]|Pre_Decoder[8]<<8;
                        distance1 = Pre_Decoder[10];



//                        int len = Pre_Decoder[8]|Pre_Decoder[9]<<8|Pre_Decoder[10]<<16|Pre_Decoder[11]<<24;
//                        uchar showbuff[len];
//                        int ptr = 0;
//                        int num = 1;
//                        int left = len;
//                        QElapsedTimer mstimer;
//                        mstimer.start();


//                        while(left>NUM_SIZE)
//                        {
//                            if(m_usedBytes->tryAcquire(NUM_SIZE,1))
//                            {
//                                for (ptr;  ptr<NUM_SIZE*num;) {
//                                    showbuff[ptr++] = m_Rec_Buffer[Pre_reader_Ptr++];

//                                    if (Pre_reader_Ptr == BUFFER_SIZE)
//                                        Pre_reader_Ptr = 0;
//                                }
//                                num++;
//                                left -= NUM_SIZE;
//                                m_freeBytes->release(NUM_SIZE);
//                            }
//                            else {
//                                usleep(1);
//                            }
//                        }
//                        if(m_usedBytes->tryAcquire(left,1))
//                        {
//                            for ( ptr;  ptr<NUM_SIZE*(num-1)+left;) {
//                                showbuff[ptr++] = m_Rec_Buffer[Pre_reader_Ptr++];

//                                if (Pre_reader_Ptr == BUFFER_SIZE)
//                                    Pre_reader_Ptr = 0;
//                            }
//                            num = 1;

//                            m_usedBytes->release(left);
//                            left = 0;
//                        }

//                        while(ptr<len)
//                        {
//                            m_usedBytes->acquire(1);
//                            showbuff[ptr++] = m_Rec_Buffer[Pre_reader_Ptr++];

//                            if (Pre_reader_Ptr == BUFFER_SIZE)
//                                Pre_reader_Ptr = 0;
//                            m_freeBytes->release(1);
//                        }
//                        float time =(double)mstimer.nsecsElapsed()/(double)1000000;
//                        qDebug() <<"time= " <<time<<"ms";

//                        uchar *sendbuff = new uchar[len];

//                        memcpy(sendbuff,showbuff,len);

//                        emit sendshowbuff(sendbuff,len);

                        emit sendmsgtomain(detect_num,tracker_x,tracker_y,distance,distance1);



//                        len = 0;
//                        ptr = 0;
                        Reader_Ptr = Pre_reader_Ptr;
                        m_freeBytes->release(8);

                    }
                    else {
                        Reader_Ptr++;

                        m_usedBytes->release(5);
                        m_freeBytes->release(3);
                    }
                 }

                 else {
                     Reader_Ptr++;

                     m_usedBytes->release(7);
                     m_freeBytes->release(1);
                 }
            }
        }

        delete [] Pre_Decoder;
        Run_stopped = false;

    }
};

class img_decode : public mythread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        Run_stopped = false;
        
        uchar *Pre_Decoder = new uchar[BUFFER_SIZE];
        
        while(!Run_stopped)
        {
            if (m_usedBytes->tryAcquire(19, 1))
            {
                
            }
        }
    }
};
        


camprotocol::camprotocol()
{
    Rec_Buffer_ptr = new uchar[BUFFER_SIZE];
    freeBytes = new QSemaphore(BUFFER_SIZE);
    usedBytes = new QSemaphore(0);

//    img_Rec_Buffer_ptr = new uchar[BUFFER_SIZE];
//    img_freeBytes = new QSemaphore(BUFFER_SIZE);
//    img_usedBytes = new QSemaphore(0);
    const QString ip = "192.168.1.114";
    //const QString ip = "192.168.3.102";
    qDebug()<<ip;


    camera = new cam();
    camera->set_Rec_Buffer_ptr(Rec_Buffer_ptr);
    camera->set_freeBytes_ptr(freeBytes);
    camera->set_usedBytes_ptr(usedBytes);
    camera->m_ip.clear();
    camera->m_ip = ip;
    camera->m_port = 6666;

    decoder = new decode();
    decoder->set_Rec_Buffer_ptr(Rec_Buffer_ptr);
    decoder->set_freeBytes_ptr(freeBytes);
    decoder->set_usedBytes_ptr(usedBytes);

    shower = new rtsp();

//    img = new cam();
//    img->set_Rec_Buffer_ptr(img_Rec_Buffer_ptr);
//    img->set_freeBytes_ptr(img_freeBytes);
//    img->set_usedBytes_ptr(img_usedBytes);
//    img->m_ip.clear();
//    img->m_ip = ip;
//    img->m_port = 8888;
    
//    img_decoder = new img_decode();
//    img_decoder->set_Rec_Buffer_ptr(img_Rec_Buffer_ptr);
//    img_decoder->set_freeBytes_ptr(img_freeBytes);
//    img_decoder->set_usedBytes_ptr(img_usedBytes);
    


}

camprotocol::~camprotocol()
{
    camera->Run_stopped = true;
    camera->wait();
    decoder->Run_stopped = true;
    decoder->wait();

    delete camera;
    delete decoder;
    delete shower;
//    delete img;
//    delete img_decoder;
    delete freeBytes;
    delete usedBytes;
    delete [] Rec_Buffer_ptr;
//    delete img_freeBytes;
//    delete img_usedBytes;
//    delete [] img_Rec_Buffer_ptr;
}

void camprotocol::stopRun()
{
    camera->Run_stopped = true;
    camera->wait();
}

void camprotocol::setMai(QObject *qMain, const QString ip, quint16 port)
{
    camera->m_ip.clear();
    camera->m_ip = ip;
    camera->m_port = port;
    this->connectToMain(qMain);
    this->startRun();
}



void camprotocol::startRun()
{
    camera->start(QThread::TimeCriticalPriority);
    decoder->start(QThread::TimeCriticalPriority);
    shower->start(QThread::TimeCriticalPriority);
}

void camprotocol::connectToMain(QObject *Main_Obj)
{
        QObject::connect(Main_Obj,SIGNAL(sendtcpdata(uchar *)),camera,SLOT(gettcpdata(uchar *)),Qt::QueuedConnection);
        QObject::connect(decoder,SIGNAL(sendmsgtomain(uchar,quint16,quint16,quint16,quint8)),Main_Obj,SLOT(getmsg(uchar,quint16,quint16,quint16,quint8))
                         ,Qt::QueuedConnection);
        QObject::connect(decoder,SIGNAL(sendshowbuff(uchar*,int)),Main_Obj,SLOT(getshowbuff(uchar *,int))
                         ,Qt::QueuedConnection);

        QObject::connect(shower,SIGNAL(sendimg(QImage,int,int)),Main_Obj,SLOT(Showpic(QImage ,int ,int ))
                         ,Qt::QueuedConnection);


}
