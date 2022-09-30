#include "mythread.h"

mythread::mythread()
{
    Run_stopped = false;
    Tcp_send = false;
    Run = false;

    Writer_Ptr = 0;
    Reader_Ptr = 0;
    Pre_reader_Ptr = 0;

    m_ip.clear();
    m_PortName.clear();
}

mythread::~mythread()
{

}

//void Delay(int msec)
//{
//    QTime dieTime = QTime::currentTime().addMSecs(msec);
//    while(QTime::currentTime() < dieTime){
//        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
//    }
//}

void mythread::set_Rec_Buffer_ptr(uchar *x_Rec_Buffer)
{
    m_Rec_Buffer = x_Rec_Buffer;
}

void mythread::set_freeBytes_ptr(QSemaphore *x_freeBytes)
{
    m_freeBytes = x_freeBytes;
}

void mythread::set_usedBytes_ptr(QSemaphore *x_usedBytes)
{
    m_usedBytes = x_usedBytes;
}

void mythread::gettcpdata(uchar *pdata)
{
    for (int i = 0;i<18;i++) {
        data[i] = pdata[i];
    }
    Tcp_send = true;
}
