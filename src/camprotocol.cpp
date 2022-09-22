#include "camprotocol.h"

class cam : public mythread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        QTcpSocket *tcpsocket = new QTcpSocket();
        char *Rec_Temp = new char[75000];
        int Rec_len = 0;

        Run_stopped = false;
        tcpsocket->abort();
        //tcpsocket->connectToHost("127.0.0.1",777);
        tcpsocket->connectToHost(m_ip,m_port);

        while (!Run_stopped) {
//            tcpsocket->abort();
//            tcpsocket->connectToHost(m_ip,m_port);

            if(tcpsocket->waitForConnected(3000))
            {
                 if(Tcp_send)
                 {
                     /*发送指令*/
                     qApp->processEvents();
                     for (int i = 0;i<8;i++)
                     {
                        qDebug("%02x",data[i]);
                     }

                     int num = tcpsocket->write((char*)data,10);
                     tcpsocket->flush();


                     Tcp_send = false;
                 }

                 /*接收数据*/
                // qDebug()<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<"start r";
                 if(tcpsocket->waitForReadyRead(1))
                 {
                    Rec_len = tcpsocket->read(Rec_Temp, 75000);

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
        uchar *Pre_Decoder = new uchar[12];
        uchar detect_num = 0;
        quint16 tracker_x = 0;
        quint16 tracker_y = 0;

        Run_stopped = false;

        while(!Run_stopped)
        {
            if(m_usedBytes->tryAcquire(8,1))
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
                        for (int i=3;i<8;i++)
                        {
                            Pre_Decoder[i] = m_Rec_Buffer[Pre_reader_Ptr++];
                        }
                        detect_num = Pre_Decoder[3];
                        tracker_x = Pre_Decoder[4]|Pre_Decoder[5]<<8;
                        tracker_y = Pre_Decoder[6]|Pre_Decoder[7]<<8;

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

                        emit sendmsgtomain(detect_num,tracker_x,tracker_y);



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
    const QString ip = "192.168.1.100";
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

void camprotocol::setMai(QObject *qMain)
{
    this->startRun();
    this->connectToMain(qMain);

}

void camprotocol::startRun()
{
    camera->start(QThread::TimeCriticalPriority);
    decoder->start(QThread::TimeCriticalPriority);
}

void camprotocol::connectToMain(QObject *Main_Obj)
{
        QObject::connect(Main_Obj,SIGNAL(sendtcpdata(uchar *)),camera,SLOT(gettcpdata(uchar *)),Qt::QueuedConnection);
        QObject::connect(decoder,SIGNAL(sendmsgtomain(uchar,quint16,quint16)),Main_Obj,SLOT(getmsg(uchar,quint16,quint16))
                         ,Qt::QueuedConnection);
        QObject::connect(decoder,SIGNAL(sendshowbuff(uchar*,int)),Main_Obj,SLOT(getshowbuff(uchar *,int))
                         ,Qt::QueuedConnection);
}
