#ifndef CAMPROTOCOL_H
#define CAMPROTOCOL_H

#include <QApplication>
#include <QTcpSocket>
#include <QTcpServer>
#include "mythread.h"
#include <QTime>
#include <qelapsedtimer.h>

#define BUFFER_SIZE 65535
#define NUM_SIZE 1600

class cam;
class decode;
class img_decode;

class camprotocol : public QObject
{
    Q_OBJECT
public:

    camprotocol();
    ~camprotocol();

    void stopRun(void);
    void setMai(QObject *main);
    QSemaphore *freeBytes;
    QSemaphore *usedBytes;
    uchar *Rec_Buffer_ptr;

    QSemaphore *img_freeBytes;
    QSemaphore *img_usedBytes;
    uchar *img_Rec_Buffer_ptr;

    cam *camera;
    cam *img;
    decode *decoder;
    img_decode *img_decoder;

private:
    void startRun(void);
     void connectToMain(QObject *Main_Obj);


};

#endif // CAMPROTOCOL_H
